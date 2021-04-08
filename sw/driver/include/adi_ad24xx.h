#ifndef ADI_AD2410_H
#define ADI_AD2410_H


/*============= D E F I N E S =============*/

#define A2B_AUTHENTICATION_FAILURE          (0xFF00u)

#define ADI_A2B_MAX_MASTER_NODES        (1)/* only 4 supported now */

/*! Abstracted timer identification number used for A2B transceiver related events (FIXED to 1) */
#define A2B_TIMER_NO                                        (1u) 
/*! Node discovery time-out period in micro-seconds */
#define A2B_NODE_DISCOVERY_TIMEOUT                          (50000u)
/*! System clock */
#define A2B_SCLK                                            (49152000u*2u)
/*! Abstracted TWI peripheral identification number used for A2B connection */
#define A2B_TWI_NO                                          (0u)
/*! A2B IRQ pin number  */ 
#define A2B_IRQ_PIN                                         (0u)
/*! Abstracted SPI peripheral identification number */
#define A2B_SPI_NO                                          (0u)
/*! Maximum number of errors stored in the framework */
#define A2B_MAX_NUM_ERROR                                   (10u) 
#define ADI_A2B_MAX_SLAVE_NODES_PER_MASTER			        (12)/* only 15 supported now */

#define ADI_A2B_MAX_GRAPH_NODES						        (ADI_A2B_MAX_SLAVE_NODES_PER_MASTER+1)      // number of slave + one master

#define A2B_ENABLE_AD242X_SUPPORT					(0x01)

#define A2B_ENABLE_AD241X_SUPPORT					(0x01)

/*! Total number of nodes per branch: 1-master + 15-slave nodes */
#define A2B_MAX_NUMBER_OF_NODES_PER_BRANCH                  (ADI_A2B_MAX_SLAVE_NODES_PER_MASTER + 1)
/*! Error count read interval- used for BERT calculation */
#define A2B_BERT_INTERVAL                                   (5000000)
/*! Maximum number of AD2410 registers configured in a function */
#define A2B_MAX_NUM_REG                                     (100)
/*! Communication buffer size in words  */
#define A2B_MAX_MSG_TXBUFFERSIZE                            (250u)
/*! Communication buffer size in words  */
#define A2B_MAX_MSG_RXBUFFERSIZE                            (1000u)
/*! EEPROM TWI device address */
#define A2B_EEPROM_I2C                                      (0x50u)
/*! A2B software version number */
#define A2B_VERSION_NUMBER                                  (0x120000u)
/*! Starting address for EEPROM Schematic save  */
#define A2B_SCHEMATIC_SAVE_EEPROM_START_ADDR                (0xFEu)
/*! 16 bit Header value used in  EEPROM schematic Save option */
#define A2B_SCHEMATIC_DATA_HEADER                           (0xADABu)   
/*! First byte of Header value used in EEPROM Schematic Save option */
#define A2B_SCHEMATIC_DATA_HEADER1                          (0xADu)
/*! Second byte of Header value used in EEPROM Schematic Save option */
#define A2B_SCHEMATIC_DATA_HEADER2                          (0xABu)
/*! Post discovery timer interval */
#define A2B_POST_DISC_TIME_INTERVAL                         (5000000u)

/*============================================== Configurable MACROs ===============================================*/
/*! SigmaStudio interactive enable macro - Reset this macro to zero for stand-alone mode  */
#define A2B_SS_ENABLE                                       (0u)
/*! Enable silicon work around for rev1.0 */
#define REV_1_0_WORKAROUND                                  (1u)
/*! Debug macro - used only for printing configuration status */ 
//#define A2B_PRINT_FOR_DEBUG                                 (1u)
/*! Discovery Timeout Enable  */
#define A2B_DISCOVERY_TIMEOUT                               (1u) 
/*! Enable/disable Schematic update from EEPROM  */
#define A2B_UPDATE_FROM_EEPROM                              (0u)
/*! Enable/disable line diagnostics during discovery  */
#define A2B_LINE_DIAGNOSTIC_ENABLE                          (1u)  
/*! Option to select discovery and diagnostics 
    1 - diagnostic and discovery by polling registers
    0-  diagnostic and discovery by Interrupt service  */
#define A2B_DISCOVERY_DIAGNOSTICS_BY_POLLING                (1u)
/*!  One level branching is future use case */
//#define A2B_BRANCH_SUPPORT                                  (1u)
/*! Concealed Fault diagnostic method */
#define A2B_LINE_DIAGNOSTICS_RESTART_FROM_MASTER			(1u)
/*! Auto discovery upon critical fault during discovery */
#define A2B_AUTO_DISC_ON_CRITICAL_FAULT						(1u)
/*============================================== Debug MACROs ===============================================*/
/*! Enable I2C word store  */
#define A2B_DEBUG_STORE_CONFIG                               (0u)
/*! Enable cycle measurement */
#define DO_CYCLE_COUNTS
/*! Enable BERT debug from target  */
#define A2B_RUN_BIT_ERROR_TEST                     			 (0u)
/*! Number of TWI words, increase according to network size */
#define A2B_TWI_DEBUG_NUM                                    (500u)


#define ADI_A2B_MAX_DEVICES_PER_NODE				(1)

#ifndef A2B_USE_EXPORTED_GRAPH 
#define A2B_USE_BUS_DESCRIPTION_FILE 
#endif

#define A2B_ENABLE_AD242X_BCFSUPPORT			(0x01) 
#define A2B_ENABLE_AD241X_BCFSUPPORT			(0x01) 
/* I2S Settings  */
#define A2B_ALTERNATE_SYNC							(0x20u)        /*  Drive SYNC Pin Alternating */
#define A2B_PULSE_SYNC								(0x00u)        /*  Drive SYNC Pin for 1 Cycle */

#define A2B_16BIT_TDM								(0x10u)        /*  16-Bit */
#define A2B_32BIT_TDM								(0x00u)        /*  32-Bit */

#define A2B_TDM2									(0x00u)        /*  TDM2 */
#define A2B_TDM4									(0x01u)        /*  TDM4 */
#define A2B_TDM8									(0x02u)        /*  TDM8 */
#define A2B_TDM12									(0x03u)        /*  No slave node support */
#define A2B_TDM16									(0x04u)        /*  TDM16 */
#define A2B_TDM20									(0x05u)        /*  No slave node support */
#define A2B_TDM24									(0x06u)        /*  No slave node support */
#define A2B_TDM32									(0x07u)        /*  TDM32 */

#define A2B_SAMPLERATE_192kHz						(0x06u)        /*  SFF x 4 */
#define A2B_SAMPLERATE_96kHz						(0x05u)        /*  SFF x 2 */
#define A2B_SAMPLERATE_48kHz						(0x00u)        /*  1x SFF */
#define A2B_SAMPLERATE_24kHz						(0x01u)        /*  0.5 x SFF */
#define A2B_SAMPLERATE_12kHz						(0x02u)        /*  0.25x SFF */
#define A2B_SAMPLERATE_RRDIV						(0x03u)        /*  1x SFF */

#define A2B_BCLKRATE_I2SGFG							(0x00)        /*  SFF x 4 */
#define A2B_BCLKRATE_SYNCx2048						(0x01)        /*  SFF x 2 */
#define A2B_BCLKRATE_SYNCx4096						(0x02)        /*  1x SFF */
#define A2B_BCLKRATE_SFFx64							(0x04)        /*  0.5 x SFF */
#define A2B_BCLKRATE_SFFx128						(0x05)        /*  0.25x SFF */
#define A2B_BCLKRATE_SFFx256						(0x06)        /*  1x SFF */

/* Clock rate definition  */
#define A2B_CODEC_CLK_12_288MHz						(0x00u)
#define A2B_CODEC_CLK_24_512MHz						(0x01u)


/* PDM Settings */
#define A2B_0_93									(0xC0u)        /*  0.93 Hz */
#define A2B_3_73									(0x80u)        /*  3.73 Hz */
#define A2B_29_8									(0x20u)        /*  29.8 Hz */
#define A2B_7_46									(0x60u)        /*  7.46 Hz */
#define A2B_59_9									(0x00u)        /*  59.9 Hz */
#define A2B_14_9									(0x40u)        /*  14.9 Hz */
#define A2B_1_86									(0xA0u)        /*  1.86 Hz */

#define A2B_PDM1SLOTS_2								(0x08u)        /*  2 Slots */
#define A2B_PDM1SLOTS_1								(0x00u)        /*  1 Slot */

#define A2B_PDM0SLOTS_2								(0x02u)        /*  2 Slots */
#define A2B_PDM0SLOTS_1								(0x00u)        /*  1 Slot */


#define A2B_PDM_RATE_SFF							(0x00u)        /*  SFF  */ 
#define A2B_PDM_RATE_SFF_DIV_2							(0x01u)        /*  SFF/2  */
#define A2B_PDM_RATE_SFF_DIV_4							(0x02u)        /*  SFF/4  */

/* A2b Slot formats */
#define A2B_UPSLOT_SIZE_8							(0x00u)	              /*  8 Bits */
#define A2B_UPSLOT_SIZE_12							(0x10u)	            /*  12 Bits */
#define A2B_UPSLOT_SIZE_16							(0x20u)	            /*  16 Bits */
#define A2B_UPSLOT_SIZE_20							(0x30u)	            /*  20 Bits */
#define A2B_UPSLOT_SIZE_28							(0x50u)	            /*  28 Bits */
#define A2B_UPSLOT_SIZE_24							(0x40u)	            /*  24 Bits */
#define A2B_UPSLOT_SIZE_32							(0x60u)	            /*  32 Bits */

#define A2B_DNSLOT_SIZE_8							(0x00u)	            /*  8 Bits */
#define A2B_DNSLOT_SIZE_12							(0x01u)	            /*  12 Bits */
#define A2B_DNSLOT_SIZE_16							(0x02u)	            /*  16 Bits */
#define A2B_DNSLOT_SIZE_20							(0x03u)	            /*  20 Bits */
#define A2B_DNSLOT_SIZE_24							(0x04u)	            /*  24 Bits */
#define A2B_DNSLOT_SIZE_28							(0x05u)	            /*  28 Bits */
#define A2B_DNSLOT_SIZE_32							(0x06u)	            /*  32 Bits */


