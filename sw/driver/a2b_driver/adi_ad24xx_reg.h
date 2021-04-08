/* ================================================================================ */
/*      */
/*     Project      :   A2B */
/*     Part         :   ADSP-AD2410 */
/*     Description  :   Register Definitions */
/*      */
/*     SVN Tag      :   AD2410_0_0 */
/*      */
/*      */
/*     !! ADI Confidential !! */
/*       INTERNAL USE ONLY */
/*      */
/*     Copyright (c) 2012-2014 Analog Devices, Inc.  All Rights Reserved. */
/*     This software is proprietary and confidential to Analog Devices, Inc. and */
/*     its licensors. */
/*      */
/*     This file was auto-generated. Do not make local changes to this file. */
/*      */
/* ================================================================================ */

#ifndef _DEF_AD2410_H
#define _DEF_AD2410_H

/* ============================================================================================================================ */
/*    A2B (A2B0) Register Address Definitions */
/* ============================================================================================================================ */
#define MOD_A2B0_BASE                        0x00000000            /*  A2B */
#define MOD_A2B0_MASK                        0x00000FFF            /*  A2B */
#define REG_A2B0_PINCFG                      0x00000052            /*  A2B0 Pin Configuration Register */
/* ============================================================================================================================ */
/*    A2B Config, Master Only, Auto-Broadcast, Shadowed Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_SLOTFMT                     0x00000010            /*  A2B0 Slot Format Register (Master Only, Auto-Broadcast) */
#define REG_A2B0_DATCTL                      0x00000011            /*  A2B0 Data Control Register (Master Only, Auto-Broadcast) */
/* ============================================================================================================================ */
/*    A2B Config, Shadowed Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_DNSLOTS                     0x0000000D            /*  A2B0 Downstream Slots Register */
#define REG_A2B0_UPSLOTS                     0x0000000E            /*  A2B0 Upstream Slots Register */
#define REG_A2B0_RESPCYCS                    0x0000000F            /*  A2B0 Response Cycles Register */
/* ============================================================================================================================ */
/*    A2B Config, Shadowed, Slave Only Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_BCDNSLOTS                   0x0000000A            /*  A2B0 Broadcast Downstream Slots Register (Slave Only) */
#define REG_A2B0_LDNSLOTS                    0x0000000B            /*  A2B0 Local Downstream Slots Register (Slave Only) */
#define REG_A2B0_LUPSLOTS                    0x0000000C            /*  A2B0 Local Upstream Slots Register (Slave Only) */
/* ============================================================================================================================ */
/*    A2B Configuration Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_SWCTL                       0x00000009            /*  A2B0 Switch Control Register */
/* ============================================================================================================================ */
/*    A2B Control Master Only Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_NODEADR                     0x00000001            /*  A2B0 Node Address Register (Master only) */
/* ============================================================================================================================ */
/*    A2B Control, Master Only Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_CONTROL                     0x00000012            /*  A2B0 Control Register (Master Only) */
/* ============================================================================================================================ */
/*    A2B Start Discovery, Master Only Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_DISCVRY                     0x00000013            /*  A2B0 Discovery Register (Master Only) */
/* ============================================================================================================================ */
/*    A2B Status Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_NODE                        0x00000029            /*  A2B0 Node Register */
#define REG_A2B0_DISCSTAT                    0x0000002B            /*  A2B0 Discovery Status Register (Master Only) */
/* ============================================================================================================================ */
/*    A2B Status Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_SWSTAT                      0x00000014            /*  A2B0 Switch Status Register */
/* ============================================================================================================================ */
/*    I2C Control Slave Only Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_CHIP                        0x00000000            /*  A2B0 I2C Chip Address Register (Slave Only) */
/* ============================================================================================================================ */
/*    I2C, I2S, and PDM Control and Configuration Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_I2CCFG                      0x0000003F            /*  A2B0 I2C Configuration Register */
#define REG_A2B0_PLLCTL                      0x00000040            /*  A2B0 PLL Control Register */
#define REG_A2B0_I2SGCFG                     0x00000041            /*  A2B0 I2S Global Configuration Register */
#define REG_A2B0_I2SCFG                      0x00000042            /*  A2B0 I2S Configuration Register */
#define REG_A2B0_I2SRATE                     0x00000043            /*  A2B0 I2S Rate Register (Slave Only) */
#define REG_A2B0_I2STXOFFSET                 0x00000044            /*  A2B0 I2S Transmit Data Offset Register (Master Only) */
#define REG_A2B0_I2SRXOFFSET                 0x00000045            /*  A2B0 I2S Receive Data Offset Register (Master Only) */
#define REG_A2B0_SYNCOFFSET                  0x00000046            /*  A2B0 SYNC Offset Register (Slave Only) */
#define REG_A2B0_PDMCTL                      0x00000047            /*  A2B0 PDM Control Register */
#define REG_A2B0_ERRMGMT                     0x00000048            /*  A2B0 Error Management Register */
#define REG_A2B0_I2STEST                     0x00000053            /*  A2B0 I2S Test Register */
/* ============================================================================================================================ */
/*    ID Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_VENDOR                      0x00000002            /*  A2B0 Vendor ID Register */
#define REG_A2B0_PRODUCT                     0x00000003            /*  A2B0 Product ID Register */
#define REG_A2B0_VERSION                     0x00000004            /*  A2B0 Version ID Register */
#define REG_A2B0_CAPABILITY                  0x00000005            /*  A2B0 Capability ID Register */
/* ============================================================================================================================ */
/*    Interrupt and Error Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_INTSTAT                     0x00000015            /*  A2B0 Interrupt Status Register */
#define REG_A2B0_INTSRC                      0x00000016            /*  A2B0 Interrupt Source Register (Master Only) */
#define REG_A2B0_INTTYPE                     0x00000017            /*  A2B0 Interrupt Type Register (Master Only) */
#define REG_A2B0_INTPND0                     0x00000018            /*  A2B0 Interrupt Pending 0 Register */
#define REG_A2B0_INTPND1                     0x00000019            /*  A2B0 Interrupt Pending 1 Register */
#define REG_A2B0_INTPND2                     0x0000001A            /*  A2B0 Interrupt Pending 2 Register (Master Only) */
#define REG_A2B0_INTMSK0                     0x0000001B            /*  A2B0 Interrupt Mask 0 Register */
#define REG_A2B0_INTMSK1                     0x0000001C            /*  A2B0 Interrupt Mask 1 Register */
#define REG_A2B0_INTMSK2                     0x0000001D            /*  A2B0 Interrupt Mask 2 Register (Master Only) */
#define REG_A2B0_BECCTL                      0x0000001E            /*  A2B0 Bit Error Count Control Register */
#define REG_A2B0_BECNT                       0x0000001F            /*  A2B0 Bit Error Count Register */
#define REG_A2B0_RAISE                       0x00000054            /*  A2B0 Raise Interrupt Register */
#define REG_A2B0_GENERR                      0x00000055            /*  A2B0 Generate Bus Error */
/* ============================================================================================================================ */
/*    PRBS Test Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_TESTMODE                    0x00000020            /*  A2B0 Testmode Register */
#define REG_A2B0_ERRCNT0                     0x00000021            /*  A2B0 PRBS Error Count Byte 0 Register */
#define REG_A2B0_ERRCNT1                     0x00000022            /*  A2B0 PRBS Error Count Byte 1 Register */
#define REG_A2B0_ERRCNT2                     0x00000023            /*  A2B0 PRBS Error Count Byte 2 Register */
#define REG_A2B0_ERRCNT3                     0x00000024            /*  A2B0 PRBS Error Count Byte 3 Register */
/* ============================================================================================================================ */
/*    Pin IO and Interrupt Register Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_CLKCFG                      0x00000049            /*  A2B0 Clock Config Register */
#define REG_A2B0_GPIODAT                     0x0000004A            /*  A2B0 GPIO Output Data Register */
#define REG_A2B0_GPIODATSET                  0x0000004B            /*  A2B0 GPIO Output Data Set Register */
#define REG_A2B0_GPIODATCLR                  0x0000004C            /*  A2B0 GPIO Output Data Clear Register */
#define REG_A2B0_GPIOOEN                     0x0000004D            /*  A2B0 GPIO Output Enable Register */
#define REG_A2B0_GPIOIEN                     0x0000004E            /*  A2B0 GPIO Input Enable Register */
#define REG_A2B0_GPIOIN                      0x0000004F            /*  A2B0 GPIO Input Value Register */
#define REG_A2B0_PINTEN                      0x00000050            /*  A2B0 Pin Interrupt Enable Register */
#define REG_A2B0_PINTINV                     0x00000051            /*  A2B0 Pin Interrupt Invert Register */

