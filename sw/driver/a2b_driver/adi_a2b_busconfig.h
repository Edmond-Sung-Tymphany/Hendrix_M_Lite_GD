/*******************************************************************************
Copyright (c) 2014 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
* @file: adi_a2b_config.h
* @brief: This file contains macros and structure definitions used for bus configuration 
* @version: $Revision$
* @date: $Date$
* BCF Version - 1.0.0
* Developed by: Automotive Software and Systems team, Bangalore, India
* THIS IS A SIGMASTUDIO GENERATED FILE AND SHALL NOT BE MODIFIED OUTSIDE OF SIGMASTUDIO
*****************************************************************************/

#ifndef _ADI_A2B_BUSCONFIG_H_
#define _ADI_A2B_BUSCONFIG_H_

/*! \addtogroup Target_Independent
* @{
*/

/** @defgroup Bus_Configuration
*
* This module has structure definitions to describe SigamStudio exported bus configuration/description file.
* The exported bus configuration is interpreted using the structure \ref ADI_A2B_BCD.
*
*/

/*! \addtogroup Bus_Configuration  Bus Configuration
* @{
*/

/*
	Terms Usage
	BCF - Bus Configuration File
	NCF - Node Configuration File

	BCD  - Bus Configuration Data
	NCD  - Node Configuration Data

*/

/*============= D E F I N E S =============*/

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

