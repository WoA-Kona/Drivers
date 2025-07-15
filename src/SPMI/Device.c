#include "driver.h"
#include "device.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, SPMICreateDevice)
#endif

#define Read32 READ_REGISTER_NOFENCE_ULONG
#define Write32 WRITE_REGISTER_NOFENCE_ULONG

NTSTATUS
SPMIInterfaceWrite(
    PVOID Context,
    UINT8 Sid,
    UINT16 Addr,
    UINT8* Data,
    UINT8 DataLen
)
{
    NTSTATUS status;
    PDEVICE_CONTEXT deviceContext;
    UINT32 apid;
    ULONG reg;
    UINT16 ppid;
    UINT8 len;
    UINT32 temp = 0;
    UINT32 offset;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    deviceContext = (PDEVICE_CONTEXT)Context;
    reg = 0;

    ppid = (Sid << 8) | (Addr >> 8);

    if (Sid >= SPMI_MAX_SLAVES || ppid >= SPMI_MAX_PPID) {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    apid = deviceContext->PpidToApid[ppid];

    if (!(apid & SPMI_APID_VALID))
    {
        status = STATUS_IO_DEVICE_ERROR;
        goto Exit;
    }

    apid &= ~SPMI_APID_VALID;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "[%d:%d] apid: %d", Sid, ppid, apid);

    if (DataLen > SPMI_MAX_TRANS_BYTES) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "SPMI supports transmission up to %d bytes but %d requested", SPMI_MAX_TRANS_BYTES, DataLen);
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    offset = SPMI_CHN_CH_OFFSET(apid);

    // Write data
    len = DataLen > 4 ? 4 : DataLen;
    memcpy(&temp, Data, len);
    Write32((PULONG)(deviceContext->Chnls + offset + SPMI_REG_WDATA0), temp);

    if (DataLen >= 5) {
        memcpy(&temp, Data + 4, len - 4);
        Write32((PULONG)(deviceContext->Chnls + offset + SPMI_REG_WDATA1), temp);
    }

    // Send write request
    reg = SPMI_FMT_CMD(SPMI_CMD_EXT_REG_WRITE_LONG, Addr, DataLen);
    Write32((PULONG)(deviceContext->Chnls + offset + SPMI_REG_CMD0), reg);

    // Wait CMD Done status
    reg = 0;
    while (!reg) {
        reg = Read32((PULONG)(deviceContext->Chnls + offset + SPMI_REG_STATUS));
    }

    if (reg ^ 0x1)
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "SPMI write failed");
        status = STATUS_IO_DEVICE_ERROR;
        goto Exit;
    }

    status = STATUS_SUCCESS;
Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}

NTSTATUS
SPMIInterfaceRead(
    PVOID Context,
    UINT8 Sid,
    UINT16 Addr,
    UINT8* Data,
    UINT8 DataLen
    )
{
    NTSTATUS status;
    PDEVICE_CONTEXT deviceContext;
    UINT32 apid, offset;
    ULONG reg;
    UINT16 ppid, len;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    deviceContext = (PDEVICE_CONTEXT)Context;
    reg = 0;

    ppid = (Sid << 8) | (Addr >> 8);

    if (Sid >= SPMI_MAX_SLAVES || ppid >= SPMI_MAX_PPID) {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    apid = deviceContext->PpidToApid[ppid];

    if (!(apid & SPMI_APID_VALID))
    {
        status = STATUS_IO_DEVICE_ERROR;
        goto Exit;
    }

    apid &= ~SPMI_APID_VALID;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "[%d:%d] apid: %d", Sid, ppid, apid);

    if (DataLen > SPMI_MAX_TRANS_BYTES) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "SPMI supports transmission up to %d bytes but %d requested", SPMI_MAX_TRANS_BYTES, DataLen);
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    offset = SPMI_OBS_CH_OFFSET(apid);

    // Request read
    reg = SPMI_FMT_CMD(SPMI_CMD_EXT_REG_READ_LONG, Addr, DataLen);
    Write32((PULONG)(deviceContext->Obsrvr + offset + SPMI_REG_CMD0), reg);

    // Wait CMD Done status
    reg = 0;
    while (!reg) {
        reg = Read32((PULONG)(deviceContext->Obsrvr + offset + SPMI_REG_STATUS));
    }

    if (reg ^ 0x1)
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "SPMI Read failed");
        status = STATUS_IO_DEVICE_ERROR;
        goto Exit;
    }

    len = DataLen > 4 ? 4 : DataLen;
    reg = Read32((PULONG)(deviceContext->Obsrvr + offset + SPMI_REG_RDATA0));
    memcpy(Data, &reg, len);

    if (DataLen >= 5) {
        reg = Read32((PULONG)(deviceContext->Obsrvr + offset + SPMI_REG_RDATA1));
        memcpy(Data + 4, &reg, DataLen - 4);
    }

    status = STATUS_SUCCESS;
Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}

