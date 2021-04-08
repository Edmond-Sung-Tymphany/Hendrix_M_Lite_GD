/**
 * @file      allplay_simulate.c
 * @brief     Simulate the behavior of Qualcomm Allplay library
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "Bootloader.h"
#include <stdlib.h>
#include <string.h> //memcpy
#include <plib.h>
#include <GenericTypeDefs.h>
#include "Bootloader.h"
#include "allplay_receiver.h"
#include "HardwareProfile.h"
#include "NVMem.h"



#ifdef BL_ALLPLAY_SIMULATOR
/*****************************************************************************
 * Defination                                                                *
 *****************************************************************************/
//UART Frame format
#define SOH 01  //start byte
#define EOT 04  //end byte
#define DLE 16  //escape character

//UART Frame buffer size
#define FRAMEWORK_BUFF_SIZE 1000



/*****************************************************************************
 * Type                                                                      *
 *****************************************************************************/
typedef enum
{
    READ_BOOT_INFO = 1,
    ERASE_FLASH,
    PROGRAM_FLASH,
    READ_CRC,
    JMP_TO_APP

}T_COMMANDS;

typedef struct
{
    uint32 Len;
    uint8 Data[FRAMEWORK_BUFF_SIZE];
}T_FRAME;

static const uint8 BootInfo[2] =
{
    MAJOR_VERSION,
    MINOR_VERSION
};



/*****************************************************************************
 * Function Prototype                                                        *
 *****************************************************************************/
extern BOOL ExitFirmwareUpgradeMode(void);

static uint32 GetTransmitFrame(uint8* Buff);
static void BuildRxFrame(uint8 *RxData, int16 RxLen);
static void AllplaySimulatorTask(void *allplay_cb_data);
static void HandleCommand(void *allplay_cb_data);
static uint16 CalculateCrc(uint8 *data, uint32 len);

#ifdef BL_ALLPLAY_SIMULATOR_UART_ECHO
static void uart_echo_server(void);
#endif



/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
static uint8 RxBuff[255];
static uint8 TxBuff[255];
static T_FRAME RxFrame;
static T_FRAME TxFrame;
static BOOL RxFrameValid;
static BOOL TriggerBaudChange;
static DWORD_VAL NewBaud;




/*****************************************************************************
 * Function Implemenation                                                    *
 *****************************************************************************/
/**
 * Start the update process.
 *
 * This function will only return if the UpdateCallbacks#reboot callback does.
 *
 * @param callbacks list of callbacks used by the update process
 */
void receive(struct UpdateCallbacks callbacks)
{
    DBG_PRINT("receive()\r\n");
    callbacks.start();

    while( !RunApplication )
    {
        __builtin_disable_interrupts();
        __builtin_enable_interrupts();

        // Check any character is received.
        size_t len_rx= readStream(RxBuff, sizeof(RxBuff));
        if( len_rx > 0 )
        {   // Pass the bytes to frame work.
            BuildRxFrame(RxBuff, len_rx);
        }

        // Get transmit frame from frame work.
        uint8 TxLen = GetTransmitFrame(TxBuff);
        writeStream(TxBuff, TxLen);

        if(RxFrameValid)
        {
            RxFrameValid = FALSE;   // Reset the flag.
            HandleCommand((void*)callbacks.data); // Valid frame received, process the command.
        }
        
        //Flash LED
        BlinkLED_Green();
        if  ( (ReadCoreTimer() & 0x7FFFF) == 0 )
        {
            DBG_PRINT("tick\r\n");
        }

#ifdef BL_ALLPLAY_SIMULATOR_UART_ECHO
        uart_echo_server();
#endif
    }
    
    callbacks.done();
    callbacks.reboot();
}


#ifdef BL_ALLPLAY_SIMULATOR_UART_ECHO
static void uart_echo_server(void)
{
    BYTE ch= 0;
    while( GetChar(UART_CONSOLE, &ch) )
    {
        PutChar(UART_CONSOLE, ch);
    }
}
#endif



