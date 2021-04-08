/**************************************************************
 * Copyright (C) 2013-2014, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/endian.h>  // for ntohs/htons
#include "BootLoader.h" //each file must include Bootloader.h
#include "util.h" //DBG_PRINT, MIN
#include "dbgprint.h"
#include "allplay_common.h"
#include "allplay_crc-ccitt.h"
#include "allplay_stream.h"
#include "allplay_protocol.h"

#define WRITE_TIMEOUT ((uint32_t)2) // 2ms

#define MAX_RESEND 10
#define MAX_RECEIVE 3

#define MRK ((uint8)0xc0) // packet marker
#define ESC ((uint8)0xdb) // to escape packet marker
#define MRK_SUB ((uint8)0xdc) // replacement for packet marker
#define ESC_SUB ((uint8)0xdd) // replacement for escape
// in data:
//   MRK => ESC+MRK_SUB
//   ESC => ESC+ESC_SUB




/*****************************************************************************
 * Defination                                                                *
 *****************************************************************************/
/* In endian.h, we know nthos(x) was defined to swap16(x) which will call
 *     __swap32gen(x) for constand value or
 *     __swap16md(x) for variable.
 * But __swap16md(x) was not defined when we set optimal level to "s".
 * Thus we must define __swap16md(x), or release build will fail with error message:
 *     allplay_protocol.c:439: undefined reference to `__swap16md'
 */
#ifndef __swap16md
#define __swap16md(x) __swap16gen(x)
#endif

/* Add another sequence field on header
 * NOTE: Sender also need to enable this feature
 */
//#define BL_ALLPLAY_ADDITIONAL_HEADER





/*****************************************************************************
 * Type                                                                      *
 *****************************************************************************/
// In GCC/Clang, use a packed enum so that we can get warnings if we forget a
// value in a switch while keeping PacketType a 8-bit type.
#ifdef __GNUC__
typedef enum __attribute__((__packed__)) {
    //AllJoyn data = 0x00,
    //AllJoyn udata  = 0x01,
    PACKET_RESET = 0x02,
    PACKET_DATA = 0x03,
    PACKET_ACK = 0x04,
    PACKET_NACK = 0x05,
    PACKET_RACK = 0x06, // ack for a reset
    PACKET_AJ_CTRL = 0x0e, // An AllJoyn control packet, used to check if an alljoyn client instead of our app
    //AllJoyn ack = 0x0f,
    PACKET_RESET_ACKED = 0xfd, // internal use only: reset received and ack'ed
    PACKET_DATA_ACKED = 0xfe, // internal use only: data received and ack'ed
    PACKET_DATA_READ = 0xff // internal use only: data received and ack'ed and read => need to be deleted during the next readData/writeData
} PacketType;
#else
#define PACKET_RESET ((uint8)0x02)
#define PACKET_DATA ((uint8)0x03)
#define PACKET_ACK ((uint8)0x04)
#define PACKET_NACK ((uint8)0x05)
#define PACKET_RACK ((uint8)0x06) // ack for a reset
#define PACKET_AJ_CTRL ((uint8)0x0e) // An AllJoyn control packet, used to check if an alljoyn client instead of our app
//#define AllJoyn ack ((uint8)0x0f)
#define PACKET_RESET_ACKED ((uint8)0xfd) // internal use only: reset received and ack'ed
#define PACKET_DATA_ACKED ((uint8)0xfe) // internal use only: data received and ack'ed
#define PACKET_DATA_READ ((uint8)0xff) // internal use only: data received and ack'ed and read => need to be deleted during the next readData/writeData
typedef uint8 PacketType;
#endif


typedef struct __attribute__((__packed__)) {
    uint8 seq;
    PacketType type;
    uint16 size;
#ifdef BL_ALLPLAY_ADDITIONAL_HEADER
    uint16 gseq;
#endif
} Header;

#define MAX_INPUT_EXTRA 16
#define MAX_UNESC_HEADER_SIZE (sizeof(MRK) + sizeof(Header))
#define MAX_UNESC_PACKET_SIZE (sizeof(MRK) + sizeof(Header) + MAX_DATA_SIZE + sizeof(Crc))
#define MAX_UNESC_ACK_SIZE (sizeof(MRK) + sizeof(Header) + sizeof(Crc))
#define MAX_ESC_HEADER_SIZE (sizeof(MRK) + sizeof(Header) * 2)
#define MAX_ESC_PACKET_SIZE (sizeof(MRK) + (sizeof(Header) + MAX_DATA_SIZE + sizeof(Crc)) * 2)
#define MAX_ESC_ACK_SIZE (sizeof(MRK) + (sizeof(Header) + sizeof(Crc)) * 2)