/* ============================================================================================================================ */
/*    A2B (A2B) Field BitMasks, Positions & Enumerations  */
/* ============================================================================================================================  */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINCFG                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_PINCFG_DRVSTR                0            /*  Digital Pin Drive Strength */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINCFG                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_PINCFG_DRVSTR               0x00000001

#define REG_A2B0_PINCFG_RESET                0x00000001            /*      Reset Value for PINCFG */
/* ============================================================================================================================ */
/*    A2B Config, Master Only, Auto-Broadcast, Shadowed Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SLOTFMT                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_SLOTFMT_UPFP                 7            /*  Upstream Floating Point */
#define BITP_A2B_SLOTFMT_UPSIZE               4            /*  Upstream Slot Size */
#define BITP_A2B_SLOTFMT_DNFP                 3            /*  Downstream Floating-Point */
#define BITP_A2B_SLOTFMT_DNSIZE               0            /*  Downstream Slot Size */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SLOTFMT                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_SLOTFMT_UPFP                0x00000080
#define BITM_A2B_SLOTFMT_UPSIZE              0x00000070
#define BITM_A2B_SLOTFMT_DNFP                0x00000008
#define BITM_A2B_SLOTFMT_DNSIZE              0x00000007
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SLOTFMT                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_SLOTFMT_UPFP_En             0x00000080            /*  Enabled */
#define ENUM_A2B_SLOTFMT_UPFP_Dis            0x00000000            /*  Disabled */
#define ENUM_A2B_SLOTFMT_UPSIZE_32           0x00000060            /*  32 Bits */
#define ENUM_A2B_SLOTFMT_UPSIZE_24           0x00000040            /*  24 Bits */
#define ENUM_A2B_SLOTFMT_UPSIZE_12           0x00000010            /*  12 Bits */
#define ENUM_A2B_SLOTFMT_UPSIZE_20           0x00000030            /*  20 Bits */
#define ENUM_A2B_SLOTFMT_UPSIZE_8            0x00000000            /*  8 Bits */
#define ENUM_A2B_SLOTFMT_UPSIZE_16           0x00000020            /*  16 Bits */
#define ENUM_A2B_SLOTFMT_UPSIZE_28           0x00000050            /*  28 Bits */
#define ENUM_A2B_SLOTFMT_DNFP_En             0x00000008            /*  Enabled */
#define ENUM_A2B_SLOTFMT_DNFP_Dis            0x00000000            /*  Disabled */
#define ENUM_A2B_SLOTFMT_DNSIZE_32           0x00000006            /*  32 Bits */
#define ENUM_A2B_SLOTFMT_DNSIZE_24           0x00000004            /*  24 Bits */
#define ENUM_A2B_SLOTFMT_DNSIZE_12           0x00000001            /*  12 Bits */
#define ENUM_A2B_SLOTFMT_DNSIZE_20           0x00000003            /*  20 Bits */
#define ENUM_A2B_SLOTFMT_DNSIZE_8            0x00000000            /*  8 Bits */
#define ENUM_A2B_SLOTFMT_DNSIZE_16           0x00000002            /*  16 Bits */
#define ENUM_A2B_SLOTFMT_DNSIZE_28           0x00000005            /*  28 Bits */

#define REG_A2B0_SLOTFMT_RESET               0x00000000            /*      Reset Value for SLOTFMT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DATCTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_DATCTL_STANDBY               7            /*  Enable Standby Mode */
#define BITP_A2B_DATCTL_UPS                   1            /*  Enable Upstream Slots */
#define BITP_A2B_DATCTL_DNS                   0            /*  Enable Downstream Slots */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DATCTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_DATCTL_STANDBY              0x00000080
#define BITM_A2B_DATCTL_UPS                  0x00000002
#define BITM_A2B_DATCTL_DNS                  0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DATCTL                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_DATCTL_STANDBY_En           0x00000080            /*  Enabled */
#define ENUM_A2B_DATCTL_STANDBY_Dis          0x00000000            /*  Disabled */
#define ENUM_A2B_DATCTL_UPS_En               0x00000002            /*  Enabled */
#define ENUM_A2B_DATCTL_UPS_Dis              0x00000000            /*  Disabled */
#define ENUM_A2B_DATCTL_DNS_En               0x00000001            /*  Enabled */
#define ENUM_A2B_DATCTL_DNS_Dis              0x00000000            /*  Disabled */

#define REG_A2B0_DATCTL_RESET                0x00000000            /*      Reset Value for DATCTL */
/* ============================================================================================================================ */
/*    A2B Config, Shadowed Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNSLOTS                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_DNSLOTS_DNSLOTS              0            /*  Number of Downstream Slots */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DNSLOTS                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_DNSLOTS_DNSLOTS             0x0000003F

#define REG_A2B0_DNSLOTS_RESET               0x00000000            /*      Reset Value for DNSLOTS */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPSLOTS                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_UPSLOTS_UPSLOTS              0            /*  Number of Upstream Slots */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        UPSLOTS                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_UPSLOTS_UPSLOTS             0x0000003F

#define REG_A2B0_UPSLOTS_RESET               0x00000000            /*      Reset Value for UPSLOTS */
#define REG_A2B0_RESPCYCS_RESET              0x00000040            /*      Reset Value for RESPCYCS */
/* ============================================================================================================================ */
/*    A2B Config, Shadowed, Slave Only Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BCDNSLOTS                            Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_BCDNSLOTS_BCDNSLOTS          0            /*  Broadcast Downstream Slots */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BCDNSLOTS                            Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_BCDNSLOTS_BCDNSLOTS         0x0000003F

#define REG_A2B0_BCDNSLOTS_RESET             0x00000000            /*      Reset Value for BCDNSLOTS */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        LDNSLOTS                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_LDNSLOTS_LDNSLOTS            0            /*  Number of Downstream Slots Targeted at Local Node */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        LDNSLOTS                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_LDNSLOTS_LDNSLOTS           0x0000003F

#define REG_A2B0_LDNSLOTS_RESET              0x00000000            /*      Reset Value for LDNSLOTS */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        LUPSLOTS                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_LUPSLOTS_LUPSLOTS            0            /*  Number of Upstream Slots Generated By Local Node */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        LUPSLOTS                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_LUPSLOTS_LUPSLOTS           0x0000003F

#define REG_A2B0_LUPSLOTS_RESET              0x00000000            /*      Reset Value for LUPSLOTS */
/* ============================================================================================================================ */
/*    A2B Configuration Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SWCTL                                Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_SWCTL_MODE                   4            /*  External Switch Mode */
#define BITP_A2B_SWCTL_ENSW                   0            /*  Enable Switch */
#define BITP_A2B_SWCTL_DIAGMODE               3
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SWCTL                                Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_SWCTL_MODE                  0x00000030
#define BITM_A2B_SWCTL_DIAGMODE              0x00000008
#define BITM_A2B_SWCTL_ENSW                  0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SWCTL                                Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_SWCTL_DIAGMODE_En           0x00000008            /*  Switch Diagnosis Mode Enabled */
#define ENUM_A2B_SWCTL_DIAGMODE_Dis          0x00000000            /*  Switch Diagnosis Mode Disabled */
#define ENUM_A2B_SWCTL_ENSW_En               0x00000001            /*  Switches Enabled */
#define ENUM_A2B_SWCTL_ENSW_Dis              0x00000000            /*  Switches Disabled */

#define REG_A2B0_SWCTL_RESET                 0x00000000            /*      Reset Value for SWCTL */
/* ============================================================================================================================ */
/*    A2B Control Master Only Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NODEADR                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_NODEADR_BRCST                7            /*  Broadcast */
#define BITP_A2B_NODEADR_PERI                 5            /*  Enable Peripheral */
#define BITP_A2B_NODEADR_NODE                 0            /*  Addressed Slave Node */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NODEADR                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_NODEADR_BRCST               0x00000080
#define BITM_A2B_NODEADR_PERI                0x00000020
#define BITM_A2B_NODEADR_NODE                0x0000000F
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NODEADR                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_NODEADR_BRCST_En            0x00000080            /*  Next write to slave nodes is handled as broadcast access */
#define ENUM_A2B_NODEADR_BRCST_Dis           0x00000000            /*  Normal, directed register access */
#define ENUM_A2B_NODEADR_PERI_En             0x00000020            /*  Remote Peripheral Access enabled */
#define ENUM_A2B_NODEADR_PERI_Dis            0x00000000            /*  Remote Peripheral Access disabled */

