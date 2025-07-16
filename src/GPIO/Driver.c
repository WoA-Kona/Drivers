#include "driver.h"
#include "driver.tmh"
#include <gpioclx.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, GPIOEvtDeviceAdd)
#pragma alloc_text (PAGE, GPIOEvtDriverContextCleanup)
#pragma alloc_text (PAGE, GPIOEvtDriverUnload)
#endif

#define Read32 READ_REGISTER_NOFENCE_ULONG
#define Write32 WRITE_REGISTER_NOFENCE_ULONG

static const UINT32 TileOffsets[] = {
    [0] = SOUTH,   [1] = SOUTH,   [2] = SOUTH,   [3] = SOUTH,   [4] = NORTH,   [5] = NORTH,
    [6] = NORTH,   [7] = NORTH,   [8] = NORTH,   [9] = NORTH,   [10] = NORTH,  [11] = NORTH,
    [12] = NORTH,  [13] = NORTH,  [14] = NORTH,  [15] = NORTH,  [16] = NORTH,  [17] = NORTH,
    [18] = NORTH,  [19] = NORTH,  [20] = NORTH,  [21] = NORTH,  [22] = NORTH,  [23] = NORTH,
    [24] = SOUTH,  [25] = SOUTH,  [26] = SOUTH,  [27] = SOUTH,  [28] = NORTH,  [29] = NORTH,
    [30] = NORTH,  [31] = NORTH,  [32] = SOUTH,  [33] = SOUTH,  [34] = SOUTH,  [35] = SOUTH,
    [36] = SOUTH,  [37] = SOUTH,  [38] = SOUTH,  [39] = SOUTH,  [40] = SOUTH,  [41] = SOUTH,
    [42] = SOUTH,  [43] = SOUTH,  [44] = SOUTH,  [45] = SOUTH,  [46] = SOUTH,  [47] = SOUTH,
    [48] = SOUTH,  [49] = SOUTH,  [50] = SOUTH,  [51] = SOUTH,  [52] = SOUTH,  [53] = SOUTH,
    [54] = SOUTH,  [55] = SOUTH,  [56] = SOUTH,  [57] = SOUTH,  [58] = SOUTH,  [59] = SOUTH,
    [60] = SOUTH,  [61] = SOUTH,  [62] = SOUTH,  [63] = SOUTH,  [64] = SOUTH,  [65] = SOUTH,
    [66] = NORTH,  [67] = NORTH,  [68] = NORTH,  [69] = SOUTH,  [70] = SOUTH,  [71] = SOUTH,
    [72] = SOUTH,  [73] = SOUTH,  [74] = SOUTH,  [75] = SOUTH,  [76] = SOUTH,  [77] = NORTH,
    [78] = NORTH,  [79] = NORTH,  [80] = NORTH,  [81] = NORTH,  [82] = NORTH,  [83] = NORTH,
    [84] = NORTH,  [85] = SOUTH,  [86] = SOUTH,  [87] = SOUTH,  [88] = SOUTH,  [89] = SOUTH,
    [90] = SOUTH,  [91] = SOUTH,  [92] = NORTH,  [93] = NORTH,  [94] = NORTH,  [95] = NORTH,
    [96] = NORTH,  [97] = NORTH,  [98] = NORTH,  [99] = NORTH,  [100] = NORTH, [101] = NORTH,
    [102] = NORTH, [103] = NORTH, [104] = NORTH, [105] = NORTH, [106] = NORTH, [107] = NORTH,
    [108] = NORTH, [109] = NORTH, [110] = NORTH, [111] = NORTH, [112] = NORTH, [113] = NORTH,
    [114] = NORTH, [115] = NORTH, [116] = NORTH, [117] = NORTH, [118] = NORTH, [119] = NORTH,
    [120] = NORTH, [121] = NORTH, [122] = NORTH, [123] = NORTH, [124] = NORTH, [125] = SOUTH,
    [126] = SOUTH, [127] = SOUTH, [128] = SOUTH, [129] = SOUTH, [130] = SOUTH, [131] = SOUTH,
    [132] = SOUTH, [133] = WEST,  [134] = WEST,  [135] = WEST,  [136] = WEST,  [137] = WEST,
    [138] = WEST,  [139] = WEST,  [140] = WEST,  [141] = WEST,  [142] = WEST,  [143] = WEST,
    [144] = WEST,  [145] = WEST,  [146] = WEST,  [147] = WEST,  [148] = WEST,  [149] = WEST,
    [150] = WEST,  [151] = WEST,  [152] = WEST,  [153] = WEST,  [154] = WEST,  [155] = WEST,
    [156] = WEST,  [157] = WEST,  [158] = WEST,  [159] = WEST,  [160] = WEST,  [161] = WEST,
    [162] = WEST,  [163] = WEST,  [164] = WEST,  [165] = WEST,  [166] = WEST,  [167] = WEST,
    [168] = WEST,  [169] = WEST,  [170] = WEST,  [171] = WEST,  [172] = WEST,  [173] = WEST,
    [174] = WEST,  [175] = WEST,  [176] = WEST,  [177] = WEST,  [178] = WEST,  [179] = WEST,
};

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    WDFDRIVER Driver;
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;
    GPIO_CLIENT_REGISTRATION_PACKET registrationPacket;

    //
    // Initialize WPP Tracing
    //
    WPP_INIT_TRACING(DriverObject, RegistryPath);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = GPIOEvtDriverContextCleanup;

    WDF_DRIVER_CONFIG_INIT(&config, GPIOEvtDeviceAdd);
    config.EvtDriverUnload = GPIOEvtDriverUnload;

    status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             WDF_NO_OBJECT_ATTRIBUTES,
                             &config,
                             &Driver
                             );

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfDriverCreate failed %!STATUS!", status);
        WPP_CLEANUP(DriverObject);
        return status;
    }

    RtlZeroMemory(&registrationPacket, sizeof(GPIO_CLIENT_REGISTRATION_PACKET));
    registrationPacket.Version = GPIO_CLIENT_VERSION;
    registrationPacket.Size = sizeof(GPIO_CLIENT_REGISTRATION_PACKET);

    registrationPacket.ControllerContextSize = sizeof(GPIO_DEVICE_CONTEXT);

    registrationPacket.CLIENT_PrepareController = GPIOPrepareController;
    registrationPacket.CLIENT_ReleaseController = GPIOReleaseController;

    registrationPacket.CLIENT_QueryControllerBasicInformation = GPIOQueryControllerBasicInformation;

    registrationPacket.CLIENT_StartController = GPIOStartController;
    registrationPacket.CLIENT_StopController = GPIOStopController;

    registrationPacket.CLIENT_ConnectIoPins = GPIOConnectIoPins;
    registrationPacket.CLIENT_DisconnectIoPins = GPIODisconnectIoPins;

    registrationPacket.CLIENT_ReadGpioPins = GPIOReadGpioPins;
    registrationPacket.CLIENT_WriteGpioPins = GPIOWriteGpioPins;

    registrationPacket.CLIENT_EnableInterrupt = GPIOEnableInterrupt;
    registrationPacket.CLIENT_DisableInterrupt = GPIODisableInterrupt;
    registrationPacket.CLIENT_ReconfigureInterrupt = GPIOReconfigureInterrupt;

    registrationPacket.CLIENT_MaskInterrupts = GPIOMaskInterrupts;
    registrationPacket.CLIENT_UnmaskInterrupt = GPIOUnmaskInterrupt;

    registrationPacket.CLIENT_QueryActiveInterrupts = GPIOQueryActiveInterrupts;
    registrationPacket.CLIENT_ClearActiveInterrupts = GPIOClearActiveInterrupts;

    status = GPIO_CLX_RegisterClient(Driver, &registrationPacket, RegistryPath);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

