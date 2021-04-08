/**
 * @file      Bootloader.c
 * @brief     The main source file for Polk CamdenSquare bootloader
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include <plib.h>
#include <GenericTypeDefs.h>
#include <p32xxxx.h>
#include "BootLoader.h" //each file must include Bootloader.h
#include "Triggers.h"
#include "assert.h"
#include "hex.h"
#include "HardwareProfile.h"
#include "dbgprint.h" //DBG_PRINT
#include "allplay_protocol.h"
#include "allplay_receiver.h"
#include "allplay_stream.h"
#include "ui.h"
#include "md5.h"

#if defined(PIC32_MOFA)
  #include "../../../project_files/mofa/pic32/include/attachedDevicesMcu.h"   //PRODUCT_VERSION_MCU
#elif defined(PIC32_POLK_CAMDEN_SQUARE)
  #include "../../../project_files/polk_allplay/pic32/include/attachedDevicesMcu.h"   //PRODUCT_VERSION_MCU
#else
  #error Can not find bootloader
#endif


/*****************************************************************************
 * Type                                                                      *
 *****************************************************************************/
/* This MCU hex-bin image header (64bytes), must the same with "hex2binhex.py"
 * Note:
 *   "header_fmt_ver" should be 0 currently. To support more features in future and also have backward compibility
 *   (i.e. let old bootloader support new upgrade package), we could increase header_fmt_ver (ex. 1) and handle new features
 *   only if "header_fmt_ver>=1"
 */
#define BL_HEADER_MAGIC_STR "IMG."
#define BL_SUPPORT_HEADER_FMT_VER 0

typedef struct {
    /*  4bytes, 0x00~0x03 */ char   magic_str[4]; //should be "IMG."
    /*  1bytes, 0x04      */ uint8  header_fmt_ver; //header format version
    /*  1bytes, 0x05      */ uint8  md5_check_ena; //0:no checksum, 1:md5
    /*  2bytes, 0x06~0x07 */ uint8  reserve1[2];
    /*  4bytes, 0x08~0x0b */ uint32 build_time_sec;
    /*  4bytes, 0x0c~0x0f */ uint32 data_len;
    /* 16bytes, 0x10~0x1f */ char   data_md5[MD5_LEN];
    /* 16bytes, 0x20~0x3f */ uint32 reserve2[8];
    /* N bytes, 0x40~     */ //data
}ImgHeader;



/*****************************************************************************
 * Function Prototype                                                        *
 *****************************************************************************/
int32 main(void);
void __attribute__((nomips16)) _general_exception_handler(void); 
static void allplay_cb_start();
static int32 allplay_cb_data(const uint8 *data, size_t len);
static int32 allplay_cb_done();
static void allplay_cb_reboot();
static void print_exception_code();
static char* get_reset_reason(bool *pIsWdogReset);
static void nvm_item_init(NVM_STORAGE_ADDR_TYPE type);
static void nvm_item_increase_one(NVM_STORAGE_ADDR_TYPE type);
static void update_bootloader_ver(const char* ver);
static bool verify_img_header(ImgHeader *p_header);

#ifdef BL_UART_ECHO_SERVER
static void uart_echo_server(void);
#endif


/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
extern size_t maxDataSize;

//statistic information
static uint32 sta_cb_start_count= 0;   //number of cb_start() executed. Shoule be 1 if the process is smooth
static uint32 sta_cb_start_time_ms= 0; //time when receiver() start
static uint32 sta_recv_len= 0;        //receive file size
static uint32 sta_pkt_num= 0;          //number of recieved packet
static uint32 sta_hex_line_num= 1;     //number of recieved packet

static uint8 buf_hex[HEX_RECORD_LEN_MAX];
static size_t hex_unprocessed_len= 0;

static ImgHeader header= {0};


/*****************************************************************************
 * Function Implemenataion                                                   *
 *****************************************************************************/
/********************************************************************
 * Function:     main()
 *
 * Precondition:
 *
 * Input:        None.
 *
 * Output:       None.
 *
 * Side Effects: None.
 *
 * Overview:     Main entry function. If there is a trigger or
 *               if there is no valid application, the device
 *               stays in firmware upgrade mode.
 *********************************************************************/