/********************************************************************
* Function:     AllplaySimulatorTask()
*
* Precondition:
*
* Input:         allplay_cb_data: callback function to process data
*
* Output:        None.
*
* Side Effects:  None.
*
* Overview:      Process the command if there is a valid fame.
*
*
* Note:          None.
********************************************************************/
static void AllplaySimulatorTask(void *allplay_cb_data)
{
    if(RxFrameValid)
    {        
        HandleCommand(allplay_cb_data); // Valid frame received, process the command.
        RxFrameValid = FALSE;   // Reset the flag.
    }
}


/********************************************************************
* Function:     HandleCommand()
*
* Precondition:
*
* Input:         None.
*
* Output:        None.
*
* Side Effects:  None.
*
* Overview:     Process the received frame and take action depending on
                the command received.
*
*
* Note:          None.
********************************************************************/
static void HandleCommand(void *allplay_cb_data)
{
    int32 (*p_cb_data)(const int32 *data, int32 len)= allplay_cb_data;

    uint8 Cmd;
    DWORD_VAL Address;
    uint8 i;
    DWORD_VAL Length;
    uint8 *DataPtr;
    uint32 Result;
    WORD_VAL crc;
    void* pFlash;

    // First byte of the data field is command.
    Cmd = RxFrame.Data[0];
    // Partially build response frame. First byte in the data field carries command.
    TxFrame.Data[0] = RxFrame.Data[0];

    // Reset the response length to 0.
    TxFrame.Len = 0;

    // Process the command.
    switch(Cmd)
    {
        case READ_BOOT_INFO: // Read boot loader version info.
            memcpy(&TxFrame.Data[1], BootInfo, 2);
            //Set the transmit frame length.
            TxFrame.Len = 2 + 1; // Boot Info Fields    + command

            //Jump to applicatino when query version for 3 times
            static int32 query_ver_count= 0;
            query_ver_count++;
            if(query_ver_count>3) {
                RunApplication = TRUE;
            }
            break;

        case ERASE_FLASH:
            pFlash = (void*)APP_FLASH_BASE_ADDRESS;
            for( i = 0; i < ((APP_FLASH_END_ADDRESS - APP_FLASH_BASE_ADDRESS + 1)/FLASH_PAGE_SIZE); i++ )
            {
                Result = NVMemErasePage( pFlash + (i*FLASH_PAGE_SIZE) );
                // Assert on NV error. This must be caught during debug phase.
                assert(Result==0);

            }
            //Set the transmit frame length.
            TxFrame.Len = 1; // Command
            break;

        case PROGRAM_FLASH:
            p_cb_data( (int32*)(&RxFrame.Data[1]), RxFrame.Len-3 );
            //Set the transmit frame length.
            TxFrame.Len = 1; // Command
            break;

        case READ_CRC:
            // Get address from the packet.
            memcpy(&Address.v[0], &RxFrame.Data[1], sizeof(Address.Val));
            memcpy(&Length.v[0], &RxFrame.Data[5], sizeof(Length.Val));
            crc.Val = CalculateCrc((uint8 *)Address.Val, Length.Val);
            memcpy(&TxFrame.Data[1], &crc.v[0], 2);

            //Set the transmit frame length.
            TxFrame.Len = 1 + 2;    // Command + 2 bytes of CRC.
            break;

        case JMP_TO_APP:
            // Exit firmware upgrade mode.
            RunApplication = TRUE;
            break;

        default:
            // Nothing to do.
            break;
    }
}


