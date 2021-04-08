/**
 * @file
 */
/******************************************************************************
 * Copyright 2013, Qualcomm Innovation Center, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/socket.h>


#include "aj_target.h"
#include "aj_status.h"
#include "aj_serial.h"
#include "aj_serial_rx.h"
#include "aj_serial_tx.h"

#include "aj_serio.h"
#define AJ_MODULE TARGET_SERIAL
uint8_t dbgTARGET_SERIAL = 0;
#include "aj_debug.h"

#include "allplaymcu.h"

#define FRAME_DELIMITER 0xC0

extern AJ_SerIOConfig g_serialConfig;

static AJ_SerIORxCompleteFunc RecvCB;
static AJ_SerIOTxCompleteFunc SendCB;

/**
 * global function pointer for serial transmit funciton
 */
AJ_SerialTxFunc g_AJ_TX;

static struct {
	uint8_t* data;
	uint32_t len;
	uint8_t* pos;
} tx_buf, rx_buf;

static int serial_fd = -1;

static pthread_t serial_thread = 0;

static int notification_fd[2] = { -1, -1 };
static volatile int stopped = 1;

static pthread_mutex_t rx_mutex;
static volatile int rx_running = 0;

static pthread_mutex_t tx_mutex;
static volatile int tx_running = 0;

static void readData() {
	static uint8_t ReadingMsg = FALSE;
	uint8_t rxData;

	while (read(serial_fd, &rxData, 1) > 0) {
		// if we overrun the buffer, throw data away until we see a new frame
		if (rx_buf.pos >= rx_buf.data + rx_buf.len) {
			// throw data away until we see a new frame
			if (rxData == FRAME_DELIMITER) {
				ReadingMsg = FALSE;
				rx_buf.pos = rx_buf.data;
			}
			continue;
		}

		*(rx_buf.pos++) = rxData;

		if (rxData == FRAME_DELIMITER) {
			if (ReadingMsg == TRUE) {
				uint8_t*buf = rx_buf.data;
				uint32_t cnt = rx_buf.pos - rx_buf.data;
				rx_buf.pos = rx_buf.data = NULL;
				rx_buf.len = 0;
				ReadingMsg = FALSE;
				rx_running = 0;
				AJ_DumpBytes(__func__, buf, cnt);
				RecvCB(buf, cnt);
				break; // gives a chance for the thread to stop
			} else {
				ReadingMsg = TRUE;
			}
		}
	}
}

static void writeData() {
	ssize_t len = write(serial_fd, tx_buf.pos, (tx_buf.data + tx_buf.len) - tx_buf.pos);
	if (len <= 0) {
		return;
	}
	tx_buf.pos += len;

	if (tx_buf.pos == (tx_buf.data + tx_buf.len)) {
		tx_running = 0;
		AJ_DumpBytes(__func__, tx_buf.data, tx_buf.pos - tx_buf.data);
		SendCB(tx_buf.data, tx_buf.pos - tx_buf.data);
	}
}

static void *serialLoop(void* arg __attribute__((__unused__)) ) {
	fd_set fdRxSet;
	fd_set fdTxSet;
	int result;
	int maxfd;

	for (;;) {
		FD_ZERO(&fdRxSet);
		FD_SET(notification_fd[1], &fdRxSet);
		maxfd = notification_fd[1];
		pthread_mutex_lock(&rx_mutex);
		if (rx_running) {
			FD_SET(serial_fd, &fdRxSet);
			if (serial_fd > maxfd) {
				maxfd = serial_fd;
			}
		}
		pthread_mutex_unlock(&rx_mutex);

		FD_ZERO(&fdTxSet);
		pthread_mutex_lock(&tx_mutex);
		if (tx_running) {
			FD_SET(serial_fd, &fdTxSet);
			if (serial_fd > maxfd) {
				maxfd = serial_fd;
			}
		}
		pthread_mutex_unlock(&tx_mutex);

		result = select(maxfd + 1, &fdRxSet, &fdTxSet, NULL, NULL);
		if (result < 0) {
			break;
		}
		if (stopped) {
			break;
		}
		if (FD_ISSET(serial_fd, &fdRxSet) != 0) {
			pthread_mutex_lock(&rx_mutex);
			if (rx_running) {
				readData();
			}
			pthread_mutex_unlock(&rx_mutex);
		}
		if (FD_ISSET(serial_fd, &fdTxSet) != 0) {
			pthread_mutex_lock(&tx_mutex);
			if (tx_running) {
				writeData();
			}
			pthread_mutex_unlock(&tx_mutex);
		}
		if (FD_ISSET(notification_fd[1], &fdRxSet) != 0) {
			char tmp[10];
			read(notification_fd[1], tmp, sizeof(tmp));
		}
	}

	return NULL;
}