int32 main(void)
{   
    // Setup configuration
    bsp_init();

#ifdef MOFA_WIFI_RF_TEST
    printf("Enter MOFA_WIFI_RF_TEST mode \n");
    bsp_disable_watchdog();
    sam_init(TRUE);
    while(1)
    {
        BlinkLED_Blue();
    }
#endif //#ifdef MOFA_WIFI_RF_TEST

    /* When upgrading, we assme the first packet from SAM include whole image header (32bytes),
     * thus MCU's receiver buffer (MAX_DATA_SIZE) should bigger than image header.
     */
    assert(MAX_DATA_SIZE>=sizeof(ImgHeader))

    //Init NVM item to zero when first boot
    nvm_item_init(NVM_STORAGE_ADDR_UPGRADE_TIMES);
    nvm_item_init(NVM_STORAGE_ADDR_WDOG_RESET_TIMES); 

    //Get reset reason and update watchdog reset times
    bool isWdogReset= FALSE;
    char *str_reset_reason= get_reset_reason(&isWdogReset);

    //Print bootloader header
    DBG_PRINT("\r\n\r\n\r\n");
    DBG_PRINT("=======================================================\r\n");
    DBG_PRINT("Bootloader\r\n");
    DBG_PRINT("  Version: v" PRODUCT_VERSION_MCU " (built at " __DATE__ ", "__TIME__ ")\r\n"); //must print after bsp_init
    DBG_PRINT("  Support header format version: v%d\r\n",BL_SUPPORT_HEADER_FMT_VER); //must print after bsp_init
    DBG_PRINT("  Upgrade times: %d\r\n", NvmDrv_ReadWord(NVM_STORAGE_ADDR_UPGRADE_TIMES));
    DBG_PRINT("  Watchdog reset times: %d\r\n", NvmDrv_ReadWord(NVM_STORAGE_ADDR_WDOG_RESET_TIMES));
    DBG_PRINT("=======================================================\r\n");

    //Print reset reason
    DBG_PRINT(str_reset_reason);

    //Assert if reset by watchdog, means we had a long-time blocking call before.
    if(isWdogReset) {
        DBG_PRINT("*** ERROR: Watchdog was triggered before ***\r\n");
        NVM_STORAGE_VALUE_SET(NVM_STORAGE_ADDR_IGNORE_PWR_KEY);
        assert(0);
    }
    
    //Write bootloader version to flash (only write one time after update flash)
    update_bootloader_ver(PRODUCT_VERSION_MCU); 
    
    //Print exception code which was set on previous exception
    print_exception_code();

#ifdef BL_UART_ECHO_SERVER
    uart_echo_server();
#endif

    // Read triggers
    BOOL trigger= check_trigger();
    BOOL app_wrong= ValidAppWrong();

    //Clear trigger
    if( trigger ) {
        clear_trigger();
        if(app_wrong) {
            DBG_PRINT("***** WARNING: Trigger is exist but application is not present *****\r\n\r\n");
        }
    }

    /* -----------------------------------------------------------------------------------------
     *       Condition               Actions
     *  trigger  app_wrong     init-sam   destroy-sam     Description
     * -----------------------------------------------------------------------------------------
     *    0          0            x           x           Normal boot
     *    1          0            V           V           Jump from application to indicate upgrade
     *    0          1            x           V           Wrong flash cause bootloader start update protocol
     *    1          1            V           V           Unknown condition
     * -----------------------------------------------------------------------------------------
     * app_wrong==TRUE means SAM was not initialized by application before, thus sam_init()
     */
    sam_init(/*real_init=*/app_wrong);

    //Decide to start update protocol or not
    if( trigger || app_wrong ) {
        //start message
        DBG_PRINT("\r\n\r\n\r\n");
        DBG_PRINT("=======================================================\r\n");
        DBG_PRINT("[%s] Start Allplay receive protocol \r\n", __func__);
        DBG_PRINT("=======================================================\r\n\r\n\r\n");

        /* When jump from application to here, RED will pause flash for 1.4sec.
         */
        dfu_mode_enable(/*enable=*/TRUE);
        
        //prepare callback functions for update protocol
        struct UpdateCallbacks callbacks;
        callbacks.start=  allplay_cb_start;
        callbacks.data=   allplay_cb_data;
        callbacks.done=   allplay_cb_done;
        callbacks.reboot= allplay_cb_reboot;

        //Start update protocol. Reboot when finish. Return when fail.
        receive(callbacks);

        //When update protocol is fail:
        DBG_PRINT("[%s] ***** Fail to update MCU firmware ***** \r\n\r\n", __func__);
        allplay_cb_reboot();
    }

    
    //DEBUGGER_PAUSE(); // break into the debugger
    DBG_PRINT("Jump to App \r\n\r\n\r\n\r\n\r\n", __func__);
    JumpToApp();    
    while(1){}; 
}