NTSTATUS
GPIOEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDFDEVICE device;

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    status = GPIO_CLX_ProcessAddDevicePreDeviceCreate(Driver, DeviceInit, &deviceAttributes);
    if (!NT_SUCCESS(status))
        goto Exit;

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status))
        goto Exit;

    status = WdfDeviceCreateDeviceInterface(device, &GUID_TLMM_DEVINTERFACE, NULL);
    if (!NT_SUCCESS(status))
        goto Exit;

    status = GPIO_CLX_ProcessAddDevicePostDeviceCreate(Driver, device);
    if (!NT_SUCCESS(status))
        goto Exit;

Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}

_Must_inspect_result_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GPIOPrepareController(
    _In_ WDFDEVICE Device,
    _In_ PVOID Context,
    _In_ WDFCMRESLIST ResourcesRaw,
    _In_ WDFCMRESLIST ResourcesTranslated
    )
{
    NTSTATUS status;
    PGPIO_DEVICE_CONTEXT gpioContext;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;
    ULONG resourceCount;

    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(ResourcesRaw);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    status = STATUS_INSUFFICIENT_RESOURCES;

    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;

    resourceCount = WdfCmResourceListGetCount(ResourcesTranslated);
    for (ULONG Index = 0; Index < resourceCount; Index++) {
        descriptor = WdfCmResourceListGetDescriptor(ResourcesTranslated, Index);
        switch (descriptor->Type) {
        case CmResourceTypeMemory:
            if (gpioContext->Base == 0) {
                gpioContext->Base = (ULONG_PTR)MmMapIoSpaceEx(descriptor->u.Memory.Start, descriptor->u.Memory.Length, PAGE_NOCACHE | PAGE_READWRITE);
                gpioContext->Length = descriptor->u.Memory.Length;

                status = STATUS_SUCCESS;

                if (gpioContext->Base == 0) {
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "MmMapIoSpaceEx failed!");
                    status = STATUS_UNSUCCESSFUL;
                }
            }
            break;
        default:
            break;
        }
        if (!NT_SUCCESS(status))
            goto Exit;
    }

    {
        TLMM_INTERFACE tlmmInterface;
        WDF_QUERY_INTERFACE_CONFIG queryInterfaceConfig;
        RtlZeroMemory(&tlmmInterface, sizeof(TLMM_INTERFACE));

        tlmmInterface.Interface.Size = sizeof(TLMM_INTERFACE);
        tlmmInterface.Interface.Version = 1;
        tlmmInterface.Interface.Context = (PVOID)gpioContext;

        tlmmInterface.Interface.InterfaceReference = WdfDeviceInterfaceReferenceNoOp;
        tlmmInterface.Interface.InterfaceDereference = WdfDeviceInterfaceDereferenceNoOp;

        tlmmInterface.ConfigurePinMux = GPIOConfigurePinMux;

        WDF_QUERY_INTERFACE_CONFIG_INIT(
            &queryInterfaceConfig,
            (PINTERFACE)&tlmmInterface,
            &GUID_TLMM_INTERFACE,
            NULL
        );

        status = WdfDeviceAddQueryInterface(
            Device,
            &queryInterfaceConfig
        );

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Failed to add interface");
            goto Exit;
        }
    }

Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}

