#ifndef DSP_TLV320AIC3254_DRIVER_CONFIG_H
#define DSP_TLV320AIC3254_DRIVER_CONFIG_H

#define TLV320_VOLUME_REG   0x6c    
#define TLV320_VOLUME_PAGE  0x0A


#define STEREOMUX_PAGE              0x0B
#define STEREOMUX_REG               0x50
#define STEREOMUX_TARGET_PROCESSOR  TLV320AIC_BUFFER_A
#define ADPT_FILTER_REG             0x01
#define MFP3_REG                    0x38

#define MUSIC_DETECT_PAGE           11
#define MUSIC_DETECT_REG            76

#define PLAIN_EQ_MUX_0_PAGE         11
#define PLAIN_EQ_MUX_0_REG          84
#define PLAIN_EQ_MUX_0_BUFFER       TLV320AIC_BUFFER_A

#define PLAIN_EQ_MUX_1_PAGE         48
#define PLAIN_EQ_MUX_1_REG          72
#define PLAIN_EQ_MUX_1_BUFFER       TLV320AIC_BUFFER_D

//#define FLAT_EQ

#endif /* DSP_TLV320AIC3254_DRIVER_CONFIG_H */
