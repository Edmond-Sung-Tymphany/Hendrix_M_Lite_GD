//#include "adi_a2b_busconfig.h" 


#define A2B_CHIP_PART                   ADI_A2B_AD2425
#define A2B_AD242x_PART		(0x25u)


const ADI_A2B_MASTER_NCD sMasterNode0= 
{
	/*! Node ID */
	.nNodeID			= 0x0u ,

	/*! Source node ID : '0xFF' indicates Target processor */
	.nSrcNodeID			= 0xFFu ,

	/*! Transceiver part number */
	.ePartNum			= ADI_A2B_AD2425 ,

	.sAuthSettings =
	{
		/*! Vendor ID - ADI */
		.nVendorID			= 0xADu ,

		/*! Silicon version  - ADI */
		.nVersionID			= 0x2u ,

		/*! Product ID - AD24xx */
		.nProductID			= 0x25u ,

		/*! Interface Capability - I2C */
		.nCapability		= 0x1u ,
	},

	.sI2SSettings =
	{
		/*! TDM mode  */
		.nTDMMode			= A2B_TDM2 ,

		/*! TDM channel size  */
		.nTDMChSize			= A2B_32BIT_TDM ,

		/*! SYNC mode - Pulse/50% duty cycle   */
		.nSyncMode			= A2B_ALTERNATE_SYNC ,

		/*! SYNC Polarity- Rising/Falling edge */
		.nSyncPolarity		= RAISING_EDGE ,

		/*! Early frame sync status */
		.bEarlySync			= DISABLED ,

		/*! DTXn change BCLK edge */
		.nBclkTxPolarity	= RAISING_EDGE ,

		/*! DRXn Sampling BCLK edge */
		.nBclkRxPolarity	= RAISING_EDGE ,

		/*! Interleave slots between Tx pins  */
		.bTXInterleave		= DISABLED ,

		/*! Interleave slots between Rx pins  */
		.bRXInterleave		= DISABLED ,

		/*! Transmit Channel offset in TDM - 0 to 63 */
		.nTxOffset			= 0x0u ,

		/*! Receive channel offset in TDM - 0 to 63 */
		.nRxOffset			= 0x0u ,

		/*! Tx Pin TriState before driving TDM slots */
		.bTriStateBeforeTx	= DISABLED ,

		/*! Tx Pin Tristate after driving TDM slots */
		.bTriStateAfterTx	= DISABLED ,

		.sI2SRateConfig =
		{
			/*! Enable RR valid bit in LSB */
			.bRRValidBitLSB	= DISABLED ,

			/*! Enable Valid RR bit in Extra bit */
			.bRRValidBitExtraBit	= DISABLED ,

			/*! Enable Reduced rate strobe in ADR1/IO1 */
			.bRRStrobe	= DISABLED ,

			/*! Strobe direction High or Low */
			.bRRStrobeDirection	= A2B_LOW ,

		},
	},
	.sConfigCtrlSettings =
	{
		/*! Early acknowledge for I2C read/write */
		.bI2CEarlyAck		= DISABLED ,

		/*! Response cycles  */
		.nRespCycle			= 0x77u ,

		/*! Pass up slots */
		.nPassUpSlots		= 0x2u ,

		/*! Pass down slots */
		.nPassDwnSlots		= 0x4u ,

	},
	.sGPIOSettings =
	{
		.sPinMuxSettings =
		{
			/*! GPIO 1 Pin multiplexing */
			.bGPIO1PinUsage	= A2B_GPIO_1_DISABLE ,

			/*! GPIO 2 Pin multiplexing */
			.bGPIO2PinUsage	= A2B_GPIO_2_DISABLE ,

			/*! GPIO 3 Pin multiplexing */
			.bGPIO3PinUsage	= A2B_GPIO_3_AS_DTX0 ,

			/*! GPIO 4 Pin multiplexing */
			.bGPIO4PinUsage	= A2B_GPIO_4_DISABLE ,

			/*! GPIO 5 Pin multiplexing */
			.bGPIO5PinUsage	= A2B_GPIO_5_AS_DRX0 ,

			/*! GPIO 6 Pin multiplexing */
			.bGPIO6PinUsage	= A2B_GPIO_6_DISABLE ,
			/*! GPIO 7 Pin multiplexing */
			.bGPIO7PinUsage	= A2B_GPIO_7_DISABLE ,

		},
		.sPinIntConfig =
		{
			/*! Enable GPIO 1 Input pin interrupt  */
			.bGPIO1Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 1 Input pin  */
			.bGPIO1IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 2 Input pin interrupt  */
			.bGPIO2Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 2 Input pin  */
			.bGPIO2IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 3 Input pin interrupt  */
			.bGPIO3Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 3 Input pin  */
			.bGPIO3IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 4 Input pin interrupt  */
			.bGPIO4Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 4 Input pin  */
			.bGPIO4IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 5 Input pin interrupt  */
			.bGPIO5Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 5 Input pin  */
			.bGPIO5IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 6 Input pin interrupt  */
			.bGPIO6Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 6 Input pin  */
			.bGPIO6IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 7 Input pin interrupt  */
			.bGPIO7Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 7 Input pin  */
			.bGPIO7IntPolarity	= RAISING_EDGE ,

		},
		.sOutPinVal =
		{
			/*! Data value for GPIO 1 output pin  */
			.bGPIO1Val = A2B_LOW ,

			/*! Data value for GPIO 2 output pin  */
			.bGPIO2Val = A2B_LOW ,

			/*! Data value for GPIO 3 output pin  */
			.bGPIO3Val = A2B_LOW ,

			/*! Data value for GPIO 4 output pin  */
			.bGPIO4Val = A2B_LOW ,

			/*! Data value for GPIO 5 output pin  */
			.bGPIO5Val = A2B_LOW ,

			/*! Data value for GPIO 6 output pin  */
			.bGPIO6Val = A2B_LOW ,

			/*! Data value for GPIO 7 output pin  */
			.bGPIO7Val = A2B_LOW ,

		},

		/*! Digital Pin drive strength */
		.bHighDriveStrength = DISABLED ,

		/*! IRQ Pin Invert */
		.bIRQInv = DISABLED ,

		/*! Enable tristate when inactive */
		.bIRQTriState = DISABLED ,

	},

	.sInterruptSettings =
	{
		/*! Report Header count error  */
		.bReportHDCNTErr		= ENABLED ,

		/*! Report Data decoding error  */
		.bReportDDErr			= ENABLED ,

		/*! Report Data CRC error  */
		.bReportCRCErr			= ENABLED ,

		/*! Report Data Parity error  */
		.bReportDataParityErr	= ENABLED ,

		/*! Report Data Bus Power error  */
		.bReportPwrErr			= ENABLED ,

		/*! Report bit error count overflow error  */
		.bReportErrCntOverFlow	= ENABLED ,

		/*! Report SRF miss error  */
		.bReportSRFMissErr	= ENABLED ,

		/*! Report GPIO  1 Interrupt */
		.bReportGPIO1			= DISABLED ,

		/*! Report GPIO  2 Interrupt */
		.bReportGPIO2			= DISABLED ,

		/*! Report GPIO  3 Interrupt */
		.bReportGPIO3			= ENABLED ,

		/*! Report GPIO  4 Interrupt */
		.bReportGPIO4			= ENABLED ,

		/*! Report GPIO  5 Interrupt */
		.bReportGPIO5			= ENABLED ,

		/*! Report GPIO  6 Interrupt */
		.bReportGPIO6			= ENABLED ,

		/*! Report GPIO  7 Interrupt */
		.bReportGPIO7			= DISABLED ,

		/*! Report I2C failure error  */
		.bReportI2CErr			= ENABLED ,

		/*! Report Discovery Completion */
		.bDiscComplete			= ENABLED ,

		/*! Report Interrupt frame error */
		.bIntFrameCRCErr		= ENABLED ,

		/*! Report Interrupt requests  */
		.bSlaveIntReq			= ENABLED ,

	},

	.sGPIODSettings =
	{
		.sGPIOD1Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},

		.sGPIOD2Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},

		.sGPIOD3Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},

		.sGPIOD4Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},

		.sGPIOD5Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},

		.sGPIOD6Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},

		.sGPIOD7Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},

	},

	.sClkOutSettings =
	{
		/*! Enable Clock1 inversion */
		.bClk1Inv			= DISABLED ,

		/*! Clk1 pre-division */
		.bClk1PreDiv			= A2B_CLKOUT_PREDIV_02,

		/*! Clk1 division */
		.bClk1Div			= A2B_CLKOUT_DIV_2,

		/*! Enable Clock2 inversion */
		.bClk2Inv			= DISABLED ,

		/*! Clk2 pre-division */
		.bClk2PreDiv			= A2B_CLKOUT_PREDIV_02,

		/*! Clk2 division */
		.bClk2Div			= A2B_CLKOUT_DIV_2,

	},

	.sRegSettings =
	{
		/*! Switch control register */
		.nSWCTL		= 0x1u ,

		/*! Error management register  */
		.nPDMCTL	= 0x0u ,

		/*! Test mode register */
		.nTESTMODE	= 0x0u ,

		/*! Error control register */
		.nBECCTL	= 0x0u ,

		/*! Error management register  */
		.nERRMGMT	= 0x0u ,

		/*! I2S test register  */
		.nI2STEST	= 0x0u ,

		/*! Generate error  */
		.nGENERR	= 0x0u ,

		/*! Raise interrupt register */
		.nRAISE		= 0x0u ,

		/*! Bus monitor configuration  */
		.nBMMCFG	= 0x0u ,

	}

};

