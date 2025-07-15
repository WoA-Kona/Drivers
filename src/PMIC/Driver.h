#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>

#include "device.h"
#include "queue.h"
#include "trace.h"

#define POOL_TAG 'CIMP'

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD PMICEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP PMICEvtDriverContextCleanup;

EXTERN_C_END