#define REG_A2B0_NODEADR_RESET               0x00000000            /*      Reset Value for NODEADR */
/* ============================================================================================================================ */
/*    A2B Control, Master Only Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CONTROL                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_CONTROL_SOFTRST              2            /*  Soft Reset of Protocol Engine */
#define BITP_A2B_CONTROL_ENDDSC               1            /*  End Discovery Mode */
#define BITP_A2B_CONTROL_NEWSTRCT             0            /*  New Structure */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CONTROL                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_CONTROL_SOFTRST             0x00000004
#define BITM_A2B_CONTROL_ENDDSC              0x00000002
#define BITM_A2B_CONTROL_NEWSTRCT            0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CONTROL                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_CONTROL_Reset_PE            0x00000004            /*  Reset Protocol Engine */
#define ENUM_A2B_CONTROL_SOFTRST_No_Action   0x00000000            /*  No Action */
#define ENUM_A2B_CONTROL_End_Discovery       0x00000002            /*  End Discovery */
#define ENUM_A2B_CONTROL_ENDDSC_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_CONTROL_Start_NS            0x00000001            /*  Enable New Structure */
#define ENUM_A2B_CONTROL_NEWSTRCT_No_Action  0x00000000            /*  No Action */

#define REG_A2B0_CONTROL_RESET               0x00000000            /*      Reset Value for CONTROL */
/* ============================================================================================================================ */
/*    A2B Start Discovery, Master Only Register Field Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_DISCVRY_RESET               0x00000000            /*      Reset Value for DISCVRY */
/* ============================================================================================================================ */
/*    A2B Status Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NODE                                 Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_NODE_LAST                    7            /*  Last Node */
#define BITP_A2B_NODE_NLAST                   6            /*  Next-to-Last Node */
#define BITP_A2B_NODE_DISCVD                  5            /*  Node Discovered */
#define BITP_A2B_NODE_NUMBER                  0            /*  Number Currently Assigned to Node */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        NODE                                 Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_NODE_LAST                   0x00000080
#define BITM_A2B_NODE_NLAST                  0x00000040
#define BITM_A2B_NODE_DISCVD                 0x00000020
#define BITM_A2B_NODE_NUMBER                 0x0000000F

#define REG_A2B0_NODE_RESET                  0x00000080            /*      Reset Value for NODE */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DISCSTAT                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_DISCSTAT_DSCACT              7            /*  Discovery Active */
#define BITP_A2B_DISCSTAT_DNODE               0            /*  Discovery Node */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        DISCSTAT                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_DISCSTAT_DSCACT             0x00000080
#define BITM_A2B_DISCSTAT_DNODE              0x0000000F

#define REG_A2B0_DISCSTAT_RESET              0x00000000            /*      Reset Value for DISCSTAT */
/* ============================================================================================================================ */
/*    A2B Status Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SWSTAT                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_SWSTAT_FAULT_NLOC            7            /*  Cable Fault Not Localized */
#define BITP_A2B_SWSTAT_FAULT_CODE            4            /*  Cable Fault Code */
#define BITP_A2B_SWSTAT_FAULT                 1            /*  Cable Fault */
#define BITP_A2B_SWSTAT_FIN                   0            /*  Switch Activation Complete */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        SWSTAT                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_SWSTAT_FAULT_NLOC           0x00000080
#define BITM_A2B_SWSTAT_FAULT_CODE           0x00000070
#define BITM_A2B_SWSTAT_FAULT                0x00000002
#define BITM_A2B_SWSTAT_FIN                  0x00000001

#define REG_A2B0_SWSTAT_RESET                0x00000000            /*      Reset Value for SWSTAT */
/* ============================================================================================================================ */
/*    I2C Control Slave Only Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CHIP                                 Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_CHIP_CHIPADR                 0            /*  I2C Chip Address */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CHIP                                 Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_CHIP_CHIPADR                0x0000007F

#define REG_A2B0_CHIP_RESET                  0x00000050            /*      Reset Value for CHIP */
/* ============================================================================================================================ */
/*    I2C, I2S, and PDM Control and Configuration Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2CCFG                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_I2CCFG_FRAMERATE             2            /*  Audio Frame Rate (A2B Slave Only) */
#define BITP_A2B_I2CCFG_EACK                  1            /*  Early Acknowledge (A2B Master Only) */
#define BITP_A2B_I2CCFG_DATARATE              0            /*  I2C Data Rate (A2B Slave Only) */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2CCFG                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_I2CCFG_FRAMERATE            0x00000004
#define BITM_A2B_I2CCFG_EACK                 0x00000002
#define BITM_A2B_I2CCFG_DATARATE             0x00000001

#define REG_A2B0_I2CCFG_RESET                0x00000000            /*      Reset Value for I2CCFG */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PLLCTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_PLLCTL_BCLKSEL               2            /*  BCLK Bypass to PLL */
#define BITP_A2B_PLLCTL_BCLKDIV               0            /*  BCLK PLL Multiplication Select */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PLLCTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_PLLCTL_BCLKSEL              0x00000004
#define BITM_A2B_PLLCTL_BCLKDIV              0x00000003
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PLLCTL                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_PLLCTL_BCLKSEL_En           0x00000004            /*  BCLK Selected */
#define ENUM_A2B_PLLCTL_BCLKSEL_Dis          0x00000000            /*  SYNC Selected */
#define ENUM_A2B_PLLCTL_BCLKDIV_4x           0x00000001            /*  2  x (BCLK=24.576 MHz in TDM16) */
#define ENUM_A2B_PLLCTL_BCLKDIV_8x           0x00000000            /*  1 x  (BCLK=12.288 MHz in TDM8) */
#define ENUM_A2B_PLLCTL_BCLKDIV_2x           0x00000002            /*  4  x  (BCLK=49.152 MHz for TDM32) */

#define REG_A2B0_PLLCTL_RESET                0x00000000            /*      Reset Value for PLLCTL */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SGCFG                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_I2SGCFG_INV                  7            /*  Invert Sync */
#define BITP_A2B_I2SGCFG_EARLY                6            /*  Early Sync */
#define BITP_A2B_I2SGCFG_ALT                  5            /*  Alternating Sync */
#define BITP_A2B_I2SGCFG_TDMSS                4            /*  TDM Slot Size */
#define BITP_A2B_I2SGCFG_TDMMODE              0            /*  TDM Mode */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SGCFG                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_I2SGCFG_INV                 0x00000080
#define BITM_A2B_I2SGCFG_EARLY               0x00000040
#define BITM_A2B_I2SGCFG_ALT                 0x00000020
#define BITM_A2B_I2SGCFG_TDMSS               0x00000010
#define BITM_A2B_I2SGCFG_TDMMODE             0x00000007
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SGCFG                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_I2SGCFG_INV_En              0x00000080            /*  Falling edge of SYNC pin corresponds to the start of an audio frame */
#define ENUM_A2B_I2SGCFG_INV_Dis             0x00000000            /*  Rising edge of SYNC pin corresponds to the start of an audio frame */
#define ENUM_A2B_I2SGCFG_EARLY_En            0x00000040            /*  Change SYNC pin in previous cycle */
#define ENUM_A2B_I2SGCFG_EARLY_Dis           0x00000000            /*  Change SYNC pin in same cycle */
#define ENUM_A2B_I2SGCFG_ALT_En              0x00000020            /*  Drive SYNC Pin Alternating */
#define ENUM_A2B_I2SGCFG_ALT_Dis             0x00000000            /*  Drive SYNC Pin for 1 Cycle */
#define ENUM_A2B_I2SGCFG_16_Bit              0x00000010            /*  16-Bit */
#define ENUM_A2B_I2SGCFG_32_Bit              0x00000000            /*  32-Bit */
#define ENUM_A2B_I2SGCFG_TDM24               0x00000006            /*  No slave node support */
#define ENUM_A2B_I2SGCFG_TDM16               0x00000004            /*  TDM16 */
#define ENUM_A2B_I2SGCFG_TDM4                0x00000001            /*  TDM4 */
#define ENUM_A2B_I2SGCFG_TDM12               0x00000003            /*  No slave node support */
#define ENUM_A2B_I2SGCFG_TDM2                0x00000000            /*  TDM2 */
#define ENUM_A2B_I2SGCFG_TDM32               0x00000007            /*  TDM32 */
#define ENUM_A2B_I2SGCFG_TDM8                0x00000002            /*  TDM8 */
#define ENUM_A2B_I2SGCFG_TDM20               0x00000005            /*  No slave node support */

