EXTERN_C_START

NTSTATUS
PMICQueueInitialize(
    _In_ WDFDEVICE Device
    );

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL PMICEvtIoDeviceControl;

EXTERN_C_END