/********************************************************************
* Function:     BuildRxFrame()
*
* Precondition:
*
* Input:         Pointer to Rx Data and Rx byte length.
*
* Output:        None.
*
* Side Effects:    None.
*
* Overview:     Builds rx frame and checks CRC.
*
*
* Note:             None.
********************************************************************/
void BuildRxFrame(uint8 *RxData, int16 RxLen)
{
    static BOOL Escape = FALSE;
    WORD_VAL crc;

    while((RxLen > 0) && (!RxFrameValid)) // Loop till len = 0 or till frame is valid
    {
        RxLen--;

        if(RxFrame.Len >= sizeof(RxFrame.Data))
        {
            RxFrame.Len = 0;
        }

        switch(*RxData)
        {
            case SOH: //Start of header
                if(Escape)
                {
                    // Received byte is not SOH, but data.
                    RxFrame.Data[RxFrame.Len++] = *RxData;
                    // Reset Escape Flag.
                    Escape = FALSE;
                }
                else
                {
                    // Received byte is indeed a SOH which indicates start of new frame.
                    RxFrame.Len = 0;
                }
                break;

            case EOT: // End of transmission
                if(Escape)
                {
                    // Received byte is not EOT, but data.
                    RxFrame.Data[RxFrame.Len++] = *RxData;
                    // Reset Escape Flag.
                    Escape = FALSE;
                }
                else
                {
                    // Received byte is indeed a EOT which indicates end of frame.
                    // Calculate CRC to check the validity of the frame.
                    if(RxFrame.Len > 1)
                    {
                        crc.byte.LB = RxFrame.Data[RxFrame.Len-2];
                        crc.byte.HB = RxFrame.Data[RxFrame.Len-1];
                        if((CalculateCrc(RxFrame.Data, (uint32)(RxFrame.Len-2)) == crc.Val) && (RxFrame.Len > 2))
                        {
                            // CRC matches and frame received is valid.
                            RxFrameValid = TRUE;
                        }
                    }
                }
                break;


            case DLE: // Escape character received.
                if(Escape)
                {
                    // Received byte is not ESC but data.
                    RxFrame.Data[RxFrame.Len++] = *RxData;
                    // Reset Escape Flag.
                    Escape = FALSE;
                }
                else
                {
                    // Received byte is an escape character. Set Escape flag to escape next byte.
                    Escape = TRUE;
                }
                break;

            default: // Data field.
                RxFrame.Data[RxFrame.Len++] = *RxData;
                // Reset Escape Flag.
                Escape = FALSE;
                break;

        }

        //Increment the pointer.
        RxData++;
    }
}


/********************************************************************
* Function:     GetTransmitFrame()
*
* Precondition:
*
* Input:         Buffer pointer.
*
* Output:        Length of the buffer.
*
* Side Effects:    None.
*
* Overview:     Gets the complete transmit frame into the "Buff".
*
*
* Note:             None.
********************************************************************/
static uint32 GetTransmitFrame(uint8* Buff)
{
    INT BuffLen = 0;
    WORD_VAL crc;
    uint8 i;

    if(TxFrame.Len)
    {
        //There is something to transmit.
        // Calculate CRC of the frame.
        crc.Val = CalculateCrc(TxFrame.Data, (uint32)TxFrame.Len);
        TxFrame.Data[TxFrame.Len++] = crc.byte.LB;
        TxFrame.Data[TxFrame.Len++] = crc.byte.HB;

        // Insert SOH (Indicates beginning of the frame)
        Buff[BuffLen++] = SOH;

        // Insert Data Link Escape Character.
        for(i = 0; i < TxFrame.Len; i++)
        {
            if((TxFrame.Data[i] == EOT) || (TxFrame.Data[i] == SOH)
                || (TxFrame.Data[i] == DLE))
            {
                // EOT/SOH/DLE repeated in the data field, insert DLE.
                Buff[BuffLen++] = DLE;
            }
            Buff[BuffLen++] = TxFrame.Data[i];
        }

        // Mark end of frame with EOT.
        Buff[BuffLen++] = EOT;

        TxFrame.Len = 0; // Purge this buffer, no more required.
    }

    return(BuffLen); // Return buffer length.

}




/**
 * Static table used for the table_driven implementation.
 *****************************************************************************/
static const uint16 crc_table[16] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};


/********************************************************************
* Function:     CalculateCrc()
*
* Precondition:
*
* Input:         Data pointer and data length
*
* Output:        CRC.
*
* Side Effects:    None.
*
* Overview:     Calculates CRC for the given data and len
*
*
* Note:             None.
********************************************************************/
static uint16 CalculateCrc(uint8 *data, uint32 len)
{
    uint32 i;
    uint16 crc = 0;

    while(len--)
    {
        i = (crc >> 12) ^ (*data >> 4);
        crc = crc_table[i & 0x0F] ^ (crc << 4);
        i = (crc >> 12) ^ (*data >> 0);
        crc = crc_table[i & 0x0F] ^ (crc << 4);
        data++;
    }

    return (crc & 0xFFFF);
}



#endif /* #ifdef BL_ALLPLAY_SIMULATOR */

/**************************End of file**************************************************/

