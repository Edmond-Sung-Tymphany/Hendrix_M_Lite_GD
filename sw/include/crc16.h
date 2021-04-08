#ifndef CRC16_H
#define CRC16_H

/* Public */

/* CRC-CCITT (used in Alljoyn, Xmodem, Bluetooth etc.) */
unsigned short crc16(const unsigned char *data, unsigned short length);

#endif /* PT_H */