/**************************************************************
 * Copyright (C) 2013-2014, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/

#ifndef RECEIVER_H_
#define RECEIVER_H_

/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdint.h> // uint8
#include <stddef.h>  //size_t
#include <GenericTypeDefs.h>
#include "Bootloader.h"
#include "allplay_protocol.h"



/*****************************************************************************
 * Type                                                                      *
 *****************************************************************************/
/**
 * @mainpage
 * Low-level functions to implement (time and serial line/UART access) (see stream.h)
 * - #getTime
 * - #openDevice
 * - #closeDevice
 * - #waitInput
 * - #waitOutput
 * - #readStream
 * - #writeStream
 *
 * High-level functions to implement (see UpdateCallbacks in allplay_receiver.h):
 * - UpdateCallbacks#start
 * - UpdateCallbacks#data
 * - UpdateCallbacks#done
 * - UpdateCallbacks#reboot
 *
 * High-level function to call (see allplay_receiver.h):
 * - #receive
 *
 * Usage:
 *
 * To compile on the MCU, use the source at the root of the <tt>src</tt>
 * directory and the <tt>receiver.*</tt> from the <tt>receiver</tt> directory.
 * You should also define the <tt>MAX_DATA_SIZE</tt> macro with the maximum
 * number of bytes that the application want to receive in the
 * UpdateCallbacks#data callback (e.g. <tt>-DMAX_DATA_SIZE=256</tt> to receive
 * up to 256 bytes of data). The application will need a bit more than 6 times
 * as much RAM to work.
 *
 * The functions defined in stream.h must be implemented in the application. An
 * example is provided by <tt>platform/unix.c</tt> which has an implementation
 * for Unix platforms.
 *
 * The application should call #receive when ready to receive a new firmware.
 * #receive will block until after the update is done. It may not even return
 * is UpdateCallbacks#reboot resets the board or blocks forever.
 * #receive will use the low-level functions to get the firmware from the
 * sender and call the #UpdateCallbacks to let the application process each
 * piece of data. <tt>recv_main.c</tt> in the <tt>receiver</tt> directory shows
 * an example of such an application (<tt>main</tt> function and
 * #UpdateCallbacks callbacks)
 */

/**
 * @file
 * @brief API for the receiving end of the update process
 */


#if defined(LOG_PACKET)
extern int logPacket;
#endif // LOG_PACKET

/**
 * List of callbacks used by the update process.
 */
struct UpdateCallbacks {
    /**
     * Called to prepare the reception of a new firmware.
     *
     * This callback will be called each time the update process is restarted.
     * This can happen if the #data or #done callbacks return an error or if
     * the sender is restarted.
     */
    void (*start)(void);

    /**
     * Data block received.
     *
     * If the callback returns an error, the update process will be restarted
     *
     * @param data piece of the firmware
     * @param len length
     * @return 0 if failure, 1 if success
     */
    int32 (*data)(const uint8 *data, size_t len);

    /**
     * The last block of firmware has been sent.
     *
     * The app should use this callback to finish the update (write any
     * unprocessed data, update bootloader environment, ...).
     * If this callback returns an error, the update process will be restarted.
     *
     * @return 0 if failre, 1 if success
     */
    int32 (*done)(void);

    /**
     * The SAM has finished its own update and is ready to reboot.
     *
     * The MCU can use this callback to notify the user that the update is
     * complete or reboot the device or MCU as needed.
     */
    void (*reboot)(void);
};


/*****************************************************************************
 * Function Protocol                                                         *
 *****************************************************************************/
/**
 * Start the update process.
 *
 * This function will only return if the UpdateCallbacks#reboot callback does.
 *
 * @param callbacks list of callbacks used by the update process
 */
void receive(struct UpdateCallbacks callbacks);


#endif /* RECEIVER_H_ */