/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
#ifdef BL_ALLPLAY_ADDITIONAL_HEADER
uint16 gseq_out= 0;
#endif
//                    _0          _maxDataSize
//                   /           /
// getDataBuffer     [           ]
//                   |           |
// output = [ header | data ] free ]
//          \_output |      |      \_output+MAX_ESC_PACKET_SIZE
//                   |      \_outputEndPos
//                   \_output+MAX_UNESC_HEADER_SIZE
static uint8 output[MAX_ESC_PACKET_SIZE];
static uint8 outputSeq = (uint8)0xff; // last sequence number sent
static uint8 *outputEndPos = output;

static uint8 ack[MAX_ESC_ACK_SIZE];
static uint8 *ackEndPos;

// Enough room for one pending data packet + one duplicate or the next one + an
// ack/nack/rack + extra space to manage unescaping data and discarding other
// packets.
// input = [ pkt1 | pkt2 | ... | unesc data   | esc data | free ]
//                             | (aka partial |          \_inputWritePos
//                             | packet)      \_inputEndPos
//                             \_inputStartPos
static uint8 input[2 * MAX_UNESC_PACKET_SIZE + MAX_UNESC_ACK_SIZE + MAX_INPUT_EXTRA];
static uint8 inputSeq = (uint8)0xff; // last sequence number received
static uint8 *inputStartPos = input; // packet being decoded
static uint8 *inputEndPos = input; // end of unescaped data (end of filled buffer or next is a packet marker)
static uint8 *inputWritePos = input; // end of received bytes

/*static */size_t maxDataSize = MAX_DATA_SIZE;

static uint32 ackTimeout = (uint32)1000;   // 1000ms
static uint32 resetTimeout = (uint32)1000; // 1000ms

enum ProtocolState {
    PROTOCOL_WAITING_RESET, // only for "sender": waiting for a RESET from "receiver"
    PROTOCOL_WAITING_FIRST_PACKET, // reset received => waiting for first data
    PROTOCOL_DATA // normal mode
};

enum AckStatus {
    ACK_NONE,
    ACK_RESET,
    ACK_ACK,
    ACK_NACK,
    ACK_AJ // got AllJoyn CONN packet => don't expect a ACK
};



/*****************************************************************************
 * Functiion Protocol                                                        *
 *****************************************************************************/
static char *get_packet_seq_string( Header* header );
static uint16 get_halfword_unalign(uint8* ptr);
static void set_halfword_unalign(uint8* ptr, uint16 value);




/*****************************************************************************
 * Functiion Implemenation                                                   *
 *****************************************************************************/
static char *get_packet_seq_string( Header* header )
{
    static char str[20];
#ifdef BL_ALLPLAY_ADDITIONAL_HEADER
    snprintf(str, sizeof(str), "seq%u, gseq%03u",  header->seq, header->gseq);
#else
    snprintf(str, sizeof(str), "seq%u",  header->seq);
#endif
    return str;
}

//Tymphany: On PIC32, the access for halfword (2bytes) with address which is not aligned on 16bit boundary, cause exception
__inline static uint16 get_halfword_unalign(uint8* ptr)
{
    uint16 ret= ptr[0];
    ret|= ptr[1]<<8;
    return ret;
}

//Tymphany: On PIC32, the access for halfword (2bytes) with address which is not aligned on 16bit boundary, cause exception
__inline static void set_halfword_unalign(uint8* ptr, uint16 value)
{
    *ptr = value & 0xFF;
    *(ptr+1) = (value>>8) & 0xFF;
}


static const char *packetTypeStr(PacketType type) {
	static char unknown[] = "UNKNOWN_XY";
	static const char *hex = "0123456789ABCDEF";

	switch (type) {
	case PACKET_RESET:
		return "RESET";
	case PACKET_DATA:
		return "DATA";
	case PACKET_ACK:
		return "ACK";
	case PACKET_NACK:
		return "NACK";
	case PACKET_RACK:
		return "RACK";
	case PACKET_AJ_CTRL:
		return "AJ_CTRL";
	case PACKET_RESET_ACKED:
		return "RESET_ACKED";
	case PACKET_DATA_ACKED:
		return "DATA_ACKED";
	case PACKET_DATA_READ:
		return "DATA_READ";
	}

	// Not using "default:" in the switch so we get a compilation warning if
	// we forget a case
	unknown[8] = hex[(type >> 4) & 0xF];
	unknown[9] = hex[type & 0xF];
	return unknown;
}




static void escapePacket(uint8 *start, uint8 **endptr) {
    size_t count = 0;
    uint8 *end = *endptr;

    // Skip packet marker
    start++;

    // Count the number of characters to escape
    while (start < end) {
        uint8 ch = *(start++);
        if ((ch == MRK) || (ch == ESC)) {
            count++;
        }
    }

    // Escape the characters (working backward)
    *endptr += count;
    end = *endptr;
    while (start != end) {
        uint8 ch = *(--start);
        switch (ch) {
        case MRK:
            *(--end) = MRK_SUB;
            *(--end) = ESC;
            break;
        case ESC:
            *(--end) = ESC_SUB;
            *(--end) = ESC;
            break;
        default:
            *(--end) = ch;
            break;
        }
    }
}

