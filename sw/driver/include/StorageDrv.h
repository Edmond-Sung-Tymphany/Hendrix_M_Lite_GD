/**
 * @file        StorageDrv.h
 * @brief       It's the general storage driver
 * @author      Johnny Fan 
 * @date        2014-03-30
 * @copyright   Tymphany Ltd.
 */

#ifndef STORAGEDRV_H
#define STORAGEDRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "deviceTypes.h"


CLASS(cStorageDrv)
    /* private data */
    const tStorageDevice* pStorageConfig;
    /* Storage the value*/
    bool(*SetValue)(cStorageDrv *me, uint32 addr, uint8* pBuf, uint32 sizeInBytes);
    /* read the value from storage*/
    bool(*GetValue)(cStorageDrv *me, uint32 addr, uint8* pBuf, uint32 sizeInBytes);
    /* erage a page*/
    bool(*ErasePage)(cStorageDrv *me, uint32 addr);
#ifdef HAS_CBUFFER
/*@brief           Store data in a circle buffer. Buffer address and data slot size
*                  are defined in NvmDrv.config.
*                  Inspired by http://www.atmel.com/images/doc2526.pdf
* @param    me     Storage Driver object
* @param    bufId  Buffer id. It coincides with buffer element index in cycBufPubConf.
* @param    pBuf   Pointer to data to be stored.
* @return   TRUE - store operation was successful. FALSE - store operation was not successful.
* @note            Each write operation involves the whole chunk of data pBuf points to.
*                  The size of that data is defined in NvmDrv.config.
*                  array cycBufPubConf, field dataLength. bufId is a sequential number
*                  of the struct in the array.
*                  cBuffer component design documentation:
*                  http://flow.tymphany.com/redmine/projects/tooling/dmsf?folder_id=16098
*/
    bool(*SetValueCBuffer)(cStorageDrv *me, uint8 bufId, uint8* pBuf);

/*@brief           Read data from a circle buffer. Buffer address and data slot size
*                  are defined in NvmDrv.config.
*                  Inspired by http://www.atmel.com/images/doc2526.pdf
* @param    me     Storage Driver object
* @param    bufId  Buffer id. It coincides with buffer element index in cycBufPubConf.
* @param    pBuf   Pointer to output buffer.
* @return   TRUE - read operation was successful. FALSE - read operation was not successful.
* @note            Each read operation involves the whole chunk of data pBuf points to.
*                  The size of that data is defined in NvmDrv.config.
*                  array cycBufPubConf, field dataLength. bufId is a sequential number
*                  of the struct in the array.
*                  cBuffer component design documentation:
*                  http://flow.tymphany.com/redmine/projects/tooling/dmsf?folder_id=16098
*/
    bool(*GetValueCBuffer)(cStorageDrv *me, uint8 bufId, uint8* pBuf);
#endif
METHODS
    /* public functions */
void StorageDrv_Ctor(cStorageDrv *me, const tStorageDevice* pStorageConfig);
void StorageDrv_Xtor(cStorageDrv *me);
END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* STORAGEDRV_H */