static void print_exception_code()
{
    uint32 exceptionCode= NvmDrv_ReadWord(NVM_STORAGE_ADDR_EXCEPTION_CODE);
    if(0xFFFFFFFF != exceptionCode)
    {
        DBG_PRINT("*** WARNING: Exception has happened before this reboot. Exception Code: %d ***\r\n", exceptionCode);
    }
}


/* Store watchdog reset times to Nvm, and return string of reset reason
 * Reference: Microchip PIC32MX Family Reference Manual, chapter 7.4
 */
static char* init_wdog_reset_times()
{
    uint32 wdog_reset_times= NvmDrv_ReadWord(NVM_STORAGE_ADDR_WDOG_RESET_TIMES);
    if(wdog_reset_times==0xFFFFFFFF) {
        NvmDrv_WriteWord(NVM_STORAGE_ADDR_WDOG_RESET_TIMES, wdog_reset_times);
    }
}


/* Update watchdog reset times, and return string of reset reason
 * Reference: Microchip PIC32MX Family Reference Manual, chapter 7.4
 */
static char* get_reset_reason(bool *pIsWdogReset)
{
    static char str[100];
    *pIsWdogReset= FALSE;

    if(RCONbits.POR==0 && RCONbits.BOR==0)
    {
        if( RCONbits.WDTO==1 ) {
            snprintf(str, sizeof(str), "\r\n\r\n*** WARNING: Watchdog Reset (RCON=0x%08X) *** \r\n\r\n\r\n", RCONbits.w);
            nvm_item_increase_one(NVM_STORAGE_ADDR_WDOG_RESET_TIMES); //increase watchdog reset times
            *pIsWdogReset= TRUE;
        }
        else if( RCONbits.EXTR==1 ) {
             snprintf(str, sizeof(str), "Reset Reason: MCLR Reset (External VCC loss, RCON=0x%08X) \r\n", RCONbits.w);
        }
        else if(RCONbits.SWR==1 ) {
             snprintf(str, sizeof(str), "Reset Reason: Soft Reset (1) (RCON=0x%08X) \r\n", RCONbits.w);
        }
        else if(RCONbits.CMR==1 ) {
             snprintf(str, sizeof(str), "Reset Reason: Configuration Word Mismatch Reset (RCON=0x%08X) \r\n", RCONbits.w);
        }
        else {
            /* Reference Manual does not describe this case, should never occurs, but soft reboot may meet this case.
             *  Soft reboot by application: RCON=0xE0000000 (this case, out of spec.)
             *  Soft reboot by bootloader: RCON=0xE0000040 (normal case)
             */
             snprintf(str, sizeof(str), "Reset Reason: Soft Reset (2) (RCON=0x%08X) \r\n", RCONbits.w);
            //snprintf(str, sizeof(str), "\r\n\r\n*** WARNING: Unknown reset (RCON=0x%08X) ***\r\n\r\n\r\n", RCONbits.w);
        }
    }
    else if( RCONbits.POR==1 && RCONbits.BOR==1 )
    {
         snprintf(str, sizeof(str), "Reset Reason: Power on Reset (RCON=0x%08X) \r\n", RCONbits.w);
    }
    else if( RCONbits.POR==1 ) //don't care for BOR
    {   //As experiment, the reset after MPLAB writing flash, also meet brown-out reset
        snprintf(str, sizeof(str), "Reset Reason: Brown-out Reset (maybe reset after flash writing, RCON=0x%08X) \r\n", RCONbits.w);
    }
    else // POR==0 && BOR==1
    {   
        snprintf(str, sizeof(str), "\r\n\r\n*** WARNING: Unknown reset (RCON=0x%08X) ***\r\n\r\n\r\n", RCONbits.w);
    }

    /* Clean reset reason */
    RCONCLR= _RCON_POR_MASK | _RCON_BOR_MASK | _RCON_IDLE_MASK | _RCON_SLEEP_MASK | _RCON_WDTO_MASK | _RCON_SWR_MASK | _RCON_EXTR_MASK | _RCON_CMR_MASK;

    return str;
}