/* Slot Enhance for AD242x  only  */
#define SLOT_0_DISABLED							(0x0)
#define SLOT_1_DISABLED							(0x0)
#define SLOT_2_DISABLED							(0x0)
#define SLOT_3_DISABLED							(0x0)
#define SLOT_4_DISABLED							(0x0)
#define SLOT_5_DISABLED							(0x0)
#define SLOT_6_DISABLED							(0x0)
#define SLOT_7_DISABLED							(0x0)
#define SLOT_8_DISABLED							(0x0)
#define SLOT_9_DISABLED							(0x0)
#define SLOT_10_DISABLED							(0x0)
#define SLOT_11_DISABLED							(0x0)
#define SLOT_12_DISABLED							(0x0)
#define SLOT_13_DISABLED							(0x0)
#define SLOT_14_DISABLED							(0x0)
#define SLOT_15_DISABLED							(0x0)
#define SLOT_16_DISABLED							(0x0)
#define SLOT_17_DISABLED							(0x0)
#define SLOT_18_DISABLED							(0x0)
#define SLOT_19_DISABLED							(0x0)
#define SLOT_20_DISABLED							(0x0)
#define SLOT_21_DISABLED							(0x0)
#define SLOT_22_DISABLED							(0x0)
#define SLOT_23_DISABLED							(0x0)
#define SLOT_24_DISABLED							(0x0)
#define SLOT_25_DISABLED							(0x0)
#define SLOT_26_DISABLED							(0x0)
#define SLOT_27_DISABLED							(0x0)
#define SLOT_28_DISABLED							(0x0)
#define SLOT_29_DISABLED							(0x0)
#define SLOT_30_DISABLED							(0x0)
#define SLOT_31_DISABLED							(0x0)
#define SLOT_0_ENABLED							(0x1)
#define SLOT_1_ENABLED							(0x1)
#define SLOT_2_ENABLED							(0x1)
#define SLOT_3_ENABLED							(0x1)
#define SLOT_4_ENABLED							(0x1)
#define SLOT_5_ENABLED							(0x1)
#define SLOT_6_ENABLED							(0x1)
#define SLOT_7_ENABLED							(0x1)
#define SLOT_8_ENABLED							(0x1)
#define SLOT_9_ENABLED							(0x1)
#define SLOT_10_ENABLED							(0x1)
#define SLOT_11_ENABLED							(0x1)
#define SLOT_12_ENABLED							(0x1)
#define SLOT_13_ENABLED							(0x1)
#define SLOT_14_ENABLED							(0x1)
#define SLOT_15_ENABLED							(0x1)
#define SLOT_16_ENABLED							(0x1)
#define SLOT_17_ENABLED							(0x1)
#define SLOT_18_ENABLED							(0x1)
#define SLOT_19_ENABLED							(0x1)
#define SLOT_20_ENABLED							(0x1)
#define SLOT_21_ENABLED							(0x1)
#define SLOT_22_ENABLED							(0x1)
#define SLOT_23_ENABLED							(0x1)
#define SLOT_24_ENABLED							(0x1)
#define SLOT_25_ENABLED							(0x1)
#define SLOT_26_ENABLED							(0x1)
#define SLOT_27_ENABLED							(0x1)
#define SLOT_28_ENABLED							(0x1)
#define SLOT_29_ENABLED							(0x1)
#define SLOT_30_ENABLED							(0x1)
#define SLOT_31_ENABLED							(0x1)
/* GPIO Mux Options  */
#define A2B_GPIO_0_INPUT							(0x00u)
#define A2B_GPIO_0_OUTPUT							(0x01u)
#define A2B_GPIO_0_DISABLE							(0x02u)

#define A2B_GPIO_1_INPUT							(0x00u)
#define A2B_GPIO_1_OUTPUT							(0x01u)
#define A2B_GPIO_1_DISABLE							(0x02u)
#define A2B_GPIO_1_AS_CLKOUT						(0x03u)

#define A2B_GPIO_2_INPUT							(0x00u)
#define A2B_GPIO_2_OUTPUT							(0x01u)
#define A2B_GPIO_2_DISABLE							(0x02u)
#define A2B_GPIO_2_AS_CLKOUT						(0x03u)

#define A2B_GPIO_3_INPUT							(0x00u)
#define A2B_GPIO_3_OUTPUT							(0x01u)
#define A2B_GPIO_3_DISABLE							(0x02u)
#define A2B_GPIO_3_AS_DTX0							(0x03u)

#define A2B_GPIO_4_INPUT							(0x00u)
#define A2B_GPIO_4_OUTPUT							(0x01u)
#define A2B_GPIO_4_DISABLE							(0x02u)
#define A2B_GPIO_4_AS_DTX1							(0x03u)

#define A2B_GPIO_5_INPUT							(0x00u)
#define A2B_GPIO_5_OUTPUT							(0x01u)
#define A2B_GPIO_5_DISABLE							(0x02u)
#define A2B_GPIO_5_AS_DRX0							(0x03u)
#define A2B_GPIO_5_AS_PDM0							(0x04u)

#define A2B_GPIO_6_INPUT							(0x00u)
#define A2B_GPIO_6_OUTPUT							(0x01u)
#define A2B_GPIO_6_DISABLE							(0x02u)
#define A2B_GPIO_6_AS_DRX1							(0x03u)
#define A2B_GPIO_6_AS_PDM1							(0x04u)


#define A2B_GPIO_7_INPUT							(0x00u)


#define A2B_GPIO_7_OUTPUT							(0x01u)


#define A2B_GPIO_7_DISABLE							(0x02u)


/* GPIOD settings for AD242x only */
#define A2B_MASK_BUSFLAG_0							(0x0)
#define A2B_MASK_BUSFLAG_1							(0x0)
#define A2B_MASK_BUSFLAG_2							(0x0)
#define A2B_MASK_BUSFLAG_3							(0x0)
#define A2B_MASK_BUSFLAG_4							(0x0)
#define A2B_MASK_BUSFLAG_5							(0x0)
#define A2B_MASK_BUSFLAG_6							(0x0)
#define A2B_MASK_BUSFLAG_7							(0x0)


#define A2B_MAP_BUSFLAG_0							(0x1)
#define A2B_MAP_BUSFLAG_1							(0x1)
#define A2B_MAP_BUSFLAG_2							(0x1)
#define A2B_MAP_BUSFLAG_3							(0x1)
#define A2B_MAP_BUSFLAG_4							(0x1)
#define A2B_MAP_BUSFLAG_5							(0x1)
#define A2B_MAP_BUSFLAG_6							(0x1)
#define A2B_MAP_BUSFLAG_7							(0x1)


/* Clkout For AD242x only */
#define A2B_CLKOUT_DIV_2								(0u)
#define A2B_CLKOUT_DIV_4								(1u)
#define A2B_CLKOUT_DIV_6								(2u)
#define A2B_CLKOUT_DIV_8								(3u)
#define A2B_CLKOUT_DIV_10								(4u)
#define A2B_CLKOUT_DIV_12								(5u)
#define A2B_CLKOUT_DIV_14								(6u)
#define A2B_CLKOUT_DIV_16								(7u)
#define A2B_CLKOUT_DIV_18								(8u)
#define A2B_CLKOUT_DIV_20								(9u)
#define A2B_CLKOUT_DIV_22								(10u)
#define A2B_CLKOUT_DIV_24								(11u)
#define A2B_CLKOUT_DIV_26								(12u)
#define A2B_CLKOUT_DIV_28								(13u)
#define A2B_CLKOUT_DIV_30								(14u)
#define A2B_CLKOUT_DIV_32								(15u)
#define A2B_CLKOUT_PREDIV_02							(0x00)
#define A2B_CLKOUT_PREDIV_32							(0x01)

/* I2C rate settings */
#define A2B_I2C_100kHz								(0x00u)
#define A2B_I2C_400kHz								(0x01u)

#define A2B_SFF_RATE_44_1kHz						(0x04u)
#define A2B_SFF_RATE_48_0kHz						(0x00u)


/* PLL Settings */
#define A2B_PLL_SYNC								(0x00u)
#define A2B_PLL_BCLK								(0x04u)

#define A2B_PLL_BCLK_12_288MHz						(0x00u)            /*  1 x  (BCLK=12.288 MHz in TDM8) */
#define A2B_PLL_BCLK_24_576MHZ						(0x01u)            /*  2  x (BCLK=24.576 MHz in TDM16) */
#define A2B_PLL_BCLK_49_152MHZ						(0x02u)            /*  4  x  (BCLK=49.152 MHz for TDM32) */


/* Different discovery types */
#define A2B_SIMPLE_DISCOVERY						(0u)
#define A2B_MODIFIED_DISCOVERY						(1u)
#define A2B_OPTIMIZED_DISCOVERY						(2u)
#define A2B_ADVANCED_DISCOVERY						(3u)

/* Diffrent types of peripheral device  */
#define A2B_AUDIO_SOURCE							(0u)
#define A2B_AUDIO_SINK								(1u)
#define A2B_AUDIO_UNKNOWN							(2u)
#define A2B_AUDIO_SOURCE_SINK						(3u)
#define A2B_AUDIO_HOST								(4u)
#define A2B_GENERIC_I2C_DEVICE						(5u)

/* I2S operational codes */
#define A2B_WRITE_OP								(0u)
#define A2B_READ_OP									(1u)
#define A2B_DEALY_OP								(2u)

#define ENABLED										(1u)
#define DISABLED									(0u)

#define A2B_HIGH									(1u)
#define A2B_LOW										(0u)

#define RAISING_EDGE								(0u)
#define FALLING_EDGE								(1u)

#define ADI_A2B_GRAPH_DATA_LENGTH					(3193)

#define MAX_NUMBER_TWI_BYTES            (300)

#define NULL_PTR        NULL

/*============= D A T A T Y P E S=============*/
/*! \enum ADI_A2B_RESULT
    Execution status
 */
typedef enum
{
    /*!  Success */
    ADI_A2B_SUCCESS,
    /*!  Failure */
    ADI_A2B_FAILURE,
    /*!  Undefined */
    ADI_A2B_UNDEFINED
}ADI_A2B_RESULT;

typedef enum
{
    ADI_TWI_SUCCESS,
    ADI_TWI_FAILURE,
    ADI_TWI_UNDEFINED
}ADI_TWI_RESULT;


/*! \enum ADI_A2B_NODE_TYPE
    Possible A2B node types
  */