_Must_inspect_result_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GPIOReleaseController(
    _In_ WDFDEVICE Device,
    _In_ PVOID Context
    )
{
    PGPIO_DEVICE_CONTEXT gpioContext;
    UNREFERENCED_PARAMETER(Device);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;
    if (gpioContext->Base != 0) {
        MmUnmapIoSpace((PULONG)gpioContext->Base, gpioContext->Length);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

_Must_inspect_result_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GPIOQueryControllerBasicInformation(
    _In_ PVOID Context,
    _Out_ PCLIENT_CONTROLLER_BASIC_INFORMATION ControllerInformation
    )
{
    UNREFERENCED_PARAMETER(Context);

    ControllerInformation->Version = GPIO_CONTROLLER_BASIC_INFORMATION_VERSION;
    ControllerInformation->Size = sizeof(CLIENT_CONTROLLER_BASIC_INFORMATION);

    ControllerInformation->TotalPins = GPIO_COUNT;
    ControllerInformation->NumberOfPinsPerBank = GPIO_PER_BANK;

    ControllerInformation->Flags.MemoryMappedController = TRUE;
    ControllerInformation->Flags.EmulateDebouncing = TRUE;
    ControllerInformation->Flags.EmulateActiveBoth = TRUE;

    return STATUS_SUCCESS;
}

_Must_inspect_result_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GPIOStartController(
    _In_ PVOID Context,
    _In_ BOOLEAN RestoreContext,
    _In_ WDF_POWER_DEVICE_STATE PreviousPowerState
    )
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(RestoreContext);
    UNREFERENCED_PARAMETER(PreviousPowerState);

    return STATUS_SUCCESS;
}

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GPIOStopController(
    _In_ PVOID Context,
    _In_ BOOLEAN SaveContext,
    _In_ WDF_POWER_DEVICE_STATE TargetState
)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(SaveContext);
    UNREFERENCED_PARAMETER(TargetState);

    return STATUS_SUCCESS;
}