static void nvm_item_init(NVM_STORAGE_ADDR_TYPE type)
{
    uint32 value= NvmDrv_ReadWord(type);
    if(0xFFFFFFFF==value)
    {   //Initialize upgrade times when first boot
        NvmDrv_WriteWord(type, 0);
    }
}


static void nvm_item_increase_one(NVM_STORAGE_ADDR_TYPE type)
{
    uint32 value= NvmDrv_ReadWord(type) + 1;
    NvmDrv_WriteWord(type, value);
}


static bool verify_img_header(ImgHeader *p_header)
{
    printf("\n\r\n\r=======================================================\r\n\n");
    printf(" Header: \r\n");
    //printf("   raw: %s \r\n",  md5_to_str(((unsigned char*)p_header)+0));
    //printf("   raw: %s \r\n",  md5_to_str(((unsigned char*)p_header)+16));
    //printf("   raw: %s \r\n",  md5_to_str(((unsigned char*)p_header)+32));
    printf("   magic_str= %c%c%c%c \r\n", p_header->magic_str[0], p_header->magic_str[1], p_header->magic_str[2], p_header->magic_str[3] );
    printf("   header_fmt_ver= v%d \r\n", p_header->header_fmt_ver );
    printf("   md5_check_ena= %d \r\n", p_header->md5_check_ena );
    printf("   data_md5= %s \r\n", md5_to_str(p_header->data_md5) );
    printf("   data_len= %d bytes \r\n", p_header->data_len);
    printf("   build_time(GMT+8)= %s\r\n", ctime(&p_header->build_time_sec));

    if(p_header->header_fmt_ver>=1)  {
        /* NOTE:
         *   If we support more features in future, and to have backward compibility (i.e. let old bootloader support new upgrade package),
         *   we could increase header_fmt_ver (ex. 0 to 1), and only handle new feature if "header_fmt_ver>=1"
         */
    }

    printf("=======================================================\r\n\r\n");

    bool ret= TRUE;
    if( 0 != strncmp(BL_HEADER_MAGIC_STR, p_header->magic_str, sizeof(p_header->magic_str)) ) {
        DBG_PRINT("*** ERROR: Magic number \"%c%c%c%c\" is wrong ***\r\n\r\n", p_header->magic_str[0], p_header->magic_str[1], p_header->magic_str[2], p_header->magic_str[3]);
        ret= FALSE;
    }
    if( p_header->data_len==0 || p_header->data_len>PROGRAM_FLASH_SIZE ) {
        DBG_PRINT("*** ERROR: data_len (%d bytes) is wrong *** \r\n\r\n", p_header->data_len);
        ret= FALSE;
    }
    return ret;
}


/* Write bootloader version to flash. Then application could read it via NvmDrv
 * Param:
 *    ver: bootloader version
 * 
 * How to read bootloader version in application:
 *    char ver[5];
 *    NvmDrv_ReadWord(NVM_STORAGE_ADDR_BOOTLOADER_VER_CHARS, ver); 
 *    ver[4]= '\0';
 */
