/**
 * @file afproto.h
 */

#ifndef AFPROTO_H
#define AFPROTO_H

/** Start/end flag sequence value from RFC 1662. */
#define AFPROTO_FLAG_SEQUENCE 0x7D

/** Control escape value as defined in RFC 1662. */
#define AFPROTO_CONTROL_ESCAPE 0x7E

/**
 * Retrieves data from specified buffer containing the frame. Frames can be parsed from
 * multiple buffers e.g. when received via UART.
 *
 * @param[in] src Source buffer with frame
 * @param[in] src_len Source buffer length
 * @param[out] dest Destination buffer (should be able to contain max frame size)
 * @param[out] dest_len Destination buffer length
 * @retval -2 Invalid FCS (size of dest_len should be discarded from source buffer)
 * @retval -1 Invalid message
 * @retval >=0 Success (size of returned value should be discarded from source buffer)
 */
int afproto_get_data(const char *src, unsigned int src_len, char *dest,
                     unsigned int *dest_len);

/**
 * Creates frame with specified data buffer.
 *
 * @param[in] src Source buffer with data
 * @param[in] src_len Source buffer length
 * @param[out] dest Destination buffer (should be bigger than source buffer)
 * @param[out] dest_len Destination buffer length
 */
void afproto_frame_data(const char *src, unsigned int src_len, char *dest,
                        unsigned int *dest_len);

#endif