#define REG_A2B0_I2SGCFG_RESET               0x00000000            /*      Reset Value for I2SGCFG */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SCFG                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_I2SCFG_RXBCLKINV             7            /*  RX BCLK Invert */
#define BITP_A2B_I2SCFG_RX2PINTL              6            /*  RX 2 Pin Interleave */
#define BITP_A2B_I2SCFG_RX1EN                 5            /*  I2S RX 1 Enable */
#define BITP_A2B_I2SCFG_RX0EN                 4            /*  I2S RX 0 Enable */
#define BITP_A2B_I2SCFG_TXBCLKINV             3            /*  TX BCLK Invert */
#define BITP_A2B_I2SCFG_TX2PINTL              2            /*  TX 2 Pin Interleave */
#define BITP_A2B_I2SCFG_TX1EN                 1            /*  I2S TX 1 Enable */
#define BITP_A2B_I2SCFG_TX0EN                 0            /*  I2S TX 0 Enable */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SCFG                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_I2SCFG_RXBCLKINV            0x00000080
#define BITM_A2B_I2SCFG_RX2PINTL             0x00000040
#define BITM_A2B_I2SCFG_RX1EN                0x00000020
#define BITM_A2B_I2SCFG_RX0EN                0x00000010
#define BITM_A2B_I2SCFG_TXBCLKINV            0x00000008
#define BITM_A2B_I2SCFG_TX2PINTL             0x00000004
#define BITM_A2B_I2SCFG_TX1EN                0x00000002
#define BITM_A2B_I2SCFG_TX0EN                0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SCFG                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_I2SCFG_RXBCLKINV_En         0x00000080            /*  Enabled */
#define ENUM_A2B_I2SCFG_RXBCLKINV_Dis        0x00000000            /*  Disabled */
#define ENUM_A2B_I2SCFG_RX2PINTL_En          0x00000040            /*  Enabled */
#define ENUM_A2B_I2SCFG_RX2PINTL_Dis         0x00000000            /*  Disabled */
#define ENUM_A2B_I2SCFG_RX1EN_En             0x00000020            /*  Enabled */
#define ENUM_A2B_I2SCFG_RX1EN_Dis            0x00000000            /*  Disabled */
#define ENUM_A2B_I2SCFG_RX0EN_En             0x00000010            /*  Enabled */
#define ENUM_A2B_I2SCFG_RX0EN_Dis            0x00000000            /*  Disabled */
#define ENUM_A2B_I2SCFG_TXBCLKINV_En         0x00000008            /*  Enabled */
#define ENUM_A2B_I2SCFG_TXBCLKINV_Dis        0x00000000            /*  Disabled */
#define ENUM_A2B_I2SCFG_TX2PINTL_En          0x00000004            /*  Enabled */
#define ENUM_A2B_I2SCFG_TX2PINTL_Dis         0x00000000            /*  Disabled */
#define ENUM_A2B_I2SCFG_TX1EN_En             0x00000002            /*  Enabled */
#define ENUM_A2B_I2SCFG_TX1EN_Dis            0x00000000            /*  Disabled */
#define ENUM_A2B_I2SCFG_TX0EN_En             0x00000001            /*  Enabled */
#define ENUM_A2B_I2SCFG_TX0EN_Dis            0x00000000            /*  Disabled */

#define REG_A2B0_I2SCFG_RESET                0x00000000            /*      Reset Value for I2SCFG */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRATE                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_I2SRATE_REDUCE               6            /*  Reduce and Duplicate */
#define BITP_A2B_I2SRATE_FRAMES               4            /*  Superframes Used */
#define BITP_A2B_I2SRATE_I2SRATE              0            /*  I2S Rate Setting */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRATE                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_I2SRATE_REDUCE              0x00000040
#define BITM_A2B_I2SRATE_FRAMES              0x00000030
#define BITM_A2B_I2SRATE_I2SRATE             0x00000007
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRATE                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_I2SRATE_Reduce_En           0x00000040            /*  Enabled */
#define ENUM_A2B_I2SRATE_Reduce_Dis          0x00000000            /*  Disabled */
#define ENUM_A2B_I2SRATE_Frames_En           0x00000010            /*  Enabled */
#define ENUM_A2B_I2SRATE_Frames_Dis          0x00000000            /*  Disabled */
#define ENUM_A2B_I2SRATE_QUAD_SFF            0x00000006            /*  SFF x 4 */
#define ENUM_A2B_I2SRATE_1x_SFF              0x00000000            /*  1x SFF */
#define ENUM_A2B_I2SRATE_DBL_SFF             0x00000005            /*  SFF x 2 */

#define REG_A2B0_I2SRATE_RESET               0x00000000            /*      Reset Value for I2SRATE */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2STXOFFSET                          Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_I2STXOFFSET_TSBEFORE         7            /*  Three-State before TX Slots */
#define BITP_A2B_I2STXOFFSET_TSAFTER          6            /*  Three-State after TX Slots */
#define BITP_A2B_I2STXOFFSET_TXOFFSET         0            /*  Serial TX Offset */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2STXOFFSET                          Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_I2STXOFFSET_TSBEFORE        0x00000080
#define BITM_A2B_I2STXOFFSET_TSAFTER         0x00000040
#define BITM_A2B_I2STXOFFSET_TXOFFSET        0x0000003F
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2STXOFFSET                          Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_I2STXOFFSET_TSBEFORE_EN     0x00000080            /*  Enable */
#define ENUM_A2B_I2STXOFFSET_TSBEFORE_DIS    0x00000000            /*  Disable */
#define ENUM_A2B_I2STXOFFSET_TSAFTER_EN      0x00000040            /*  Enable */
#define ENUM_A2B_I2STXOFFSET_TSAFTER_DIS     0x00000000            /*  Disable */
#define ENUM_A2B_I2STXOFFSET_TXOFFSET_01     0x00000001            /*  TX Offset of 1 TDM Slot */
#define ENUM_A2B_I2STXOFFSET_TXOFFSET_63     0x0000003F            /*  TX Offset of 63 TDM Slots */
#define ENUM_A2B_I2STXOFFSET_TXOFFSET_00     0x00000000            /*  No TX Offset */
#define ENUM_A2B_I2STXOFFSET_TXOFFSET_62     0x0000003E            /*  TX Offset of 62 TDM Slots */

#define REG_A2B0_I2STXOFFSET_RESET           0x00000000            /*      Reset Value for I2STXOFFSET */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRXOFFSET                          Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_I2SRXOFFSET_RXOFFSET         0            /*  Serial RX Offset */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRXOFFSET                          Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_I2SRXOFFSET_RXOFFSET        0x0000003F
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2SRXOFFSET                          Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_I2SRXOFFSET_Zero            0x00000000            /*  No RX Offset */

#define REG_A2B0_I2SRXOFFSET_RESET           0x00000000            /*      Reset Value for I2SRXOFFSET */
#define REG_A2B0_SYNCOFFSET_RESET            0x00000000            /*      Reset Value for SYNCOFFSET */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PDMCTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_PDMCTL_HPFCUTOFF             5            /*  Highpass Filter Cutoff */
#define BITP_A2B_PDMCTL_HPFEN                 4            /*  Highpass Filter Enable */
#define BITP_A2B_PDMCTL_PDM1SLOTS             3            /*  PDM1 Slots */
#define BITP_A2B_PDMCTL_PDM1EN                2            /*  PDM1 Enable */
#define BITP_A2B_PDMCTL_PDM0SLOTS             1            /*  PDM0 Slots */
#define BITP_A2B_PDMCTL_PDM0EN                0            /*  PDM0 Enable */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PDMCTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_PDMCTL_HPFCUTOFF            0x000000E0
#define BITM_A2B_PDMCTL_HPFEN                0x00000010
#define BITM_A2B_PDMCTL_PDM1SLOTS            0x00000008
#define BITM_A2B_PDMCTL_PDM1EN               0x00000004
#define BITM_A2B_PDMCTL_PDM0SLOTS            0x00000002
#define BITM_A2B_PDMCTL_PDM0EN               0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PDMCTL                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_PDMCTL_0_93                 0x000000C0            /*  0.93 Hz */
#define ENUM_A2B_PDMCTL_3_73                 0x00000080            /*  3.73 Hz */
#define ENUM_A2B_PDMCTL_29_8                 0x00000020            /*  29.8 Hz */
#define ENUM_A2B_PDMCTL_7_46                 0x00000060            /*  7.46 Hz */
#define ENUM_A2B_PDMCTL_59_9                 0x00000000            /*  59.9 Hz */
#define ENUM_A2B_PDMCTL_14_9                 0x00000040            /*  14.9 Hz */
#define ENUM_A2B_PDMCTL_1_86                 0x000000A0            /*  1.86 Hz */
#define ENUM_A2B_PDMCTL_HPFEN_En             0x00000010            /*  Enable Filter */
#define ENUM_A2B_PDMCTL_HPFEN_Dis            0x00000000            /*  Disable Filter */
#define ENUM_A2B_PDMCTL_PDM1SLOTS_2          0x00000008            /*  2 Slots */
#define ENUM_A2B_PDMCTL_PDM1SLOTS_1          0x00000000            /*  1 Slot */
#define ENUM_A2B_PDMCTL_PDM1EN_En            0x00000004            /*  Enable PDM reception on the pin */
#define ENUM_A2B_PDMCTL_PDM1EN_Dis           0x00000000            /*  Disable PDM reception on the pin. */
#define ENUM_A2B_PDMCTL_PDM0SLOTS_2          0x00000002            /*  2 Slots */
#define ENUM_A2B_PDMCTL_PDM0SLOTS_1          0x00000000            /*  1 Slot */
#define ENUM_A2B_PDMCTL_PDM0EN_En            0x00000001            /*  Enable PDM reception on the pin */
#define ENUM_A2B_PDMCTL_PDM0EN_Dis           0x00000000            /*  Disable PDM reception on the pin */

