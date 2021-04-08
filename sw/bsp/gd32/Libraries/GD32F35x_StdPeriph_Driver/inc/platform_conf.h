  /**
  ******************************************************************************
  * @file    platform_config.h 
  * @author  Hardware  Team
  * @version V1.0
  * @date    03/12/2014
  * @brief   GD32150R-EVAl board specific configuration file.
  ******************************************************************************/
  


#ifndef __PLATFORM_CONF_H
#define __PLATFORM_CONF_H



/***** LED����ض���-----------------------------------------------------------**/

  #define RCC_GPIO_LED                    RCC_AHBPeriph_GPIOC     /*LEDʹ�õ�GPIOʱ��*/
  #define LEDn                            3                       /*LED����*/
  #define GPIO_LED                        GPIOC                   /*LED��ʹ�õ�GPIO��*/
  
  #define DS1_PIN                         GPIO_Pin_10             /*DS1ʹ�õ�GPIO�ܽ�*/
  #define DS2_PIN                         GPIO_Pin_11			  /*DS2ʹ�õ�GPIO�ܽ�*/
  #define DS3_PIN                         GPIO_Pin_12  			  /*DS3ʹ�õ�GPIO�ܽ�*/
 
  #define GPIO_LED_ALL                    (DS1_PIN |DS2_PIN |DS3_PIN )
  #define ON                              1
  #define OFF                             0
 /*KEY��ض���---------------------------------------------------------------*/
  #define RCC_KEY1                                    RCC_AHBPeriph_GPIOA
  #define GPIO_KEY1_PORT                              GPIOA    
  #define GPIO_KEY1                                   GPIO_Pin_0

  #define RCC_KEY2                                    RCC_AHBPeriph_GPIOF
  #define GPIO_KEY2_PORT                              GPIOF    
  #define GPIO_KEY2                                   GPIO_Pin_7
    
  #define RCC_KEY3                                    RCC_AHBPeriph_GPIOC
  #define GPIO_KEY3_PORT                              GPIOC    
  #define GPIO_KEY3                                   GPIO_Pin_13 
  
  #define GPIO_KEY_ANTI_TAMP                          GPIO_KEY3
  #define GPIO_KEY_WEAK_UP                            GPIO_KEY1
  
  
  /* Values magic to the Board keys */
  #define  NOKEY  0
  #define  KEY1   1
  #define  KEY2   2
  #define  KEY3   3

#endif
/* __PLATFORM_CONFIG_H */