static void update_bootloader_ver(const char* ver)
{
    assert(ver);
    //To save flash life cycle, we only write flash if needed.
    
    //Firstly, read bootloader version from flash
    char ver_read[5];
    *((uint32*)ver_read)= NvmDrv_ReadWord(NVM_STORAGE_ADDR_BOOTLOADER_VER_CHARS);
    ver_read[4]= '\0';

    //Secondly, write flash if bootlaoder version is wrong
    char ver_write[5];    
    strncpy(ver_write, ver, sizeof(ver_write));
    assert(strlen(ver)<=sizeof(uint32)); //we only write 4 characters to flash
    //DBG_PRINT("[%s] ver_write:%s, ver_read:%s\r\n", __func__, ver_write, ver_read);
    if( 0!=strncmp(ver, ver_read, sizeof(ver_write)) ) {
        DBG_PRINT("First boot, write bootloader version to flash \r\n", ver_write, ver_read);
        NvmDrv_WriteWord(NVM_STORAGE_ADDR_BOOTLOADER_VER_CHARS, *((uint32*)ver_write) );
    }
}

static void allplay_cb_start()
{ 
   /* NOTE for SAM 2.0.34-S:
    *   When reboot here, SAM may *NOT* restart upgrading. If we check error and must reboot here in future,
    *   we need to careful confirm if the behavior have any problem
    */
    hex_unprocessed_len= 0;

    //update statistic information for debugging
    sta_recv_len= 0;
    sta_pkt_num= 0;
    sta_hex_line_num= 0;
    sta_cb_start_count++;
    sta_cb_start_time_ms= TimerUtil_getTimeMs();
    
    md5_init();
}