typedef enum
{
 /*!  Enum for Master node  */
  ADI_A2B_MASTER,
 /*!  Enum for Slave node  */
  ADI_A2B_SLAVE,
 /*!  Enum for Invalid node  */
  ADI_A2B_INVALID

}ADI_A2B_NODE_TYPE;


/*! \enum ADI_A2B_PARTNUM 
    Possible A2B part numbers
 */
typedef enum
{
 /*!  Enum forAD2410  */
    ADI_A2B_AD2410,
  /*!  Enum forAD2401  */ 
    ADI_A2B_AD2401,
/*!  Enum forAD2402 */
    ADI_A2B_AD2402 ,
 /*!  Enum for AD2403 */
    ADI_A2B_AD2403,
 /*!  Enum for AD2425 */
    ADI_A2B_AD2425,/*!  Enum forAD2421 */
    ADI_A2B_AD2421 ,
 /*!  Enum for AD2422 */
    ADI_A2B_AD2422

}ADI_A2B_PARTNUM;



/*! \enum ADI_A2B_DEVICE_TYPE
    Possible A2B node types
 */
typedef enum
{
  /*!  Enum for audio source device  */
    ADI_A2B_AUDIO_SOURCE,
  /*!  Enum for audio sink device  */
    ADI_A2B_AUDIO_SINK,
/*!  Enum for unknown audio device type */
    ADI_A2B_AUDIO_UNKNOWN ,
 /*!  Enum for Source and sink */
    ADI_A2B_AUDIO_SOURCE_SINK , 
/*!  Enum for audio host */
    ADI_A2B_AUDIO_HOST ,
 /*!  Enum for Generic I2C device */
    ADI_A2B_GENERIC_I2C_DEVICE , 

}ADI_A2B_DEVICE_TYPE;


/*! \struct ADI_A2B_ID_REGISTERS
    A2B transceiver Identification registers
 */
typedef struct
{
    uint8 nREG_A2B0_VENDOR;
    uint8 nREG_A2B0_PRODUCT;
    uint8 nREG_A2B0_VERSION;
    uint8 nREG_A2B0_CAPABILITY;
}ADI_A2B_ID_REGISTERS;


/*! \struct ADI_A2B_CTL_REGISTERS
    A2B transceiver Config and Control registers
 */
typedef struct
{
    uint8 nREG_A2B0_NODEADR;
    uint8 nREG_A2B0_SWCTL;
    uint8 nREG_A2B0_BCDNSLOTS;
    uint8 nREG_A2B0_LDNSLOTS;
    uint8 nREG_A2B0_LUPSLOTS;
    uint8 nREG_A2B0_DNSLOTS;
    uint8 nREG_A2B0_UPSLOTS;
    uint8 nREG_A2B0_RESPCYCS;
    uint8 nREG_A2B0_SLOTFMT;
    uint8 nREG_A2B0_DATCTL;
    uint8 nREG_A2B0_CONTROL;
    uint8 nREG_A2B0_DISCVRY;
    #if A2B_ENABLE_AD242X_SUPPORT
    /* Only for AD242X */ 
    uint8 nREG_AD242X0_UPMASK0;   
    uint8 nREG_AD242X0_UPMASK1;   
    uint8 nREG_AD242X0_UPMASK2;   
    uint8 nREG_AD242X0_UPMASK3;   
    uint8 nREG_AD242X0_UPOFFSET;  
    uint8 nREG_AD242X0_DNMASK0;   
    uint8 nREG_AD242X0_DNMASK1;   
    uint8 nREG_AD242X0_DNMASK2;   
    uint8 nREG_AD242X0_DNMASK3;   
    uint8 nREG_AD242X0_DNOFFSET;  
    uint8 nREG_AD242X0_BMMCFG;  

    #endif
}ADI_A2B_CTL_REGISTERS;

/*! \struct ADI_A2B_INT_REGISTERS    
A2B transceiver Interrupt and error registers
 */
typedef struct
{
    uint8 nREG_A2B0_INTSTAT;
    uint8 nREG_A2B0_INTSRC;
    uint8 nREG_A2B0_INTTYPE;
    uint8 nREG_A2B0_INTPND0;
    uint8 nREG_A2B0_INTPND1;
    uint8 nREG_A2B0_INTPND2;
    uint8 nREG_A2B0_INTMSK0;
    uint8 nREG_A2B0_INTMSK1;
    uint8 nREG_A2B0_INTMSK2;
    uint8 nREG_A2B0_BECCTL;
    uint8 nREG_A2B0_BECNT;
    uint8 nREG_A2B0_RAISE;  /* Added for 1.0 silicon */
    uint8 nREG_A2B0_GENERR; /* Added for 1.0 silicon */
#if A2B_ENABLE_AD242X_SUPPORT
    uint8 nREG_AD242X0_LINTTYPE;  /* Added for AD242x */ 
#endif
}ADI_A2B_INT_REGISTERS;


/*! \struct ADI_A2B_PRBS_REGISTERS
    A2B transceiver Pseudo random bitstream generation registers
 */
typedef struct
{
    uint8 nREG_A2B0_TESTMODE;
    uint8 nREG_A2B0_ERRCNT0;
    uint8 nREG_A2B0_ERRCNT1;
    uint8 nREG_A2B0_ERRCNT2;
    uint8 nREG_A2B0_ERRCNT3;
}ADI_A2B_PRBS_REGISTERS;


 /*! \struct ADI_A2B_STAT_REGISTERS
    A2B transceiver Status registers
 */
typedef struct
{
    uint8 nREG_A2B0_SWSTAT;
    uint8 nREG_A2B0_NODE;
    uint8 nREG_A2B0_DISCSTAT;

}ADI_A2B_STAT_REGISTERS;


/*! \struct ADI_A2B_GPIOD_REGISTERS
    AD242x Mail box register
*/
typedef struct
{
     uint8 nREG_AD242X0_GPIODEN;   
     uint8 nREG_AD242X0_GPIOD0MSK; 
     uint8 nREG_AD242X0_GPIOD1MSK; 
     uint8 nREG_AD242X0_GPIOD2MSK; 
     uint8 nREG_AD242X0_GPIOD3MSK; 
     uint8 nREG_AD242X0_GPIOD4MSK; 
     uint8 nREG_AD242X0_GPIOD5MSK; 
     uint8 nREG_AD242X0_GPIOD6MSK; 
     uint8 nREG_AD242X0_GPIOD7MSK; 
     uint8 nREG_AD242X0_GPIODDAT;  
     uint8 nREG_AD242X0_GPIODINV;  

}ADI_A2B_GPIOD_REGISTERS;


/*! \struct ADI_A2B_MAILBOX_REGISTERS
    AD242x Mail box register
*/
typedef struct
{
    uint8 nREG_AD242X0_MBOX0CTL;  
    uint8 nREG_AD242X0_MBOX0STAT; 
    uint8 nREG_AD242X0_MBOX0B0;   
    uint8 nREG_AD242X0_MBOX0B1;   
    uint8 nREG_AD242X0_MBOX0B2;   
    uint8 nREG_AD242X0_MBOX0B3;   
    uint8 nREG_AD242X0_MBOX1CTL;  
    uint8 nREG_AD242X0_MBOX1STAT; 
    uint8 nREG_AD242X0_MBOX1B0;   
    uint8 nREG_AD242X0_MBOX1B1;   
    uint8 nREG_AD242X0_MBOX1B2;   
    uint8 nREG_AD242X0_MBOX1B3;   

}ADI_A2B_MAILBOX_REGISTERS;



/*! \struct ADI_A2B_I2CI2S_REGISTERS
    A2B transceiver I2C, I2S and PDM Config and Control registers
*/
typedef struct
{
    uint8 nREG_A2B0_CHIP;
    uint8 nREG_A2B0_I2CCFG;
    uint8 nREG_A2B0_PLLCTL;
    uint8 nREG_A2B0_I2SGCFG;
    uint8 nREG_A2B0_I2SCFG;
    uint8 nREG_A2B0_I2SRATE;
    uint8 nREG_A2B0_I2STXOFFSET;
    uint8 nREG_A2B0_I2SRXOFFSET;
    uint8 nREG_A2B0_SYNCOFFSET;
    uint8 nREG_A2B0_PDMCTL;
    uint8 nREG_A2B0_ERRMGMT;
    uint8 nREG_A2B0_I2STEST;    /* Added for 1.0 silicon */
#if A2B_ENABLE_AD242X_SUPPORT
    uint8 nREG_AD242X0_I2SRRATE;                      
    uint8 nREG_AD242X0_I2SRRCTL;                      
    uint8 nREG_AD242X0_I2SRRSOFFS;                    
    uint8 nREG_AD242X0_SUSCFG;  /* Added for AD242x */
#endif

}ADI_A2B_I2CI2S_REGISTERS;


/*! \struct ADI_A2B_PINIO_REGISTERS
    A2B transceiver PINIO registers
*/
typedef struct
{
    uint8 nREG_A2B0_CLKCFG;
    uint8 nREG_A2B0_GPIODAT;
    uint8 nREG_A2B0_GPIODATSET;
    uint8 nREG_A2B0_GPIODATCLR;
    uint8 nREG_A2B0_GPIOOEN;
    uint8 nREG_A2B0_GPIOIEN;
    uint8 nREG_A2B0_GPIOIN;
    uint8 nREG_A2B0_PINTEN;
    uint8 nREG_A2B0_PINTINV;
    uint8 nREG_A2B0_PINCFG;
#if A2B_ENABLE_AD242X_SUPPORT
    uint8 nREG_AD242X0_CLK1CFG;  /* Added for AD242x */ 
    uint8 nREG_AD242X0_CLK2CFG;
#endif
}ADI_A2B_PINIO_REGISTERS;