const ADI_A2B_SLAVE_NCD sChain0_SlaveNode0= 
{
	/* Node ID  */
	.nNodeID			= 0x0u ,

	/* Source node ID  */
	.nSrcNodeID			= 0x0u ,

	/*! Transceiver part number */
	.ePartNum			= ADI_A2B_AD2425 ,

	/* Auto-configure Node Enabled */
	.bEnableAutoConfig			= DISABLED ,

	.sAuthSettings =
	{
		/*! Vendor ID - ADI */
		.nVendorID			= 0xADu ,

		/*! Silicon version  */
		.nVersionID			= 0x2u ,

		/*! PRODUCT ID -AD2410 */
		.nProductID			= 0x25u ,

		/*! Expected capablity - I2C */
		.nCapability		= 0x1u ,
	},

	.sI2SSettings =
	{
		/*! TDM mode  */
		.nTDMMode			= A2B_TDM2 ,

		/*! TDM channel size  */
		.nTDMChSize			= A2B_32BIT_TDM ,

		/*! SYNC mode - Pulse/50% duty cycle   */
		.nSyncMode			= A2B_ALTERNATE_SYNC ,

		/*! SYNC Polarity- Rising/Falling edge */
		.nSyncPolarity		= RAISING_EDGE ,

		/*! Early frame sync status */
		.bEarlySync			= DISABLED ,

		/*! SYNC offset with Super frame */
		.nSyncOffset		= 0x0 ,

		/*! DTXn change BCLK edge */
		.nBclkTxPolarity	= RAISING_EDGE ,

		/*! DRXn sampling BCLK edge */
		.nBclkRxPolarity	= RAISING_EDGE ,

		/*! Interleave slots between Tx pins  */
		.bTXInterleave		= DISABLED ,

		/*! Interleave slots between Rx pins  */
		.bRXInterleave		= DISABLED ,

		.sI2SRateConfig =
		{
			/* Sample rate */
			.nSamplingRate		= A2B_SAMPLERATE_48kHz ,

			/*! Reduce / re-transmit higher frequency samples  */
		.bReduce			= DISABLED ,

			/*! Share A2B bus slots for reduced sampling */
			.bShareBusSlot	= DISABLED ,

			/*! Enable Valid RR bit in Extra bit */
			.bRRValidBitLSB 	= DISABLED ,

			/*! Enable Valid RR bit in Extra Channel */
			.bRRValidBitExtraBit	= DISABLED ,

			/*! Enable Reduced rate strobe in ADR1/IO1 */
			.bRRStrobe	= DISABLED ,

			/*! Strobe direction High or Low */
			.bRRStrobeDirection	= A2B_LOW ,

		},
	},

	.sPDMSettings =
	{
		/*! Number of PDM0 slots  */
		.nNumSlotsPDM0		= A2B_PDM0SLOTS_1 ,

		/*! Number of PDM1 slots  */
		.nNumSlotsPDM1		= A2B_PDM1SLOTS_2 ,

		/*! Use High Pass Filter  */
		.bHPFUse		= ENABLED ,

		/*! PDM rate  */
		.nPDMRate		= A2B_PDM_RATE_SFF ,

	},

	.sConfigCtrlSettings =
	{
		/*! I2C interface frequency */
		.nI2CFrequency		= A2B_I2C_400kHz ,

		/*! Response cycles  */
		.nRespCycle			= 0x77u ,

		/*! Expected super/audio frame rate */
		.nSuperFrameRate	= A2B_SFF_RATE_48_0kHz ,

		/*! Number of broadcast slots */
		.nBroadCastSlots	= 0x2u ,

		/*! Local down slots */
		.nLocalDwnSlotsConsume		= 0x0u ,

		/*! Local Up slots */
		.nLocalUpSlotsContribute		= 0x2u ,

		/*! Pass up slots */
		.nPassUpSlots		= 0x0u ,

		/*! Pass down slots */
		.nPassDwnSlots		= 0x2u ,

		/*! Number of slots for contribution */
		.nSlotsforDwnstrmContribute		= 0x0u ,

		/*! Number of Upslots consumed */
		.nLocalUpSlotsConsume		= 0x0u ,

		/*! Array of downstream  masks */ 
		.anUpstreamConsumeSlots = { SLOT_0_DISABLED, SLOT_1_DISABLED, SLOT_2_DISABLED, SLOT_3_DISABLED, SLOT_4_DISABLED, SLOT_5_DISABLED, SLOT_6_DISABLED, SLOT_7_DISABLED, 
 									 SLOT_8_DISABLED, SLOT_9_DISABLED, SLOT_10_DISABLED, SLOT_11_DISABLED, SLOT_12_DISABLED, SLOT_13_DISABLED, SLOT_14_DISABLED, SLOT_15_DISABLED, 
 									 SLOT_16_DISABLED, SLOT_17_DISABLED, SLOT_18_DISABLED, SLOT_19_DISABLED, SLOT_20_DISABLED, SLOT_21_DISABLED, SLOT_22_DISABLED, SLOT_23_DISABLED, 
 									 SLOT_24_DISABLED, SLOT_25_DISABLED, SLOT_26_DISABLED, SLOT_27_DISABLED, SLOT_28_DISABLED, SLOT_29_DISABLED, SLOT_30_DISABLED, SLOT_31_DISABLED},
		/*! Offset from the RX Buffer for downstream contribution */
		.nOffsetDwnstrmContribute		= 0x0u ,

		/*! Enable Down slot consume through mask */
		.bUseDwnslotConsumeMasks		= DISABLED ,

		/*! Array of downstream  masks */ 
		.anDwnstreamConsumeSlots = { SLOT_0_DISABLED, SLOT_1_DISABLED, SLOT_2_DISABLED, SLOT_3_DISABLED, SLOT_4_DISABLED, SLOT_5_DISABLED, SLOT_6_DISABLED, SLOT_7_DISABLED, 
 									 SLOT_8_DISABLED, SLOT_9_DISABLED, SLOT_10_DISABLED, SLOT_11_DISABLED, SLOT_12_DISABLED, SLOT_13_DISABLED, SLOT_14_DISABLED, SLOT_15_DISABLED, 
 									 SLOT_16_DISABLED, SLOT_17_DISABLED, SLOT_18_DISABLED, SLOT_19_DISABLED, SLOT_20_DISABLED, SLOT_21_DISABLED, SLOT_22_DISABLED, SLOT_23_DISABLED, 
 									 SLOT_24_DISABLED, SLOT_25_DISABLED, SLOT_26_DISABLED, SLOT_27_DISABLED, SLOT_28_DISABLED, SLOT_29_DISABLED, SLOT_30_DISABLED, SLOT_31_DISABLED},
		/*! Offset from the RX Buffer for Upstream contribution*/
		.nOffsetUpstrmContribute		= 0x0u ,

	},
	.sGPIOSettings =
	{
		.sPinMuxSettings =
		{
			/*! GPIO 0 Pin multiplexing */
			.bGPIO0PinUsage	= A2B_GPIO_0_DISABLE ,

			/*! GPIO 1 Pin multiplexing */
			.bGPIO1PinUsage	= A2B_GPIO_1_DISABLE ,

			/*! GPIO 2 Pin multiplexing */
			.bGPIO2PinUsage	= A2B_GPIO_2_AS_CLKOUT ,

			/*! GPIO 3 Pin multiplexing */
			.bGPIO3PinUsage	= A2B_GPIO_3_AS_DTX0 ,

			/*! GPIO 4 Pin multiplexing */
			.bGPIO4PinUsage	= A2B_GPIO_4_OUTPUT ,

			/*! GPIO 5 Pin multiplexing */
			.bGPIO5PinUsage	= A2B_GPIO_5_AS_DRX0 ,

			/*! GPIO 6 Pin multiplexing */
			.bGPIO6PinUsage	= A2B_GPIO_6_DISABLE ,

			/*! GPIO 7 Pin multiplexing */
			.bGPIO7PinUsage	= A2B_GPIO_7_DISABLE ,
		},
		.sPinIntConfig =
		{
			/*! Enable GPIO 0 Input pin interrupt  */
			.bGPIO0Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 0 Input pin  */
			.bGPIO0IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 1 Input pin interrupt  */
			.bGPIO1Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 1 Input pin  */
			.bGPIO1IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 2 Input pin interrupt  */
			.bGPIO2Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 2 Input pin  */
			.bGPIO2IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 3 Input pin interrupt  */
			.bGPIO3Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 3 Input pin  */
			.bGPIO3IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 4 Input pin interrupt  */
			.bGPIO4Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 4 Input pin  */
			.bGPIO4IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 5 Input pin interrupt  */
			.bGPIO5Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 5 Input pin  */
			.bGPIO5IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 6 Input pin interrupt  */
			.bGPIO6Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 6 Input pin  */
			.bGPIO6IntPolarity	= RAISING_EDGE ,

			/*! Enable GPIO 7 Input pin interrupt  */
			.bGPIO7Interrupt	= DISABLED ,

			/*! Interrupt polarity - GPIO 7 Input pin  */
			.bGPIO7IntPolarity	= RAISING_EDGE ,

		},
		.sOutPinVal =
		{
			/*! Data value for GPIO 0 output pin  */
			.bGPIO0Val = A2B_LOW ,

			/*! Data value for GPIO 1 output pin  */
			.bGPIO1Val = A2B_LOW ,

			/*! Data value for GPIO 2 output pin  */
			.bGPIO2Val = A2B_LOW ,

			/*! Data value for GPIO 3 output pin  */
			.bGPIO3Val = A2B_LOW ,

			/*! Data value for GPIO 4 output pin  */
			.bGPIO4Val = A2B_HIGH ,

			/*! Data value for GPIO 5 output pin  */
			.bGPIO5Val = A2B_LOW ,

			/*! Data value for GPIO 6 output pin  */
			.bGPIO6Val = A2B_LOW ,

			/*! Data value for GPIO 7 output pin  */
			.bGPIO7Val = A2B_LOW ,

		},

		/*! Digital Pin drive strength */
		.bHighDriveStrength = DISABLED ,

		/*! IRQ Pin Invert */
		.bIRQInv = DISABLED ,

		/*! Enable tristate when inactive */
		.bIRQTriState = DISABLED ,

	},

	.sInterruptSettings =
	{
		/*! Report Header count error  */
		.bReportHDCNTErr		= ENABLED ,

		/*! Report Data decoding error  */
		.bReportDDErr			= ENABLED ,

		/*! Report Data CRC error  */
		.bReportCRCErr			= ENABLED ,

		/*! Report Data Parity error  */
		.bReportDataParityErr	= ENABLED ,

		/*! Report Data Bus Power error  */
		.bReportPwrErr			= ENABLED ,

		/*! Report bit error count overflow error  */
		.bReportErrCntOverFlow	= ENABLED ,

		/*! Report SRF miss error  */
		.bReportSRFMissErr	= ENABLED ,

		/*! Report SRF crc error  */
		.bReportSRFCrcErr		= DISABLED ,

		/*! Report GPIO  0 Interrupt */
		.bReportGPIO0			= ENABLED ,

		/*! Report GPIO  1 Interrupt */
		.bReportGPIO1			= ENABLED ,

		/*! Report GPIO  2 Interrupt */
		.bReportGPIO2			= ENABLED ,

		/*! Report GPIO  3 Interrupt */
		.bReportGPIO3			= ENABLED ,

		/*! Report GPIO  4 Interrupt */
		.bReportGPIO4			= ENABLED ,

		/*! Report GPIO  5 Interrupt */
		.bReportGPIO5			= ENABLED ,

		/*! Report GPIO  6 Interrupt */
		.bReportGPIO6			= ENABLED ,

		/*! Report GPIO  7 Interrupt */
		.bReportGPIO7			= DISABLED ,

	},

	.sGPIODSettings =
	{
		.sGPIOD0Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},
		.sGPIOD1Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},
		.sGPIOD2Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},
		.sGPIOD3Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},
		.sGPIOD4Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},
		.sGPIOD5Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},
		.sGPIOD6Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},
		.sGPIOD7Config =
		{
			/*! Enable/Disable GPIO over distance   */
			.bGPIODistance	= DISABLED ,

			/*! Enable/Disable  */
			.bGPIOSignalInv	= DISABLED ,

			/*! Bus port masks */ 
			.abBusPortMask = { A2B_MASK_BUSFLAG_0, A2B_MASK_BUSFLAG_1, A2B_MASK_BUSFLAG_2, A2B_MASK_BUSFLAG_3, A2B_MASK_BUSFLAG_4, A2B_MASK_BUSFLAG_5, A2B_MASK_BUSFLAG_6, A2B_MASK_BUSFLAG_7},


		},
	},

	.sClkOutSettings =
	{
		/*! Enable Clock1 inversion */
		.bClk1Inv			= DISABLED ,

		/*! Clk1 pre-division */
		.bClk1PreDiv		= A2B_CLKOUT_PREDIV_02,

		/*! Clk1 division */
		.bClk1Div			= A2B_CLKOUT_DIV_2,

		/*! Enable Clock2 inversion */
		.bClk2Inv			= ENABLED ,

		/*! Clk2 pre-division */
		.bClk2PreDiv		= A2B_CLKOUT_PREDIV_02,

		/*! Clk2 division */
		.bClk2Div			= A2B_CLKOUT_DIV_4,

	},
	.sRegSettings =
	{
		/*! Switch control register */
		.nSWCTL		= 0x1u ,

		/*! Test mode register */
		.nTESTMODE	= 0x0u ,

		/*! Error control register */
		.nBECCTL	= 0x0u ,

		/*! Error management register  */
		.nERRMGMT	= 0x0u ,

		/*! I2S test register  */
		.nI2STEST	= 0x0u ,

		/*! Generate error  */
		.nGENERR	= 0x0u ,

		/*! Raise interrupt register */
		.nRAISE		= 0x0u ,

		/*! Clock sustain configuration   */
		.nSUSCFG	= 0x0u ,

		/*!  Mailbox 0 control */
		.nMBOX0CTL		= 0x0u ,

		/*!  Mailbox 1 control */
		.nMBOX1CTL		= 0x0u ,

	},

		 /* Number of peripheral devices connected  */
		.nNumPeriDevice = 2,

		 /*Array of peripheral configuration pointers */ 
		.apPeriConfig = { NULL_PTR	}

};

