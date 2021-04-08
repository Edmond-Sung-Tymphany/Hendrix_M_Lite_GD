#ifndef DSP_TLV320AIC3254_DRIVER_CONFIG_H
#define DSP_TLV320AIC3254_DRIVER_CONFIG_H

#define TLV320_VOLUME_REG   0x6c    
#define TLV320_VOLUME_PAGE  0x0A


#define STEREOMUX_PAGE              0x0B
#define STEREOMUX_REG               0x50
#define STEREOMUX_TARGET_PROCESSOR  TLV320AIC_BUFFER_A
#define ADPT_FILTER_REG             0x01
#define MFP3_REG                    0x38

#define MUSIC_DETECT_PAGE           10
#define MUSIC_DETECT_REG            36



//#define FLAT_EQ

#endif /* DSP_TLV320AIC3254_DRIVER_CONFIG_H */