/*! \struct ADI_A2B_NODE_PROPERTIES
    A2B transceiver node properties
*/
typedef struct
{
    uint8 nMasterI2CAddress;
    uint8 nBusI2CAddress;
    ADI_A2B_ID_REGISTERS        nIDRegister;        /*!  A2B ID registers */
    ADI_A2B_CTL_REGISTERS       nCTLRegister;       /*!  A2B Control registers */
    ADI_A2B_STAT_REGISTERS      nSTATRegister;       /*!  A2B Status registers */
    ADI_A2B_INT_REGISTERS       nINTRegister;       /*!  A2B Interrupt registers */
    ADI_A2B_PRBS_REGISTERS      nPRBSRegister;      /*!  A2B PRBS registers */
    ADI_A2B_I2CI2S_REGISTERS    nI2CI2SRegister;        /*!  A2B I2C and I2S config registers */
    ADI_A2B_PINIO_REGISTERS     nPINIORegister;         /*!  A2B Pin in/out registers */
#if A2B_ENABLE_AD242X_SUPPORT
    /*!  242x GPIO over distance registers  */
    ADI_A2B_GPIOD_REGISTERS nGPIODRegister;    /*!  242x Mail box registers  */
    ADI_A2B_MAILBOX_REGISTERS	nMailBoxRegister;
#endif
}ADI_A2B_NODE_PROPERTIES;

/*! \struct ADI_A2B_CONNECTED_DEVICE
    A2B node structure
*/
typedef struct
{
    uint8 nI2CAddress;         /*! I2C address of the connected device */
    uint8  eDeviceID;          /*! Audio source/sink devices ID */
    uint8  eDeviceType;        /*! Audio source/sink devices type */
    uint8 bActive;         /*! Device active status */
}ADI_A2B_CONNECTED_DEVICE;


/*! \struct ADI_A2B_NODE
    A2B node structure
*/
typedef struct
{
    uint8   nID;           
    uint8   nNumSlave;             /*!  Number of slave nodes connected */
    ADI_A2B_NODE_TYPE   eType;             /*!  A2B node type */
    ADI_A2B_PARTNUM   ePartNum;          /*! Transceiver part number */ 
    ADI_A2B_NODE_PROPERTIES  oProperties;       /*!  A2B node properties */
    uint8   bActive;               /*!  A2B node active status */
    uint8   nA2BSrcNodeID;         /*!  A2B connection source node ID */
    uint8   bEnableAutoConfig;     /*!  Auto-configure node enabled */
    ADI_A2B_CONNECTED_DEVICE oConnectedTo[ADI_A2B_MAX_DEVICES_PER_NODE];        /*!  I2C devices connected to the node */
}ADI_A2B_NODE;

/*! \struct ADI_A2B_CONFIG_TABLE
    Structure to store config words
*/

typedef struct 
{
    uint8 nAddress:8;         /*! Register address */
    uint8 nValue:8;           /*! Value  */
}ADI_A2B_CONFIG_TABLE;

/*! \struct A2B_NODE_AUTHENTICATION
Node authentication settings (common for master & slave )
*/
typedef struct
{
	uint8  nVendorID;           /*! AD2410 manufacturer ID */
	uint8  nVersionID;          /*! AD2410 Silicon version */
	uint8  nProductID;          /*! AD2410 product ID */
	uint8  nCapability;         /*! AD2410 capability */
}A2B_NODE_AUTHENTICATION;

/*! \struct A2B_CONFIG_CONTROL
Basic control settings for slave node
*/
typedef struct
{
	uint8  nI2CFrequency;               /*! I2C interface frequency */
	uint8  nRespCycle;                  /*! Response cycles  */
	uint8 nSuperFrameRate;              /*! Expected super/audio frame rate */
	uint8 nBroadCastSlots;              /*! Number of broadcast slots */
	uint8 nLocalDwnSlotsConsume ;       /*! Local down slots */
	uint8 nLocalUpSlotsContribute ;     /*! Local Up slots */
	uint8 nPassUpSlots ;                /*! Pass up slots */
	uint8 nPassDwnSlots ;               /*! Pass down slots */
	uint8 bUseDwnslotConsumeMasks ;     /*! Enable Down slot consume through mask */
	uint8 nSlotsforDwnstrmContribute ;      /* Number of slots for contribution */
	uint8 nLocalUpSlotsConsume ;            /*! Number of Upslots consumed  */
	uint8 nOffsetDwnstrmContribute ;        /* Offset from the RX Buffer for downstream contribution */
	uint8 nOffsetUpstrmContribute ;         /* Offset from the RX Buffer for Upstream contribution */
	uint8 anDwnstreamConsumeSlots[32] ;     /*! Downstream slots consume mask */
	uint8 anUpstreamConsumeSlots[32] ;      /*! Upstream slots consume mask */
}A2B_SLAVE_CONFIG_CONTROL;


/*! \struct A2B_MASTER_CONFIG_CONTROL
Basic control settings for master
*/
typedef struct 
{
	uint8  bI2CEarlyAck;            /*! Early acknowledge for I2C read/write */
	uint8  nRespCycle;          /*! Response cycles  */
	uint8 nPLLTimeBase;     /*! PLL time base -SYNC or BCLK */
	uint8 nBCLKRate;        /*! BCLK rate - 12.288MHz; 24.576MHz and 49.152MHz  */
	uint8 nPassUpSlots ;        /*! Pass up slots */
	uint8 nPassDwnSlots ;       /*! Upstream slots consume mask */
}A2B_MASTER_CONFIG_CONTROL;

/*! \struct A2B_SLAVE_I2S_RATE_CONFIG
I2S rate settings for AD24xx
*/
typedef struct 
{
	uint8  bRRValidBitLSB;      /*! Enable RR valid bit in LSB read/write */
	uint8  bRRValidBitExtraBit;         /*! Enable Valid RR bit in Extra bit  */
	uint8 bRRValidBitExtraCh;           /*! Enable Valid RR bit in Extra Channel */
	uint8 bRRStrobe;                    /*! Enable Reduced rate strobe in ADR1/IO1  */
	uint8 bRRStrobeDirection;           /*! Strobe direction High or Low */
}A2B_MASTER_I2S_RATE_CONFIG;


	/*! \struct A2B_MASTER_I2S_SETTINGS
	I2S interface Settings for master AD2410
	*/
	 typedef struct
	{
		/*! TDM mode  */
		uint8 nTDMMode;
		/*! TDM channel size  */
		uint8 nTDMChSize;
		/*! SYNC mode - Pulse/50% duty cycle   */
		uint8 nSyncMode;
		/*! SYNC Polarity- Rising/Falling edge */
		uint8 nSyncPolarity;
		/*! Early frame sync status */
		uint8 bEarlySync;
		/*! DTXn change BCLK edge */
		uint8 nBclkTxPolarity;
		/*! DRXn sampling BCLK edge */
		uint8 nBclkRxPolarity;
		/*! Interleave slots between TX pins  */
		uint8 bTXInterleave;
		/*! Interleave slots between RX pins  */
		uint8 bRXInterleave;
		/*! Transmit data offset in TDM - 0 to 63 */
		uint8 nTxOffset;
		/*! Receive data offset in TDM - 0 to 63 */
		uint8 nRxOffset;
		/*! TxPin TriState before driving TDM slots */
		uint8 bTriStateBeforeTx;
		/*! TxPin Tristate after driving TDM slots */
		uint8 bTriStateAfterTx;
		A2B_MASTER_I2S_RATE_CONFIG sI2SRateConfig;
	}A2B_MASTER_I2S_SETTINGS;


	/*! \struct A2B_MASTER_PIN_MUX_SETTINGS
	GPIO pin multiplication settings  
	*/
	typedef struct 
	{
		/*! GPIO 1 Pin multiplexing */
		uint8 bGPIO1PinUsage;
		/*! GPIO 2 Pin multiplexing */
		uint8 bGPIO2PinUsage;
		/*! GPIO 3 Pin multiplexing */
		uint8 bGPIO3PinUsage;
		/*! GPIO 4 Pin multiplexing */
		uint8 bGPIO4PinUsage;
		/*! GPIO 5 Pin multiplexing */
		uint8 bGPIO5PinUsage;
		/*! GPIO 6 Pin multiplexing */
		uint8 bGPIO6PinUsage;
		/*! GPIO 7 Pin multiplexing */
		uint8 bGPIO7PinUsage;
	}A2B_MASTER_PIN_MUX_SETTINGS;

	/*! \struct A2B_MASTER_INPUTPIN_INTERRUPT_CONFIG
	GPIO input pin interrupt configurations 
	*/
	typedef struct 
	{
		/*! Enable GPIO 1 Input pin interrupt  */
		uint8 bGPIO1Interrupt;
		/*! Interrupt polarity - GPIO 1 Input pin  */
		uint8 bGPIO1IntPolarity;
		/*! Enable GPIO 2 Input pin interrupt  */
		uint8 bGPIO2Interrupt;
		/*! Interrupt polarity - GPIO 2 Input pin  */
		uint8 bGPIO2IntPolarity;
		/*! Enable GPIO 3 Input pin interrupt  */
		uint8 bGPIO3Interrupt;
		/*! Interrupt polarity - GPIO 3 Input pin  */
		uint8 bGPIO3IntPolarity;
		/*! Enable GPIO 4 Input pin interrupt  */
		uint8 bGPIO4Interrupt;
		/*! Interrupt polarity - GPIO 4 Input pin  */
		uint8 bGPIO4IntPolarity;
		/*! Enable GPIO 5 Input pin interrupt  */
		uint8 bGPIO5Interrupt;
		/*! Interrupt polarity -  GPIO 5 Input pin  */
		uint8 bGPIO5IntPolarity;
		/*! Enable GPIO 6 Input pin interrupt  */
		uint8 bGPIO6Interrupt;
		/*! Interrupt polarity - GPIO 6 Input pin  */
		uint8 bGPIO6IntPolarity;
		/*! Enable GPIO 7 Input pin interrupt  */
		uint8 bGPIO7Interrupt;
		/*! Interrupt polarity - GPIO 7 Input pin  */
		uint8 bGPIO7IntPolarity;

	}A2B_MASTER_INPUTPIN_INTERRUPT_CONFIG;

	/*! \struct A2B_OUTPIN_VALUE
	GPIO output pin configuration for master 
	*/
	typedef struct 
	{
		uint8 bGPIO1Val;/*! Data value for GPIO 1 output pin  */
		uint8 bGPIO2Val;/*! Data value for GPIO 2 output pin  */
		uint8 bGPIO3Val;/*! Data value for GPIO 3 output pin  */
		uint8 bGPIO4Val;/*! Data value for GPIO 4 output pin  */
		uint8 bGPIO5Val;/*! Data value for GPIO 5 output pin  */
		uint8 bGPIO6Val;/*! Data value for GPIO 6 output pin  */
		uint8 bGPIO7Val;/*! Data value for GPIO 7 output pin  */
	}A2B_MASTER_OUTPIN_CONFIG;


	/*! \struct A2B_MASTER_GPIO_SETTINGS
	Master GPIO Configuration
	*/
	typedef struct 
	{
		A2B_MASTER_PIN_MUX_SETTINGS          sPinMuxSettings;/*! GPIO Pin multiplex Settings */
		A2B_MASTER_INPUTPIN_INTERRUPT_CONFIG sPinIntConfig;/*! Input Pin interrupt configuration */
		A2B_MASTER_OUTPIN_CONFIG             sOutPinVal;/*! Input Pin interrupt Settings */
		uint8 bHighDriveStrength;          /*! Digital Pin drive strength */
		uint8 bIRQInv;                      /*! IRQ Pin Invert */
		uint8 bIRQTriState;                 /*! Enable tristate when inactive */
	}A2B_MASTER_GPIO_SETTINGS;

	/*! \struct A2B_MASTER_INTERRUPT_CONFIG
	AD2410 Interrupt configuration
	*/
	typedef struct
	{
		uint8 bReportHDCNTErr;/*! Report Header count error  */
		uint8 bReportDDErr;/*! Report Data decoding error  */
		uint8 bReportCRCErr;/*! Report Data CRC error  */
		uint8 bReportDataParityErr;/*! Report Data Parity error  */
		uint8 bReportPwrErr;/*! Report Data Bus Power error  */
		uint8 bReportErrCntOverFlow;    /*! Report bit error count overflow error  */
		uint8 bReportSRFMissErr;        /*! Report SRF miss error  */
		uint8 bReportGPIO1;             /*! Report GPIO  1 Interrupt */
		uint8 bReportGPIO2;             /*! Report GPIO  2 Interrupt */
		uint8 bReportGPIO3;             /*! Report GPIO  3 Interrupt */
		uint8 bReportGPIO4;             /*! Report GPIO  4 Interrupt */
		uint8 bReportGPIO5;             /*! Report GPIO  5 Interrupt */
		uint8 bReportGPIO6;             /*! Report GPIO  6 Interrupt */
		uint8 bReportGPIO7;             /*! Report GPIO  7 Interrupt */
		uint8 bReportI2CErr;            /*! Report I2C failure error  */
		uint8 bDiscComplete;            /*! Report Discovery Completion */
		uint8 bIntFrameCRCErr;          /*! Report Interrupt frame error */
		uint8 bSlaveIntReq;              /*! Report Interrupt requests  */
	}A2B_MASTER_INTERRUPT_SETTINGS;

	/*! \struct A2B_AD242x_CLKOUT_CONFIG 
	Clock out config for AD242x
	*/
	typedef struct
	{
		uint8 	bClk1Inv;           /*! Enable Clock1 inversion  */
		uint8 	bClk1PreDiv;        /*! Clk1 pre-division */
		uint8 	bClk1Div;           /*! CLK1 division factor */
		uint8 	bClk2Inv;           /*! Enable Clock 2 inversion  */
		uint8 	bClk2PreDiv;        /*! Clk2 pre-division */
		uint8   bClk2Div;           /*! CLK2 division factor */

	}A2B_AD242x_CLKOUT_CONFIG;

	/*! \struct A2B_GPIOD_PIN_CONFIG 
	AD242x GPIOD settings
	*/
	typedef struct
	{
		uint8 	bGPIODistance;      /*! Enable/Disable GPIO over distance */
		uint8 	bGPIOSignalInv;     /*! Enable/Disable  */
		uint8   abBusPortMask[8];   /*! Bus port masks  */
	}A2B_GPIOD_PIN_CONFIG;


	/*! \struct A2B_GPIOD_SETTINGS
	AD242x GPIOD settings
	*/
	typedef struct
	{
		A2B_GPIOD_PIN_CONFIG sGPIOD1Config;
		A2B_GPIOD_PIN_CONFIG sGPIOD2Config;
		A2B_GPIOD_PIN_CONFIG sGPIOD3Config;
		A2B_GPIOD_PIN_CONFIG sGPIOD4Config;
		A2B_GPIOD_PIN_CONFIG sGPIOD5Config;
		A2B_GPIOD_PIN_CONFIG sGPIOD6Config;
		A2B_GPIOD_PIN_CONFIG sGPIOD7Config;
	}A2B_MASTER_GPIOD_SETTINGS;

	/*! \struct A2B_SLAVE_REGISTER_SETTINGS
	AD2410 Interrupt configuration
	*/
	typedef struct
	{
		uint8 	nSWCTL;                 /*! Switch control register */
		uint8 	nPDMCTL;                /*! Error management register  */
		uint8 	nTESTMODE;              /*! Test mode register */
		uint8 	nBECCTL;                /*! Error count register */
		uint8 	nERRMGMT;               /*! Error management register  */
		uint8 	nI2STEST;               /*! I2S test register  */
		uint8 	nGENERR;                /*! Generate error  */
		uint8 	nRAISE;                 /*! Raise interrupt register  */
		uint8   nBMMCFG;                /*! Bus monitor configuration */
	}A2B_MASTER_REGISTER_SETTINGS;


