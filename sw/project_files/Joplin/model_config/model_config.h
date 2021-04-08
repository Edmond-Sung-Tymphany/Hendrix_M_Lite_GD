

/* Joplin Model specific config  */
#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#define JOPLIN_L
#define TP_PRODUCT  "Joplin_L"

/**************************************************/
/************ Firmware Version Control  *****************/
/**************************************************/
#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#define PASTE(a, b) a##.b

/// PIU version, i.e. swv5.05
#define PIU_MAJOR_VERSION   SW_MAJOR_VERSION
#define PIU_MINOR_VERSION1  SW_MINOR_VERSION1
#define PIU_MINOR_VERSION2  SW_MINOR_VERSION2
#define PIU_MINOR_VERSION3  SW_MINOR_VERSION3
#define PIU_VERSION         STRINGIFY(PIU_MAJOR_VERSION.PIU_MINOR_VERSION1.PIU_MINOR_VERSION2.PIU_MINOR_VERSION3)

/// UBL Bootloader version, i.e. swv5.05
#define BL_MAJOR_VERSION   SW_MAJOR_VERSION
#define BL_MINOR_VERSION1  SW_MINOR_VERSION1
#define BL_MINOR_VERSION2  SW_MINOR_VERSION2
#define BL_MINOR_VERSION3  SW_MINOR_VERSION3
#define BOOTLOADER_VERSION STRINGIFY(BL_MAJOR_VERSION.BL_MINOR_VERSION1.BL_MINOR_VERSION2.BL_MINOR_VERSION3)

/// SW version, i.e. swv5.0.5
#define SW_MAJOR_VERSION   1
#define SW_MINOR_VERSION1  0
#define SW_MINOR_VERSION2  1
#define SW_MINOR_VERSION3  0
#define PRODUCT_VERSION_MCU STRINGIFY(SW_MAJOR_VERSION.SW_MINOR_VERSION1.SW_MINOR_VERSION2.SW_MINOR_VERSION3)

#define PRODUCT_VERSION_DSP     "1.02"

#define SOFTWARE_VERSION_STRING TP_PRODUCT PRODUCT_VERSION_MCU
#define HARDWARE_VERSION_STRING "Joplin L ES"  // should be one of string from hwVerMap[] in powerdrv_light.c

/************ Firmware Version Control  *****************/


/******************************** audio **************************/
#define HAS_RCA_IN

/******************************** audio **************************/


#endif

