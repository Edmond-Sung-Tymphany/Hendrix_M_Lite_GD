/**************************************************************
 * Copyright (C) 2013, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/

#include "Bootloader.h"
#ifndef BL_ALLPLAY_SIMULATOR


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "dbgprint.h"
#include "hex.h"
#include "util.h"
#include "allplay_common.h"
#include "allplay_protocol.h"
#include "allplay_receiver.h"


/*****************************************************************************
 * Definition                                                                *
 *****************************************************************************/


/*****************************************************************************
 * Function Implementation                                                   *
 *****************************************************************************/
static int32 receiveFile(struct UpdateCallbacks *callbacks) {
    int32 result;
    size_t len;
    uint8 *buf;

    DBG_PRINT("====================================\r\n");
    DBG_PRINT("[%s] STAGE1: receiveFile start \r\n\r\n", __func__);

    // Read first packet, if we timeout, send a reset
    do {        
        DBG_PRINT("[%s] STAGE2: buildReset \r\n\r\n", __func__);
        result = writeReset(); //blocking up to UPDATE_TIMEOUS_MS ms, feed watchdog inside
        if (result < 0) {
            DBG_PRINT("[%s] Failed write reset: %d\r\n", __func__, result);
            return 0;
        }

        DBG_PRINT("\r\n\r\n[%s] STAGE3: readData for first DATA\r\n\r\n", __func__);
        result = readData(&buf, &len, DATA_1ST_TIMEOUT_MS); //blocking up to DATA_1ST_TIMEOUT_MS ms
        if (result < 0) {
            DBG_PRINT("[%s] Failed to read first packet\r\n", __func__);
            return 0;
            //We use continue instead of return. Then we can set timeout in this function
            //continue;
        }
        if (result > 0) {
            // Got data => proceed
            break;
        }
    } while (result == 0); //read no data then continue


    DBG_PRINT("\r\n\r\n[%s] STAGE4: allplay_cb_start \r\n\r\n", __func__);
    bsp_feed_watchdog();
    (*callbacks->start)(); //spent 200ms to earse flash

    // Read data until we get an empty packet (EOF), or no data after ACK_TIMEOUT
    while ((result > 0) && (len > 0)) {
        bsp_feed_watchdog();
        
         if (!(*callbacks->data)(buf, len)) {
            DBG_PRINT("[%s] 'data' callback failed\r\n", __func__);
            return 0;
        }
        
        //result = readData(&buf, &len, -1); //wait forever
        result = readData(&buf, &len, DATA_TIMEOUT_MS); //tymphany modify
        if (result < 0) {
            DBG_PRINT("[%s] Failed to read data\r\n", __func__);
            return 0;
        }       
        else if (result==0) //no data (Tymphany add)
        {
            DBG_PRINT("[%s] Timeout to read data\r\n", __func__);
            return 0;
        }
    }

    bsp_feed_watchdog();
    if (!(*callbacks->done)()) {
        DBG_PRINT("[%s] 'done' callback failed\r\n", __func__);
        return 0;
    }

    // Read next data (reboot)
    DBG_PRINT("[%s] Wait for reboot packet\r\n", __func__);

    //result = readData(&buf, &len, -1); //org code, wait forever
    result = readData(&buf, &len, REBOOT_TIMEOUT_MS); //tymphany modify, avoid SAM does not send data
    if (result < 0) {
        DBG_PRINT("[%s] Failed to read reboot\r\n", __func__);
        return 0;
    }
    if (len != 0) {
        DBG_PRINT("[%s] Reboot packet not empty\r\n", __func__);
        return 0;
    }

    printf("\r\n\r\n");
    DBG_PRINT("[%s] STAGE5: Reboot (finish) \r\n", __func__);
    bsp_feed_watchdog();
    (*callbacks->reboot)();

    return 1;
}

void receive(struct UpdateCallbacks callbacks) {
    uint32 targetTime = getTime() + UPDATE_TIMEOUS_MS;

#if defined(LOG_PACKET)
    logPacket = 1;
#endif // LOG_PACKET
    
    openDevice(); //init uart_sam    
    while ( !receiveFile(&callbacks) )
    {
        /* If app flash is borken, we must wait firmware forever
         * If app flash is good, we wait RESET packet no longer than UPDATE_TIMEOUS_MS
         */
        if( getTime()>targetTime && !ValidAppWrong() ) {
            DBG_PRINT("\r\n\r\n", __func__, UPDATE_TIMEOUS_MS);
            DBG_PRINT("[%s] ***** Timeout(%dms) to wait RESET pakcet ***** \r\n\r\n", __func__, UPDATE_TIMEOUS_MS);
            break;
        }
    }
    closeDevice(); //destroy uart_sam
}


#endif /* #ifndef BL_ALLPLAY_SIMULATOR */