/*! \struct ADI_A2B_MASTER_NCD
   Master configuration 
*/
typedef struct
{
	uint8              nNodeID;        /*! Slave node ID */
	uint8              nSrcNodeID;     /*! Connected Node ID - upstream node */
	ADI_A2B_PARTNUM     ePartNum;       /*! Transceiver part number   */
	A2B_NODE_AUTHENTICATION     sAuthSettings;          /*! Expected authentication settings  */
	A2B_MASTER_CONFIG_CONTROL   sConfigCtrlSettings;    /*! Basic configuration & control  */
	A2B_MASTER_I2S_SETTINGS  	sI2SSettings;           /*! A2B I2S Settings */
	A2B_MASTER_GPIO_SETTINGS    	sGPIOSettings;      /*! GPIO settings  */
	A2B_MASTER_INTERRUPT_SETTINGS  	sInterruptSettings; /*! Interrupt configuration */
	A2B_AD242x_CLKOUT_CONFIG  	sClkOutSettings;        /*! AD242x clock out config */
	A2B_MASTER_GPIOD_SETTINGS  	sGPIODSettings;         /*! AD242x clock out config */
	A2B_MASTER_REGISTER_SETTINGS 	sRegSettings;       /*! AD2410 Register configuration - for advanced use */

}ADI_A2B_MASTER_NCD;

/*! \struct ADI_A2B_COMMON_CONFIG
   Common configurations for master as well as slave node
*/
typedef struct
{
	uint8   nDwnSlotSize;        /*! Down slot size */
	uint8   nUpSlotSize;         /*! Up slot size  */
	uint8	bUpstreamCompression;       /*! Floating point compression for upstream  */
	uint8	bDwnstreamCompression;      /*! Floating point compression for downstream  */
	uint8   bEnableUpstream;            /*! Enable Upstream */
	uint8   bEnableDwnstream;           /*! Enable Downstream */
	uint8   bEnableReduceRate;          /* Reduce Data Rate on A2B Bus*/
	uint8   nSysRateDivFactor;          /*! System level reduced rate factor */
	uint8   nMasterI2CAddr;             /*! Master I2C address - 7 bit */
	uint8   nBusI2CAddr;            /*! Bus I2C address - 7bit */
}ADI_A2B_COMMON_CONFIG;

    /*! \struct A2B_SLAVE_I2S_RATE_CONFIG
I2S rate settings for AD24xx
*/
 typedef struct  
{
	uint8 nSamplingRate;            /*! I2S sampling rate  */
	uint8 bReduce;                  /*! Reduce / re-transmit higher frequency samples  */
	uint8 bShareBusSlot;            /*! Share A2B bus slots for reduced sampling */
	uint8 nRBCLKRate;               /*! BCLK as a factor of SYNC/SFF for reduced rates */
	uint8 nRROffset;                /*! Reduced rate sync offset */
	uint8 bRRValidBitLSB;           /*! Enable RR valid bit in LSB */
	uint8 bRRValidBitExtraBit;      /*! Enable Valid RR bit in Extra bit */
	uint8 bRRValidBitExtraCh;       /*! Enable Valid RR bit in Extra Channel */
	uint8 bRRStrobe;                /*! Enable Reduced rate strobe in ADR1/IO1 */
	uint8  bRRStrobeDirection;      /*! Strobe direction High or Low  */
}A2B_SLAVE_I2S_RATE_CONFIG;


/*! \struct A2B_SLAVE_I2S_SETTINGS
I2S interface settings for slave AD2410
*/
 typedef struct  
{
	uint8 nTDMMode;             /*! TDM mode  */
	uint8 nTDMChSize;           /*! TDM channel size  */
	uint8 nSyncMode;            /*! SYNC mode- Pulse/50% duty cycle */
	uint8 nSyncPolarity;            /*! SYNC Polarity- Rising/Falling edge */
	uint8 bEarlySync;               /*! Early frame sync status */
	uint8 nSyncOffset;              /*! SYNC offset with Super frame */
	uint8 nBclkTxPolarity;          /*! DTXn change BCLK edge */
	uint8 nBclkRxPolarity;          /*! DRXn sampling BCLK edge */
	uint8 bTXInterleave;            /*! Interleave slots between TX pins  */
	uint8 bRXInterleave;            /*! Interleave slots between RX pins  */
	A2B_SLAVE_I2S_RATE_CONFIG sI2SRateConfig;
	uint8  nCodecClkRate;           /*! Codec clock rate - applicable only for AD241x  */
}A2B_SLAVE_I2S_SETTINGS;

/*! \struct A2B_PDM_SETTINGS
PDM Settings( Only for slaves )
*/
typedef struct
{
	uint8 nNumSlotsPDM0;            /*! Number of PDM0 slots */
	uint8 nNumSlotsPDM1;            /*! Number of PDM1 slots */
	uint8 bHPFUse;              /*! Use High Pass Filter    */
	uint8 nPDMRate;             /*! PDM rate for AD242x */
	uint8 nHPFCutOff;           /*! Filter Cut-off frequency */
}A2B_SLAVE_PDM_SETTINGS;

