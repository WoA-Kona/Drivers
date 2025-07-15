#include "SPMI.h"

EXTERN_C_START

#define BIT(n) (1U << (n))

#define SPMI_VERSION 0x0
#define SPMI_VERSION_V5 0x50000000
#define SPMI_VERSION_V7 0x70000000

#define SPMI_CH_OFFSET(x) ((x) * 0x4)
#define SPMI_OBS_CH_OFFSET(x) ((x) * 0x80)
#define SPMI_CHN_CH_OFFSET(x) ((x) * 0x10000)

#define SPMI_OWNERSHIP(x) ((x) & 0x7)

#define SPMI_REG_CMD0 0x0
#define SPMI_REG_STATUS 0x8
#define SPMI_REG_WDATA0 0x10
#define SPMI_REG_WDATA1 0x14
#define SPMI_REG_RDATA0 0x18
#define SPMI_REG_RDATA1 0x1C

#define APID_MAP_OFFSET 0x900

#define SPMI_MAX_SLAVES 16
#define SPMI_MAX_PERIPH 512
#define SPMI_MAX_TRANS_BYTES 8
#define SPMI_MAX_PPID BIT(12)

#define SPMI_APID_VALID BIT(15)

#define SPMI_CMD_EXT_REG_WRITE_LONG 0x00
#define SPMI_CMD_EXT_REG_READ_LONG 0x01

#define SPMI_FMT_CMD(opc, off, len) ((opc) << 27) | ((off & 0xFF) << 4) | ((len) & 0x7)

typedef struct _APID_DATA
{
    UINT16 ppid;
    UINT8 write_ee;
    UINT8 irq_ee;
} APID_DATA, *PAPID_DATA;

typedef struct _DEVICE_CONTEXT
{
    ULONG_PTR Core;
    ULONG CoreLength;
    ULONG_PTR Chnls;
    ULONG ChnlsLength;
    ULONG_PTR Obsrvr;
    ULONG ObsrvrLength;
    ULONG_PTR Intr;
    ULONG IntrLength;
    ULONG_PTR Cnfg;
    ULONG CnfgLength;
    UINT16 PpidToApid[SPMI_MAX_PPID];
    APID_DATA ApidData[SPMI_MAX_PERIPH];
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

EVT_WDF_DEVICE_PREPARE_HARDWARE SPMIPrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE SPMIReleaseHardware;

NTSTATUS
SPMICreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    );

EXTERN_C_END