VOID
ReadApidMap(PDEVICE_CONTEXT deviceContext)
{
    UINT32 regval, offset;
    UINT16 apid, ppid;
    BOOLEAN valid, isIrqEE;
    PAPID_DATA apidData = deviceContext->ApidData;
    PAPID_DATA prevApidData;

    for (UINT16 i = 0;; i++, apidData++) {
        offset = APID_MAP_OFFSET + SPMI_CH_OFFSET(i);

        if (offset >= deviceContext->CoreLength)
            break;

        regval = Read32((PULONG)(deviceContext->Core + offset));
        if (!regval)
            continue;

        ppid = (regval >> 8) & 0xFFF;
        isIrqEE = (regval & BIT(24)) > 0;

        regval = Read32((PULONG)(deviceContext->Cnfg + 0x700 + SPMI_CH_OFFSET(i)));
        apidData->write_ee = SPMI_OWNERSHIP(regval);
        apidData->irq_ee = isIrqEE ? apidData->write_ee : 0xFF;

        valid = (deviceContext->PpidToApid[ppid] & SPMI_APID_VALID) > 0;
        apid = (UINT16)(deviceContext->PpidToApid[ppid] & ~SPMI_APID_VALID);
        prevApidData = &deviceContext->ApidData[apid];

        if (!valid || apidData->write_ee == 0)
        {
            deviceContext->PpidToApid[ppid] = i | SPMI_APID_VALID;
        }
        else if (valid && isIrqEE && prevApidData->write_ee == 0) {
            prevApidData->irq_ee = apidData->irq_ee;
        }

        apidData->ppid = ppid;
    }

    // Dump mapping table
    /*TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "PPID APID Write-EE IRQ-EE");
    for (ppid = 0; ppid < SPMI_MAX_PPID; ppid++) {
        apid = deviceContext->PpidToApid[ppid];
        if (apid & SPMI_APID_VALID) {
            apid &= ~SPMI_APID_VALID;
            apidData = &deviceContext->ApidData[apid];
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%#03x %3u %2u %2u", ppid, apid, apidData->write_ee, apidData->irq_ee);
        }
    }*/
}