/*! \struct A2B_SLAVE_PIN_MUX_SETTINGS
GPIO pin multiplication status
*/
typedef struct
{
	uint8 bGPIO0PinUsage;           /*! GPIO 0 Pin multiplexing */
	uint8 bGPIO1PinUsage;           /*! GPIO 1 Pin multiplexing */
	uint8 bGPIO2PinUsage;           /*! GPIO 2 Pin multiplexing */
	uint8 bGPIO3PinUsage;           /*! GPIO 3 Pin multiplexing */
	uint8 bGPIO4PinUsage;           /*! GPIO 4 Pin multiplexing */
	uint8 bGPIO5PinUsage;           /*! GPIO 5 Pin multiplexing */
	uint8 bGPIO6PinUsage;           /*! GPIO 6 Pin multiplexing */
	uint8 bGPIO7PinUsage;           /*! GPIO 7 Pin multiplexing */
}A2B_SLAVE_PIN_MUX_SETTINGS;

/*! \struct A2B_SLAVE_INPUTPIN_INTERRUPT_CONFIG
GPIO input pin settings for slave node
*/
typedef struct
{
	uint8 bGPIO0Interrupt;      /*! Enable GPIO 0 Input pin interrupt  */
	uint8 bGPIO0IntPolarity;        /*! Interrupt polarity - GPIO 0 Input pin  */
	uint8 bGPIO1Interrupt;          /*! Enable GPIO 1 Input pin interrupt  */
	uint8 bGPIO1IntPolarity;        /*! Interrupt polarity -  GPIO 1 Input pin  */
	uint8 bGPIO2Interrupt;          /*! Enable GPIO 2 Input pin interrupt  */
	uint8 bGPIO2IntPolarity;        /*! Interrupt polarity - GPIO 2 Input pin  */
	uint8 bGPIO3Interrupt;          /*! Enable GPIO 3 Input pin interrupt  */
	uint8 bGPIO3IntPolarity;        /*! Interrupt polarity - GPIO 3 Input pin  */
	uint8 bGPIO4Interrupt;          /*! Enable GPIO 4 Input pin interrupt  */
	uint8 bGPIO4IntPolarity;            /*! Interrupt polarity - GPIO 4 Input pin  */
	uint8 bGPIO5Interrupt;              /*! Enable GPIO 5 Input pin interrupt  */
	uint8 bGPIO5IntPolarity;            /*! Interrupt polarity -  GPIO 5 Input pin  */
	uint8 bGPIO6Interrupt;              /*! Enable GPIO 6 Input pin interrupt  */
	uint8 bGPIO6IntPolarity;            /*! Enable GPIO 6 Input pin interrupt  */
	uint8 bGPIO7Interrupt;              /*! Enable GPIO 7 Input pin interrupt  */
	uint8 bGPIO7IntPolarity;            /*! Interrupt polarity - GPIO 7 Input pin  */

}A2B_SLAVE_INPUTPIN_INTERRUPT_CONFIG;

/*! \struct A2B_SLAVE_OUTPIN_CONFIG
GPIO output pin configuration
*/
typedef struct 
{
	uint8 bGPIO0Val;            /*! Data value for GPIO 0 output pin  */
	uint8 bGPIO1Val;            /*! Data value for GPIO 1 output pin  */
	uint8 bGPIO2Val;            /*! Data value for GPIO 2 output pin  */
	uint8 bGPIO3Val;            /*! Data value for GPIO 3 output pin  */
	uint8 bGPIO4Val;            /*! Data value for GPIO 4 output pin  */
	uint8 bGPIO5Val;            /*! Data value for GPIO 5 output pin  */
	uint8 bGPIO6Val;            /*! Data value for GPIO 6 output pin  */
	uint8 bGPIO7Val;            /*! Data value for GPIO 7 output pin  */
}A2B_SLAVE_OUTPIN_CONFIG;



/*! \struct A2B_SLAVE_GPIO_SETTINGS
Slave GPIO Configuration
*/
typedef struct
{
	A2B_SLAVE_PIN_MUX_SETTINGS  sPinMuxSettings;            /*! GPIO Pin multiplex Settings */
	A2B_SLAVE_INPUTPIN_INTERRUPT_CONFIG   	sPinIntConfig;  /*! Input Pin interrupt configuration */
	A2B_SLAVE_OUTPIN_CONFIG					sOutPinVal;     /*! Input Pin interrupt Settings */
	uint8 bHighDriveStrength;           /*! Digital Pin drive strength */
	uint8 bIRQInv;          /*! IRQ Pin Invert */
	uint8 bIRQTriState;     /*! Enable tristate when inactive */
}A2B_SLAVE_GPIO_SETTINGS;

/*! \struct A2B_SLAVE_INTERRUPT_SETTINGS
AD2410 Interrupt configuration
*/
typedef struct
{
    /*! Report Header count error  */
	uint8 bReportHDCNTErr;

	/*! Report Data decoding error  */
	uint8 bReportDDErr;

	/*! Report Data CRC error  */
	uint8 bReportCRCErr;

	/*! Report Data Parity error  */
	uint8 bReportDataParityErr;

	/*! Report Data Bus Power error  */
	uint8 bReportPwrErr;

	/*! Report bit error count overflow error  */
	uint8 bReportErrCntOverFlow;

	/*! Report SRF miss error  */
	uint8 bReportSRFMissErr;

	/*! Report SRF crc error  */
	uint8 bReportSRFCrcErr;

	/*! Report GPIO  0 Interrupt */
	uint8 bReportGPIO0;

	/*! Report GPIO  1 Interrupt */
	uint8 bReportGPIO1;

	/*! Report GPIO  2 Interrupt */
	uint8 bReportGPIO2;

	/*! Report GPIO  3 Interrupt */
	uint8 bReportGPIO3;

	/*! Report GPIO  4 Interrupt */
	uint8 bReportGPIO4;

	/*! Report GPIO  5 Interrupt */
	uint8 bReportGPIO5;

	/*! Report GPIO  6 Interrupt */
	uint8 bReportGPIO6;

	/*! Report GPIO  7 Interrupt */
	uint8 bReportGPIO7;

}A2B_SLAVE_INTERRUPT_SETTINGS;

/*! \struct A2B_GPIOD_SETTINGS
AD242x GPIOD settings
*/
typedef struct
{
	A2B_GPIOD_PIN_CONFIG sGPIOD0Config;
	A2B_GPIOD_PIN_CONFIG sGPIOD1Config;
	A2B_GPIOD_PIN_CONFIG sGPIOD2Config;
	A2B_GPIOD_PIN_CONFIG sGPIOD3Config;
	A2B_GPIOD_PIN_CONFIG sGPIOD4Config;
	A2B_GPIOD_PIN_CONFIG sGPIOD5Config;
	A2B_GPIOD_PIN_CONFIG sGPIOD6Config;
	A2B_GPIOD_PIN_CONFIG sGPIOD7Config;
}A2B_SLAVE_GPIOD_SETTINGS;

/*! \struct A2B_SLAVE_REGISTER_SETTINGS
AD2410 Interrupt configuration
*/
typedef struct
{
    uint8 	nSWCTL;             /*! Switch control register */
    uint8 	nTESTMODE;          /*! Test mode register */
    uint8 	nBECCTL;            /*! Error count register */
    uint8 	nERRMGMT;           /*! Error management register  */
    uint8   nPLLCTL;            /*! PLL control register  */
    uint8 	nI2STEST;           /*! I2S test register  */
    uint8 	nGENERR;            /*! Generate error register  */
    uint8 	nRAISE;             /*! Raise interrupt register  */
    uint8 	nBMMCFG;            /*! Bus monitor configuration */
    uint8 	nSUSCFG;            /*! Clock sustain configuration */
    uint8 	nMBOX0CTL;          /*! Mailbox 0 control */
    uint8   nMBOX1CTL;          /*! Mailbox 1 control */
}A2B_SLAVE_REGISTER_SETTINGS;


/*! \struct ADI_A2B_PERI_CONFIG_UNIT 
	A2B peripheral config unit structure
 */
typedef struct 
 { 
	uint32 eOpCode;                 /*!  Operational code. Write - 0, Read - 1, Delay - 2*/
	uint32 nAddrWidth;              /*!  Sub address width */
	uint32 nAddr;               /*!  Sub address */
	uint32 nDataWidth;          /*!  Data width */
	uint32 nDataCount;          /*!  Data count */
	uint8* paConfigData;        /*!  Pointer to config data */
} ADI_A2B_PERI_CONFIG_UNIT;


/*! \struct PERIPHERAL_DEVICE_CONFIG
   Peripheral device configuration
*/
typedef struct
{
	uint8  bI2CInterfaceUse;            /*! I2C interface status  */
	uint8  nI2Caddr;                    /*! 7 bit I2C address */
	uint8  eDeviceType;             /*! Device type -audio source/sink/host  */
	uint8  bUseTx0;             /*! Tx0 Pin in use */
	uint8  bUseRx0;             /*! Rx0 Pin in use */
	uint8  bUseTx1;             /*! Tx1 Pin in use */
	uint8  bUseRx1;             /*! Rx1 Pin in use */
	uint8  nChTx0;              /*! No of Tx0 channels  */
	uint8  nChRx0;              /*! No of Rx0 channels  */
	uint8  nChTx1;              /*! No of Tx1 channels  */
	uint8  nChRx1;              /*! No of Rx1 channels  */
	uint8  nNumPeriConfigUnit;      /*! Enable Upstream */
	/*! Pointer to peripheral configuration unit */
	ADI_A2B_PERI_CONFIG_UNIT* paPeriConfigUnit;

}A2B_PERIPHERAL_DEVICE_CONFIG;