#define REG_A2B0_PDMCTL_RESET                0x00000000            /*      Reset Value for PDMCTL */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        ERRMGMT                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_ERRMGMT_ERRSLOT              2            /*  Add Error Indicating Channel to I2S/TDM Output */
#define BITP_A2B_ERRMGMT_ERRSIG               1            /*  Show Data Error on Remaining Bits */
#define BITP_A2B_ERRMGMT_ERRLSB               0            /*  Show Data Error on LSB */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        ERRMGMT                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_ERRMGMT_ERRSLOT             0x00000004
#define BITM_A2B_ERRMGMT_ERRSIG              0x00000002
#define BITM_A2B_ERRMGMT_ERRLSB              0x00000001

#define REG_A2B0_ERRMGMT_RESET               0x00000000            /*      Reset Value for ERRMGMT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2STEST                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_I2STEST_SELRX1               3            /*  Select RX1 Block to Loopback Buffer */
#define BITP_A2B_I2STEST_RX2LOOPBK            2            /*  RX Block to Loopback Buffer */
#define BITP_A2B_I2STEST_LOOPBK2TX            1            /*  Loopback Data to TX Blocks */
#define BITP_A2B_I2STEST_PATTRN2TX            0            /*  Default Bit Pattern to Serial TX Blocks */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        I2STEST                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_I2STEST_SELRX1              0x00000008
#define BITM_A2B_I2STEST_RX2LOOPBK           0x00000004
#define BITM_A2B_I2STEST_LOOPBK2TX           0x00000002
#define BITM_A2B_I2STEST_PATTRN2TX           0x00000001

#define REG_A2B0_I2STEST_RESET               0x00000000            /*      Reset Value for I2STEST */
/* ============================================================================================================================ */
/*    ID Register Field Definitions */
/* ============================================================================================================================ */
#define REG_A2B0_VENDOR_RESET                0x000000AD            /*      Reset Value for VENDOR */
#define REG_A2B0_PRODUCT_RESET               0x00000010            /*      Reset Value for PRODUCT */
#define REG_A2B0_VERSION_RESET               0x00000010            /*      Reset Value for VERSION */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CAPABILITY                           Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_CAPABILITY_MINFO             7            /*  On-Chip Module Info Available */
#define BITP_A2B_CAPABILITY_SPIAVAIL          1            /*  Module Info Available over SPI */
#define BITP_A2B_CAPABILITY_I2CAVAIL          0            /*  I2C Interface Available */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CAPABILITY                           Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_CAPABILITY_MINFO            0x00000080
#define BITM_A2B_CAPABILITY_SPIAVAIL         0x00000002
#define BITM_A2B_CAPABILITY_I2CAVAIL         0x00000001

#define REG_A2B0_CAPABILITY_RESET            0x00000001            /*      Reset Value for CAPABILITY */
/* ============================================================================================================================ */
/*    Interrupt and Error Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTSTAT                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_INTSTAT_IRQ                  0            /*  Interrupt Currently Asserted */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTSTAT                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_INTSTAT_IRQ                 0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTSTAT                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_INTSTAT_IRQ_High            0x00000001            /*  Interrupt request */
#define ENUM_A2B_INTSTAT_IRQ_Low             0x00000000            /*  No Interrupt request */

#define REG_A2B0_INTSTAT_RESET               0x00000000            /*      Reset Value for INTSTAT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTSRC                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_INTSRC_MSTINT                7            /*  Master Interrupt */
#define BITP_A2B_INTSRC_SLVINT                6            /*  Slave Interrupt */
#define BITP_A2B_INTSRC_INODE                 0            /*  ID for SLVINT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTSRC                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_INTSRC_MSTINT               0x00000080
#define BITM_A2B_INTSRC_SLVINT               0x00000040
#define BITM_A2B_INTSRC_INODE                0x0000000F

#define REG_A2B0_INTSRC_RESET                0x00000000            /*      Reset Value for INTSRC */
#define REG_A2B0_INTTYPE_RESET               0x00000000            /*      Reset Value for INTTYPE */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND0                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_INTPND0_SRFERR               6            /*  Missed Synchronization Response Frame (SRF) */
#define BITP_A2B_INTPND0_BECOVF               5            /*  Bit Error Count error */
#define BITP_A2B_INTPND0_PWRERR               4            /*  Switch reporting error on downstream power */
#define BITP_A2B_INTPND0_DPERR                3            /*  Data Parity Error */
#define BITP_A2B_INTPND0_CRCERR               2            /*  CRC Error */
#define BITP_A2B_INTPND0_DDERR                1            /*  Data Decoding Error */
#define BITP_A2B_INTPND0_HDCNTERR             0            /*  Header Count Error */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND0                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_INTPND0_SRFERR              0x00000040
#define BITM_A2B_INTPND0_BECOVF              0x00000020
#define BITM_A2B_INTPND0_PWRERR              0x00000010
#define BITM_A2B_INTPND0_DPERR               0x00000008
#define BITM_A2B_INTPND0_CRCERR              0x00000004
#define BITM_A2B_INTPND0_DDERR               0x00000002
#define BITM_A2B_INTPND0_HDCNTERR            0x00000001

#define REG_A2B0_INTPND0_RESET               0x00000000            /*      Reset Value for INTPND0 */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND1                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_INTPND1_IO6PND               6            /*  IO6 Int Pending */
#define BITP_A2B_INTPND1_IO5PND               5            /*  IO5 Int Pending */
#define BITP_A2B_INTPND1_IO4PND               4            /*  IO4 Int Pending */
#define BITP_A2B_INTPND1_IO3PND               3            /*  IO3 Int Pending */
#define BITP_A2B_INTPND1_IO2PND               2            /*  IO2 Int Pending */
#define BITP_A2B_INTPND1_IO1PND               1            /*  IO1 Int Pending */
#define BITP_A2B_INTPND1_IO0PND               0            /*  IO0 Int Pending (Slave Only) */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND1                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_INTPND1_IO6PND              0x00000040
#define BITM_A2B_INTPND1_IO5PND              0x00000020
#define BITM_A2B_INTPND1_IO4PND              0x00000010
#define BITM_A2B_INTPND1_IO3PND              0x00000008
#define BITM_A2B_INTPND1_IO2PND              0x00000004
#define BITM_A2B_INTPND1_IO1PND              0x00000002
#define BITM_A2B_INTPND1_IO0PND              0x00000001

#define REG_A2B0_INTPND1_RESET               0x00000000            /*      Reset Value for INTPND1 */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND2                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_INTPND2_SLVIRQ               3            /*  Slave Interrupt Received Master Only */
#define BITP_A2B_INTPND2_ICRCERR              2            /*  Int Frame CRC Error Master Only */
#define BITP_A2B_INTPND2_I2CERR               1            /*  I2C Transaction Error Master Only */
#define BITP_A2B_INTPND2_DSCDONE              0            /*  Node Discovered Master Only */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        INTPND2                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_INTPND2_SLVIRQ              0x00000008
#define BITM_A2B_INTPND2_ICRCERR             0x00000004
#define BITM_A2B_INTPND2_I2CERR              0x00000002
#define BITM_A2B_INTPND2_DSCDONE             0x00000001

