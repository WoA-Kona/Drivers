#include "driver.h"
#include "device.tmh"
#include <Acpiioct.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, PMICCreateDevice)
#endif

NTSTATUS
ParsePMICConfig(
    DEVICE_CONTEXT* Context
    )
{
    NTSTATUS status;
    ACPI_EVAL_INPUT_BUFFER inputBuffer;
    PACPI_EVAL_OUTPUT_BUFFER outputBuffer;
    WDF_MEMORY_DESCRIPTOR inputDescriptor;
    WDF_MEMORY_DESCRIPTOR outputDescriptor;
    PACPI_METHOD_ARGUMENT packageArgument;
    PACPI_METHOD_ARGUMENT dataArgument;
    WDFIOTARGET ioTarget;
    ULONG sizeReturned = 0;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    RtlZeroMemory(&inputBuffer, sizeof(ACPI_EVAL_INPUT_BUFFER));
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;

    inputBuffer.MethodNameAsUlong = (ULONG)'GFCP';
    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&inputDescriptor, (PVOID)&inputBuffer, sizeof(ACPI_EVAL_INPUT_BUFFER));

    outputBuffer = (PACPI_EVAL_OUTPUT_BUFFER)ExAllocatePool2(POOL_FLAG_PAGED, 4096, POOL_TAG);

    if (outputBuffer == NULL) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Unable to allocate memory");
        status = STATUS_MEMORY_NOT_ALLOCATED;
        goto Exit;
    }

    RtlZeroMemory(outputBuffer, 4096);
    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&outputDescriptor, (PVOID)outputBuffer, 4096);

    ioTarget = WdfDeviceGetIoTarget(Context->Device);

    status = WdfIoTargetSendIoctlSynchronously(ioTarget,
        NULL,
        IOCTL_ACPI_EVAL_METHOD,
        &inputDescriptor,
        &outputDescriptor,
        NULL,
        (PULONG_PTR)&sizeReturned);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Failed to read PMIC Config from ACPI");
        status = STATUS_ACPI_INVALID_DATA;
        goto Exit;
    }

    if (outputBuffer->Signature != ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE || outputBuffer->Count == 0)
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Invalid buffer signature or count");
        status = STATUS_ACPI_INVALID_DATA;
        goto Exit;
    }

    if (outputBuffer->Argument[0].Type != ACPI_METHOD_ARGUMENT_INTEGER) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Invalid argument type");
        status = STATUS_ACPI_INVALID_DATA;
        goto Exit;
    }

    Context->NumberOfPMICs = outputBuffer->Argument[0].Argument;
    packageArgument = (ACPI_METHOD_ARGUMENT*)ACPI_METHOD_NEXT_ARGUMENT(&(outputBuffer->Argument[0]));

    for (UINT32 i = 0; i < Context->NumberOfPMICs; i++) {
        if (packageArgument->Type != ACPI_METHOD_ARGUMENT_PACKAGE || packageArgument->DataLength == 0)
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Invalid argument type");
            status = STATUS_ACPI_INVALID_DATA;
            goto Exit;
        }

        Context->PMICList[i].Number = i;

        dataArgument = (PACPI_METHOD_ARGUMENT)packageArgument->Data;
        for (UINT8 j = 0; j < 2; j++) {
            if (dataArgument->Type != ACPI_METHOD_ARGUMENT_INTEGER)
            {
                TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Invalid argument type");
                status = STATUS_ACPI_INVALID_DATA;
                goto Exit;
            }

            Context->PMICList[i].SlaveID[j] = (UINT8)dataArgument->Argument;

            dataArgument = (ACPI_METHOD_ARGUMENT*)ACPI_METHOD_NEXT_ARGUMENT(dataArgument);
        }

        packageArgument = (ACPI_METHOD_ARGUMENT*)ACPI_METHOD_NEXT_ARGUMENT(packageArgument);
    }

    status = STATUS_SUCCESS;
Exit:
    if (outputBuffer != NULL) {
        ExFreePool2(outputBuffer, POOL_TAG, NULL, 0);
    }
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}