/************************************************* NCD DEFINITION **********************************/
	/*! \struct ADI_A2B_SLAVE_NCD
	   Slave configuration
    */
	typedef struct
	{
		uint8  					nNodeID;        /*! Slave node ID */
		uint8  					nSrcNodeID;     /*! Connected Node ID - upstream node */
		uint8  					bEnableAutoConfig;      /*! Auto-Configuration Enabled */
		uint8  						 nNumPeriDevice;        /*! Number of Peripheral device  */
 		ADI_A2B_PARTNUM             ePartNum;       /*! Transceiver part number   */
		A2B_NODE_AUTHENTICATION 	sAuthSettings;          /*! Authentication settings  */
		A2B_SLAVE_CONFIG_CONTROL	sConfigCtrlSettings;    /*! Basic configuration & control  */
		A2B_SLAVE_I2S_SETTINGS  	sI2SSettings;       /*! A2B I2S Settings */
		A2B_SLAVE_PDM_SETTINGS		sPDMSettings;       /*! PDM settings  */
		A2B_SLAVE_GPIO_SETTINGS    	sGPIOSettings;      /*! GPIO settings  */
		A2B_SLAVE_INTERRUPT_SETTINGS  sInterruptSettings;   /*! Interrupt configuration */
		A2B_AD242x_CLKOUT_CONFIG  	sClkOutSettings;        /*! AD242x clock out config */
		A2B_SLAVE_GPIOD_SETTINGS  	sGPIODSettings;         /*! AD242x clock out config */
		A2B_SLAVE_REGISTER_SETTINGS sRegSettings;           /*! AD2410 Register configuration - for advanced use */
		A2B_PERIPHERAL_DEVICE_CONFIG  *apPeriConfig[ADI_A2B_MAX_DEVICES_PER_NODE];/*! Array of pointers  */

    }ADI_A2B_SLAVE_NCD;

/*! \enum ADI_A2B_CONNECTED_DEVICES
    Possible A2B node types
  */
typedef enum
{
    ADAU1461,         /*!  SigmaDSP ADAU1461 */
    ADAU176x,         /*!  SigmaDSP ADAU176x */
    ADAU1961,         /*!  SigmaDSP ADAU1961 */
    GENERIC,          /*!  SigmaDSP ADAU1961 */
    ADAU1452,          /*!  Enum for ADAU1452-Sigma300 */    
    UNKNOWN           /*!  Unknown device */
}ADI_A2B_DEVICE_ID;

/*! \struct ADI_A2B_MASTER_SLAVE_CHAIN_CONFIG  
   Configuration for one master and associated slaves
*/
typedef struct
{
	uint8  nNumSlaveNode;           /*! Slave node ID */
	const ADI_A2B_MASTER_NCD* pMasterConfig;      /*! Pointer to master node configuration  */
	const ADI_A2B_SLAVE_NCD *apSlaveConfig[ADI_A2B_MAX_SLAVE_NODES_PER_MASTER];       /*! Slave node configuration array */
	const ADI_A2B_COMMON_CONFIG sCommonSetting;       /*! Common network configuration for one daisy chain */
}ADI_A2B_MASTER_SLAVE_CONFIG;

/*! \struct ADI_A2B_NETWORK_CONTROL_GUIDANCE
Network configuration & monitoring guidance
*/
typedef struct 
{
    /*! Discovery mode  */
    uint8  eDiscoveryMode;

    /* Enable/disable Line diagnostics */
	uint8  bLineDiagnostics;

    /* Auto rediscovery on critical line fault during discovery */
	uint8  bAutoDiscCriticalFault;

    /* Number of redisocvery attempts on critical faults */
	uint8  nAttemptsCriticalFault;

    /* Auto rediscovery upon post-discovery line fault */
    uint8  bAutoRediscOnFault;

    /*! Number of Peripheral device(s) connected to Target  */
    uint8  nNumPeriDevice;

    /*! Array of pointers to peripheral configuration unit */
    const A2B_PERIPHERAL_DEVICE_CONFIG  *apPeriConfig[ADI_A2B_MAX_DEVICES_PER_NODE];

}ADI_A2B_NETWORK_CONFIG;


/*! \struct ADI_A2B_BCD  
   Bus configuration Data for entire network
*/
typedef struct
{
	/*! Number of master node in network */
	//uint8  nNumMasterNode;

	/*! Pointer to master-slave chains */
	ADI_A2B_MASTER_SLAVE_CONFIG *apNetworkconfig[ADI_A2B_MAX_MASTER_NODES];

    /*! Network Configuration */
	ADI_A2B_NETWORK_CONFIG sTargetProperties; 

}ADI_A2B_BCD;

/*! \struct ADI_A2B_NODE
    A2B graph/schematic structure
*/
typedef struct
{
    /*!  Total no. of nodes in A2B network/graph */
    uint8 nNodeCount;
    // the count of slave nodes
    uint8 NumberOfSlaveDiscovered;
    /*!  A2B node object array */
    ADI_A2B_NODE oNode[ADI_A2B_MAX_GRAPH_NODES];
}ADI_A2B_GRAPH;

/*! \enum ADI_A2B_ERROR_TYPE
    Possible A2B error types 
 */
typedef enum
{
 /*!  Slave discovery failure  */
  ADI_A2B_NODE_DISCOVERY_FAILURE        = 0x100,
  
 /*!  Slave configuration error  */ 
  ADI_A2B_SLAVE_CONFIGURATION_ERROR     = 0x101,
  
 /*!  Master configuration error  */ 
  ADI_A2B_MASTER_CONFIGURATION_ERROR    = 0x102,
  
  /*! Target configuration error*/
  ADI_A2B_HOST_CONFIGURATION_ERROR      = 0x103,
    
  /*! Target initialization error */
  ADI_A2B_HOST_INITIALIZATION_ERROR     = 0x104,
  
  /*! Network Initialization error */
  ADI_A2B_NETWORK_CONFIGURATION_ERROR   = 0x105

}ADI_A2B_ERROR_TYPE;

/*! \enum ADI_A2B_INTERRUPT_CONTEXT
    Defines the context of AD2410 interrupt service. 
    Framework uses interrupts for discovery acknowledgment, Line diagnostics and post-discovery network monitoring
 */
typedef enum
{

  /*!  Interrupts before or after discovery  */  
  ADI_A2B_INTERRUPT_AFTER_DISCOVERY = 0x100u,
  
   /*! Interrupts during discovery wait period  */
  ADI_A2B_INTERRUPT_DURING_DISCOVERY = 0x200u

}ADI_A2B_INTERRUPT_CONTEXT;


/*! \struct ADI_A2B_TARGET_ERROR_STATUS
    Error state preserving structure
*/
typedef struct 
{
  /*! Enum for error type*/ 
  ADI_A2B_ERROR_TYPE  eError;
  
  /*! Error causing node Id*/       /* Note: May not be relevant for all types of error */
  uint32 nNodeID;

}ADI_A2B_TARGET_ERROR_STATUS;

typedef void (*GPIO_CALL_BACK)(uint8 nGPIONum, void* pHandle);

/*! \struct ADI_A2B_ERROR_COUNT_HANDLER
     Bit error count handling structure
*/
typedef struct 
{
  /*! Counter for Header count error*/  
  volatile uint32 nHeaderErrorCount;
  
  /*! Counter for parity  error count */
  volatile uint32 nParityErrorCount;
  
  /*! Counter for CRC error*/  
  volatile uint32 nCRCErrorCount;
  
  /*! Counter for Decode error*/  
  volatile uint32 nDecodeErrorCount;
 

}ADI_A2B_ERROR_COUNT_HANDLER,*ADI_A2B_ERROR_COUNT_HANDLER_PTR;


/*! \struct ADI_A2B_EVENT_HANDLER
    Structure for GPIO event handling 
*/
typedef struct 
{
  /*! Flag to indicate interrupts */
  volatile uint8 bEvent;
  
  /*! Flag to indicate completion of interrupt process during discovery   */
  volatile uint8 bIntProcessDiscDone;
  
  /*! Flag to indicate node discovery status  */
  volatile uint8 bNodeFound;
    
  /*! Interrupt service context */
  volatile ADI_A2B_INTERRUPT_CONTEXT eIntContext;  
  
  /*! Application level event callback function pointer */
  GPIO_CALL_BACK pfCallbackhandle;
  
  /*! Buffer to count the number of different bit errors */
  ADI_A2B_ERROR_COUNT_HANDLER nBitErrCount[A2B_MAX_NUMBER_OF_NODES_PER_BRANCH];
  
}ADI_A2B_EVENT_HANDLER,*ADI_A2B_EVENT_HANDLER_PTR;

/*! Timer callback */
typedef void (*TIMER_CALL_BACK)(void* pHandle);

/*! \enum ADI_A2B_TIMER_CONTEXT
    Context of timer run 
 */
typedef enum
{
 /*!  During discovery   */
  ADI_A2B_DISCOVERY,
 /*!  For I2C event  */  
  ADI_A2B_I2C,
  /*! Post discovery line fault */ 
  ADI_A2B_POST_DISCOVERY,
  /*!  Time out for Bit error indication  */
  ADI_A2B_BIT_ERROR_INDICATION

}ADI_A2B_TIMER_CONTEXT;


/*! \struct ADI_A2B_TIMER_HANDLER
    Configuration structure for GPT
*/
typedef struct 
{
  /*! Flag to notify Timer expiry*/  
  volatile uint8 bTimeout;
  
  /*! Application call back function pointer*/
  TIMER_CALL_BACK pCallbackhandle;
  
  /*! Timer Number */
  uint32 nTimerNo;
  
  /*! Timer Expire value in milliSecond */
  uint32 nTimerExpireVal;
  
  /*! A2B timer context */
  ADI_A2B_TIMER_CONTEXT eContext; 

}ADI_A2B_TIMER_HANDLER,*ADI_A2B_TIMER_HANDLER_PTR;

/*! \enum ADI_A2B_DISCOVERY_TYPE
    Possible discovery modes 
 */
typedef enum
{
 /*!  Simple discovery  */
  ADI_A2B_SIMPLE_DISCOVERY ,
 /*!  Modified discovery */  
  ADI_A2B_MODIFED_DISCOVERY ,
  /*! Optimized discovery  */ 
  ADI_A2B_OPTIMIZED_DISCOVERY ,
  /*! Advanced discovery */ 
  ADI_A2B_ADVANCED_DISCOVERY

}ADI_A2B_DISCOVERY_TYPE;

/*! \enum ADI_A2B_NETWORK_CLK_SRC
    Network clock source 
 */
typedef enum
{
 /*! Target frame sync as source  */
  ADI_A2B_TARGET_FS,
 /*! Master codec as clock source  */ 
  ADI_A2B_MASTER_CODEC 

}ADI_A2B_NETWORK_CLK_SRC;

/*! \enum ADI_A2B_I2C_CLK_RATE
    Host I2C or TWI rate options 
 */
typedef enum
{
   /*! I2C clock rate 100K  */
  ADI_A2B_I2C_100KHz, 
  
  /*! I2C clock rate 400K  */ 
  ADI_A2B_I2C_400KHz,

}ADI_A2B_I2C_CLK_RATE;