// Decode data until the next packet marker or end of buffer
//
// return 1 if marker found, 0 if no more data
static int32 unescapeData(void) {
    uint8 *readPos;

    // Skip initial unescaped data (don't need to move it)
    while (inputEndPos < inputWritePos) {
        uint8 ch = *inputEndPos;
        if (ch == MRK) {
            if (inputEndPos == inputStartPos) {
                // expected initial packet marker
            }
            else {
                return 1;
            }
        }
        if (ch == ESC) {
            break;
        }
        inputEndPos++;
    }

    // unescape data (up to next packet)
    readPos = inputEndPos;
    while (readPos < inputWritePos) {
        uint8 ch = *readPos;
        if (ch == MRK) {
            // end of packet
            break;
        }
        else if (ch == ESC) {
            if ((readPos + 1) == inputWritePos) {
                // not enough data
                break;
            }
            readPos++;
            ch = *(readPos++);
            switch (ch) {
            case MRK_SUB:
                *(inputEndPos++) = MRK;
                break;
            case ESC_SUB:
                *(inputEndPos++) = ESC;
                break;
            default:
                // unexpected data ignore (CRC will take care of that)
                break;
            }
        }
        else {
            readPos++;
            *(inputEndPos++) = ch;
        }
    }

    // compact the buffer
    memmove(inputEndPos, readPos, inputWritePos - readPos);
    inputWritePos -= (readPos - inputEndPos);

    if ((inputEndPos < inputWritePos) && (*(inputEndPos) == MRK)) {
        return 1;
    }
    else {
        return 0;
    }
}

static Header* getHeader(uint8 *start) {
    return (Header*)(start + sizeof(MRK));
}

static uint8* getData(uint8 *start) {
    return (start + sizeof(MRK) + sizeof(Header));
}

static size_t getPacketSize(uint8 *start) {
    return (sizeof(MRK) + sizeof(Header) + getHeader(start)->size + sizeof(Crc));
}

static uint8* getNextPacket(uint8 *start) {
    if (start == NULL) { //get first packet
        if (inputStartPos > input) { 
            return input; //have data
        }
        else {
            return NULL; //do data
        }
    }
    else {
        uint8 *next = start + getPacketSize(start);
        if (next >= inputStartPos) {
            return NULL;
        }
        else {
            return next;
        }
    }
}

#if defined(LOG_PACKET)
int logPacket;
static void dumpPacket(const char *prefix, uint8_t *packet, int swapSize) {
	if (logPacket) {
		Header *header = getHeader(packet);
		uint16_t size = swapSize ? ntohs(header->size) : header->size;
		DBG_PRINT("%s seq%u (type %s, %u bytes)\r\n", prefix, header->seq, packetTypeStr(header->type), size);
	}
}
#endif // LOG_PACKET

static int32 validateAjPacket(int32 isComplete) {
    static uint8 ajConn[] = { 0xC0, 0x0, 0xE, 0x0, 0x4, 'C', 'O', 'N', 'N', 0x5E, 0xFB };

    size_t sizePkt= inputEndPos - inputStartPos;
    if ( sizePkt < sizeof(ajConn)) {
        return isComplete ? -1 : 0;
    }

    if (memcmp(inputStartPos, ajConn, sizeof(ajConn)) != 0) {
        // Unknown AJ packet
        return -1;
    }

    // Connection packet
    return 1;
}


