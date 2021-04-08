/**
 * @file
 */
/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
 * Copyright (C) 2015, Qualcomm Connected Experiences, Inc.
 *
 *    All rights reserved.
 *    This file is licensed under the 3-clause BSD license in the NOTICE.txt
 *    file for this project. A copy of the 3-clause BSD license is found at:
 *
 *        http://opensource.org/licenses/BSD-3-Clause.
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the license is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the license for the specific language governing permissions and
 *    limitations under the license.
 ******************************************************************************/

#include "aj_net.h"
#include "aj_serial.h"

AJ_Status AJ_Net_Send(AJ_IOBuffer* buf)
{
    AJ_Status ret = AJ_OK;
    size_t tx = AJ_IO_BUF_AVAIL(buf);

    assert(buf->direction == AJ_IO_BUF_TX);

    if (tx > 0) {
        ret = AJ_SerialSend(buf->readPtr, tx);
        if (ret != AJ_OK) {
            return ret;
        }
        buf->readPtr += tx;
    }
    if (AJ_IO_BUF_AVAIL(buf) == 0) {
        AJ_IO_BUF_RESET(buf);
    }
    return AJ_OK;
}

AJ_Status AJ_Net_Recv(AJ_IOBuffer* buf, uint32_t len, uint32_t timeout)
{
    AJ_Status status = AJ_OK;
    size_t rx = AJ_IO_BUF_SPACE(buf);
    uint16_t recv = 0;

    assert(buf->direction == AJ_IO_BUF_RX);

    rx = min(rx, len);
    if (rx) {
        status = AJ_SerialRecv(buf->writePtr, rx, timeout, &recv);
        if (status == AJ_OK) {
            buf->writePtr += recv;
        }
    }
    return status;
}


static uint8_t rxData[768];
static uint8_t txData[768];

AJ_Status AJ_Net_Connect(AJ_NetSocket* netSock, uint16_t port, uint8_t addrType, const uint32_t* addr)
{
    AJ_IOBufInit(&netSock->rx, rxData, sizeof(rxData), AJ_IO_BUF_RX, NULL);
    netSock->rx.recv = AJ_Net_Recv;
    AJ_IOBufInit(&netSock->tx, txData, sizeof(txData), AJ_IO_BUF_TX, NULL);
    netSock->tx.send = AJ_Net_Send;
    return AJ_OK;
}

void AJ_Net_Disconnect(AJ_NetSocket* netSock)
{
    //TODO AJ_SerialShutdown
}