/*! \struct ADI_A2B_TRGT_PROPERTY_HANDLER
    Configurable target properties structure
*/
typedef struct 
{
   
  /*! Discovery mechanism */
  ADI_A2B_DISCOVERY_TYPE  eDiscvryType;  
       
  /*! Identification of Network clock source */
  ADI_A2B_NETWORK_CLK_SRC eNtworkSrc;
   
  /*! Flag  to enable/disable beep on error */  
  volatile uint8 bBeepEnable;
  
  /*! Flag to enable/disable optical error indication */
  volatile uint8 bOpticalSrcEnable;

  /*! Flag  to enable/disable LED on error */  
  volatile uint8 bDebugLEDEnable;
  
  /*! Flag  to enable/disable saving schematic info in EEPROM connected to Master */  
  volatile uint8 bWriteToEEPROMEnable;
 
  /*! Flag to enable/disable line Diagnostics */
  volatile uint8 bLineDiagnostic;
  
  /*! Flag to enable/disable auto discovery on critical fault */  
  volatile uint8 bAutoDiscCriticalFault;
  
  /*! Number of discovery attempts on critical line fault  */  
  volatile uint8 nAttemptsCriticalFault;
  
  /*! Flag for auto re-discovery upon post discovery line faults */
  volatile uint8 bAutoRediscovery;
  
  /*! I2C/ TWI clock rate */
  ADI_A2B_I2C_CLK_RATE eI2CClkRate;
 

}ADI_A2B_TRGT_PROPERTY_HANDLER,*ADI_A2B_TRGT_PROPERTY_HANDLE_PTR;

typedef struct 
{
  /*! BERT window in micro-seconds */
  uint32 nReadTime;
  
  /*! Test Mode */
  uint32 nBERTMode;
  
  /*! PRBS error counter */  
  uint32 nPRBSCount[ADI_A2B_MAX_MASTER_NODES][A2B_MAX_NUMBER_OF_NODES_PER_BRANCH];
  
  /*! Counter for various errors */  
  //uint32 nErrorCount[ADI_A2B_MAX_MASTER_NODES][A2B_MAX_NUMBER_OF_NODES_PER_BRANCH];
  
  /*! Auto reset flag */
  uint8  bResetFlag; 
  
  /*! Auto reset window in Microseconds */
  uint32 nAutoResetWindowTime; 
  
  /*! Read interval counter */
  uint32 nCount;
  
  /*  Overflow flag */
  uint32 bOverFlowCount[ADI_A2B_MAX_MASTER_NODES][ADI_A2B_MAX_SLAVE_NODES_PER_MASTER + 1];

}ADI_A2B_BERT_HANDLER,*ADI_A2B_BERT_HANDLER_PTR;

/*! Event callback */
typedef void (*A2B_CALL_BACK)(void* pHandle,uint32 Event);


typedef struct 
{
    /*! Flag to indicate the node discovery status */
    volatile uint8 bRxComplete;

    /*! Flag to indicate the node discovery status */
    volatile uint8 bTxComplete;

    /*! Application level call-back function pointer */
    A2B_CALL_BACK pfCallbackhandle;

     /*! Pointer to message read buffer */ 
    uint32* pMsgRead;

    /*!  Pointer to message write buffer */
    uint32* pMsgWrite;
   
     /*! Read transfer length in 32 bit word */
    uint32 nReadLength;
    
     /*! Write transfer length in 32 bit word */
    uint32 nWriteLength;
    
     /*! Flag to indicate SigmaStudio connection  */  
     volatile uint8 bSSConnected;

}ADI_A2B_SS_MESSAGE_HANDLER,*ADI_A2B_SS_MESSAGE_HANDLER_PTR;

typedef struct 
{
    /*! Current fault code */
    uint8 nCurrentFaultCode;
    
    /*! Current Fault localization status */
    uint8 bCurrrentFaultNodeID;
    
    /*! Current Fault Node type */
    uint8 bCurrentFaultNodeType;
    
    /*! Flag for concealed Fault */
    uint8 bCurrentFaultNonLoc;
    
    /*! Current fault found  */
    uint8 bCurrentFaultFound; 
    
    /*! Current fault found  */
    uint8 bPreviousFaultFound; 
    
}ADI_A2B_FAULT_STATUS_HANDLER,*ADI_A2B_FAULT_STATUS_HANDLER_PTR;


typedef struct 
{
    /*! Line fault found */
    uint8 bFaultFound;

    /*! Node ID of the line fault */
    uint8 bNodeID;
    
    /*! Node type  */
    ADI_A2B_NODE_TYPE eNodeType;

    /*! Hardware localization status */
    uint8 bConcealedFault;
    
    /*! Critical status of line fault */
    uint8 bCriticalFault;

    /*! Fault code */ 
    uint8 nFaultCode;
    
     /*! Error found in diagnostic mode */
    uint8 bDiagnosticMode;
            
}ADI_A2B_LINE_DIAGNOSTIC_RESULT,*ADI_A2B_LINE_DIAGNOSTIC_RESULT_PTR;

typedef struct 
{
    /*! Line diagnostic result */
    ADI_A2B_LINE_DIAGNOSTIC_RESULT  sFaultRes[ADI_A2B_MAX_MASTER_NODES];
    
    /*! Concealed Fault status */
    ADI_A2B_FAULT_STATUS_HANDLER sFaultStatus;
        
}ADI_A2B_LINE_DIAGNOSTIC_HANDLER,*ADI_A2B_LINE_DIAGNOSTIC_HANDLER_PTR;

/*! \struct ADI_A2B_PERI_DEVICE_CONFIG 
	A2B peripheral device config structure
 */typedef struct 
 { 
	/*!  I2C address of the device to be configured  */
	uint32 nDeviceAddress;

	/*!  ID of the node to which the peripheral is connected  */
	uint32 nConnectedNodeID;

	/*!  Number of peripheral config units to be programmed to the device  */
	uint32 nNumPeriConfigUnit;

	/*!  Pointer to peripheral configuration unit */
	ADI_A2B_PERI_CONFIG_UNIT* paPeriConfigUnit;


} ADI_A2B_PERI_DEVICE_CONFIG, *ADI_A2B_PERI_DEVICE_CONFIG_PTR;


typedef struct
{
   /*! Array of device configurations  */
   ADI_A2B_PERI_DEVICE_CONFIG  aDeviceConfig[ADI_A2B_MAX_DEVICES_PER_NODE];
   
   /*! Number of valid configurations */
   uint8  nNumConfig;
   
}ADI_A2B_NODE_PERICONFIG;


/*! \struct ADI_A2B_FRAMEWORK_HANDLER
    A2B target framework configuration structure 
*/
typedef struct 
{
    /*! Error handling structure */
    //ADI_A2B_TARGET_ERROR_STATUS sErrorMessage[A2B_MAX_NUM_ERROR];
    
    /*! Number of errors found */
    //uint32 nErrorCount;

    /*! Event handling structure*/
    ADI_A2B_EVENT_HANDLER   oEventInt;
    
    /*! Timer service handling structure */
    ADI_A2B_TIMER_HANDLER sTimerHandle;

    /*! BERT handler */  
    //ADI_A2B_BERT_HANDLER  oBertHandler;
    
    /*! Pointer to graph structure */
    ADI_A2B_GRAPH *pgraph;
    
    /*! Boolean flag for  graph structure availability */
    //uint32 bGraphReady;
    
    /*! Target properties for handling user options */
    ADI_A2B_TRGT_PROPERTY_HANDLER oTrgtProprty;

    /*! Line Fault diagnostics status maintanance */
    //ADI_A2B_LINE_DIAGNOSTIC_HANDLER oLineFault;
    
    /*! Table to convert master node ID to graph index i e position of the node in graphdata  */
    uint8 aMasterNodeReferenceTable;
    
    /*! Table to convert slave node ID to graph index i e position of the node in graphdata  */
    uint8 aSlaveNodeReferenceTable[ADI_A2B_MAX_SLAVE_NODES_PER_MASTER];
    
    /*! Table to get peripheral configuration structure */
    ADI_A2B_NODE_PERICONFIG aPeriDownloadTable[A2B_MAX_NUMBER_OF_NODES_PER_BRANCH];
    
#if A2B_SS_ENABLE    
    /*! SigmaStudio communication handling structure */
    ADI_A2B_SS_MESSAGE_HANDLER   oSSMsgHandler;
#endif


}ADI_A2B_FRAMEWORK_HANDLER,*ADI_A2B_FRAMEWORK_HANDLER_PTR;

/*! \struct ADI_A2B_TWI_ADDR_CONFIG
    TWI Sub address(Register) configuration structure
*/

typedef struct
{
    /*! Register address length/width in bytes(0/1/2/4 sub-address) */
    uint32 nRegAddrLen;

    /*! Register Value (sub-address)  */
    uint32 nRegVal;

}ADI_A2B_TWI_ADDR_CONFIG;

/*! \struct ADI_A2B_TWI_DATA_CONFIG
    TWI data configuration structure
*/
typedef struct
{
    /*! Data width in bytes(0/1/2/4)  */
    uint32 nDataLen;

    /*! Register address length */
    uint32 nDataCount;

    /*! value  */
    uint8* paConfigData;

}ADI_A2B_TWI_DATA_CONFIG;


typedef struct cAdiAD2410Drv
{
    bool            isCreated:1;
    cI2CDrv      i2cObj;  /* A i2c obj should be ctored before ctor cAdiAD2410Drv*/
    uint8           deviceAddr;
    bool            i2cEnable; /* When DSP cable insert, disable I2C */
    //ADI_A2B_GRAPH*  pGraph;
    ADI_A2B_FRAMEWORK_HANDLER* pFrameworkHandle;
}cAdiAD2410Drv;

typedef struct
{
    void (*initSectionFunc)(void* me);         // function point to a specific initialization section
    uint16 delaytime;   // time duration to next intialization section in ms
}tA2bInitSection;


void adi_a2b_Ctor(cAdiAD2410Drv* me);
void adi_a2b_Xtor(cAdiAD2410Drv* me);
void adi_a2b_drv_init(cAdiAD2410Drv* me);
uint32 adi_a2b_slave_peripheral_config(cAdiAD2410Drv* me, uint8 nTWIDeviceNo, uint8 nDeviceAddress,  ADI_A2B_PERI_CONFIG_UNIT* pOPUnit);

#endif