// isComplete: we receive a packet marker, the packet must be complete
//
// returns:
// -1: packet corrupted and should be dropped
//  0: packet incomplete
//  1: packet complete and validated
static int32 validatePacket(int32 isComplete) {
    Crc crc;
    Header *header;
    uint16 size;
    uint16 isCompleteSize;
    uint8 *queuedPacket = input;
    size_t pktSize= ((size_t)(inputEndPos - inputStartPos));

    assert(pktSize>=0); //Gavin add

    if (inputWritePos == (input + sizeof(input))) {
        // Input buffer is full, we can't read any more data so this packet must
        // be complete or discarded
        isComplete = 1;
    }

    if (pktSize < (sizeof(MRK) + sizeof(Header))) {
        // Not enough data for the header
        //DBG_PRINT("[%s] Incomplete header (%u < %u)\r\n", __func__, (inputEndPos - inputStartPos), (sizeof(MRK) + sizeof(Header)));
        return isComplete ? -1 : 0;
    }

    if (*inputStartPos != MRK) {
        //DBG_PRINT("[%s] Invalid marker: %02x (expected: %02x)\r\n", __func__, *(inputStartPos), MRK);
        return -1;
    }

    header = getHeader(inputStartPos);

    switch (header->type) {
    case PACKET_RESET:
        break;
    case PACKET_DATA:
        break;
    case PACKET_ACK:
    case PACKET_NACK:
    case PACKET_RACK:
        if (queuedPacket != inputStartPos) { //if queue have received packet
            do {
                Header *queuedHeader = getHeader(queuedPacket);
                if ((queuedHeader->type == PACKET_ACK) || (queuedHeader->type == PACKET_NACK) || (queuedHeader->type == PACKET_RACK)) {
                    // We already have a nack packet
                    DBG_PRINT("[%s] Too many ACK/NACK/RACK packets. Drop it: %s(type %s, %u bytes)\r\n", __func__, get_packet_seq_string(header), packetTypeStr(header->type), header->size);
                    return -1;
                }
                queuedPacket = getNextPacket(queuedPacket);
            } while (queuedPacket != NULL);
        }
        break;
    case PACKET_AJ_CTRL:
        return validateAjPacket(isComplete);
    case PACKET_RESET_ACKED:
    case PACKET_DATA_ACKED:
    case PACKET_DATA_READ:
    default:
        DBG_PRINT("[%s] Invalid type: %u\r\n", __func__, header->type);
        return -1;
    }

    size = ntohs(header->size);
    if (size > maxDataSize) {
        // Packet too big
        DBG_PRINT("[%s] Invalid size: %u (max %u)\r\n", __func__, size, maxDataSize);
        return -1;
    }
    isCompleteSize = (sizeof(MRK) + sizeof(Header) + size + sizeof(Crc));
    if ((inputEndPos - inputStartPos) < isCompleteSize) {
        // Not enough data for whole packet
        //DBG_PRINT("[%s] Not enough data (%u < %u)\r\n", __func__, (inputEndPos - inputStartPos), isCompleteSize);
        return isComplete ? -1 : 0;
    }

    crc = CRC_INIT;
    computeCrc(inputStartPos, isCompleteSize, &crc);
    if (crc != 0) {
        // Invalid CRC
        DBG_PRINT("[%s] Invalid crc: 0x%04x (expected 0)\r\n", __func__, crc);
        return -1;
    }

    return 1;
}

static void queuePacket(void) {
    Header *header = getHeader(inputStartPos);
    header->size = ntohs(header->size);
    DBG_PRINT("[%s] Queueing %s (type %s, %u bytes)\r\n", __func__, get_packet_seq_string(header), packetTypeStr(header->type), header->size);
#if defined(LOG_PACKET)
	dumpPacket("<", inputStartPos, 0);
#endif // LOG_PACKET
    inputStartPos += getPacketSize(inputStartPos);
    assert(inputStartPos>=input && inputStartPos< ((uint8*)input)+sizeof(input)); //Gavin add
}

// Delete specific packet and return next one
static uint8 *deletePacket(uint8 *start) {
    size_t size;
    uint8 *next = start + getPacketSize(start);
    if (next == NULL) {
        next = inputStartPos;
    }
    //DBG_PRINT("[%s] Delete %s (type %s, %u bytes)\r\n", __func__, get_packet_seq_string(getHeader(start)), packetTypeStr(getHeader(start)->type), getHeader(start)->size);
    //dumpPacket("deletePacket (before delete)");
    size = next - start;
    memmove(start, next, inputWritePos - next);
    inputWritePos -= size;
    inputEndPos -= size;
    inputStartPos -= size;
    assert(inputStartPos>=input && inputStartPos< ((uint8*)input)+sizeof(input)); //Gavin add
    assert(inputEndPos>=input   &&   inputEndPos< ((uint8*)input)+sizeof(input)); //Gavin add
    assert(inputWritePos>=input && inputWritePos< ((uint8*)input)+sizeof(input)); //Gavin add
    next -= size;
    //dumpPacket("deletePacket (after delete)");

    if (next >= inputStartPos) {
        return NULL;
    }
    else {
        return next;
    }
}

// Delete everything up to specific packet
static uint8 *deletePreviousPackets(uint8 *start) {
    size_t size = (start - input);
    memmove(input, start, inputWritePos - start);
    inputWritePos -= size;
    inputEndPos -= size;
    inputStartPos -= size;
    assert(inputStartPos>=input && inputStartPos< ((uint8*)input)+sizeof(input)); //gavin add
    assert(inputEndPos>=input &&   inputEndPos< ((uint8*)input)+sizeof(input));   //gavin add
    assert(inputWritePos>=input && inputWritePos< ((uint8*)input)+sizeof(input)); //gavin add
    return input;
}

