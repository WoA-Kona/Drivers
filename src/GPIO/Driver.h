#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>
#include <gpioclx.h>
#include <TLMM.h>

#include "trace.h"

EXTERN_C_START

#define GENMASK(h, l) (((~0U) >> (sizeof(unsigned int) * 8 - (h) - 1)) & ~((1U << (l)) - 1))
#define BIT(n) (1U << (n))

#define GPIO_PER_BANK 64
#define GPIO_COUNT 180

#define WEST  0x00000000
#define SOUTH 0x00400000
#define NORTH 0x00800000

#define CTL_REG(x) ((x) * 0x1000)
#define IO_REG(x) ((x) * 0x1000 + 0x4)
#define INTR_REG(x) ((x) * 0x1000 + 0x8)
#define INTR_STATUS_REG(x) ((x) * 0x1000 + 0xC)

#define INTR_TARGET_MASK GENMASK(2, 0)
#define PULL_MASK GENMASK(1, 0)
#define MUX_MASK GENMASK(5, 2)

#define PULL_BIT 0
#define IN_BIT   0
#define OUT_BIT  1
#define MUX_BIT  2
#define OE_BIT   9

#define INTR_STATUS_BIT 0
#define INTR_ENABLE_BIT 0
#define INTR_POLARITY_BIT 1
#define INTR_DETECTION_BIT 2
#define INTR_TARGET_KPSS_VAL 3
#define INTR_RAW_STATUS_BIT 4
#define INTR_TARGET_BIT 5

#define NO_PULL   0
#define PULL_DOWN 1
#define PULL_UP   3

typedef struct _GPIO_DEVICE_CONTEXT
{
    ULONG_PTR Base;
    ULONG Length;
} GPIO_DEVICE_CONTEXT, * PGPIO_DEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(GPIO_DEVICE_CONTEXT, DeviceGetContext)

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_OBJECT_CONTEXT_CLEANUP GPIOEvtDriverContextCleanup;

EVT_WDF_DRIVER_DEVICE_ADD GPIOEvtDeviceAdd;
EVT_WDF_DRIVER_UNLOAD GPIOEvtDriverUnload;

GPIO_CLIENT_PREPARE_CONTROLLER GPIOPrepareController;
GPIO_CLIENT_RELEASE_CONTROLLER GPIOReleaseController;
GPIO_CLIENT_QUERY_CONTROLLER_BASIC_INFORMATION GPIOQueryControllerBasicInformation;

GPIO_CLIENT_START_CONTROLLER GPIOStartController;
GPIO_CLIENT_STOP_CONTROLLER GPIOStopController;

GPIO_CLIENT_CONNECT_IO_PINS GPIOConnectIoPins;
GPIO_CLIENT_DISCONNECT_IO_PINS GPIODisconnectIoPins;
GPIO_CLIENT_READ_PINS GPIOReadGpioPins;
GPIO_CLIENT_WRITE_PINS GPIOWriteGpioPins;

GPIO_CLIENT_ENABLE_INTERRUPT GPIOEnableInterrupt;
GPIO_CLIENT_DISABLE_INTERRUPT GPIODisableInterrupt;
GPIO_CLIENT_RECONFIGURE_INTERRUPT GPIOReconfigureInterrupt;

GPIO_CLIENT_MASK_INTERRUPTS GPIOMaskInterrupts;
GPIO_CLIENT_UNMASK_INTERRUPT GPIOUnmaskInterrupt;

GPIO_CLIENT_QUERY_ACTIVE_INTERRUPTS GPIOQueryActiveInterrupts;
GPIO_CLIENT_CLEAR_ACTIVE_INTERRUPTS GPIOClearActiveInterrupts;

NTSTATUS GPIOConfigurePinMux(
    PVOID Context,
    PIN_NUMBER pin,
    UCHAR function
    );

EXTERN_C_END