#define REG_A2B0_INTPND2_RESET               0x00000000            /*      Reset Value for INTPND2 */
#define REG_A2B0_INTMSK0_RESET               0x00000000            /*      Reset Value for INTMSK0 */
#define REG_A2B0_INTMSK1_RESET               0x00000000            /*      Reset Value for INTMSK1 */
#define REG_A2B0_INTMSK2_RESET               0x00000000            /*      Reset Value for INTMSK2 */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BECCTL                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_BECCTL_THRESHLD              5            /*  Threshold to Generate an Interrupt */
#define BITP_A2B_BECCTL_ENICRC                4            /*  Enable ICRCERR count */
#define BITP_A2B_BECCTL_ENDP                  3            /*  Enable DPERR count */
#define BITP_A2B_BECCTL_ENCRC                 2            /*  Enable CRCERR count */
#define BITP_A2B_BECCTL_ENDD                  1            /*  Enable DDERR count */
#define BITP_A2B_BECCTL_ENHDCNT               0            /*  Enable HDCNTERR count */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BECCTL                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_BECCTL_THRESHLD             0x000000E0
#define BITM_A2B_BECCTL_ENICRC               0x00000010
#define BITM_A2B_BECCTL_ENDP                 0x00000008
#define BITM_A2B_BECCTL_ENCRC                0x00000004
#define BITM_A2B_BECCTL_ENDD                 0x00000002
#define BITM_A2B_BECCTL_ENHDCNT              0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        BECCTL                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_BECCTL_THRESHLD_128         0x000000C0            /*  Interrupt After 128 Errors */
#define ENUM_A2B_BECCTL_THRESHLD_32          0x00000080            /*  Interrupt After 32 Errors */
#define ENUM_A2B_BECCTL_THRESHLD_4           0x00000020            /*  Interrupt After 4 Errors */
#define ENUM_A2B_BECCTL_THRESHLD_16          0x00000060            /*  Interrupt After 16 Errors */
#define ENUM_A2B_BECCTL_THRESHLD_2           0x00000000            /*  Interrupt After 2 Errors */
#define ENUM_A2B_BECCTL_THRESHLD_256         0x000000E0            /*  Interrupt After 256 Errors */
#define ENUM_A2B_BECCTL_THRESHLD_8           0x00000040            /*  Interrupt After 8 Errors */
#define ENUM_A2B_BECCTL_THRESHLD_64          0x000000A0            /*  Interrupt After 64 Errors */
#define ENUM_A2B_BECCTL_ENICRC_En            0x00000010            /*  Enable Bit Error Counting */
#define ENUM_A2B_BECCTL_ENICRC_Dis           0x00000000            /*  Disabled */
#define ENUM_A2B_BECCTL_ENDP_En              0x00000008            /*  Parity Error */
#define ENUM_A2B_BECCTL_ENDP_Dis             0x00000000            /*  No Parity error */
#define ENUM_A2B_BECCTL_ENCRC_En             0x00000004            /*  CRC Error */
#define ENUM_A2B_BECCTL_ENCRC_Dis            0x00000000            /*  No CRC Error */
#define ENUM_A2B_BECCTL_ENDD_En              0x00000002            /*  Enabled */
#define ENUM_A2B_BECCTL_ENDD_Dis             0x00000000            /*  Disabled */
#define ENUM_A2B_BECCTL_ENHDCNT_En           0x00000001            /*  Enabled */
#define ENUM_A2B_BECCTL_ENHDCNT_Dis          0x00000000            /*  Disabled */

#define REG_A2B0_BECCTL_RESET                0x00000000            /*      Reset Value for BECCTL */
#define REG_A2B0_BECNT_RESET                 0x00000000            /*      Reset Value for BECNT */
#define REG_A2B0_RAISE_RESET                 0x00000000            /*      Reset Value for RAISE */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GENERR                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_GENERR_GENICRCERR            4            /*  Generate Int Frame CRC Error (Slave Only) */
#define BITP_A2B_GENERR_GENDPERR              3            /*  Generate Data Parity Error */
#define BITP_A2B_GENERR_GENCRCERR             2            /*  Generate CRC Error */
#define BITP_A2B_GENERR_GENDDERR              1            /*  Generate Data Decoding Error */
#define BITP_A2B_GENERR_GENHCERR              0            /*  Generate Header Count Error */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GENERR                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_GENERR_GENICRCERR           0x00000010
#define BITM_A2B_GENERR_GENDPERR             0x00000008
#define BITM_A2B_GENERR_GENCRCERR            0x00000004
#define BITM_A2B_GENERR_GENDDERR             0x00000002
#define BITM_A2B_GENERR_GENHCERR             0x00000001

#define REG_A2B0_GENERR_RESET                0x00000000            /*      Reset Value for GENERR */
/* ============================================================================================================================ */
/*    PRBS Test Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        TESTMODE                             Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_TESTMODE_PRBSN2N             2            /*  PRBS N2N Mode */
#define BITP_A2B_TESTMODE_PRBSDN              1            /*  PRBS Data Downstream */
#define BITP_A2B_TESTMODE_PRBSUP              0            /*  PRBS Data Upstream */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        TESTMODE                             Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_TESTMODE_PRBSN2N            0x00000004
#define BITM_A2B_TESTMODE_PRBSDN             0x00000002
#define BITM_A2B_TESTMODE_PRBSUP             0x00000001

#define REG_A2B0_TESTMODE_RESET              0x00000000            /*      Reset Value for TESTMODE */
#define REG_A2B0_ERRCNT0_RESET               0x00000000            /*      Reset Value for ERRCNT0 */
#define REG_A2B0_ERRCNT1_RESET               0x00000000            /*      Reset Value for ERRCNT1 */
#define REG_A2B0_ERRCNT2_RESET               0x00000000            /*      Reset Value for ERRCNT2 */
#define REG_A2B0_ERRCNT3_RESET               0x00000000            /*      Reset Value for ERRCNT3 */
/* ============================================================================================================================ */
/*    Pin IO and Interrupt Register Field Definitions */
/* ============================================================================================================================ */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKCFG                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_CLKCFG_CCLKRATE              1            /*  Output Clock Rate */
#define BITP_A2B_CLKCFG_CCLKEN                0            /*  Enable Output Clock */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKCFG                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_CLKCFG_CCLKRATE             0x00000002
#define BITM_A2B_CLKCFG_CCLKEN               0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        CLKCFG                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_CLKCFG_CLKOUT_DIV512        0x00000060            /*  CLKOUT = pllclk/512 */
#define ENUM_A2B_CLKCFG_CLKOUT_DIV128        0x00000040            /*  CLKOUT = pllclk/128 */
#define ENUM_A2B_CLKCFG_CLKOUT_DIV16         0x00000010            /*  CLKOUT = pllclk/16 */
#define ENUM_A2B_CLKCFG_CLKOUT_DIV64         0x00000030            /*  CLKOUT = pllclk/64 */
#define ENUM_A2B_CLKCFG_USE_CCLKRATE         0x00000000            /*  Use CCLKRATE */
#define ENUM_A2B_CLKCFG_CLKOUT_DIV1024       0x00000070            /*  CLKOUT = pllclk/1024 */
#define ENUM_A2B_CLKCFG_CLKOUT_DIV32         0x00000020            /*  CLKOUT = pllclk/32 */
#define ENUM_A2B_CLKCFG_CLKOUT_DIV256        0x00000050            /*  CLKOUT = pllclk/256 */

#define REG_A2B0_CLKCFG_RESET                0x00000000            /*      Reset Value for CLKCFG */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODAT                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_GPIODAT_IO6DAT               6            /*  IO6 Output Data */
#define BITP_A2B_GPIODAT_IO5DAT               5            /*  IO5 Output Data */
#define BITP_A2B_GPIODAT_IO4DAT               4            /*  IO4 Output Data */
#define BITP_A2B_GPIODAT_IO3DAT               3            /*  IO3 Output Data */
#define BITP_A2B_GPIODAT_IO2DAT               2            /*  IO2 Output Data */
#define BITP_A2B_GPIODAT_IO1DAT               1            /*  IO1 Output Data */
#define BITP_A2B_GPIODAT_IO0DAT               0            /*  IO0 Output Data */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODAT                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_GPIODAT_IO6DAT              0x00000040
#define BITM_A2B_GPIODAT_IO5DAT              0x00000020
#define BITM_A2B_GPIODAT_IO4DAT              0x00000010
#define BITM_A2B_GPIODAT_IO3DAT              0x00000008
#define BITM_A2B_GPIODAT_IO2DAT              0x00000004
#define BITM_A2B_GPIODAT_IO1DAT              0x00000002
#define BITM_A2B_GPIODAT_IO0DAT              0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODAT                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_GPIODAT_IO6_High            0x00000040            /*  Output High */
#define ENUM_A2B_GPIODAT_IO6_Low             0x00000000            /*  Output Low */
#define ENUM_A2B_GPIODAT_IO5_High            0x00000020            /*  Output High */
#define ENUM_A2B_GPIODAT_IO5_Low             0x00000000            /*  Output Low */
#define ENUM_A2B_GPIODAT_IO4_High            0x00000010            /*  Output High */
#define ENUM_A2B_GPIODAT_IO4_Low             0x00000000            /*  Output Low */
#define ENUM_A2B_GPIODAT_IO3_High            0x00000008            /*  Output High */
#define ENUM_A2B_GPIODAT_IO3_Low             0x00000000            /*  Output Low */
#define ENUM_A2B_GPIODAT_IO2_High            0x00000004            /*  Output High */
#define ENUM_A2B_GPIODAT_IO2_Low             0x00000000            /*  Output Low */
#define ENUM_A2B_GPIODAT_IO1_High            0x00000002            /*  Output High */
#define ENUM_A2B_GPIODAT_IO1_Low             0x00000000            /*  Output Low */
#define ENUM_A2B_GPIODAT_IO0_High            0x00000001            /*  Output High */
#define ENUM_A2B_GPIODAT_IO0_Low             0x00000000            /*  Output Low */