static int32 readPacket(void) {
    ssize_t readCount;
    uint8 *packet;
    int32 isComplete;

    // Drop read packet (should always be the first packet since other packet
    // should have been deleted by then)
    packet = getNextPacket(NULL);
    if (packet && (getHeader(packet)->type == PACKET_DATA_READ)) {
        deletePacket(packet);
    }

    readCount = readStream(inputWritePos, (input + sizeof(input)) - inputWritePos);
    if (readCount <= 0) {
        return (readCount < 0) ? -1 : 0;
    }
    inputWritePos += readCount;
    //assert(inputWritePos>=input && inputWritePos< ((uint8*)input)+sizeof(input)); //Gavin add

    do {
        int32 valid;
        isComplete = unescapeData();
        valid = validatePacket(isComplete);
        if (valid == -1) {
            // Drop packet
            size_t size = inputEndPos - inputStartPos;
            memmove(inputStartPos, inputEndPos, inputWritePos - inputEndPos);
            inputWritePos -= size;
            inputEndPos -= size;
            assert(inputEndPos>=input &&   inputEndPos< ((uint8*)input)+sizeof(input));   //Gavin add
            assert(inputWritePos>=input && inputWritePos< ((uint8*)input)+sizeof(input)); //Gavin add
        }
        else if (valid == 0) {
            // Packet incomplete, wait for more data
        }
        else {
            // Packet is complete
            queuePacket();
        }
    } while (isComplete);

    return 1;
}

static int32 sendPacket(uint8 *start, uint8 *end) {
    ssize_t writeCount;
    int32 result;
    uint8 *pos = start;
    DBG_PRINT("[%s] send a packet\r\n", __func__);

    while (pos < end) {
        writeCount = writeStream(pos, end - pos);
        if (writeCount < 0) {
            return -1;
        }
        pos += writeCount;
        if (pos == end) {
            break;
        }

        // Read some data in case the receiver is waiting to write something
        if (waitInput(0) > 0) {
            result = readPacket();
            if (result < 0) {
                // ignore all read errors for now
                result = 0;
            }
        }

        // Wait for free space in output
        result = waitOutput((int32)WRITE_TIMEOUT);
        if (result < 0) {
            return result;
        }
        // free space (try to write more) or timeout (try to read more)
    }

    return 1;
}

void buildReset(void) {
    Crc crc;
    Header *header = (Header*)(output + sizeof(MRK));


    outputSeq = (uint8)0xff;

    // Write header
    output[0] = MRK;
    header->type = PACKET_RESET;
    header->seq = ++outputSeq;

#ifdef BL_ALLPLAY_ADDITIONAL_HEADER
    gseq_out++;
    header->gseq = gseq_out;
#endif
    
    header->size = htons(sizeof(uint16));
    outputEndPos = output + sizeof(MRK) + sizeof(Header);

    set_halfword_unalign(outputEndPos, htons(MAX_DATA_SIZE));
    //*(uint16*)outputEndPos = htons(MAX_DATA_SIZE); //org code, cause exception
    
    outputEndPos += sizeof(uint16); 

    // Compute and save the CRC (little-endian format for one-pass CRC check)
    crc = CRC_INIT;
    computeCrc(output, outputEndPos - output, &crc);
    *(outputEndPos++) = (crc & 0xFF);
    *(outputEndPos++) = ((crc >> 8) & 0xFF);

    //DBG_PRINT("\r\n\r\n\r\n[%s] Reset %s, crc 0x%04x\r\n", __func__, outputSeq, get_packet_seq_string(header), crc);

#if defined(LOG_PACKET)
	dumpPacket(">", output, 1);
#endif // LOG_PACKET
    escapePacket(output, &outputEndPos);
}

void buildData(size_t outLen) {
    Crc crc;
    Header *header = (Header*)(output + sizeof(MRK));

    // Write header
    output[0] = MRK;
    header->type = PACKET_DATA;
    header->seq = ++outputSeq;

#ifdef BL_ALLPLAY_ADDITIONAL_HEADER
    header->gseq= ++gseq_out;
#endif

    header->size = htons(outLen);
    outputEndPos = output + sizeof(MRK) + sizeof(Header) + outLen;

    // Compute and save the CRC (little-endian format for one-pass CRC check)
    crc = CRC_INIT;
    computeCrc(output, outputEndPos - output, &crc);
    *(outputEndPos++) = (crc & 0xFF);
    *(outputEndPos++) = ((crc >> 8) & 0xFF);
    DBG_PRINT("[%s] Data %s, size %u bytes, crc 0x%04x\r\n", __func__, outputSeq, get_packet_seq_string(header), outLen, crc);

#if defined(LOG_PACKET)
	dumpPacket(">", output, 1);
#endif // LOG_PACKET
    escapePacket(output, &outputEndPos);
}

