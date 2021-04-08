#ifndef __CS8422_PRIV_H__
#define __CS8422_PRIV_H__

/* data routing control register : 0Ah */
// SDOUT1 source
#define SDOUT1_SOURCE_SDIN   0x80
#define SDOUT1_SOURCE_SRC    0x00
#define SDOUT1_SOURCE_AES3   0x40
// SDOUT2 source
#define SDOUT2_SOURCE_SDIN   0x20
#define SDOUT2_SOURCE_SRC    0x00
#define SDOUT2_SOURCE_AES3   0x10
// SDOUT1 mute
#define SDOUT1_MUTED        0x08
#define SDOUT1_UNMUTE       0x00
// SDOUT2 mute
#define SDOUT2_MUTED        0x04
#define SDOUT2_UNMUTE       0x00
// SRC source
#define SRC_SOURCE_SDIN     0x00
#define SRC_SOURCE_AES3     0x02

/* SAI control register : 0Bh */
#define SAIMS_MASTER        0x80
#define SAIMS_SLAVE         0x00
// I2S format
#define SAIF_LJ     0x00
#define SAIF_I2S    0x08

/* SAO control register : 0Ch */
#define SAOMS_MASTER        0x80
#define SAOMS_SLAVE         0x00
// resolution
#define SAORES_24BITS       0x00
#define SAORES_16BTIS       0x30
// I2S format
#define SAOF_LJ         0x00
#define SAOF_I2S        0x04


#endif  // __CS8422_PRIV_H__