NTSTATUS
PMICPrepareHardware(
        WDFDEVICE    Device,
        WDFCMRESLIST ResourcesRaw,
        WDFCMRESLIST ResourcesTranslated
    )
{
    NTSTATUS status;
    DEVICE_CONTEXT* deviceContext;
    UNREFERENCED_PARAMETER(ResourcesRaw);
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    deviceContext = DeviceGetContext(Device);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Number of PMICs: %d", deviceContext->NumberOfPMICs);
    for (UINT32 i = 0; i < deviceContext->NumberOfPMICs; i++) {
        PMIC_INFO pmic = deviceContext->PMICList[i];

        if (pmic.SlaveID[0] == 16 && pmic.SlaveID[1] == 16)
            continue;

        status = deviceContext->SPMIInterface.Read(deviceContext->SPMIInterface.Interface.Context, pmic.SlaveID[0], PMIC_REG_SUBTYPE, (PUINT8)&pmic.Model, sizeof(pmic.Model));
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Failed to read pmic subtype %!STATUS!", status);
            goto Exit;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "PMIC%d: %!pmic_model!", pmic.Number, pmic.Model);

        // Initialize RTC
        switch (pmic.Model)
        {
        case PMIC_PM6150:
        case PMIC_PM8150:
            break;
        }

        // Initialize GPIO
        switch (pmic.Model)
        {
        // 10 GPIO
        case PMIC_PM6150:
        case PMIC_PM8150:
            break;
        // 12 GPIO
        case PMIC_PM8150B:
        case PMIC_PM8150L:
        case PMIC_PM8150A:
            break;
        // 4 GPIO
        case PMIC_PM8009:
            break;
        // 11 GPIO
        case PMIC_PMX55:
            break;
        }

        // Initialize LPG
        switch (pmic.Model)
        {
        case PMIC_PM8150L:
        case PMIC_PM8150A:
            break;
        case PMIC_PM8150B:
            break;
        }

        // Initialize WLED
        if (pmic.Model == PMIC_PM8150L)
        {

        }

        // Initialize IBBLAB
        if (pmic.Model == PMIC_PM8150A)
        {

        }
    }

    status = STATUS_SUCCESS;
Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}

NTSTATUS
PMICReleaseHardware(
    WDFDEVICE    Device,
    WDFCMRESLIST ResourcesTranslated
    )
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    return STATUS_SUCCESS;
}

NTSTATUS
PMICCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
    WDF_PNPPOWER_EVENT_CALLBACKS pnpCallbacks;
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    PDEVICE_CONTEXT deviceContext;
    WDFDEVICE device;
    NTSTATUS status;
    WDFIOTARGET ioTarget;
    WDF_IO_TARGET_OPEN_PARAMS openParams;
    WDF_OBJECT_ATTRIBUTES objectAttributes;

    PAGED_CODE();

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpCallbacks);

    pnpCallbacks.EvtDevicePrepareHardware = PMICPrepareHardware;
    pnpCallbacks.EvtDeviceReleaseHardware = PMICReleaseHardware;

    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpCallbacks);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (NT_SUCCESS(status)) {
        deviceContext = DeviceGetContext(device);
        RtlZeroMemory(&deviceContext->SPMIInterface, sizeof(SPMI_INTERFACE));
        deviceContext->Device = device;

        WDF_OBJECT_ATTRIBUTES_INIT(&objectAttributes);
        objectAttributes.ParentObject = device;

        status = WdfIoTargetCreate(device, &objectAttributes, &ioTarget);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfIoTargetCreate failed %!STATUS!", status);
            goto Exit;
        }

        DECLARE_CONST_UNICODE_STRING(spmiDosDeviceName, SPMI_SYMBOLIC_NAME_STRING);

        WDF_IO_TARGET_OPEN_PARAMS_INIT_OPEN_BY_NAME(
            &openParams,
            &spmiDosDeviceName,
            (GENERIC_READ | GENERIC_WRITE));

        openParams.ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;
        openParams.CreateDisposition = FILE_OPEN;
        openParams.FileAttributes = FILE_ATTRIBUTE_NORMAL;

        status = WdfIoTargetOpen(ioTarget, &openParams);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfIoTargetOpen failed %!STATUS!", status);
            goto Exit;
        }

        status = WdfIoTargetQueryForInterface(ioTarget,
            &GUID_SPMI_INTERFACE,
            (PINTERFACE)&deviceContext->SPMIInterface,
            sizeof(SPMI_INTERFACE),
            1,
            NULL);

        WdfIoTargetClose(ioTarget);

        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfIoTargetQueryForInterface failed %!STATUS!", status);
            goto Exit;
        }

        ParsePMICConfig(deviceContext);
    }

Exit:
    return status;
}