static int32 allplay_cb_data(const uint8 *data,size_t data_len)
{
    ssize_t data_processed_len= 0;
    int32 ret= 1;
    ssize_t hex_len= 0;
    
    DBG_PRINT("[%s] %d bytes / %d packets (%d bytes)\r\n\r\n", __func__, data_len, sta_pkt_num, sta_recv_len+data_len);
    sta_pkt_num++;

    /* The RESET packet from MCU to SAM notify the MCU buffer is MAX_DATA_SIZE (2048 bytes),
     * Thus SAM should send big enough data to include image header (32bytes)
     */
    if( data_len < sizeof(ImgHeader) ) {
        DBG_PRINT("[%s] *** First data from SAM (%d bytes) should be larger than header size (%d bytes) ***\r\n", __func__, data_len, sizeof(ImgHeader));
        assert(0);
        return 0;
    }

    /* Process image header */
    if( sta_recv_len== 0 ) {
        memcpy(&header, data, sizeof(ImgHeader));
        if( !verify_img_header(&header) ) {
            return 0;
        }

        sta_recv_len+= sizeof(ImgHeader);
        data_len-= sizeof(ImgHeader);
        data+= sizeof(ImgHeader);
        
        //erase flash
        NVMemEraseApplication(); //spent 200ms to earse 50 pages
        NVM_STORAGE_VALUE_SET(NVM_STORAGE_ADDR_FLASH_WRITING);
    }
    
    /* Calculate MD5 check-sum */
    if(header.md5_check_ena) {
        md5_update(data, data_len);
    }

    //First memcpy, may include previous hex data
    sta_recv_len+= data_len;
    assert(hex_unprocessed_len>=0 && hex_unprocessed_len<=sizeof(buf_hex));
    hex_len= min(data_len, sizeof(buf_hex)-hex_unprocessed_len);
    assert(hex_len>=0 && hex_unprocessed_len+hex_len<=sizeof(buf_hex));
    memcpy( &buf_hex[hex_unprocessed_len], data, hex_len);
    //DBG_PRINT("[%s] memcpy( buf_hex[%d], data , %d )\r\n", __func__, hex_unprocessed_len, hex_len);
    hex_len+= hex_unprocessed_len;
    assert(hex_len>=0 && hex_len<=sizeof(buf_hex));
    if(hex_unprocessed_len>0) {
        data_processed_len= -(hex_unprocessed_len);
    }
    hex_unprocessed_len= 0;
    

    //2nd~nth memcpy
    do
    {
        ssize_t hex_ret;
        hex_ret= WriteHexRecord2Flash(buf_hex, hex_len, sta_hex_line_num); //Negate length of command and CRC RxBuff.Len
        if(hex_ret<0) {
            DBG_PRINT("[%s] WriteHexRecord2Flash Fail\r\n", __func__);
            ret= 0; //fail
            break;
        }
        else if(hex_ret==0) { //not enough data
            //DBG_PRINT("[%s] The latest packet is not complete\r\n", __func__);
            hex_unprocessed_len= hex_len;
            ret= 1; //pass
            break;
        }
        sta_hex_line_num++;
        data_processed_len+= hex_ret;
        assert(data_processed_len>=0 && data_processed_len<=data_len);

        hex_len= min(data_len-data_processed_len, sizeof(buf_hex));
        assert(hex_len>=0 && hex_len+data_processed_len<=data_len);
        //DBG_PRINT("[%s] hex_len= %d = min(%d-%d , %d) \r\n", __func__, hex_len, data_len, data_processed_len, sizeof(buf_hex));
        memcpy(buf_hex, &data[data_processed_len], hex_len);
        //DBG_PRINT("[%s] memcpy( buf_hex, data[%d] , %d )\r\n", __func__, data_processed_len, hex_len);
        //TimerUtil_delay_ms(100); //debug: delay to print complete message
    }while(1);


    /* We check md5 on allplay_cb_data() instead of allplay_cb_done() becuase,
     *   allplay_cb_done() return 0 ==> SAM idle and do not restart upgrading
     *   allplay_cb_data() return 0 ==> SAM restart upgrading
     */
    uint32 recv_data_len= sta_recv_len - sizeof(ImgHeader);
    if( recv_data_len > header.data_len )
    {   //ERROR: receive too more data
        DBG_PRINT("[%s] header data length: %d bytes\r\n", __func__, header.data_len );
        DBG_PRINT("[%s] received data length: %d bytes (do not include header %d bytes)\r\n\n\r", __func__, recv_data_len, sizeof(ImgHeader));
        DBG_PRINT("[%s] *** Data length mis-match, fail to upgrade\r\n\r\n\r\n", __func__ );
        ret= 0; //measn fail
    }
    else if( recv_data_len == header.data_len )
    {   //Receive complete, start md5 checking
        if(header.md5_check_ena) {
            uint8 *md5_result= md5_final();
            printf("\r\n");
            DBG_PRINT("[%s] header md5:     %s\r\n", __func__, md5_to_str(header.data_md5) );
            DBG_PRINT("[%s] calculated md5: %s\r\n\r\n", __func__, md5_to_str(md5_result) );
            if( 0!=memcmp(md5_result, header.data_md5, MD5_LEN) ) {
                DBG_PRINT("[%s] *** Checksum (Md5) mis-match, fail to upgrade\r\n\r\n\r\n", __func__ );
                ret= 0; //measn fail
            }
        }
        else {
            DBG_PRINT("[%s] Do not check md5\r\n\r\n", __func__);
        }
    }
    else
    {
        //upgrading is not complete
    }
    
    return ret;
}