static A2B_PERIPHERAL_DEVICE_CONFIG  sChain0_Slave0_PeriConfig0=
{
	/* I2C interface status  */ 
	 .bI2CInterfaceUse = ENABLED, 

	/*7 bit I2C address */ 
	.nI2Caddr = 0x39<<1, 

	/* Device type -audio source/sink/host  */ 
	.eDeviceType = A2B_AUDIO_SOURCE_SINK, 

	/* Tx0 Pin in use */
	.bUseTx0 = ENABLED, 

	/* Rx0 Pin in use */
	.bUseRx0 = ENABLED, 

	/* Tx1 Pin in use */
	.bUseTx1 = DISABLED, 

	/* Rx1 Pin in use */
	.bUseRx1 = DISABLED, 

	/* No of Tx0 channels  */ 
	.nChTx0 = 2, 

	/* No of Rx0 channels  */ 
	.nChRx0 = 2, 

	/* No of Tx1 channels  */ 
	.nChTx1 = 0, 

	/* No of Rx1 channels  */ 
	.nChRx1 = 0, 

	/* Number of configuration units  */ 
	.nNumPeriConfigUnit = 35, 

	/* Pointer to configuration units  */ 
	//.paPeriConfigUnit =  &gaPeriCfg_1_39_Unit[0] 
 };