void buildAck(PacketType type, uint8 seq) {
    Crc crc;
    Header *header = (Header*)(ack + sizeof(MRK));

    // Write header
    ack[0] = MRK;
    header->type = type;
    header->seq = seq;


#ifdef BL_ALLPLAY_ADDITIONAL_HEADER
    header->gseq= ++gseq_out;
#endif
    
    header->size = htons(0);
    ackEndPos = ack + sizeof(MRK) + sizeof(Header);

    // Compute and save the CRC (little-endian format for one-pass CRC check)
    crc = CRC_INIT;
    computeCrc(ack, ackEndPos - ack, &crc);
    *(ackEndPos++) = (crc & 0xFF);
    *(ackEndPos++) = ((crc >> 8) & 0xFF);

    DBG_PRINT("[%s] %s %s, crc 0x%04x, type 0x%02X\r\n", __func__, packetTypeStr(header->type), get_packet_seq_string(header), crc, header->type);

#if defined(LOG_PACKET)
	dumpPacket(">", ack, 1);
#endif // LOG_PACKET
    escapePacket(ack, &ackEndPos);
}

// return next packet
uint8 *ackPacket(uint8 *packet) {
    Header *header = getHeader(packet);
    switch (header->type) {
    case PACKET_RESET:
        // reset
        buildAck(PACKET_RACK, header->seq);
        sendPacket(ack, ackEndPos);
        header->type = PACKET_RESET_ACKED;
        return deletePreviousPackets(packet);

    case PACKET_DATA:
        if (header->seq == inputSeq) {
            // duplicate packet
            buildAck(PACKET_ACK, header->seq);
            sendPacket(ack, ackEndPos);
            return deletePacket(packet);
        }
        else if (header->seq == (uint8)(inputSeq + 1)) {
            // expected packet
            buildAck(PACKET_ACK, header->seq);
            sendPacket(ack, ackEndPos);
            header->type = PACKET_DATA_ACKED;
            inputSeq++;
            return packet;
        }
        else {
            // Not the packet we wanted, probably a corruption => NACK

            DBG_PRINT("[%s] Unwanted packet %s(expected seq%u)\r\n", __func__, header->seq, get_packet_seq_string(header), (uint8)(inputSeq + 1));
            buildAck(PACKET_NACK, inputSeq);
            sendPacket(ack, ackEndPos);

            return deletePacket(packet);
        }
    case PACKET_AJ_CTRL:
    case PACKET_ACK:
    case PACKET_NACK:
    case PACKET_RACK:
    case PACKET_RESET_ACKED:
    case PACKET_DATA_ACKED:
    case PACKET_DATA_READ:
        // Don't need acks
        return getNextPacket(packet);
    }

    // Not using "default:" in the switch so we get a compilation warning if
    // we forget a case
    return NULL;
}

enum AckStatus findAck(void) {
    uint8 *packet = getNextPacket(NULL);
    while (packet) {
        Header *header = getHeader(packet);
        switch (header->type) {
        case PACKET_RESET:
            DBG_PRINT("[%s] Receive Reset\r\n", __func__);
            ackPacket(packet);
            return ACK_RESET;

        case PACKET_DATA:
            DBG_PRINT("[%s] Receive Data\r\n", __func__);
            packet = ackPacket(packet);  //return next packet
            break;

        case PACKET_ACK:
            if (header->seq == outputSeq) {
                DBG_PRINT("[%s] Receive Ack\r\n", __func__);
                // found the ack
                deletePacket(packet);
                return ACK_ACK;
            }
            else {

                DBG_PRINT("[%s] Receive Duplicate ack (got %s, expected seq%u)\r\n", __func__, header->seq, get_packet_seq_string(header), outputSeq);
                // duplicate => delete and continue
                packet = deletePacket(packet);
                break;
            }

        case PACKET_NACK:
            if (header->seq == outputSeq) {
                DBG_PRINT("[%s] Receive Nack\r\n", __func__);
                // found the nack
                deletePacket(packet);
                return ACK_NACK;
            }
            else {

                DBG_PRINT("[%s] Receive Duplicate nack (got %s, expected seq%u)\r\n", __func__, get_packet_seq_string(header), outputSeq);
                // duplicate => delete and continue
                packet = deletePacket(packet);  //return next packet
                break;
            }

        case PACKET_RACK:
            if (header->seq == outputSeq) {
                DBG_PRINT("[%s] Receive Rack\r\n", __func__);
                // found a rack => finish the reset
                packet = deletePreviousPackets(packet);
                deletePacket(packet);
                inputSeq = (uint8)0xff;
                return ACK_ACK;
            }
            else {
                // duplicate
                DBG_PRINT("[%s] Receive Duplicate rack (got %s, expected seq%u)\r\n", __func__, get_packet_seq_string(header), outputSeq);
                packet = deletePacket(packet);
                break;
            }

        case PACKET_AJ_CTRL:
            return ACK_AJ;

        case PACKET_RESET_ACKED:
            DBG_PRINT("[%s] Receive Reset (acked)\r\n", __func__);
            return ACK_RESET;

        case PACKET_DATA_ACKED:
        case PACKET_DATA_READ:
            DBG_PRINT("[%s] Receive Other (type 0x%02x)\r\n", __func__, header->type);
            packet = getNextPacket(packet);
            break;
        }
    }

    return ACK_NONE;
}

