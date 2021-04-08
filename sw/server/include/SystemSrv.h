/*****************************************************************************
*  @file      SystemSrv.h 
*  @brief     Header file for base system server class
*  @author    Viking Wang
*  @date      2016-07-22
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef __SYSTEM_SERVER_H__
#define	__SYSTEM_SERVER_H__

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum
{
    SYSTEM_SET_ID_AMP_SD,
    SYSTEM_SET_ID_AMP_UNMUTE,
    SYSTEM_SET_ID_MAX
}SystemSet_ID_t;

typedef struct tagSystemSetEvt
{
	QEvt super; 		
	QActive * sender;
    SystemSet_ID_t  setting_id;
    BOOL          enable;
    uint32_t      param;
}SystemSetEvt_t;

typedef struct tagSystemServer
{
    cServer super_;
    int16_t timer;
}cSystemSrv;

void SystemSrv_StartUp(cPersistantObj *me);
void SystemSrv_ShutDown(cPersistantObj *me);
void SystemSrv_Set(SystemSet_ID_t set_id, BOOL enable, uint32_t param);

#ifdef	__cplusplus
}
#endif

#endif	/* __SYSTEM_SERVER_H__ */