NTSTATUS
SPMIPrepareHardware(
    WDFDEVICE    Device,
    WDFCMRESLIST ResourcesRaw,
    WDFCMRESLIST ResourcesTranslated
    )
{
    NTSTATUS status;
    PDEVICE_CONTEXT deviceContext;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;
    ULONG resourceCount;
    ULONG memoryResourceCount;
    ULONG version;

    UNREFERENCED_PARAMETER(ResourcesRaw);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    status = STATUS_INSUFFICIENT_RESOURCES;
    deviceContext = DeviceGetContext(Device);
    memoryResourceCount = 0;

    resourceCount = WdfCmResourceListGetCount(ResourcesTranslated);
    for (ULONG Index = 0; Index < resourceCount; Index++) {
        descriptor = WdfCmResourceListGetDescriptor(ResourcesTranslated, Index);
        switch (descriptor->Type) {
        case CmResourceTypeMemory:
            switch (memoryResourceCount) {
            case 0:
                deviceContext->Core = (ULONG_PTR)MmMapIoSpaceEx(descriptor->u.Memory.Start, descriptor->u.Memory.Length, PAGE_NOCACHE | PAGE_READWRITE);
                deviceContext->CoreLength = descriptor->u.Memory.Length;

                if (deviceContext->Core == 0) {
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "MmMapIoSpaceEx failed!");
                    status = STATUS_UNSUCCESSFUL;
                    goto Exit;
                }
                break;
            case 1:
                deviceContext->Chnls = (ULONG_PTR)MmMapIoSpaceEx(descriptor->u.Memory.Start, descriptor->u.Memory.Length, PAGE_NOCACHE | PAGE_READWRITE);
                deviceContext->ChnlsLength = descriptor->u.Memory.Length;

                if (deviceContext->Chnls == 0) {
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "MmMapIoSpaceEx failed!");
                    status = STATUS_UNSUCCESSFUL;
                    goto Exit;
                }
                break;
            case 2:
                deviceContext->Obsrvr = (ULONG_PTR)MmMapIoSpaceEx(descriptor->u.Memory.Start, descriptor->u.Memory.Length, PAGE_NOCACHE | PAGE_READWRITE);
                deviceContext->ObsrvrLength = descriptor->u.Memory.Length;

                if (deviceContext->Obsrvr == 0) {
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "MmMapIoSpaceEx failed!");
                    status = STATUS_UNSUCCESSFUL;
                    goto Exit;
                }
                break;
            case 3:
                deviceContext->Intr = (ULONG_PTR)MmMapIoSpaceEx(descriptor->u.Memory.Start, descriptor->u.Memory.Length, PAGE_NOCACHE | PAGE_READWRITE);
                deviceContext->IntrLength = descriptor->u.Memory.Length;

                if (deviceContext->Intr == 0) {
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "MmMapIoSpaceEx failed!");
                    status = STATUS_UNSUCCESSFUL;
                    goto Exit;
                }
                break;
            case 4:
                deviceContext->Cnfg = (ULONG_PTR)MmMapIoSpaceEx(descriptor->u.Memory.Start, descriptor->u.Memory.Length, PAGE_NOCACHE | PAGE_READWRITE);
                deviceContext->CnfgLength = descriptor->u.Memory.Length;
                status = STATUS_SUCCESS;

                if (deviceContext->Cnfg == 0) {
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "MmMapIoSpaceEx failed!");
                    status = STATUS_UNSUCCESSFUL;
                    goto Exit;
                }
                break;
            default:
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            memoryResourceCount++;
            break;
        default:
            break;
        }
    }

    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    version = Read32((PULONG)(deviceContext->Core + SPMI_VERSION));

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "SPMI Version 0x%X", version);

    if (version >= SPMI_VERSION_V7 || version < SPMI_VERSION_V5) {
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "SPMI Version 0x%X is not supported", version);
        status = STATUS_NOT_SUPPORTED;
        goto Exit;
    }

    ReadApidMap(deviceContext);

    {
        SPMI_INTERFACE spmiInterface;
        WDF_QUERY_INTERFACE_CONFIG queryInterfaceConfig;
        RtlZeroMemory(&spmiInterface, sizeof(SPMI_INTERFACE));

        spmiInterface.Interface.Size = sizeof(SPMI_INTERFACE);
        spmiInterface.Interface.Version = 1;
        spmiInterface.Interface.Context = (PVOID)deviceContext;

        spmiInterface.Interface.InterfaceReference = WdfDeviceInterfaceReferenceNoOp;
        spmiInterface.Interface.InterfaceDereference = WdfDeviceInterfaceDereferenceNoOp;

        spmiInterface.Read = SPMIInterfaceRead;
        spmiInterface.Write = SPMIInterfaceWrite;

        WDF_QUERY_INTERFACE_CONFIG_INIT(
            &queryInterfaceConfig,
            (PINTERFACE)&spmiInterface,
            &GUID_SPMI_INTERFACE,
            NULL
        );

        status = WdfDeviceAddQueryInterface(
            Device,
            &queryInterfaceConfig
        );

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Failed to add device interface");
            goto Exit;
        }
    }

Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}

NTSTATUS
SPMIReleaseHardware(
    WDFDEVICE    Device,
    WDFCMRESLIST ResourcesTranslated
    )
{
    PDEVICE_CONTEXT deviceContext;

    UNREFERENCED_PARAMETER(ResourcesTranslated);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    deviceContext = DeviceGetContext(Device);

    if (deviceContext->Core != 0) {
        MmUnmapIoSpace((PULONG)deviceContext->Core, deviceContext->CoreLength);
    }

    if (deviceContext->Chnls != 0) {
        MmUnmapIoSpace((PULONG)deviceContext->Chnls, deviceContext->ChnlsLength);
    }

    if (deviceContext->Obsrvr != 0) {
        MmUnmapIoSpace((PULONG)deviceContext->Obsrvr, deviceContext->ObsrvrLength);
    }

    if (deviceContext->Intr != 0) {
        MmUnmapIoSpace((PULONG)deviceContext->Intr, deviceContext->IntrLength);
    }

    if (deviceContext->Cnfg != 0) {
        MmUnmapIoSpace((PULONG)deviceContext->Cnfg, deviceContext->CnfgLength);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return STATUS_SUCCESS;
}


NTSTATUS
SPMICreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
    NTSTATUS status;
    WDFDEVICE device;
    WDF_PNPPOWER_EVENT_CALLBACKS pnpCallbacks;
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    PDEVICE_CONTEXT deviceContext;

    PAGED_CODE();

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpCallbacks);

    pnpCallbacks.EvtDevicePrepareHardware = SPMIPrepareHardware;
    pnpCallbacks.EvtDeviceReleaseHardware = SPMIReleaseHardware;

    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpCallbacks);

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Failed to create device");
        goto Exit;
    }

    deviceContext = DeviceGetContext(device);

    DECLARE_CONST_UNICODE_STRING(dosDeviceName, SPMI_SYMBOLIC_NAME_STRING);

    status = WdfDeviceCreateSymbolicLink(device, &dosDeviceName);

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE, "Failed to create symbolic link");
        goto Exit;
    }

Exit:
    return status;
}