uint8 *getDataBuffer(void) {
    return (output + MAX_UNESC_HEADER_SIZE);
}

void processReset(void) {
    uint8 *packet;
    Header *header;

    packet = getNextPacket(NULL);
    if (!packet) {
        // shouldn't happen
        DBG_PRINT("[%s] No packet for reset\r\n", __func__);
        return;
    }

    header = getHeader(packet);
    if (header->type != PACKET_RESET_ACKED) {
        // shouldn't happen
        DBG_PRINT("[%s] Unexpected packet seq%u %s (type %s, %u bytes)\r\n", __func__, get_packet_seq_string(header), packetTypeStr(header->type), header->size);
        return;
    }

    outputSeq = (uint8)0xff;
    inputSeq = header->seq;

    uint8 *pdata= getData(packet);
    uint16 dataSize= get_halfword_unalign(pdata);
    maxDataSize = MIN(ntohs(dataSize), maxDataSize);
    //maxDataSize = MIN(ntohs(   *(uint16*)getData(packet)    ), maxDataSize);  //cause exception
    //DBG_PRINT("[%s] maxDataSize = min(rack:%d , maxDataSize:%d) = %d \r\n", __func__, dataSizeNtohs, org_maxDataSize, maxDataSize);

    deletePacket(packet);
}

void processAJPacket(void) {
    uint8 *packet;
    Header *header;

    packet = getNextPacket(NULL);
    if (!packet) {
        // shouldn't happen
        DBG_PRINT("[%s] No packet\r\n", __func__);
        return;
    }

    header = getHeader(packet);
    if (header->type != PACKET_AJ_CTRL) {
        // shouldn't happen
        assert(0);
        DBG_PRINT("[%s] Unexpected packet %s (type %s, %u bytes)\r\n", __func__, get_packet_seq_string(header), packetTypeStr(header->type), header->size);
        return;
    }

    deletePacket(packet);
}

// Return: 1: packet sent, -1 error, -2: reset, -3: AllJoyn
//int32 sendOutput(uint32_t timeout) {  // org code
int32 sendOutput(uint32 timeout, uint32 timeout_break) {
    int32 remainingTimeout;
    int32 result;
    uint32 lastSend = getTime() - timeout;
    uint32 begin= getTime();
    //DBG_PRINT("[%s] ============================> START\r\n", __func__);

#if defined(LOG_PACKET)
	int first = 1;
#endif // LOG_PACKET

    for (;;) {
        bsp_feed_watchdog();

        if( (getTime()-begin) >= timeout_break )
        {
            DBG_PRINT("[%s] *****  Resent RESET many times but does not receive RACK ***** \r\n", __func__);
            return -1;  //resent many times but does not receive ACK
        }
        //if ((getTime() - lastSend) > timeout) {  //org
        if ((getTime() - lastSend) >= timeout) { //Tymphany modify
#if defined(LOG_PACKET)
            if (logPacket) {
                if (first) {
                    first = 0;
                }
                else {
                    DBG_PRINT("> [dup]\r\n");
                }
            }
#endif // LOG_PACKET
            result = sendPacket(output, outputEndPos); //blocking up to WRITE_TIMEOUT
            if (result < 0) {
                return result;
            }
            lastSend = getTime();
        }

        switch (findAck()) {  //blocking up to WRITE_TIMEOUT ms
        case ACK_NONE:
            remainingTimeout = (int32)(lastSend + timeout - getTime());
            //DBG_PRINT("[%s] ==== no ack, wait %d ms\r\n", __func__, remainingTimeout);
            if (remainingTimeout <= 0) {
                continue;
            }
            result = waitInput(remainingTimeout);
            if (result < 0) {
                DBG_PRINT("[%s] fail return (waitInput error)\r\n", __func__);
                return -1;
            }
            else if (result == 0) {
                // timeout
            }
            else {
                result = readPacket();
                if (result < 0) {
                    DBG_PRINT("[%s] fail return (readPacket error)\r\n", __func__);
                    return -1;
                }
            }
            break;
        case ACK_RESET:
            DBG_PRINT("[%s] ==== reset from sendOutput\r\n", __func__);
            processReset();
            //DBG_PRINT("[%s] ============================> END (ACK_RESET -2)\r\n", __func__);
            return -2;
        case ACK_NACK:
            DBG_PRINT("[%s] ==== receive nack\r\n", __func__);
            // force resend (fake expiration)
            lastSend = getTime() - timeout; 
            break;
        case ACK_ACK:
            DBG_PRINT("[%s] ==== receive ack\r\n", __func__);
            //dumpPacket(__func__);
            //DBG_PRINT("[%s] ============================> END (ACK_ACK 1)\r\n", __func__);
            return 1;
        case ACK_AJ:
            DBG_PRINT("[%s] ==== receive aj\r\n", __func__);
            processAJPacket();
            //DBG_PRINT("[%s] ============================> END (ACK_AJ -3)\r\n", __func__);
            return -3;
        }
    }

    // Unreachable
    assert(0);
    //DBG_PRINT("[%s] ============================> END (Unreachable)\r\n", __func__);
    return -1;
}

