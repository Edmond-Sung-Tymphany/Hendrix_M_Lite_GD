

/* Joplin Model specific config  */
#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#define HENDRIX_Lite

/**************************************************/
/************ Firmware Version Control  *****************/
/**************************************************/

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define SW_VERSION_LENGTH    (10)
#define HW_VERSION_LENGTH    (8)
#define DSP_VERSION_LENGTH   (10)
#define BT_VERSION_LENGTH    (4)


#define SW_MAJOR_VERSION      7
#define SW_MINOR_VERSION1    0
#define SW_MINOR_VERSION2    3
#define SW_MINOR_VERSION3    0
#define PRODUCT_VERSION_MCU       STRINGIFY(SW_MAJOR_VERSION.SW_MINOR_VERSION1.SW_MINOR_VERSION2.SW_MINOR_VERSION3)
#define HAS_ADAU1761_DSP_VERSION


/************ Firmware Version Control  *****************/


/******************************** audio **************************/
#define HAS_TWO_CH_WF
/******************************** audio **************************/
#define DSP_BT_CHANNEL_DETECTION
#define HAS_AUTO_BOOST_CONTROL
#define PRINT_LOGx
#endif