static int32 allplay_cb_done()
{
  /* NOTE for SAM 2.0.34-S:
   *   When enter this function, SAM send REBOOT then terminate "update" process.
   *   If we return 0 here then restart upgrade loop, SAM is still waiting reboot and do not restart,
   *   Thus if we find error here, the correct behavior is REBOOT, not return 0.
   */
    assert(hex_unprocessed_len==0);
    NVM_STORAGE_VALUE_CLEAR(NVM_STORAGE_ADDR_FLASH_WRITING);
    
    //increase upgrade number
    uint32 upgrade_times= NvmDrv_ReadWord(NVM_STORAGE_ADDR_UPGRADE_TIMES) + 1;
    NvmDrv_WriteWord(NVM_STORAGE_ADDR_UPGRADE_TIMES, upgrade_times);
    DBG_PRINT("[%s] This is the %dth upgrade\r\n", __func__, upgrade_times);

    uint32 speed_bps;
    uint32 time_consume_sec= (TimerUtil_getTimeMs() - sta_cb_start_time_ms)/1000;
    if(time_consume_sec>0)
        speed_bps= (sta_recv_len*8) / time_consume_sec;
    else
        speed_bps= 0;

    printf("\n\r\n\r=======================================================\r\n\r\n\r\n");
    printf(" allplay_cb_reboot: \r\n" \
           "    DATA_STAGE x %d \r\n" \
           "    PACKET x %d (%dbytes for each packet) \r\n" \
           "    HEADER: %dbytes \r\n" \
           "    DATA-HEX: %dbytes , %d lines \r\n" \
           "    TIME_DATA_STAGE: %dsec \r\n" \
           "    SPEED: %dbps (%d%% of LINK speed) \r\n\r\n",
                 sta_cb_start_count, sta_pkt_num, MAX_DATA_SIZE, sizeof(ImgHeader), sta_recv_len-sizeof(ImgHeader), sta_hex_line_num, time_consume_sec, speed_bps, speed_bps*100/115200 );
    printf("=======================================================\r\n\r\n\r\n\n\r\n\r\n");

    return 1; //pass
}


static void allplay_cb_reboot()
{
    NVM_STORAGE_VALUE_SET(NVM_STORAGE_ADDR_IGNORE_PWR_KEY); //indicate application not wait power key for one time
    
    //DBG_PRINT("Reboot\r\n"); //must print before bsp_destroy()
    reboot(); //destroy SAM and reboot
}


#ifdef BL_UART_ECHO_SERVER
//Shoule be execute after bsp_init()
static void uart_echo_server(void)
{
    BOOL stay_in_echo_server= TRUE;
    size_t read_len, write_len;
    uint8 buf[20]= {0};
    uint32 read_max= 1;

    while( stay_in_echo_server )
    {
        read_len= console_read_stream(buf, read_max);
        assert(read_len<=read_max);
        buf[read_len]= '\0';
        if(read_len==0)
            continue;

        if(buf[0]=='\r')
        {
            buf[0]= '\n';
            buf[1]= '\r';
            read_len= 2;
        }

        //write_len= console_write_stream(buf, read_len);
        //assert(write_len<=read_len);
        DBG_PRINT("%s", buf);
    }
}
#endif


static enum {
    EXCEP_IRQ  = 0,         // 0x00 interrupt
    EXCEP_AdEL = 4,         // 0x04 address error exception (load or ifetch)
    EXCEP_AdES,             // 0x05 address error exception (store)
    EXCEP_IBE,              // 0x06 bus error (ifetch)
    EXCEP_DBE,              // 0x07 bus error (load/store)
    EXCEP_Sys,              // 0x08 syscall
    EXCEP_Bp,               // 0x09 breakpoint
    EXCEP_RI,               // 0x0A reserved instruction
    EXCEP_CpU,              // 0x0B coprocessor unusable
    EXCEP_Overflow,         // 0x0C arithmetic overflow
    EXCEP_Trap,             // 0x0D trap (possible divide by zero)
    EXCEP_IS1 = 16,         // 0x10 implementation specfic 1
    EXCEP_CEU,              // 0x11 CorExtend Unuseable
    EXCEP_C2E               // 0x12 coprocessor 2
} _excep_code;


uint32 _excep_addr= 1;
/* This function overrides the normal _weak_ generic handler
 * should be non-static so that it can replace the default _general_exception_handler()
 */
void __attribute__((nomips16)) _general_exception_handler(void)
{
    char msg[128];
    // Mask off Mask of the ExcCode Field from the Cause Register
    // Refer to the MIPs M4K Software User's manual
    _excep_code=_CP0_GET_CAUSE() & _CP0_CAUSE_EXCCODE_MASK >> _CP0_CAUSE_EXCCODE_POSITION;
    _excep_addr=_CP0_GET_EPC();
    snprintf(msg, sizeof(msg), "\r\n\r\nexception: code=0x%02x, addr=0x%0p", _excep_code, _excep_addr);
    //DEBUGGER_PAUSE(); // break into the debugger
    assert_handler(msg, 0);
}

/*********************End of File************************************/



