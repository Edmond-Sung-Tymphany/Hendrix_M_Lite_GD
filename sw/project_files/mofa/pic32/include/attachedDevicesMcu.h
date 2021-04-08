/**
* @file attachedDevicesMcu.h
* @brief The devices attached to the product.
* @author Gavin Lee
* @date 11-April-2014
* @copyright Tymphany Ltd.
*/

/* TODO: Auto generate this data */
#ifndef ATTACHEDDEVICESMCU_H
#define ATTACHEDDEVICESMCU_H

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 *  General settings configs go here
 ******************************************************************************/
/*  SAM will check PRODUCT_VERSION_NUM to determine upgrade or not.
 *  When you update version, please remember to update
 *    \tymphany_platform\sw\test_tools\cdk\tym_tool\gen_tym_bundle.bat
 */
#define STRINGIFY2( x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#define PASTE2( a, b) a##b
#define PASTE( a, b) PASTE2( a, b)

#define MAJOR_VER 3
#define MINOR_VER1 0
#define MINOR_VER2 2
#define PRODUCT_VERSION_MCU STRINGIFY(PASTE(MAJOR_VER.MINOR_VER1, MINOR_VER2))



#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICESMCU_H */