void AJ_RX(uint8_t* buf, uint32_t len) {
	AJ_ASSERT(buf != NULL);
	pthread_mutex_lock(&rx_mutex);
	rx_buf.data = buf;
	rx_buf.pos = buf;
	rx_buf.len = len;
	pthread_mutex_unlock(&rx_mutex);
}

void AJ_PauseRX() {
	pthread_mutex_lock(&rx_mutex);
	rx_running = 0;
	pthread_mutex_unlock(&rx_mutex);
}

void AJ_ResumeRX() {
	char b = 0;

	pthread_mutex_lock(&rx_mutex);
	rx_running = 1;
	write(notification_fd[0], &b, 1);
	pthread_mutex_unlock(&rx_mutex);
}

void __AJ_TX(uint8_t* buf, uint32_t len) {
	pthread_mutex_lock(&tx_mutex);
	tx_buf.data = buf;
	tx_buf.pos = buf;
	tx_buf.len = len;
	pthread_mutex_unlock(&tx_mutex);
}

void AJ_TX(uint8_t* buf, uint32_t len) {
	g_AJ_TX(buf, len); // should call the inner implementation
}

void AJ_PauseTX() {
	pthread_mutex_lock(&tx_mutex);
	tx_running = 0;
	pthread_mutex_unlock(&tx_mutex);
}

void AJ_ResumeTX() {
	char b = 0;

	pthread_mutex_lock(&tx_mutex);
	tx_running = 1;
	write(notification_fd[0], &b, 1);
	pthread_mutex_unlock(&tx_mutex);
}

#define AJ_SERIAL_WINDOW_SIZE   4
#define AJ_SERIAL_PACKET_SIZE   512 + AJ_SERIAL_HDR_LEN

static void InitMutexes() {
	pthread_mutexattr_t mattr;

	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&rx_mutex, &mattr);
	pthread_mutex_init(&tx_mutex, &mattr);

	pthread_mutexattr_destroy(&mattr);
}

static void DestroyMutexes() {
	pthread_mutex_destroy(&rx_mutex);
	pthread_mutex_destroy(&tx_mutex);
}

AJ_Status AJ_SerialTargetInit(const char* UNUSED(ttyName), uint16_t UNUSED(bitRate)) {
	return AJ_OK;
}

AJ_Status AJ_Serial_Up() {
	AJ_Status status;

	// Reset (nothing calls it otherwise)
	AJ_SerialIOShutdown();

	status = AJ_SerialIOInit(&g_serialConfig);
	if (status == AJ_OK) {
		return AJ_SerialInit(NULL, g_serialConfig.bitrate, AJ_SERIAL_WINDOW_SIZE, AJ_SERIAL_PACKET_SIZE);
	}
	return status;
}

AJ_Status AJ_SerialIOEnable(uint32_t direction, uint8_t enable) {
	if (direction == AJ_SERIO_RX) {
		if (enable) {
			AJ_ResumeRX();
		} else {
			AJ_PauseRX();
		}
	} else if (direction == AJ_SERIO_TX) {
		if (enable) {
			AJ_ResumeTX();
		} else {
			AJ_PauseTX();
		}
	}

	return AJ_OK;
}

void AJ_SetRxCB(AJ_SerIORxCompleteFunc rx_cb) {
	RecvCB = rx_cb;
}

void AJ_SetTxCB(AJ_SerIOTxCompleteFunc tx_cb) {
	SendCB = tx_cb;
}

void AJ_SetTxSerialTransmit(AJ_SerialTxFunc tx_func) {
	g_AJ_TX = tx_func;
}

