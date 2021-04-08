#include "product.config"
#ifdef BLUETOOTH_V3
#include "BluetoothDrv_v3.h"
#elif defined BLUETOOTH_V2
#include "BluetoothDrv_v2.h"
#else
#include "BluetoothDrv_v1.h"
#endif