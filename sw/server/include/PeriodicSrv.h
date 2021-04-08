#ifndef __PERIODIC_SERVER_H__
#define __PERIODIC_SERVER_H__


#define PERIODIC_SRV_TIMEOUT_IN_MS      50


typedef struct tagPeriodicSrv
{
	cServer super_; 
}cPeriodicSrv;


void PeriodicSrv_StartUp(cPersistantObj *me);
void PeriodicSrv_ShutDown(cPersistantObj *me);

#endif  // __PERIODIC_SERVER_H__

