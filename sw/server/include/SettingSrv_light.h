#ifndef SETTINGSRV_LIGHT_H
#define SETTINGSRV_LIGHT_H
#include "setting_id.h"  


void Setting_Bookkeeping(void);
void Setting_Set(eSettingId id, const void* pValue);
#endif