/**************************************************** STRUCTURE DEFINITION *****************************/

	/*! \struct ADI_A2B_COMMON_CONFIG
	   Common configurations for master as well as slave node
    */
	typedef struct
	{
		/*! Down slot size */
		uint8  nDwnSlotSize;

		/*! Up slot size  */
		uint8  nUpSlotSize;

		/*! Floating point compression for upstream  */
		uint8	bUpstreamCompression;

		/*! Floating point compression for downstream  */
		uint8	bDwnstreamCompression;

		/*! Enable Upstream */
		uint8   bEnableUpstream;

		/*! Enable Downstream */
		uint8   bEnableDwnstream;

		/* Reduce Data Rate on A2B Bus*/
		uint8   bEnableReduceRate;

		/*! System level reduced rate factor */
		uint8   nSysRateDivFactor;

		/*! Master I2C address - 7 bit */
		uint8   nMasterI2CAddr;

		/*! Bus I2C address - 7bit */
		uint8   nBusI2CAddr;


    }ADI_A2B_COMMON_CONFIG;



	/*! \struct PERIPHERAL_DEVICE_CONFIG
	   Peripheral device configuration
    */
	typedef struct
	{
		/*! I2C interface status  */
		uint8  bI2CInterfaceUse;

		/*! 7 bit I2C address */
		uint8  nI2Caddr;

		/*! Device type -audio source/sink/host  */
		uint8  eDeviceType;

		/*! Tx0 Pin in use */
		uint8  bUseTx0;

		/*! Rx0 Pin in use */
		uint8  bUseRx0;

		/*! Tx1 Pin in use */
		uint8  bUseTx1;

		/*! Rx1 Pin in use */
		uint8  bUseRx1;

		/*! No of Tx0 channels  */
		uint8  nChTx0;

		/*! No of Rx0 channels  */
		uint8  nChRx0;

		/*! No of Tx1 channels  */
		uint8  nChTx1;

		/*! No of Rx1 channels  */
		uint8  nChRx1;

		/*! Enable Upstream */
		uint8  nNumPeriConfigUnit;

		/*! Pointer to peripheral configuration unit */
		ADI_A2B_PERI_CONFIG_UNIT* paPeriConfigUnit;

    }A2B_PERIPHERAL_DEVICE_CONFIG;


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
		A2B_PERIPHERAL_DEVICE_CONFIG  *apPeriConfig[ADI_A2B_MAX_DEVICES_PER_NODE];

	}ADI_A2B_NETWORK_CONFIG;


  /*! \struct A2B_NODE_AUTHENTICATION
	Node authentication settings (common for master & slave )
	*/
	typedef struct
	{
		/*! AD2410 manufacturer ID */
		uint8  nVendorID;

		/*! AD2410 Silicon version */
		uint8  nVersionID;

		/*! AD2410 product ID */
		uint8  nProductID;

		/*! AD2410 capability */
		uint8  nCapability;

	}A2B_NODE_AUTHENTICATION;

	/*! \struct A2B_CONFIG_CONTROL
	Basic control settings for slave node
	*/
	typedef struct
	{
		/*! I2C interface frequency */
		uint8  nI2CFrequency;

		/*! Response cycles  */
		uint8  nRespCycle;

		/*! Expected super/audio frame rate */
		uint8 nSuperFrameRate;

		/*! Number of broadcast slots */
		uint8 nBroadCastSlots;

		/*! Local down slots */
		uint8 nLocalDwnSlotsConsume ;

		/*! Local Up slots */
		uint8 nLocalUpSlotsContribute ;

		/*! Pass up slots */
		uint8 nPassUpSlots ;

		/*! Pass down slots */
		uint8 nPassDwnSlots ;

		/*! Enable Down slot consume through mask */
		uint8 bUseDwnslotConsumeMasks ;

		/* Number of slots for contribution */
		uint8 nSlotsforDwnstrmContribute ;

		/*! Number of Upslots consumed  */
		uint8 nLocalUpSlotsConsume ;

		/* Offset from the RX Buffer for downstream contribution */
		uint8 nOffsetDwnstrmContribute ;

		/* Offset from the RX Buffer for Upstream contribution */
		uint8 nOffsetUpstrmContribute ;

		/*! Downstream slots consume mask */
		uint8 anDwnstreamConsumeSlots[32] ;

		/*! Upstream slots consume mask */
		uint8 anUpstreamConsumeSlots[32] ;


	}A2B_SLAVE_CONFIG_CONTROL;


	/*! \struct A2B_MASTER_CONFIG_CONTROL
	Basic control settings for master
	*/
	typedef struct 
	{
		/*! Early acknowledge for I2C read/write */
		uint8  bI2CEarlyAck;

		/*! Response cycles  */
		uint8  nRespCycle;

		/*! PLL time base -SYNC or BCLK */
		uint8 nPLLTimeBase;

		/*! BCLK rate - 12.288MHz; 24.576MHz and 49.152MHz  */
		uint8 nBCLKRate;

		/*! Pass up slots */
		uint8 nPassUpSlots ;

		/*! Upstream slots consume mask */
		uint8 nPassDwnSlots ;

	}A2B_MASTER_CONFIG_CONTROL;

	    /*! \struct A2B_SLAVE_I2S_RATE_CONFIG
	I2S rate settings for AD24xx
	*/
	 typedef struct  
	{
		/*! I2S sampling rate  */
		uint8 nSamplingRate;

		/*! Reduce / re-transmit higher frequency samples  */
		uint8 bReduce;

		/*! Share A2B bus slots for reduced sampling */
		uint8 bShareBusSlot;

		/*! BCLK as a factor of SYNC/SFF for reduced rates */
		uint8 nRBCLKRate;

		/*! Reduced rate sync offset */
		uint8 nRROffset;

		/*! Enable RR valid bit in LSB */
		uint8 bRRValidBitLSB;

		/*! Enable Valid RR bit in Extra bit */
		uint8 bRRValidBitExtraBit;

		/*! Enable Valid RR bit in Extra Channel */
		uint8 bRRValidBitExtraCh;

		/*! Enable Reduced rate strobe in ADR1/IO1 */
		uint8 bRRStrobe;

		/*! Strobe direction High or Low  */
		uint8  bRRStrobeDirection;

	}A2B_SLAVE_I2S_RATE_CONFIG;

	/*! \struct A2B_SLAVE_I2S_RATE_CONFIG
	I2S rate settings for AD24xx
	*/
	typedef struct 
	{
		/*! Enable RR valid bit in LSB read/write */
		uint8  bRRValidBitLSB;

		/*! Enable Valid RR bit in Extra bit  */
		uint8  bRRValidBitExtraBit;

		/*! Enable Valid RR bit in Extra Channel */
		uint8 bRRValidBitExtraCh;

		/*! Enable Reduced rate strobe in ADR1/IO1  */
		uint8 bRRStrobe;

		/*! Strobe direction High or Low */
		uint8 bRRStrobeDirection;

	}A2B_MASTER_I2S_RATE_CONFIG;

	/*! \struct A2B_SLAVE_I2S_SETTINGS
	I2S interface settings for slave AD2410
	*/
	 typedef struct  
	{
		/*! TDM mode  */
		uint8 nTDMMode;

		/*! TDM channel size  */
		uint8 nTDMChSize;

		/*! SYNC mode- Pulse/50% duty cycle */
		uint8 nSyncMode;

		/*! SYNC Polarity- Rising/Falling edge */
		uint8 nSyncPolarity;

		/*! Early frame sync status */
		uint8 bEarlySync;

		/*! SYNC offset with Super frame */
		uint8 nSyncOffset;

		/*! DTXn change BCLK edge */
		uint8 nBclkTxPolarity;

		/*! DRXn sampling BCLK edge */
		uint8 nBclkRxPolarity;

		/*! Interleave slots between TX pins  */
		uint8 bTXInterleave;

		/*! Interleave slots between RX pins  */
		uint8 bRXInterleave;

		A2B_SLAVE_I2S_RATE_CONFIG sI2SRateConfig;

		/*! Codec clock rate - applicable only for AD241x  */
		uint8  nCodecClkRate;

	}A2B_SLAVE_I2S_SETTINGS;

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


	/*! \struct A2B_PDM_SETTINGS
	PDM Settings( Only for slaves )
	*/
	typedef struct
	{
		/*! Number of PDM0 slots */
		uint8 nNumSlotsPDM0;

		/*! Number of PDM1 slots */
		uint8 nNumSlotsPDM1;

		/*! Use High Pass Filter    */
		uint8 bHPFUse;

		/*! PDM rate for AD242x */
		uint8 nPDMRate;

		/*! Filter Cut-off frequency */
		uint8 nHPFCutOff;

	}A2B_SLAVE_PDM_SETTINGS;

	/*! \struct A2B_SLAVE_PIN_MUX_SETTINGS
	GPIO pin multiplication status
	*/
	typedef struct
	{
		/*! GPIO 0 Pin multiplexing */
		uint8 bGPIO0PinUsage;

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

	}A2B_SLAVE_PIN_MUX_SETTINGS;


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



	/*! \struct A2B_SLAVE_OUTPIN_CONFIG
	GPIO output pin configuration
	*/
	typedef struct 
	{
		/*! Data value for GPIO 0 output pin  */
		uint8 bGPIO0Val;

		/*! Data value for GPIO 1 output pin  */
		uint8 bGPIO1Val;

		/*! Data value for GPIO 2 output pin  */
		uint8 bGPIO2Val;

		/*! Data value for GPIO 3 output pin  */
		uint8 bGPIO3Val;

		/*! Data value for GPIO 4 output pin  */
		uint8 bGPIO4Val;

		/*! Data value for GPIO 5 output pin  */
		uint8 bGPIO5Val;

		/*! Data value for GPIO 6 output pin  */
		uint8 bGPIO6Val;

		/*! Data value for GPIO 7 output pin  */
		uint8 bGPIO7Val;

	    }A2B_SLAVE_OUTPIN_CONFIG;


	/*! \struct A2B_SLAVE_INPUTPIN_INTERRUPT_CONFIG
	GPIO input pin settings for slave node
	*/
	typedef struct
	{
		/*! Enable GPIO 0 Input pin interrupt  */
		uint8 bGPIO0Interrupt;

		/*! Interrupt polarity - GPIO 0 Input pin  */
		uint8 bGPIO0IntPolarity;

		/*! Enable GPIO 1 Input pin interrupt  */
		uint8 bGPIO1Interrupt;

		/*! Interrupt polarity -  GPIO 1 Input pin  */
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

		/*! Enable GPIO 6 Input pin interrupt  */
		uint8 bGPIO6IntPolarity;

		/*! Enable GPIO 7 Input pin interrupt  */
		uint8 bGPIO7Interrupt;

		/*! Interrupt polarity - GPIO 7 Input pin  */
		uint8 bGPIO7IntPolarity;

	}A2B_SLAVE_INPUTPIN_INTERRUPT_CONFIG;



	/*! \struct A2B_OUTPIN_VALUE
	GPIO output pin configuration for master 
	*/
	typedef struct 
	{
		/*! Data value for GPIO 1 output pin  */
		uint8 bGPIO1Val;

		/*! Data value for GPIO 2 output pin  */
		uint8 bGPIO2Val;

		/*! Data value for GPIO 3 output pin  */
		uint8 bGPIO3Val;

		/*! Data value for GPIO 4 output pin  */
		uint8 bGPIO4Val;

		/*! Data value for GPIO 5 output pin  */
		uint8 bGPIO5Val;

		/*! Data value for GPIO 6 output pin  */
		uint8 bGPIO6Val;

		/*! Data value for GPIO 7 output pin  */
		uint8 bGPIO7Val;

		}A2B_MASTER_OUTPIN_CONFIG;


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


	/*! \struct A2B_SLAVE_GPIO_SETTINGS
	Slave GPIO Configuration
	*/
	typedef struct
	{
		/*! GPIO Pin multiplex Settings */
		A2B_SLAVE_PIN_MUX_SETTINGS  sPinMuxSettings;

		/*! Input Pin interrupt configuration */
		A2B_SLAVE_INPUTPIN_INTERRUPT_CONFIG   	sPinIntConfig;

		/*! Input Pin interrupt Settings */
		A2B_SLAVE_OUTPIN_CONFIG					sOutPinVal;

		/*! Digital Pin drive strength */
		uint8 bHighDriveStrength;

		/*! IRQ Pin Invert */
		uint8 bIRQInv;

		/*! Enable tristate when inactive */
		uint8 bIRQTriState;

	}A2B_SLAVE_GPIO_SETTINGS;


	/*! \struct A2B_MASTER_GPIO_SETTINGS
	Master GPIO Configuration
	*/
	typedef struct 
	{
		/*! GPIO Pin multiplex Settings */
		A2B_MASTER_PIN_MUX_SETTINGS  sPinMuxSettings;

		/*! Input Pin interrupt configuration */
		A2B_MASTER_INPUTPIN_INTERRUPT_CONFIG   	    sPinIntConfig;

		/*! Input Pin interrupt Settings */
		A2B_MASTER_OUTPIN_CONFIG					sOutPinVal;

		/*! Digital Pin drive strength */
		uint8 bHighDriveStrength;

		/*! IRQ Pin Invert */
		uint8 bIRQInv;

		/*! Enable tristate when inactive */
		uint8 bIRQTriState;

	}A2B_MASTER_GPIO_SETTINGS;


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

	/*! \struct A2B_MASTER_INTERRUPT_CONFIG
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

		/*! Report I2C failure error  */
		uint8 bReportI2CErr;

		/*! Report Discovery Completion */
		uint8 bDiscComplete;

		/*! Report Interrupt frame error */
		uint8 bIntFrameCRCErr;

		/*! Report Interrupt requests  */
		uint8 bSlaveIntReq;

	}A2B_MASTER_INTERRUPT_SETTINGS;


	/*! \struct A2B_GPIOD_PIN_CONFIG 
	AD242x GPIOD settings
	*/
	typedef struct
	{
		/*! Enable/Disable GPIO over distance */
		uint8 	bGPIODistance;

		/*! Enable/Disable  */
		uint8 	bGPIOSignalInv;

		/*! Bus port masks  */
		uint8   abBusPortMask[8];

	}A2B_GPIOD_PIN_CONFIG;


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


	/*! \struct A2B_AD242x_CLKOUT_CONFIG 
	Clock out config for AD242x
	*/
	typedef struct
	{
		/*! Enable Clock1 inversion  */
		uint8 	bClk1Inv;

		/*! Clk1 pre-division */
		uint8 	bClk1PreDiv; 

		/*! CLK1 division factor */
		uint8 	bClk1Div;

		/*! Enable Clock 2 inversion  */
		uint8 	bClk2Inv;

		/*! Clk2 pre-division */
		uint8 	bClk2PreDiv;

		/*! CLK2 division factor */
		uint8   bClk2Div;

	}A2B_AD242x_CLKOUT_CONFIG;


	/*! \struct A2B_SLAVE_REGISTER_SETTINGS
	AD2410 Interrupt configuration
	*/
	typedef struct
	{
		/*! Switch control register */
		uint8 	nSWCTL;

		/*! Test mode register */
		uint8 	nTESTMODE;

		/*! Error count register */
		uint8 	nBECCTL; 

		/*! Error management register  */
		uint8 	nERRMGMT;

		/*! PLL control register  */
		uint8   nPLLCTL;

		/*! I2S test register  */
		uint8 	nI2STEST;

		/*! Generate error register  */
		uint8 	nGENERR;

		/*! Raise interrupt register  */
		uint8 	nRAISE;

		/*! Bus monitor configuration */
		uint8 	nBMMCFG;

		/*! Clock sustain configuration */
		uint8 	nSUSCFG;

		/*! Mailbox 0 control */
		uint8 	nMBOX0CTL;

		/*! Mailbox 1 control */
		uint8   nMBOX1CTL;

	}A2B_SLAVE_REGISTER_SETTINGS;


	/*! \struct A2B_SLAVE_REGISTER_SETTINGS
	AD2410 Interrupt configuration
	*/
	typedef struct
	{
		/*! Switch control register */
		uint8 	nSWCTL;

		/*! Error management register  */
		uint8 	nPDMCTL;

		/*! Test mode register */
		uint8 	nTESTMODE;

		/*! Error count register */
		uint8 	nBECCTL; 

		/*! Error management register  */
		uint8 	nERRMGMT;

		/*! I2S test register  */
		uint8 	nI2STEST;

		/*! Generate error  */
		uint8 	nGENERR;

		/*! Raise interrupt register  */
		uint8 	nRAISE;

		/*! Bus monitor configuration */
		uint8   nBMMCFG;

	}A2B_MASTER_REGISTER_SETTINGS;