UINT32
GetTileOffsetByPin(
    _In_ PIN_NUMBER PinNumber
    )
{
    return TileOffsets[PinNumber];
}

NTSTATUS
GPIOConfigurePin(
    _In_ GPIO_DEVICE_CONTEXT* Context,
    _In_ PIN_NUMBER pinNumber,
    _In_ GPIO_CONNECT_IO_PINS_MODE ConnectMode,
    _In_ UCHAR PullConfiguration
    )
{
    NTSTATUS status;
    UINT32 pinOffset;
    ULONG ctlReg;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    status = STATUS_SUCCESS;

    pinOffset = GetTileOffsetByPin(pinNumber) + CTL_REG(pinNumber);
    ctlReg = Read32((PULONG)(Context->Base + pinOffset));

    switch (PullConfiguration) {
    case GPIO_PIN_PULL_CONFIGURATION_NONE:
        ctlReg &= PULL_MASK;
        ctlReg |= NO_PULL;
        break;
    case GPIO_PIN_PULL_CONFIGURATION_PULLUP:
        ctlReg &= PULL_MASK;
        ctlReg |= PULL_UP;
        break;
    case GPIO_PIN_PULL_CONFIGURATION_PULLDOWN:
        ctlReg &= PULL_MASK;
        ctlReg |= PULL_DOWN;
        break;
    case GPIO_PIN_PULL_CONFIGURATION_DEFAULT:
        break;
    default:
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
        break;
    }

    switch (ConnectMode) {
    case ConnectModeInput:
        ctlReg &= ~BIT(OE_BIT);
        break;
    case ConnectModeOutput:
        ctlReg |= BIT(OE_BIT);
        break;
    default:
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
        break;
    }

    Write32((PULONG)(Context->Base + pinOffset), ctlReg);

Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}

_Must_inspect_result_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GPIOConnectIoPins(
    _In_ PVOID Context,
    _In_ PGPIO_CONNECT_IO_PINS_PARAMETERS ConnectParameters
    )
{
    NTSTATUS status;
    PGPIO_DEVICE_CONTEXT gpioContext;
    PPIN_NUMBER pinNumberTable;
    PIN_NUMBER pinNumber;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    status = STATUS_SUCCESS;

    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;

    pinNumberTable = ConnectParameters->PinNumberTable;
    for (ULONG index = 0; index < ConnectParameters->PinCount; index++) {
        pinNumber = pinNumberTable[index] + ConnectParameters->BankId * GPIO_PER_BANK;

        GPIOConfigurePin(gpioContext, pinNumber, ConnectParameters->ConnectMode, ConnectParameters->PullConfiguration);
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "GPIO[%d] mode: %d, pull: %d", pinNumber, ConnectParameters->ConnectMode, ConnectParameters->PullConfiguration);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}

_Must_inspect_result_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GPIODisconnectIoPins(
    _In_ PVOID Context,
    _In_ PGPIO_DISCONNECT_IO_PINS_PARAMETERS DisconnectParameters
    )
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(DisconnectParameters);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