static A2B_PERIPHERAL_DEVICE_CONFIG  sChain0_Slave0_PeriConfig1=
{
	/* I2C interface status  */ 
	 .bI2CInterfaceUse = ENABLED, 

	/*7 bit I2C address */ 
	.nI2Caddr = 0x50<<1, 

	/* Device type -audio source/sink/host  */ 
	.eDeviceType = A2B_GENERIC_I2C_DEVICE, 

	/* Tx0 Pin in use */
	.bUseTx0 = DISABLED, 

	/* Rx0 Pin in use */
	.bUseRx0 = DISABLED, 

	/* Tx1 Pin in use */
	.bUseTx1 = DISABLED, 

	/* Rx1 Pin in use */
	.bUseRx1 = DISABLED, 

	/* No of Tx0 channels  */ 
	.nChTx0 = 0, 

	/* No of Rx0 channels  */ 
	.nChRx0 = 0, 

	/* No of Tx1 channels  */ 
	.nChTx1 = 0, 

	/* No of Rx1 channels  */ 
	.nChRx1 = 0, 

	/* Number of configuration units  */ 
	.nNumPeriConfigUnit = 0, 

	/* Pointer to configuration units  */ 
	.paPeriConfigUnit = NULL_PTR 
 };




const ADI_A2B_COMMON_CONFIG sCommonSetting=
{
		.nMasterI2CAddr = 0xd0,                 /*! Master I2C address - 7 bit */ 
		.nBusI2CAddr = 0xd2,                    /*! Bus I2C address - 7 bit */ 
		.bEnableDwnstream = ENABLED,            /*! Enable down stream */ 
		.bEnableUpstream = ENABLED,             /*! Enable down stream */
		.nDwnSlotSize = A2B_DNSLOT_SIZE_24,     /*! Down slot size */ 
		.nUpSlotSize = A2B_UPSLOT_SIZE_24 ,     /*! Up slot size  */ 
		.bUpstreamCompression  = DISABLED,      /*! Floating point compression for upstream  */
		.bDwnstreamCompression = DISABLED,      /*! Floating point compression for downstream  */  
		.bEnableReduceRate = DISABLED,          /*! Reduce Data Rate on A2B Bus   */ 
		.nSysRateDivFactor =0,                  /*! System level reduced rate factor   */  
};


 ADI_A2B_MASTER_SLAVE_CONFIG sChainConfig0= 
 { 
 	/* Pointer to master node */ 
	.pMasterConfig = &sMasterNode0, 
 
	/*! Number of Slave nodes */ 
	.nNumSlaveNode = 8, 
 
	/*! Array of slave node pointers */ 
	.apSlaveConfig = { &sChain0_SlaveNode0, &sChain0_SlaveNode0, &sChain0_SlaveNode0, &sChain0_SlaveNode0, &sChain0_SlaveNode0, &sChain0_SlaveNode0, &sChain0_SlaveNode0, &sChain0_SlaveNode0}, 

	.sCommonSetting = 
 	{ 
		/*! Master I2C address - 7 bit */ 
		.nMasterI2CAddr = 0xd0, 

		/*! Bus I2C address - 7 bit */ 
		.nBusI2CAddr = 0xd2, 

		/*! Enable down stream */ 
		.bEnableDwnstream = ENABLED, 

		/*! Enable down stream */ 
		.bEnableUpstream = ENABLED, 

		/*! Down slot size */ 
		.nDwnSlotSize = A2B_DNSLOT_SIZE_24, 

 		/*! Up slot size  */ 
		.nUpSlotSize = A2B_UPSLOT_SIZE_24 ,

 		/*! Floating point compression for upstream  */ 
		.bUpstreamCompression  = DISABLED,

		/*! Floating point compression for downstream  */  
		.bDwnstreamCompression = DISABLED,

		/*! Reduce Data Rate on A2B Bus   */  
		.bEnableReduceRate = DISABLED,

		/*! System level reduced rate factor   */  
		.nSysRateDivFactor =0,

	} 
 };


/* BUS CONFIGURATION */ 
const ADI_A2B_BCD sBusDescription = 
{ 
//    .nNumMasterNode  = 1u,          /* Number of master nodes/daisy chains in the network */ 
    .apNetworkconfig = { &sChainConfig0},       /* Array of Master-slave daisy chains */ 
    .sTargetProperties    =         /* Network control guidance */ 
	{ 
	.eDiscoveryMode     = A2B_SIMPLE_DISCOVERY,     /* Discovery Mode Selection */ 
	.bLineDiagnostics   = DISABLED,         /*  Enable/Disable  Line diagnostics */ 
	.bAutoDiscCriticalFault = DISABLED,     /*  Enable/Disable automatic discovery upon critical faults (during discovery process) */ 
	.nAttemptsCriticalFault = 0,            /*  Number of automatic discovery attempts (during discovery process) */
	.bAutoRediscOnFault = DISABLED,         /* Auto-rediscovery upon post discovery line fault */ 
	.nNumPeriDevice = 1,            /*! Number of peripheral devices connected to Target */
	/*! Array of peripheral configuration pointers */
	.apPeriConfig = { 	NULL_PTR}
	}, 

};