/************************************************* NCD DEFINITION **********************************/
	/*! \struct ADI_A2B_SLAVE_NCD
	   Slave configuration
    */
	typedef struct
	{
		/*! Slave node ID */
		uint16  					nNodeID;

		/*! Connected Node ID - upstream node */
		uint16  					nSrcNodeID;

     /*! Transceiver part number   */
 		ADI_A2B_PARTNUM                     ePartNum;

		/*! Auto-Configuration Enabled */
		uint16  					bEnableAutoConfig;

		/*! Authentication settings  */
		A2B_NODE_AUTHENTICATION 	sAuthSettings;

		/*! Basic configuration & control  */
		A2B_SLAVE_CONFIG_CONTROL	sConfigCtrlSettings;

		/*! A2B I2S Settings */
		A2B_SLAVE_I2S_SETTINGS  	sI2SSettings;

		/*! PDM settings  */
		A2B_SLAVE_PDM_SETTINGS		sPDMSettings;

		/*! GPIO settings  */
		A2B_SLAVE_GPIO_SETTINGS    	sGPIOSettings;

		/*! Interrupt configuration */
		A2B_SLAVE_INTERRUPT_SETTINGS  sInterruptSettings;

		/*! AD242x clock out config */
		A2B_AD242x_CLKOUT_CONFIG  	sClkOutSettings;

		/*! AD242x clock out config */
		A2B_SLAVE_GPIOD_SETTINGS  	sGPIODSettings;

		/*! AD2410 Register configuration - for advanced use */
		A2B_SLAVE_REGISTER_SETTINGS sRegSettings;

		/*! Number of Peripheral device  */
		uint8  						 nNumPeriDevice;

		/*! Array of pointers  */
		A2B_PERIPHERAL_DEVICE_CONFIG  *apPeriConfig[ADI_A2B_MAX_DEVICES_PER_NODE];

    }ADI_A2B_SLAVE_NCD;



	/*! \struct ADI_A2B_MASTER_NCD
	   Master configuration 
    */
	typedef struct
	{
		/*! Slave node ID */
		uint16  						nNodeID;

		/*! Connected Node ID - upstream node */
		uint16  						nSrcNodeID;

     /*! Transceiver part number   */
 		ADI_A2B_PARTNUM                     ePartNum;

		/*! Expected authentication settings  */
		A2B_NODE_AUTHENTICATION 		sAuthSettings;

		/*! Basic configuration & control  */
		A2B_MASTER_CONFIG_CONTROL		sConfigCtrlSettings;

		/*! A2B I2S Settings */
		A2B_MASTER_I2S_SETTINGS  		sI2SSettings;

		/*! GPIO settings  */
		A2B_MASTER_GPIO_SETTINGS    	sGPIOSettings;

		/*! Interrupt configuration */
		A2B_MASTER_INTERRUPT_SETTINGS  	sInterruptSettings;

		/*! AD242x clock out config */
		A2B_AD242x_CLKOUT_CONFIG  	sClkOutSettings;

		/*! AD242x clock out config */
		A2B_MASTER_GPIOD_SETTINGS  	sGPIODSettings;

		/*! AD2410 Register configuration - for advanced use */
		A2B_MASTER_REGISTER_SETTINGS 	sRegSettings;

    }ADI_A2B_MASTER_NCD;

