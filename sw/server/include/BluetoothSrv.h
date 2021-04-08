#include "product.config"

#if defined(BLUETOOTH_V3)
#include "BluetoothSrv_v3.h"
#elif defined(BLUETOOTH_V2)
#include "BluetoothSrv_v2.h"
#else
#include "BluetoothSrv_v1.h"
#endif
