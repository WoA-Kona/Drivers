#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>

#include "device.h"
#include "trace.h"

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD SPMIEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP SPMIEvtDriverContextCleanup;

EXTERN_C_END