#define REG_A2B0_GPIODAT_RESET               0x00000000            /*      Reset Value for GPIODAT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATSET                           Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_GPIODATSET_IO6DSET           6            /*  Write 1 to set IO6DAT */
#define BITP_A2B_GPIODATSET_IO5DSET           5            /*  Write 1 to set IO5DAT */
#define BITP_A2B_GPIODATSET_IO4DSET           4            /*  Write 1 to set IO4DAT */
#define BITP_A2B_GPIODATSET_IO3DSET           3            /*  Write 1 to set IO3DAT */
#define BITP_A2B_GPIODATSET_IO2DSET           2            /*  Write 1 to set IO2DAT */
#define BITP_A2B_GPIODATSET_IO1DSET           1            /*  Write 1 to set IO1DAT */
#define BITP_A2B_GPIODATSET_IO0DSET           0            /*  Write 1 to set IO0DAT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATSET                           Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_GPIODATSET_IO6DSET          0x00000040
#define BITM_A2B_GPIODATSET_IO5DSET          0x00000020
#define BITM_A2B_GPIODATSET_IO4DSET          0x00000010
#define BITM_A2B_GPIODATSET_IO3DSET          0x00000008
#define BITM_A2B_GPIODATSET_IO2DSET          0x00000004
#define BITM_A2B_GPIODATSET_IO1DSET          0x00000002
#define BITM_A2B_GPIODATSET_IO0DSET          0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATSET                           Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_GPIODATSET_IO6_Set          0x00000040            /*  Set Bit */
#define ENUM_A2B_GPIODATSET_IO6_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATSET_IO5_Set          0x00000020            /*  Set Bit */
#define ENUM_A2B_GPIODATSET_IO5_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATSET_IO4_Set          0x00000010            /*  Set Bit */
#define ENUM_A2B_GPIODATSET_IO4_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATSET_IO3_Set          0x00000008            /*  Set Bit */
#define ENUM_A2B_GPIODATSET_IO3_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATSET_IO2_Set          0x00000004            /*  Set Bit */
#define ENUM_A2B_GPIODATSET_IO2_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATSET_IO1_Set          0x00000002            /*  Set Bit */
#define ENUM_A2B_GPIODATSET_IO1_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATSET_IO0_Set          0x00000001            /*  Set Bit */
#define ENUM_A2B_GPIODATSET_IO0_No_Action    0x00000000            /*  No Action */

#define REG_A2B0_GPIODATSET_RESET            0x00000000            /*      Reset Value for GPIODATSET */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATCLR                           Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_GPIODATCLR_IO6DCLR           6            /*  Write 1 to clear IO6DAT */
#define BITP_A2B_GPIODATCLR_IO5DCLR           5            /*  Write 1 to clear IO5DAT */
#define BITP_A2B_GPIODATCLR_IO4DCLR           4            /*  Write 1 to clear IO4DAT */
#define BITP_A2B_GPIODATCLR_IO3DCLR           3            /*  Write 1 to clear IO3DAT */
#define BITP_A2B_GPIODATCLR_IO2DCLR           2            /*  Write 1 to clear IO2DAT */
#define BITP_A2B_GPIODATCLR_IO1DCLR           1            /*  Write 1 to clear IO1DAT */
#define BITP_A2B_GPIODATCLR_IO0DCLR           0            /*  Write 1 to clear IO0DAT */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATCLR                           Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_GPIODATCLR_IO6DCLR          0x00000040
#define BITM_A2B_GPIODATCLR_IO5DCLR          0x00000020
#define BITM_A2B_GPIODATCLR_IO4DCLR          0x00000010
#define BITM_A2B_GPIODATCLR_IO3DCLR          0x00000008
#define BITM_A2B_GPIODATCLR_IO2DCLR          0x00000004
#define BITM_A2B_GPIODATCLR_IO1DCLR          0x00000002
#define BITM_A2B_GPIODATCLR_IO0DCLR          0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIODATCLR                           Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_GPIODATCLR_IO6_Clear        0x00000040            /*  Clear Bit */
#define ENUM_A2B_GPIODATCLR_IO6_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATCLR_IO5_Clear        0x00000020            /*  Clear Bit */
#define ENUM_A2B_GPIODATCLR_IO5_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATCLR_IO4_Clear        0x00000010            /*  Clear Bit */
#define ENUM_A2B_GPIODATCLR_IO4_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATCLR_IO3_Clear        0x00000008            /*  Clear Bit */
#define ENUM_A2B_GPIODATCLR_IO3_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATCLR_IO2_Clear        0x00000004            /*  Clear Bit */
#define ENUM_A2B_GPIODATCLR_IO2_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATCLR_IO1_Clear        0x00000002            /*  Clear Bit */
#define ENUM_A2B_GPIODATCLR_IO1_No_Action    0x00000000            /*  No Action */
#define ENUM_A2B_GPIODATCLR_IO0_Clear        0x00000001            /*  Clear Bit */
#define ENUM_A2B_GPIODATCLR_IO0_No_Action    0x00000000            /*  No Action */

#define REG_A2B0_GPIODATCLR_RESET            0x00000000            /*      Reset Value for GPIODATCLR */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOOEN                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_GPIOOEN_IO6OEN               6            /*  IO6 Output Enable */
#define BITP_A2B_GPIOOEN_IO5OEN               5            /*  IO5 Output Enable */
#define BITP_A2B_GPIOOEN_IO4OEN               4            /*  IO4 Output Enable */
#define BITP_A2B_GPIOOEN_IO3OEN               3            /*  IO3 Output Enable */
#define BITP_A2B_GPIOOEN_IO2OEN               2            /*  IO2 Output Enable */
#define BITP_A2B_GPIOOEN_IO1OEN               1            /*  IO1 Output Enable */
#define BITP_A2B_GPIOOEN_IO0OEN               0            /*  IO0 Output Enable */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOOEN                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_GPIOOEN_IO6OEN              0x00000040
#define BITM_A2B_GPIOOEN_IO5OEN              0x00000020
#define BITM_A2B_GPIOOEN_IO4OEN              0x00000010
#define BITM_A2B_GPIOOEN_IO3OEN              0x00000008
#define BITM_A2B_GPIOOEN_IO2OEN              0x00000004
#define BITM_A2B_GPIOOEN_IO1OEN              0x00000002
#define BITM_A2B_GPIOOEN_IO0OEN              0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOOEN                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_GPIOOEN_IO6_En              0x00000040            /*  Enable */
#define ENUM_A2B_GPIOOEN_IO6_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOOEN_IO5_En              0x00000020            /*  Enable */
#define ENUM_A2B_GPIOOEN_IO5_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOOEN_IO4_En              0x00000010            /*  Enable */
#define ENUM_A2B_GPIOOEN_IO4_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOOEN_IO3_En              0x00000008            /*  Enable */
#define ENUM_A2B_GPIOOEN_IO3_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOOEN_IO2_En              0x00000004            /*  Enable */
#define ENUM_A2B_GPIOOEN_IO2_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOOEN_IO1_En              0x00000002            /*  Enable */
#define ENUM_A2B_GPIOOEN_IO1_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOOEN_IO0_En              0x00000001            /*  Enable */
#define ENUM_A2B_GPIOOEN_IO0_Dis             0x00000000            /*  Disable */