/***************************************** BCD & CHIAN DEFINITION ************************************/

    /*! \struct ADI_A2B_MASTER_SLAVE_CHAIN_CONFIG  
	   Configuration for one master and associated slaves
    */
	typedef struct
	{
		/*! Slave node ID */
		uint8  nNumSlaveNode;

		/*! Pointer to master node configuration  */
		ADI_A2B_MASTER_NCD* pMasterConfig;

		/*! Slave node configuration array */
		ADI_A2B_SLAVE_NCD *apSlaveConfig[ADI_A2B_MAX_SLAVE_NODES_PER_MASTER];

	    /*! Common network configuration for one daisy chain */
		ADI_A2B_COMMON_CONFIG sCommonSetting;

	}ADI_A2B_MASTER_SLAVE_CONFIG;


	/*! \struct ADI_A2B_BCD  
	   Bus configuration Data for entire network
    */
	typedef struct
	{
		/*! Number of master node in network */
		uint8  nNumMasterNode;

		/*! Pointer to master-slave chains */
		ADI_A2B_MASTER_SLAVE_CONFIG *apNetworkconfig[ADI_A2B_MAX_MASTER_NODES];

	    /*! Network Configuration */
		ADI_A2B_NETWORK_CONFIG sTargetProperties; 

	}ADI_A2B_BCD;



/*============= D A T A T Y P E S=============*/


/*============= E X T E R N A L S ============*/

#endif /*_ADI_A2B_BUSCONFIG_H_*/

extern ADI_A2B_BCD sBusDescription;
/**
 @}
*/
/**
 @}
*/


/*
**
** EOF: adi_a2b_busconfig.h
**
*/