_Must_inspect_result_
NTSTATUS
GPIOReadGpioPins(
    _In_ PVOID Context,
    _In_ PGPIO_READ_PINS_PARAMETERS ReadParameters
)
{
    PGPIO_DEVICE_CONTEXT gpioContext;
    PPIN_NUMBER pinNumberTable;
    PIN_NUMBER pinNumber;
    UINT32 pinOffset;
    ULONG ioReg;
    RTL_BITMAP bitMap;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;
    RtlInitializeBitMap(&bitMap, ReadParameters->Buffer, GPIO_PER_BANK);

    pinNumberTable = ReadParameters->PinNumberTable;
    for (ULONG index = 0; index < ReadParameters->PinCount; index++) {
        pinNumber = pinNumberTable[index] + ReadParameters->BankId * GPIO_PER_BANK;

        pinOffset = GetTileOffsetByPin(pinNumber) + IO_REG(pinNumber);

        ioReg = Read32((PULONG)(gpioContext->Base + pinOffset));

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Read GPIO[%d]=%d", pinNumber, (ioReg & BIT(IN_BIT)));

        if(ioReg & BIT(IN_BIT))
            RtlSetBit(&bitMap, index);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

_Must_inspect_result_
NTSTATUS
GPIOWriteGpioPins(
    _In_ PVOID Context,
    _In_ PGPIO_WRITE_PINS_PARAMETERS WriteParameters
)
{
    PGPIO_DEVICE_CONTEXT gpioContext;
    PPIN_NUMBER pinNumberTable;
    PIN_NUMBER pinNumber;
    UINT32 pinOffset;
    ULONG ioReg;
    UINT32 gpioVal;
    RTL_BITMAP bitMap;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;
    RtlInitializeBitMap(&bitMap, WriteParameters->Buffer, GPIO_PER_BANK);

    pinNumberTable = WriteParameters->PinNumberTable;
    for (ULONG index = 0; index < WriteParameters->PinCount; index++) {
        pinNumber = pinNumberTable[index] + WriteParameters->BankId * GPIO_PER_BANK;

        pinOffset = GetTileOffsetByPin(pinNumber) + IO_REG(pinNumber);

        ioReg = Read32((PULONG)(gpioContext->Base + pinOffset));

        gpioVal = RtlCheckBit(&bitMap, index);

        if (!!gpioVal)
            ioReg |= BIT(OUT_BIT);
        else
            ioReg &= ~BIT(OUT_BIT);

        Write32((PULONG)(gpioContext->Base + pinOffset), ioReg);

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Write GPIO[%d]=%d", pinNumber, gpioVal);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

VOID
GPIOMaskUnmaskInterrupt(
    _In_ PGPIO_DEVICE_CONTEXT Context,
    _In_ PIN_NUMBER pinNumber,
    _In_ BOOLEAN Mask
)
{
    UINT32 pinOffset;
    ULONG intrReg;

    pinOffset = GetTileOffsetByPin(pinNumber) + INTR_REG(pinNumber);

    intrReg = Read32((PULONG)(Context->Base + pinOffset));

    if (Mask) {
        intrReg &= ~BIT(INTR_ENABLE_BIT);
        intrReg &= ~BIT(INTR_RAW_STATUS_BIT);
    }
    else {
        intrReg |= BIT(INTR_ENABLE_BIT);
        intrReg |= BIT(INTR_RAW_STATUS_BIT);
    }

    Write32((PULONG)(Context->Base + pinOffset), intrReg);
}

_Must_inspect_result_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GPIOEnableInterrupt(
    _In_ PVOID Context,
    _In_ PGPIO_ENABLE_INTERRUPT_PARAMETERS EnableParameters
    )
{
    NTSTATUS status;
    PGPIO_DEVICE_CONTEXT gpioContext;
    PIN_NUMBER pinNumber;
    UINT32 pinOffset;
    ULONG intrReg;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    if (EnableParameters->Polarity != InterruptActiveHigh && EnableParameters->Polarity != InterruptActiveLow) {
        status = STATUS_NOT_SUPPORTED;
        goto Exit;
    }

    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;
    pinNumber = EnableParameters->PinNumber + EnableParameters->BankId * GPIO_PER_BANK;
    status = STATUS_SUCCESS;

    pinOffset = GetTileOffsetByPin(pinNumber) + INTR_REG(pinNumber);

    GPIO_CLX_AcquireInterruptLock(Context, EnableParameters->BankId);

    intrReg = Read32((PULONG)(gpioContext->Base + pinOffset));

    // Route interrupts to application cpu
    intrReg &= ~(INTR_TARGET_MASK << INTR_TARGET_BIT);
    intrReg |= INTR_TARGET_KPSS_VAL << INTR_TARGET_BIT;

    // Update configuration
    intrReg &= ~(3 << INTR_DETECTION_BIT);
    intrReg &= ~(1 << INTR_POLARITY_BIT);
    switch (EnableParameters->Polarity)
    {
    case InterruptActiveLow:
        break;
    case InterruptActiveHigh:
        intrReg |= BIT(INTR_POLARITY_BIT);
        break;
    }

    // Unmask interrupt
    intrReg |= BIT(INTR_ENABLE_BIT);
    intrReg |= BIT(INTR_RAW_STATUS_BIT);

    Write32((PULONG)(gpioContext->Base + pinOffset), intrReg);

    // Configure gpio
    status = GPIOConfigurePin(gpioContext, pinNumber, ConnectModeInput, EnableParameters->PullConfiguration);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "GPIO[%d] interrupt enabled", pinNumber);

    GPIO_CLX_ReleaseInterruptLock(Context, EnableParameters->BankId);

Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}

_Must_inspect_result_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
GPIODisableInterrupt(
    _In_ PVOID Context,
    _In_ PGPIO_DISABLE_INTERRUPT_PARAMETERS DisableParameters
)
{
    PGPIO_DEVICE_CONTEXT gpioContext;
    PIN_NUMBER pinNumber;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;
    pinNumber = DisableParameters->PinNumber + DisableParameters->BankId * GPIO_PER_BANK;

    // Clear interrupt status
    Write32((PULONG)(gpioContext->Base + GetTileOffsetByPin(pinNumber) + INTR_STATUS_REG(pinNumber)), 0);

    // Mask interrupt
    GPIOMaskUnmaskInterrupt(gpioContext, pinNumber, TRUE);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return STATUS_SUCCESS;
}

NTSTATUS
GPIOReconfigureInterrupt(
    _In_ PVOID Context,
    _In_ PGPIO_RECONFIGURE_INTERRUPTS_PARAMETERS ReconfigureParameters
)
{
    PGPIO_DEVICE_CONTEXT gpioContext;
    PIN_NUMBER pinNumber;
    UINT32 pinOffset;
    ULONG intrReg;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;
    pinNumber = ReconfigureParameters->PinNumber + ReconfigureParameters->BankId * GPIO_PER_BANK;

    pinOffset = GetTileOffsetByPin(pinNumber) + INTR_REG(pinNumber);

    intrReg = Read32((PULONG)(gpioContext->Base + pinOffset));

    intrReg |= BIT(INTR_RAW_STATUS_BIT);
    intrReg &= ~(3 << 2);
    intrReg &= ~(1 << 1);
    switch (ReconfigureParameters->Polarity)
    {
    case InterruptActiveLow:
        break;
    case InterruptActiveHigh:
        intrReg |= BIT(1);
        break;
    }

    Write32((PULONG)(gpioContext->Base + pinOffset), intrReg);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "GPIO[%d] reconfigured", pinNumber);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return STATUS_SUCCESS;
}

_Must_inspect_result_
_IRQL_requires_same_
NTSTATUS
GPIOMaskInterrupts(
    _In_ PVOID Context,
    _In_ PGPIO_MASK_INTERRUPT_PARAMETERS MaskParameters
)
{
    PGPIO_DEVICE_CONTEXT gpioContext;
    PIN_NUMBER pinNumber;
    RTL_BITMAP bitMap;
    ULONG numberOfBits;
    ULONG currentBit;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;
    RtlInitializeBitMap(&bitMap, (PULONG)&MaskParameters->PinMask, GPIO_PER_BANK);

    numberOfBits = RtlNumberOfSetBits(&bitMap);

    for (ULONG index = 0; index < numberOfBits; index++) {
        currentBit = RtlFindSetBits(&bitMap, 1, index);

        pinNumber = (USHORT)currentBit + MaskParameters->BankId * GPIO_PER_BANK;

        GPIOMaskUnmaskInterrupt(gpioContext, pinNumber, TRUE);
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "GPIO[%d] masked", pinNumber);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS
GPIOUnmaskInterrupt(
    _In_ PVOID Context,
    _In_ PGPIO_ENABLE_INTERRUPT_PARAMETERS UnmaskParameters
)
{
    PGPIO_DEVICE_CONTEXT gpioContext;
    PIN_NUMBER pinNumber;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;
    pinNumber = UnmaskParameters->PinNumber + UnmaskParameters->BankId * GPIO_PER_BANK;

    GPIOMaskUnmaskInterrupt(gpioContext, pinNumber, FALSE);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "GPIO[%d] unmasked", pinNumber);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return STATUS_SUCCESS;
}

_Must_inspect_result_
_IRQL_requires_same_
NTSTATUS
GPIOQueryActiveInterrupts(
    _In_ PVOID Context,
    _In_ PGPIO_QUERY_ACTIVE_INTERRUPTS_PARAMETERS QueryActiveParameters
)
{
    PGPIO_DEVICE_CONTEXT gpioContext;
    PIN_NUMBER pinNumber;
    UINT32 pinOffset;
    RTL_BITMAP EnabledBitMap;
    RTL_BITMAP ActiveBitMap;
    ULONG numberOfBits;
    USHORT pin;
    ULONG intrStatusReg;

    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;
    RtlInitializeBitMap(&EnabledBitMap, (PULONG)&QueryActiveParameters->EnabledMask, GPIO_PER_BANK);
    RtlInitializeBitMap(&ActiveBitMap, (PULONG)&QueryActiveParameters->ActiveMask, GPIO_PER_BANK);

    numberOfBits = RtlNumberOfSetBits(&EnabledBitMap);

    for (ULONG index = 0; index < numberOfBits; index++) {
        pin = (USHORT)RtlFindSetBits(&EnabledBitMap, 1, index);

        pinNumber = pin + QueryActiveParameters->BankId * GPIO_PER_BANK;

        pinOffset = GetTileOffsetByPin(pinNumber) + INTR_STATUS_REG(pinNumber);

        intrStatusReg = Read32((PULONG)(gpioContext->Base + pinOffset));

        if (intrStatusReg & BIT(INTR_STATUS_BIT)) {
            RtlSetBits(&ActiveBitMap, pin, 1);
        }
    }

    return STATUS_SUCCESS;
}

_Must_inspect_result_
_IRQL_requires_same_
NTSTATUS
GPIOClearActiveInterrupts(
    _In_ PVOID Context,
    _In_ PGPIO_CLEAR_ACTIVE_INTERRUPTS_PARAMETERS ClearParameters
)
{
    PGPIO_DEVICE_CONTEXT gpioContext;
    PIN_NUMBER pinNumber;
    UINT32 pinOffset;
    RTL_BITMAP cleaActiveBitMap;
    ULONG numberOfBits;
    ULONG currentBit;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;
    RtlInitializeBitMap(&cleaActiveBitMap, (PULONG)&ClearParameters->ClearActiveMask, 64);

    numberOfBits = RtlNumberOfSetBits(&cleaActiveBitMap);

    for (ULONG index = 0; index < numberOfBits; index++) {
        currentBit = RtlFindSetBits(&cleaActiveBitMap, 1, index);

        pinNumber = (USHORT)currentBit + ClearParameters->BankId * GPIO_PER_BANK;

        pinOffset = GetTileOffsetByPin(pinNumber) + INTR_STATUS_REG(pinNumber);

        Write32((PULONG)(gpioContext->Base + pinOffset), 0);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

NTSTATUS GPIOConfigurePinMux(
    PVOID Context,
    PIN_NUMBER pinNumber,
    UCHAR function
    )
{
    NTSTATUS status;
    PGPIO_DEVICE_CONTEXT gpioContext;
    UINT32 pinOffset, ctlReg;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    gpioContext = (PGPIO_DEVICE_CONTEXT)Context;

    if (function > 9) {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    pinOffset = GetTileOffsetByPin(pinNumber) + CTL_REG(pinNumber);
    ctlReg = Read32((PULONG)(gpioContext->Base + pinOffset));

    ctlReg &= ~MUX_MASK;
    ctlReg |= function << MUX_BIT;

    Write32((PULONG)(gpioContext->Base + pinOffset), ctlReg);

Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}

VOID
GPIOEvtDriverUnload(
    _In_ WDFDRIVER Driver
    )
{
    NTSTATUS status;

    PAGED_CODE();

    status = GPIO_CLX_UnregisterClient(Driver);
    NT_ASSERT(NT_SUCCESS(status));
}

VOID
GPIOEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
    )
{
    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    //
    // Stop WPP Tracing
    //
    WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER)DriverObject));
}