#define REG_A2B0_GPIOOEN_RESET               0x00000000            /*      Reset Value for GPIOOEN */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOIEN                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_GPIOIEN_IO6IEN               6            /*  IO6 Input Enable */
#define BITP_A2B_GPIOIEN_IO5IEN               5            /*  IO5 Input Enable */
#define BITP_A2B_GPIOIEN_IO4IEN               4            /*  IO4 Input Enable */
#define BITP_A2B_GPIOIEN_IO3IEN               3            /*  IO3 Input Enable */
#define BITP_A2B_GPIOIEN_IO2IEN               2            /*  IO2 Input Enable */
#define BITP_A2B_GPIOIEN_IO1IEN               1            /*  IO1 Input Enable */
#define BITP_A2B_GPIOIEN_IO0IEN               0            /*  IO0 Input Enable */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOIEN                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_GPIOIEN_IO6IEN              0x00000040
#define BITM_A2B_GPIOIEN_IO5IEN              0x00000020
#define BITM_A2B_GPIOIEN_IO4IEN              0x00000010
#define BITM_A2B_GPIOIEN_IO3IEN              0x00000008
#define BITM_A2B_GPIOIEN_IO2IEN              0x00000004
#define BITM_A2B_GPIOIEN_IO1IEN              0x00000002
#define BITM_A2B_GPIOIEN_IO0IEN              0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOIEN                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_GPIOIEN_IO6_En              0x00000040            /*  Enable */
#define ENUM_A2B_GPIOIEN_IO6_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOIEN_IO5_En              0x00000020            /*  Enable */
#define ENUM_A2B_GPIOIEN_IO5_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOIEN_IO4_En              0x00000010            /*  Enable */
#define ENUM_A2B_GPIOIEN_IO4_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOIEN_IO3_En              0x00000008            /*  Enable */
#define ENUM_A2B_GPIOIEN_IO3_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOIEN_IO2_En              0x00000004            /*  Enable */
#define ENUM_A2B_GPIOIEN_IO2_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOIEN_IO1_En              0x00000002            /*  Enable */
#define ENUM_A2B_GPIOIEN_IO1_Dis             0x00000000            /*  Disable */
#define ENUM_A2B_GPIOIEN_IO0_En              0x00000001            /*  Enable */
#define ENUM_A2B_GPIOIEN_IO0_Dis             0x00000000            /*  Disable */

#define REG_A2B0_GPIOIEN_RESET               0x00000000            /*      Reset Value for GPIOIEN */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOIN                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_GPIOIN_IO6IN                 6            /*  IO6 Input Value */
#define BITP_A2B_GPIOIN_IO5IN                 5            /*  IO5 Input Value */
#define BITP_A2B_GPIOIN_IO4IN                 4            /*  IO4 Input Value */
#define BITP_A2B_GPIOIN_IO3IN                 3            /*  IO3 Input Value */
#define BITP_A2B_GPIOIN_IO2IN                 2            /*  IO2 Input Value */
#define BITP_A2B_GPIOIN_IO1IN                 1            /*  IO1 Input Value */
#define BITP_A2B_GPIOIN_IO0IN                 0            /*  IO0 Input Value */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        GPIOIN                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_GPIOIN_IO6IN                0x00000040
#define BITM_A2B_GPIOIN_IO5IN                0x00000020
#define BITM_A2B_GPIOIN_IO4IN                0x00000010
#define BITM_A2B_GPIOIN_IO3IN                0x00000008
#define BITM_A2B_GPIOIN_IO2IN                0x00000004
#define BITM_A2B_GPIOIN_IO1IN                0x00000002
#define BITM_A2B_GPIOIN_IO0IN                0x00000001

#define REG_A2B0_GPIOIN_RESET                0x00000000            /*      Reset Value for GPIOIN */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTEN                               Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_PINTEN_IO6IE                 6            /*  DRX1/IO6 Interrupt Request Capability Enable */
#define BITP_A2B_PINTEN_IO5IE                 5            /*  DRX0/IO5 Interrupt Request Capability Enable */
#define BITP_A2B_PINTEN_IO4IE                 4            /*  DTX1/IO4 Interrupt Request Capability Enable */
#define BITP_A2B_PINTEN_IO3IE                 3            /*  DTX0/IO3 Interrupt Request Capability Enable */
#define BITP_A2B_PINTEN_IO2IE                 2            /*  ADR2/IO2 Interrupt Request Capability Enable */
#define BITP_A2B_PINTEN_IO1IE                 1            /*  ADR1/IO1 Interrupt Request Capability Enable */
#define BITP_A2B_PINTEN_IO0IE                 0            /*  IRQ/IO0 Interrupt Request Capability Enable */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTEN                               Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_PINTEN_IO6IE                0x00000040
#define BITM_A2B_PINTEN_IO5IE                0x00000020
#define BITM_A2B_PINTEN_IO4IE                0x00000010
#define BITM_A2B_PINTEN_IO3IE                0x00000008
#define BITM_A2B_PINTEN_IO2IE                0x00000004
#define BITM_A2B_PINTEN_IO1IE                0x00000002
#define BITM_A2B_PINTEN_IO0IE                0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTEN                               Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_PINTEN_IO6_En               0x00000040            /*  Enable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO6_Dis              0x00000000            /*  Disable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO5_En               0x00000020            /*  Enable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO5_Dis              0x00000000            /*  Disable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO4_En               0x00000010            /*  Enable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO4_Dis              0x00000000            /*  Disable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO3_En               0x00000008            /*  Enable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO3_Dis              0x00000000            /*  Disable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO2_En               0x00000004            /*  Enable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO2_Dis              0x00000000            /*  Disable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO1_En               0x00000002            /*  Enable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO1_Dis              0x00000000            /*  Disable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO0_En               0x00000001            /*  Enable Interrupt Request Capability */
#define ENUM_A2B_PINTEN_IO0_Dis              0x00000000            /*  Disable Interrupt Request Capability */

#define REG_A2B0_PINTEN_RESET                0x00000000            /*      Reset Value for PINTEN */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTINV                              Pos               Description */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_A2B_PINTINV_IO6INV               6            /*  Invert IO6 */
#define BITP_A2B_PINTINV_IO5INV               5            /*  Invert IO5 */
#define BITP_A2B_PINTINV_IO4INV               4            /*  Invert IO4 */
#define BITP_A2B_PINTINV_IO3INV               3            /*  Invert IO3 */
#define BITP_A2B_PINTINV_IO2INV               2            /*  Invert IO2 */
#define BITP_A2B_PINTINV_IO1INV               1            /*  Invert IO1 */
#define BITP_A2B_PINTINV_IO0INV               0            /*  Invert IO0 */
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTINV                              Masks  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define BITM_A2B_PINTINV_IO6INV              0x00000040
#define BITM_A2B_PINTINV_IO5INV              0x00000020
#define BITM_A2B_PINTINV_IO4INV              0x00000010
#define BITM_A2B_PINTINV_IO3INV              0x00000008
#define BITM_A2B_PINTINV_IO2INV              0x00000004
#define BITM_A2B_PINTINV_IO1INV              0x00000002
#define BITM_A2B_PINTINV_IO0INV              0x00000001
/* ------------------------------------------------------------------------------------------------------------------------- */
/*        PINTINV                              Enumerations  */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define ENUM_A2B_PINTINV_IO6_En              0x00000040            /*  Active Low (Falling Edge) */
#define ENUM_A2B_PINTINV_IO6_Dis             0x00000000            /*  Disable Inverter */
#define ENUM_A2B_PINTINV_IO5_En              0x00000020            /*  Active Low (Falling Edge) */
#define ENUM_A2B_PINTINV_IO5_Dis             0x00000000            /*  Active High (Rising Edge) */
#define ENUM_A2B_PINTINV_IO4_En              0x00000010            /*  Active Low (Falling Edge) */
#define ENUM_A2B_PINTINV_IO4_Dis             0x00000000            /*  Active High (Rising Edge) */
#define ENUM_A2B_PINTINV_IO3_En              0x00000008            /*  Active Low (Falling Edge) */
#define ENUM_A2B_PINTINV_IO3_Dis             0x00000000            /*  Active High (Rising Edge) */
#define ENUM_A2B_PINTINV_IO2_En              0x00000004            /*  Active Low (Falling Edge) */
#define ENUM_A2B_PINTINV_IO2_Dis             0x00000000            /*  Active High (Rising Edge) */
#define ENUM_A2B_PINTINV_IO1_En              0x00000002            /*  Active Low (Falling Edge) */
#define ENUM_A2B_PINTINV_IO1_Dis             0x00000000            /*  Active High (Rising Edge) */
#define ENUM_A2B_PINTINV_IO0_En              0x00000001            /*  Active Low (Falling Edge) */
#define ENUM_A2B_PINTINV_IO0_Dis             0x00000000            /*  Active High (Rising Edge) */

#define REG_A2B0_PINTINV_RESET               0x00000000            /*      Reset Value for PINTINV */
/* ============================================================================================================================ */
/*    Trigger Master Definitions */
/* ============================================================================================================================ */

/* ============================================================================================================================ */
/*    Trigger Slave Definitions */
/* ============================================================================================================================ */

/* ------------------------------------------------------------------------------------------------------------------------- */
/*      */
/*  A2B (A2B) Parameters */
/*      */
/* ------------------------------------------------------------------------------------------------------------------------- */
#define PARAM_A2B0_ALLREGS                            1 
#define PARAM_A2B0_MASTER                             0 
#define PARAM_A2B0_REV1                               1 

/* ============================================================================ */
/*     Memory Map Macros */
/* ============================================================================ */


/* ADSP-AD2410 is a single-core processor */
#define MEM_NUM_CORES    

#endif	/* end ifndef _DEF_AD2410_H */
