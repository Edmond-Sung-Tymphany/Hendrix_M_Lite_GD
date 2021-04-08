#ifndef PT_POLK_ALLPLAY_EXTERNAL_H
#define PT_POLK_ALLPLAY_EXTERNAL_H

/* Production test (via byte stream) external definitions specific for Polk Allplay
 *
 */

/* Command 0x02: Read adc (raw) values
 *
 * Product specifc buttons(or request id) here
 */
/*buttons */
#define PT_COMMAND_ADC_BUTTON_POWER     0x01
#define PT_COMMAND_ADC_BUTTON_VOL_DOWN  0x02
#define PT_COMMAND_ADC_BUTTON_VOL_UP    0x03
#define PT_COMMAND_ADC_BUTTON_PREV      0x04
#define PT_COMMAND_ADC_BUTTON_NEXT      0x05
#define PT_COMMAND_ADC_BUTTON_PLAY      0x06
#define PT_COMMAND_ADC_BUTTON_SETUP     0x07
#define PT_COMMAND_ADC_BUTTON_DIRECT    0x08
#define PT_COMMAND_ADC_BUTTON_MAX       0x09
/* battery adc raw value request id */
#define PT_COMMAND_ADC_BATTERY          (PT_COMMAND_ADC_BUTTON_MAX)
/* AC input request id */
#define PT_COMMAND_ADC_AC               (PT_COMMAND_ADC_BUTTON_MAX+1)

/* keep this at the bottom with updated value if add extra adc id req */
#define PT_COMMAND_ADC_MAX              (PT_COMMAND_ADC_BUTTON_MAX+2)
/* Command 0x03: Action MMI (press a button, illuminated an led etc.)
 *
 * Product specifc MMI instructions here
 */
/* Data byte 1 */
#define PT_COMMAND_MMI_BUTTON 0x01
#define PT_COMMAND_MMI_LED    0x02

/* Data byte 2 */
#define PT_COMMAND_MMI_BUTTON_POWER     PT_COMMAND_ADC_BUTTON_POWER
#define PT_COMMAND_MMI_BUTTON_VOL_DOWN  PT_COMMAND_ADC_BUTTON_VOL_DOWN
#define PT_COMMAND_MMI_BUTTON_VOL_UP    PT_COMMAND_ADC_BUTTON_VOL_UP
#define PT_COMMAND_MMI_BUTTON_PREV      PT_COMMAND_ADC_BUTTON_PREV
#define PT_COMMAND_MMI_BUTTON_NEXT      PT_COMMAND_ADC_BUTTON_NEXT
#define PT_COMMAND_MMI_BUTTON_PLAY      PT_COMMAND_ADC_BUTTON_PLAY
#define PT_COMMAND_MMI_BUTTON_SETUP     PT_COMMAND_ADC_BUTTON_SETUP
#define PT_COMMAND_MMI_BUTTON_DIRECT    PT_COMMAND_ADC_BUTTON_DIRECT

#define PT_COMMAND_MMI_LED_RED           0x09
#define PT_COMMAND_MMI_LED_BLUE          0x0A
#define PT_COMMAND_MMI_LED_GREEN         0x0B

/* Data byte 3 */
#define PT_COMMAND_MMI_BUTTON_UP         0x01
#define PT_COMMAND_MMI_BUTTON_DOWN       0x02

#define PT_COMMAND_MMI_LED_OFF           0x03
#define PT_COMMAND_MMI_LED_ON            0x04


/* Command 0x04: Allplay player indication state
 */
#define PT_INDICATION_ALLPLAY_PLAYER_STATE_UNKNOWN         0x01
#define PT_INDICATION_ALLPLAY_PLAYER_STATE_STOPPED         0x02
#define PT_INDICATION_ALLPLAY_PLAYER_STATE_TRANSITIONING   0x03
#define PT_INDICATION_ALLPLAY_PLAYER_STATE_BUFFERING       0x04
#define PT_INDICATION_ALLPLAY_PLAYER_STATE_PLAYING         0x05
#define PT_INDICATION_ALLPLAY_PLAYER_STATE_PAUSED          0x06

/* Command 0x05: Set mode
 */
#define PT_COMMAND_AUX_AUDIOSOURCE_MODE         0x01
#define PT_COMMAND_ALLPLAY_AUDIOSOURCE_MODE     0x02
#define PT_COMMAND_DIRECT_MODE                  0x03
#define PT_COMMAND_WIFI_SETUP_MODE              0x04

/* Command 0x6: dsp
 */

#define PT_COMMAND_DSP_VOL_UP                   0x01
#define PT_COMMAND_DSP_VOL_DOWN                 0x02


/* Fill in the rest of the commands/responses here... */

#endif /* PT_EXTERNAL_H */