int32 recvInput(uint8 **packet_, int32 timeout) {
    int32 result;
    uint8 *packet;

    int32 remainingTimeout;
    uint32 startTime = getTime();

    packet = input;
    for (;;) {
        // Ack all packets that need ack'ing
        // Find a data packet
        packet = getNextPacket(NULL);
        while (packet) {
            Header *header = getHeader(packet);
            switch (header->type) {
            case PACKET_RESET:
            case PACKET_DATA:
                // non-ack'ed packet => ack it then continue (i.e. either
                // reprocess same packet but this time as ack'ed or process
                // next packet depending)
                packet = ackPacket(packet);
                break;
            case PACKET_ACK:
            case PACKET_NACK:
            case PACKET_RACK:
                // not sending, must be duplicate => delete
                packet = deletePacket(packet);
                break;
            case PACKET_AJ_CTRL:
            case PACKET_RESET_ACKED:
            case PACKET_DATA_ACKED:
                *packet_ = packet;
                return 1;
            case PACKET_DATA_READ:
                // already returned, we can now delete
                packet = deletePacket(packet);
                break;
            }
        }

        // no packet, get more
        if (timeout < 0) {
            remainingTimeout = -1;
        }
        else {
            remainingTimeout = (int32)(startTime + ((uint32)timeout) - getTime());
            if (remainingTimeout <= 0) {
                return 0;
            }
        }

        // Read whatever we have
        result = readPacket();
        if (result < 0) {
            DBG_PRINT("[%s] Error on read\r\n", __func__);
            return -1;
        }
        else if (result == 0) {
            result = waitInput(remainingTimeout);
            if (result < 0) {
                DBG_PRINT("[%s] Error on wait \r\n", __func__); // should be unreachable
                assert(0);
                return -1;
            }
            else if (result == 0) {
                return 0;
            }
        }
    }

    // Unreachable
    assert(0);
    return 0;
}

uint32_t getResetTimeout() {
	return resetTimeout;
}
void setResetTimeout(uint32_t timeout) {
	resetTimeout = timeout;
}

uint32_t getAckTimeout() {
	return ackTimeout;
}
void setAckTimeout(uint32_t timeout) {
	ackTimeout = timeout;
}

size_t getMaxDataSize(void) {
    return maxDataSize;
}

int32 writeReset(void) {
    buildReset();

    //resent RESET every "resetTimeout" ms. But no londer then UPDATE_TIMEOUS_MS
    return sendOutput(resetTimeout, UPDATE_TIMEOUS_MS);
}

int32 writeData(size_t outLen) {
    if (outLen > maxDataSize) {
        return -1;
    }
    buildData(outLen);

    //resent RESET every "resetTimeout" ms. But no londer then UPDATE_TIMEOUS_MS
    return sendOutput(ackTimeout, UPDATE_TIMEOUS_MS); //blocking up to UPDATE_TIMEOUS_MS ms
}

// Return: 0: no data, 1: buffer/len set, -1: error, -2: reset pending, -3: AllJoyn traffic
int32 readData(uint8 **buffer, size_t *len, int32 timeout) {
    uint8 *packet;
    Header *header;
    int32 result; 

    //DBG_PRINT("[%s] read data\r\n", __func__);

    result = recvInput(&packet, timeout); //blocking up to timeout ms
    if (result < 0) {
        return -1;
    }
    if (result == 0) {
        *buffer = NULL;
        *len = 0;
        return 0;
    }

    header = getHeader(packet);
    if (header->type == PACKET_RESET_ACKED) {
        DBG_PRINT("[%s] Reset\r\n", __func__);
        processReset();
        return -2;
    }
    if (header->type == PACKET_AJ_CTRL) {
        DBG_PRINT("[%s] AllJoyn\r\n", __func__);
        processAJPacket();
        return -3;
    }

    header->type = PACKET_DATA_READ;
    *buffer = getData(packet);
    *len = header->size;
    return 1;
} 

int32 peekData(uint8 **buffer, size_t *len) {
    uint8 *packet;
    Header *header;
    int32 result;

    result = recvInput(&packet, 0);
    if (result < 0) {
        return -1;
    }
    if (result == 0) {
        *buffer = NULL;
        *len = 0;
        return 0;
    }

    header = getHeader(packet);
    if (header->type == PACKET_RESET_ACKED) {
        DBG_PRINT("[%s] Reset\r\n", __func__);
        return -2;
    }
    if (header->type == PACKET_AJ_CTRL) {
        DBG_PRINT("[%s] AllJoyn\r\n", __func__);
        return -3;
    }

    *buffer = getData(packet);
    *len = header->size;
    return 1;
}
