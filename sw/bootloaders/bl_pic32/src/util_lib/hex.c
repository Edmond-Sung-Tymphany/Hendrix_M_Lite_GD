/**
 * @file      hex.c
 * @brief     The main source file for hex file parsing
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include <p32xxxx.h>
#include <plib.h>
#include <GenericTypeDefs.h>
#include "Bootloader.h"
#include "HardwareProfile.h"
#include "util.h" //DBG_PRINT
#include "hex.h"
#include "assert.h"
#include "dbgprint.h"



/*****************************************************************************
 * Structure                                                                 *
 *****************************************************************************/
typedef struct
{
    uint8 RecDataLen;
    DWORD_VAL Address;
    uint8 RecType;
    uint8* Data;
    uint8 CheckSum;
    DWORD_VAL ExtSegAddress;
    DWORD_VAL ExtLinAddress;
}T_HEX_RECORD;

//Intel HEX format
#define DATA_RECORD         0
#define END_OF_FILE_RECORD  1
#define EXT_SEG_ADRS_RECORD 2
#define EXT_LIN_ADRS_RECORD 4



/*****************************************************************************
 * Function Prototype                                                        *
 *****************************************************************************/




/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/



/********************************************************************
* Function:     WriteHexRecord2Flash()
*
* Precondition:
*
* Input:         HexRecord buffer.
*
* Output:        Return the nubmer of process bytes
*                Return 0 means the packet is not complete
*                Return -1 means the hex is incorrect
*
* Side Effects:  None.
*
* Overview:      Writes hex record to flash.
*
* Note:          None.
********************************************************************/
ssize_t WriteHexRecord2Flash(uint8* HexRecord, size_t totalHexRecLen, uint32 hex_line)
{
    static T_HEX_RECORD HexRecordSt;
    uint8 checksum = 0;
    uint32 i;
    uint32 WrData;
    uint32 RdData;
    void* ProgAddress;
    uint32 Result;
    uint8 temp[4];
    ssize_t ret= 0;
    assert(totalHexRecLen<=HEX_RECORD_LEN_MAX);


    do //Data Len(1byte) + type(1byte) + address(2bytes) + CRC(1bytes)
    {
        if(totalHexRecLen<HEX_HEADER_LEN) {
            ret= 0; //hex record is not complete
            break;
        }

        HexRecord = &HexRecord[0];
        HexRecordSt.RecDataLen = HexRecord[0];
        HexRecordSt.RecType = HexRecord[3];
        HexRecordSt.Data = &HexRecord[4];
        ret= HexRecordSt.RecDataLen + HEX_HEADER_LEN;
        //DBG_PRINT("[%s] totalHexRecLen=%d, HEX: RecDataLen=%d, RecType=%d\r\n", __func__, totalHexRecLen, HexRecordSt.RecDataLen,  HexRecordSt.RecType);

        //error checking
        if( HexRecordSt.RecDataLen> HEX_DATA_LEN_MAX) { //should not happean because HEX_DATA_LEN_MAX is the maximun value of uint8
            DBG_PRINT("[%s] LINE[%4d] format error: data length should be longer than %d (get lentgh %d bytes)\r\n", __func__, hex_line, HEX_DATA_LEN_MAX, HexRecordSt.RecDataLen);
            ret= -1;
            break;
        }

        //The latest packet is not complete
        if( (HexRecordSt.RecDataLen + HEX_HEADER_LEN) > totalHexRecLen ) {
            //DBG_PRINT("[%s] LINE[%4d] (%2d/%3dbytes) Not complete\r\n", __func__, hex_line, ret, totalHexRecLen);
            ret= 0;
            break;
        }

        // Hex Record checksum check.
        checksum = 0;
        for(i=0; i < (HexRecordSt.RecDataLen + HEX_HEADER_LEN); i++) {
            checksum += HexRecord[i];
        }
        if(checksum != 0) {
            DBG_PRINT("[%s] LINE[%4d] (%2d/%3dbytes) CRC fail (checksum check result = 0x%08x)\r\n", __func__, hex_line, ret, totalHexRecLen, checksum);
            DBG_PRINT_DATA(HexRecord, HexRecordSt.RecDataLen + HEX_HEADER_LEN,  "   HEX:",  "\r\n");
            ret= -1;
            break;
        }
        //DBG_PRINT("[%s] LINE[%4d] (%2d/%3dbytes) \r\n", __func__, hex_line, ret, totalHexRecLen, checksum);
        //dbgprint_data(HexRecord, HexRecordSt.RecDataLen + HEX_HEADER_LEN,  "   HEX:",  "\r\n");

        // Hex record checksum OK.
        switch(HexRecordSt.RecType)
        {
            case DATA_RECORD:  //Record Type 00, data record.
                HexRecordSt.Address.byte.MB = 0;
                HexRecordSt.Address.byte.UB = 0;
                HexRecordSt.Address.byte.HB = HexRecord[1];
                HexRecordSt.Address.byte.LB = HexRecord[2];

                // Derive the address.
                HexRecordSt.Address.Val = HexRecordSt.Address.Val + HexRecordSt.ExtLinAddress.Val + HexRecordSt.ExtSegAddress.Val;
                //DBG_PRINT("[%s] LINE[%4d] (%2d/%3dbytes) process DATA_RECORD: 0x%08x\r\n", __func__, hex_line, ret, totalHexRecLen, HexRecordSt.Address.Val );
                if( HexRecordSt.Address.Val<APP_FLASH_PHY_BASE_ADDRESS || HexRecordSt.Address.Val+HexRecordSt.RecDataLen-1>APP_FLASH_PHY_END_ADDRESS  ) {
                    DBG_PRINT("[%s] LINE[%4d] (%2d/%3dbytes) format error: writing data should fill into 0x%08x~0x%08x (get data 0x%08x~0x%08x)\r\n",
                                     __func__, hex_line, ret, totalHexRecLen,
                                    APP_FLASH_PHY_BASE_ADDRESS, APP_FLASH_PHY_END_ADDRESS,
                                    HexRecordSt.Address.Val, HexRecordSt.Address.Val+HexRecordSt.RecDataLen-1);
                    ret= -1;
                    break; //break for switch, not for while
                }

                while(HexRecordSt.RecDataLen) // Loop till all bytes are done.
                {
                    // Convert the Physical address to Virtual address.
                    ProgAddress = PA_TO_KVA0(HexRecordSt.Address.Val);

                    // Make sure we are not writing boot area and device configuration bits.
                    if(((ProgAddress >= (void *)APP_FLASH_BASE_ADDRESS) && (ProgAddress <= (void *)APP_FLASH_END_ADDRESS))
                       && ((ProgAddress < (void*)DEV_CONFIG_REG_BASE_ADDRESS) || (ProgAddress > (void*)DEV_CONFIG_REG_END_ADDRESS)))
                    {
                        if(HexRecordSt.RecDataLen < 4)
                        {
                            // Sometimes record data length will not be in multiples of 4. Appending 0xFF will make sure that..
                            // we don't write junk data in such cases.
                            WrData = 0xFFFFFFFF;
                            memcpy(&WrData, HexRecordSt.Data, HexRecordSt.RecDataLen);
                        }
                        else
                        {
                            memcpy(&WrData, HexRecordSt.Data, 4);
                        }
                        // Write the data into flash.
                        Result = NVMWriteWord(ProgAddress, WrData);
                        // Assert on error. This must be caught during debug phase.
                        assert(Result==0);
                    }

                    // Increment the address.
                    HexRecordSt.Address.Val += 4;
                    // Increment the data pointer.
                    HexRecordSt.Data += 4;
                    // Decrement data len.
                    if(HexRecordSt.RecDataLen > 3)
                    {
                        HexRecordSt.RecDataLen -= 4;
                    }
                    else
                    {
                        HexRecordSt.RecDataLen = 0;
                    }
                }
                //DBG_PRINT("[%s] Write END\r\n", __func__);
                break;

            case EXT_SEG_ADRS_RECORD:  // Record Type 02, defines 4th to 19th bits of the data address.
                HexRecordSt.ExtSegAddress.byte.MB = 0;
                HexRecordSt.ExtSegAddress.byte.UB = HexRecordSt.Data[0];
                HexRecordSt.ExtSegAddress.byte.HB = HexRecordSt.Data[1];
                HexRecordSt.ExtSegAddress.byte.LB = 0;
                // Reset linear address.
                HexRecordSt.ExtLinAddress.Val = 0;
                //DBG_PRINT("[%s] LINE[%4d] (%2d/%3dbytes) process EXT_SEG_ADRS_RECORD: 0x%08x\r\n", __func__, hex_line,  ret, totalHexRecLen, HexRecordSt.ExtLinAddress.Val );
                break;

            case EXT_LIN_ADRS_RECORD:   // Record Type 04, defines 16th to 31st bits of the data address.
                HexRecordSt.ExtLinAddress.byte.MB = HexRecordSt.Data[0];
                HexRecordSt.ExtLinAddress.byte.UB = HexRecordSt.Data[1];
                HexRecordSt.ExtLinAddress.byte.HB = 0;
                HexRecordSt.ExtLinAddress.byte.LB = 0;
                // Reset segment address.
                HexRecordSt.ExtSegAddress.Val = 0;
                //DBG_PRINT("[%s] LINE[%4d] (%2d/%3dbytes) process EXT_LIN_ADRS_RECORD: 0x%08x\r\n", __func__, hex_line,  ret, totalHexRecLen, HexRecordSt.ExtLinAddress.Val );
                break;

            case END_OF_FILE_RECORD:  //Record Type 01, defines the end of file record.
                HexRecordSt.ExtSegAddress.Val = 0;
                HexRecordSt.ExtLinAddress.Val = 0;
                DBG_PRINT("[%s] LINE[%4d] (%2d/%3dbytes) process END_OF_FILE_RECORD\r\n", __func__, hex_line, ret, totalHexRecLen);
                break;
                
            default: //wrong type
                DBG_PRINT("[%s] LINE[%4d] (%2d/%3dbytes) format error: incorrect type (get type %d)\r\n", __func__, hex_line, HexRecordSt.RecType);
                ret= -1;
                break;
        } //end of switch(HexRecordSt.RecType)
    }while(0);
    
    return ret;
}

/*********************End of File************************************/



