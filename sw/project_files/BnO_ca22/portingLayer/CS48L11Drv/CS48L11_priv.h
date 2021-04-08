#ifndef __CS48L11_PRIV_H__
#define __CS48L11_PRIV_H__

// Boot messages (responsed from DSP)
#define DSP_MSG_SOFTBOOT_ACK        0x00000005
#define DSP_MSG_BOOT_READY          0x00000005
#define DSP_MSG_BOOT_START          0x00000001 
#define DSP_MSG_BOOT_SUCCESS        0x00000002 
#define DSP_MSG_APP_START           0x00000004 
#define DSP_MSG_BOOT_ERROR_CHECKSUM 0x000000FF 
#define DSP_MSG_INVALID_BOOT_TYPE   0x000000FE
#define DSP_MSG_BOOT_FAILURE        0x000000F8
#define DSP_MSG_APPLICATION_FAILURE 0xF0000000

// Host-to-DSP boot related commands
#define DSP_CMD_SLAVE_BOOT          0x80000000
#define DSP_CMD_SOFT_RESET          0x40000000
#define DSP_CMD_SOFT_RESET_DSP_A    0x50000000 	// for CS495314
#define DSP_CMD_SOFT_BOOT_WORD1     0x81000009
#define DSP_CMD_SOFT_BOOT_WORD2     0x00000001

// DSP-to-Host boot related messages 
#define DSP_READ_RESPONSE_BOOT_READY        0x00000005 
#define DSP_UNSOLICITED_MSG_AUTODECT_WORD1  0x81000000


/*
 * when write a mount data to DSP, we need to check the BUSY pin
 * hard code to read the BUSY pin, now use PC14
 */
#define IS_CS48L11_BUSY_PIN_LOW     (!(GPIOC->IDR & GPIO_Pin_14))


#endif  // __CS48L11_PRIV_H__