AJ_Status AJ_SerialIOInit(AJ_SerIOConfig* config) {
	int speed;
	struct termios tios;
	int result;

	InitMutexes();

	result = socketpair(AF_UNIX, SOCK_STREAM, 0, notification_fd);
	if (result < 0) {
		AJ_Printf("[%s] Failed to create notification sockets: %d - %s\n", __func__, errno, strerror(errno));
		return AJ_ERR_FAILURE;
	}

	result = open((const char *)config->config, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (result < 0) {
		AJ_Printf("[%s] Failed to open input (%s): %d (%d - %s)\n", __func__, (const char *)config->config, result, errno, strerror(errno));
		return AJ_ERR_FAILURE;
	}
	serial_fd = result;

	if (strcmp((const char *)config->config, "/dev/ptmx") == 0) {
		char *slaveDevice;

		signal(SIGCHLD, SIG_IGN);
		if (grantpt(serial_fd) != 0) {
			AJ_Printf("[%s] Failed to grant access to slave: %d - %s\n", __func__, errno, strerror(errno));
			return AJ_ERR_FAILURE;
		}
		if (unlockpt(serial_fd)) {
			AJ_Printf("[%s] Failed to unlock terminal: %d - %s\n", __func__, errno, strerror(errno));
			return AJ_ERR_FAILURE;
		}

		slaveDevice = ptsname(serial_fd);
		if (!slaveDevice) {
			AJ_Printf("[%s] Failed to get slave device: %d - %s\n", __func__, errno, strerror(errno));
			return AJ_ERR_FAILURE;
		}
		AJ_Printf("Slave device: %s\n", slaveDevice);
	}

	result = flock(serial_fd, LOCK_EX /*| LOCK_NB*/);
	if (result < 0) {
		AJ_Printf("[%s] Failed to get lock: %d (%d - %s)\n", __func__, result, errno, strerror(errno));
		goto error;
	}

	switch (config->bitrate) {
	case 9600: speed = B9600; break;
	case 19200: speed = B19200; break;
	case 38400: speed = B38400; break;
	case 57600: speed = B57600; break;
	case 115200: speed = B115200; break;
	case 230400: speed = B230400; break;
	case 460800: speed = B460800; break;
	case 500000: speed = B500000; break;
	case 576000: speed = B576000; break;
	case 921600: speed = B921600; break;
	case 1000000: speed = B1000000; break;
	case 1152000: speed = B1152000; break;
	case 1500000: speed = B1500000; break;
	case 2000000: speed = B2000000; break;
	case 2500000: speed = B2500000; break;
	case 3000000: speed = B3000000; break;
	case 3500000: speed = B3500000; break;
	case 4000000: speed = B4000000; break;
	default:
		AJ_Printf("[%s] Unsupported baudrate %d\n", __func__, config->bitrate);
		goto error;
	}

	memset(&tios, 0, sizeof(tios));

	cfsetispeed(&tios, speed);
	cfsetospeed(&tios, speed);

	switch (config->bits) {
	case 5: tios.c_cflag |= CS5; break;
	case 6: tios.c_cflag |= CS6; break;
	case 7: tios.c_cflag |= CS7; break;
	case 8: tios.c_cflag |= CS8; break;
	default:
		AJ_Printf("[%s] Unsupported bit size %d\n", __func__, config->bits);
		goto error;
	}

	switch (config->parity) {
	case 0: break;
	case 1: tios.c_cflag |= PARENB | PARODD; break;
	case 2: tios.c_cflag |= PARENB; break;
	default:
		AJ_Printf("[%s] Unsupported parity %d\n", __func__, config->parity);
		goto error;
	}

	switch (config->stopBits) {
	case 1: break;
	case 2: tios.c_cflag |= CSTOPB; break;
	default:
		AJ_Printf("[%s] Unsupported stop bits %d\n", __func__, config->stopBits);
		goto error;
	}

	tios.c_cflag |= (CLOCAL | CREAD);

	tcflush(serial_fd, TCIOFLUSH);
	tcsetattr(serial_fd, TCSANOW, &tios);

	stopped = 0;
	result = pthread_create(&serial_thread, NULL, serialLoop, NULL);
	if (result < 0) {
		AJ_Printf("[%s] Failed to start thread: %d (%d - %s)\n", __func__, result, errno, strerror(errno));
		stopped = 1;
		goto error;
	}

	return AJ_OK;

error:
	close(serial_fd);
	serial_fd = -1;
	return AJ_ERR_FAILURE;
}

AJ_Status AJ_SerialIOShutdown(void) {
	if (!stopped) {
		stopped = 1;

		if (notification_fd[0] >= 0) {
			char b = 0;
			write(notification_fd[0], &b, 1);
		}

		pthread_join(serial_thread, NULL);
	}

	if (serial_fd >= 0) {
		close(serial_fd);
		serial_fd = -1;
	}

	if (notification_fd[0] >= 0) {
		close(notification_fd[0]);
		notification_fd[0] = -1;
	}
	if (notification_fd[1] >= 0) {
		close(notification_fd[1]);
		notification_fd[1] = -1;
	}

	DestroyMutexes();

	return AJ_OK;
}
