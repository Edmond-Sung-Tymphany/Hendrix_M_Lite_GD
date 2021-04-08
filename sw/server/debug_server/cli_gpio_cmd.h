#ifndef CLI_GPIO_CMD_H
#define CLI_GPIO_CMD_H



#define    POWER_KEY_PIN                PORTDbits.RD0
#define    NET_RESET_KEY_PIN            PORTBbits.RB7
#define    DIRECT_MODE_KEY_PIN          PORTGbits.RG8
#define    CH_STATUS_PIN                PORTGbits.RG7
/* MCU_SAM_RESET PIC32 RB12 - >SAM J11 Pin 16 (This is the SAM processor reset */
#define    SAM_RESET_PIN                PORTBbits.RB12
#define    SAM_RESET_PIN_LAT            LATBbits.LATB12
/* I2S buffer control F0 & F1 */
#define    OE_CTRL_PIN1                 PORTFbits.RF0
#define    OE_CTRL_PIN2                 PORTFbits.RF1

/* Mute pin */
#define AMP_MUTE_PIN                    PORTFbits.RF4




extern CLI_Command_Definition_t cli_gpio_cmd;

#endif /* CLI_GPIO_CMD_H */