#pragma once

typedef unsigned char    UINT8;
typedef unsigned short   UINT16;
typedef unsigned __int32 UINT32;
typedef unsigned __int64 UINT64;
typedef void             VOID;

#define SIGNATURE4(a, b, c, d)             (                                            \
                                             ((a) | (b << 8))                           \
                                             |                                          \
                                             ((((c) | (d << 8)) << 16))                 \
                                           )

#define SIGNATURE8(a, b, c, d, e, f, g, h) (                                            \
                                             SIGNATURE4 (a, b, c, d)                    \
                                             |                                          \
                                             ((UINT64) (SIGNATURE4 (e, f, g, h)) << 32) \
                                           )

#define ACPI_FACP_SIGNATURE SIGNATURE4('F', 'A', 'C', 'P')
#define ACPI_DBG2_SIGNATURE SIGNATURE4('D', 'B', 'G', '2')
#define ACPI_CSRT_SIGNATURE SIGNATURE4('C', 'S', 'R', 'T')
#define ACPI_APIC_SIGNATURE SIGNATURE4('A', 'P', 'I', 'C')
#define ACPI_GTDT_SIGNATURE SIGNATURE4('G', 'T', 'D', 'T')
#define ACPI_MCFG_SIGNATURE SIGNATURE4('M', 'C', 'F', 'G')
#define ACPI_PPTT_SIGNATURE SIGNATURE4('P', 'P', 'T', 'T')
#define ACPI_FACP_REVISION 0x06
#define ACPI_DBG2_REVISION 0x01
#define ACPI_APIC_REVISION 0x05
#define ACPI_CSRT_REVISION 0x00
#define ACPI_GTDT_REVISION 0x02
#define ACPI_MCFG_REVISION 0x01
#define ACPI_PPTT_REVISION 0x01

#define ACPI_RESERVED 0

#define ACPI_KONA_ID          'K','O','N','A',' ',' '
#define ACPI_KONA_TABLE_ID    SIGNATURE8('K','O','N','A','8','2','5','0')
#define ACPI_KONA_REVISION    0x0000000
#define ACPI_CREATOR_ID       SIGNATURE4('K','O','N','A')
#define ACPI_CREATOR_REVISION 0x00000000

#pragma pack(push, 1)

typedef struct
{
    UINT32 Signature;
    UINT32 Length;
    UINT8  Revision;
    UINT8  Checksum;
    UINT8  OEMID[6];
    UINT64 OEMTableID;
    UINT32 OEMRevision;
    UINT32 CreatorID;
    UINT32 CreatorRevision;
} ACPI_HEADER;

#define ACPI_SYSTEM_MEMORY                  0
#define ACPI_SYSTEM_IO                      1
#define ACPI_PCI_CONFIGURATION_SPACE        2
#define ACPI_EMBEDDED_CONTROLLER            3
#define ACPI_SMBUS                          4
#define ACPI_PLATFORM_COMMUNICATION_CHANNEL 0x0A
#define ACPI_FUNCTIONAL_FIXED_HARDWARE      0x7F

#define ACPI_ACCESS_UNDEFINED 0
#define ACPI_ACCESS_BYTE      1
#define ACPI_ACCESS_WORD      2
#define ACPI_ACCESS_DWORD     3
#define ACPI_ACCESS_QWORD     4

typedef struct
{
    UINT8  AddressSpaceID;
    UINT8  RegisterBitWidth;
    UINT8  RegisterBitOffset;
    UINT8  AccessSize;
    UINT64 Address;
} ACPI_GAS;

#define GTDT_TIMER_EDGE_TRIGGERED 0x00000001
#define GTDT_TIMER_ACTIVE_LOW     0x00000002
#define GTDT_TIMER_ALWAYS_ON      0x00000004

#define GTDT_PLATFORM_GT_EDGE_TRIGGERED 0x00000001
#define GTDT_PLATFORM_GT_ACTIVE_LOW     0x00000002

#define GTDT_PLATFORM_GT_COMMON_SECURE    0x00000001
#define GTDT_PLATFORM_GT_COMMON_ALWAYS_ON 0x00000002

#define GTDT_WATCHDOG_EDGE_TRIGGERED 0x00000001
#define GTDT_WATCHDOG_ACTIVE_LOW     0x00000002
#define GTDT_WATCHDOG_SECURE         0x00000004

typedef struct _GTDT_PLATFORM_TIMER_HEADER {
    UINT8  Type;
    UINT16 Length;
} GTDT_PLATFORM_TIMER_HEADER, *PGTDT_PLATFORM_TIMER_HEADER;

typedef struct _GTDT_PLATFORM_GT_INSTANCE {
    UINT8  FrameNumber;
    UINT8  Reserved1;
    UINT8  Reserved2;
    UINT8  Reserved3;
    UINT64 CntBasePhysicalAddress;
    UINT64 CntEL0BasePhysicalAddress;
    UINT32 PhysicalTimerGsiv;
    UINT32 PhysicalTimerFlags;
    UINT32 VirtualTimerGsiv;
    UINT32 VirtualTimerFlags;
    UINT32 CommonFlags;
} GTDT_PLATFORM_GT_INSTANCE, *PGTDT_PLATFORM_GT_INSTANCE;

typedef struct _GTDT_PLATFORM_GT_BLOCK {
    GTDT_PLATFORM_TIMER_HEADER Header;
    UINT8  Reserved;
    UINT64 CntCtlBasePhysicalAddress;
    UINT32 BlockTimerCount;
    UINT32 BlockTimerOffset;
    GTDT_PLATFORM_GT_INSTANCE TimerInstances[1];
} GTDT_PLATFORM_GT_BLOCK, *PGTDT_PLATFORM_GT_BLOCK;

typedef struct
{
    ACPI_HEADER Header;
    UINT64 CntControlBasePhysicalAddress;
    UINT32 Reserved;
    UINT32 SecurePhysicalTimerGsiv;
    UINT32 SecurePhysicalTimerFlags;
    UINT32 NonSecurePhysicalTimerGsiv;
    UINT32 NonSecurePhysicalTimerFlags;
    UINT32 VirtualTimerEventGsiv;
    UINT32 VirtualTimerEventFlags;
    UINT32 NonSecurePhysicalTimer2Gsiv;
    UINT32 NonSecurePhysicalTimer2Flags;
    UINT64 CntReadBasePhysicalAddress;
    UINT32 TimerBlockCount;
    UINT32 TimerBlockOffset;

    GTDT_PLATFORM_GT_BLOCK TimerBlocks[1];
} ACPI_GTDT;

#define ACPI_PM_PROFILE_UNSPECIFIED        0
#define ACPI_PM_PROFILE_DESKTOP            1
#define ACPI_PM_PROFILE_MOBILE             2
#define ACPI_PM_PROFILE_WORKSTATION        3
#define ACPI_PM_PROFILE_ENTERPRISE_SERVER  4
#define ACPI_PM_PROFILE_SOHO_SERVER        5
#define ACPI_PM_PROFILE_APPLIANCE_PC       6
#define ACPI_PM_PROFILE_PERFORMANCE_SERVER 7
#define ACPI_PM_PROFILE_SLATE              8

#define ACPI_HARDWARE_REDUCED   (1 << 20)
#define LOW_POWER_S0_CAPABLE    (1 << 21)

typedef struct
{
    ACPI_HEADER  Header;
    UINT32       FIRMWARE_CTRL;
    UINT32       DSDT;
    UINT8        Reserved0;
    UINT8        Preferred_PM_Profile;
    UINT16       SCI_INT;
    UINT32       SMI_CMD;
    UINT8        ACPI_ENABLE;
    UINT8        ACPI_DISABLE;
    UINT8        S4BIOS_REQ;
    UINT8        PSTATE_CNT;
    UINT32       PM1a_EVT_BLK;
    UINT32       PM1b_EVT_BLK;
    UINT32       PM1a_CNT_Blk;
    UINT32       PM1b_CNT_Blk;
    UINT32       PM2_CNT_BLk;
    UINT32       PM_TMR_BLK;
    UINT32       GPE0_BLK;
    UINT32       GPE1_BLK;
    UINT8        PM1_EVT_LEN;
    UINT8        PM1_CNT_LEN;
    UINT8        PM2_CNT_LEN;
    UINT8        PM_TMR_LEN;
    UINT8        GPE0_BLK_LEN;
    UINT8        GPE1_BLK_LEN;
    UINT8        GPE1_BASE;
    UINT8        CST_CNT;
    UINT16       P_LVL2_LAT;
    UINT16       P_LVL3_LAT;
    UINT16       FLUSH_SIZE;
    UINT16       FLUSH_STRIDE;
    UINT8        DUTY_OFFSET;
    UINT8        DUTY_WIDTH;
    UINT8        DAY_ALRM;
    UINT8        MON_ALRM;
    UINT8        CENTURY;
    UINT16       IAPC_BOOT_ARCH;
    UINT8        Reserved1;
    UINT32       Flags;
    ACPI_GAS     RESET_REG;
    UINT8        RESET_VALUE;
    UINT16       ARM_BOOT_ARCH;
    UINT8        FADT_Minor_Version;
    UINT64       X_FIRMWARE_CTRL;
    UINT64       X_DSDT;
    ACPI_GAS     X_PM1a_EVT_BLK;
    ACPI_GAS     X_PM1b_EVT_BLK;
    ACPI_GAS     X_PM1a_CNT_BLK;
    ACPI_GAS     X_PM1b_CNT_BLK;
    ACPI_GAS     X_PM2_CNT_BLK;
    ACPI_GAS     X_PM_TMR_BLK;
    ACPI_GAS     X_GPE0_BLK;
    ACPI_GAS     X_GPE1_BLK;
    ACPI_GAS     SLEEP_CONTROL_REG;
    ACPI_GAS     SLEEP_STATUS_REG;
    UINT64       Hypervisor_Vendor_Identity;
} ACPI_FACP;

#define ACPI_PROCESSOR_LOCAL_GIC 11
#define ACPI_GIC_DISTRIBUTOR     12
#define ACPI_GIC_MSI_FRAME       13
#define ACPI_GIC_REDISTRIBUTOR   14

#define PLGF_ENABLED_BIT 0
#define PLGF_ENABLED (1 << PLGF_ENABLED_BIT)

typedef struct {
    ACPI_HEADER                 Header;
    UINT32                      LocalApicAddress;
    UINT32                      Flags;
} APIC_DESCRIPTION_TABLE_HEADER;

typedef struct _ACPI_PROCESSOR_LOCAL_APIC_STRUCTURE   {
    UINT8 Type;
    UINT8 Length;
    UINT16 Reserved;
    UINT32 Identifier;
    UINT32 AcpiProcessorId;
    UINT32 Flags;
    UINT32 ParkingProtocolVersion;
    UINT32 PerformanceInterruptGsi;
    UINT64 MailboxPhysicalAddress;
    UINT64 ControllerPhysicalAddress;
    UINT64 GICVirtual;
    UINT64 GICH;
    UINT32 VGICMaintenanceInterrupt;
    UINT64 GICRedistributorBaseAddress;
    UINT64 MPIDR;
    UINT8  ProcessorPowerEfficiencyClass;
    UINT8  Reserved2[3];
} ACPI_PROCESSOR_LOCAL_APIC_STRUCTURE;

typedef struct _ACPI_GIC_DISTRIBUTOR_STRUCTURE  {
    UINT8 Type;
    UINT8 Length;
    UINT16 Reserved1;
    UINT32 Identifier;
    UINT64 ControllerPhysicalAddress;
    UINT32 GsivBase;
    UINT8  GicVersion;
    UINT8  Reserved[3];
} ACPI_GIC_DISTRIBUTOR_STRUCTURE;

typedef struct _ACPI_GIC_REDISTRIBUTOR_STRUCTURE  {
    UINT8 Type;
    UINT8 Length;
    UINT16 Reserved;
    UINT64 GICRedistributorPhysicalBaseAddress;
    UINT32 GICRDiscoveryRangeLength;
} ACPI_GIC_REDISTRIBUTOR_STRUCTURE;

typedef struct _ACPI_GIC_MSI_FRAME_STRUCTURE  {
    UINT8 Type;
    UINT8 Length;
    UINT16 Reserved1;
    UINT32 Identifier;
    UINT64 ControllerPhysicalAddress;
    UINT32 Flags;
    UINT16 SPI_Count;
    UINT16 SPI_Base;
} ACPI_GIC_MSI_FRAME_STRUCTURE;

typedef struct {
    APIC_DESCRIPTION_TABLE_HEADER       Header;
    ACPI_PROCESSOR_LOCAL_APIC_STRUCTURE LocalGic0;
    ACPI_PROCESSOR_LOCAL_APIC_STRUCTURE LocalGic1;
    ACPI_PROCESSOR_LOCAL_APIC_STRUCTURE LocalGic2;
    ACPI_PROCESSOR_LOCAL_APIC_STRUCTURE LocalGic3;
    ACPI_PROCESSOR_LOCAL_APIC_STRUCTURE LocalGic4;
    ACPI_PROCESSOR_LOCAL_APIC_STRUCTURE LocalGic5;
    ACPI_PROCESSOR_LOCAL_APIC_STRUCTURE LocalGic6;
    ACPI_PROCESSOR_LOCAL_APIC_STRUCTURE LocalGic7;
    ACPI_GIC_DISTRIBUTOR_STRUCTURE      Distributor;
    ACPI_GIC_REDISTRIBUTOR_STRUCTURE    ReDistributor;
    ACPI_GIC_MSI_FRAME_STRUCTURE        MSIFrame0;
} ACPI_APIC;

#define ACPI_PROCESSOR_NODE 0
#define ACPI_CACHE_TYPE     1
#define ACPI_ID_TYPE	    2

typedef struct _ACPI_CACHE_TYPE_STRUCTURE {
    UINT8  Type;
    UINT8  Length;
    UINT16 Reserved;
    UINT32 Flags;
    UINT32 NextLevelCache;
    UINT32 Size;
    UINT32 NumOfSets;
    UINT8  Associativity;
    UINT8  Attributes;
    UINT16 LineSize;
} ACPI_CACHE_TYPE_STRUCTURE;

typedef struct _ACPI_ID_TYPE_STRUCTURE {
    UINT8  Type;
    UINT8  Length;
    UINT16 Reserved;
    UINT32 vendorId;
    UINT64 level1Id;
    UINT64 level2Id;
    UINT16 majorRev;
    UINT16 minorRev;
    UINT16 spinRev;
} ACPI_ID_TYPE_STRUCTURE;

typedef struct _ACPI_PROCESSOR_NODE_STRUCTURE_L3 {
    UINT8  Type;
    UINT8  Length;
    UINT16 Reserved;
    UINT32 Flags;
    UINT32 Parent;
    UINT32 AcpiProcessorId;
    UINT32 numPrivateResources;
    UINT32 CommonCacheL3;
    UINT32 SocId;
} ACPI_PROCESSOR_NODE_STRUCTURE_L3;

typedef struct _ACPI_PROCESSOR_NODE_STRUCTURE {
    UINT8  Type;
    UINT8  Length;
    UINT16 Reserved;
    UINT32 Flags;
    UINT32 Parent;
    UINT32 AcpiProcessorId;
    UINT32 numPrivateResources;
    UINT32 localCacheL1D;
    UINT32 LocalCacheL1I;
} ACPI_PROCESSOR_NODE_STRUCTURE;

typedef struct {
    ACPI_HEADER                      Header;
    ACPI_CACHE_TYPE_STRUCTURE        localCacheL3U;
    ACPI_ID_TYPE_STRUCTURE           SocId;
    ACPI_PROCESSOR_NODE_STRUCTURE_L3 NodeForL3;
    ACPI_CACHE_TYPE_STRUCTURE        localCacheL2U;
    ACPI_CACHE_TYPE_STRUCTURE        localCacheL1D;
    ACPI_CACHE_TYPE_STRUCTURE        localCacheL1I;
    ACPI_PROCESSOR_NODE_STRUCTURE    LocalPpt0;
    ACPI_PROCESSOR_NODE_STRUCTURE    LocalPpt1;
    ACPI_PROCESSOR_NODE_STRUCTURE    LocalPpt2;
    ACPI_PROCESSOR_NODE_STRUCTURE    LocalPpt3;
    ACPI_PROCESSOR_NODE_STRUCTURE    LocalPpt4;
    ACPI_PROCESSOR_NODE_STRUCTURE    LocalPpt5;
    ACPI_PROCESSOR_NODE_STRUCTURE    LocalPpt6;
    ACPI_PROCESSOR_NODE_STRUCTURE    LocalPpt7;
} ACPI_PPTT;

typedef struct {
    UINT8  Revision;
    UINT16 Length;
    UINT8  BaseAddressRegisterCount;
    UINT16 NameSpaceStringLength;
    UINT16 NameSpaceStringOffset;
    UINT16 OemDataLength;
    UINT16 OemDataOffset;
    UINT16 PortType;
    UINT16 PortSubtype;
    UINT16 Reserved;
    UINT16 BaseAddressRegisterOffset;
    UINT16 AddressSizeOffset;
} ACPI_DBI2;

typedef struct {
    ACPI_HEADER Header;
    UINT32      OffsetDbgDeviceInfo;
    UINT32      NumberDbgDeviceInfo;
} ACPI_DBG2;

typedef struct {
    UINT16 PortType;
    UINT16 Reserved;
    UINT32 Signature;
    UINT32 WriteCount;

    struct {
        UINT8 BaseAddressRegister;
        UINT8 Phase;
        UINT16 Reserved;
        UINT32 Offset;
        UINT32 AndValue;
        UINT32 OrValue;
    } Data[8];

    struct {
        UINT8 UsbCore;
        UINT8 Reserved1;
        UINT16 Reserved2;
        UINT32 Signature; // = 'USBC'
    } ACPI_USB_INIT_CORE_OEM_DATA;
} ACPI_DB_OEM_DATA;

typedef struct {
    ACPI_DBI2 DebugDeviceInformation;

    ACPI_GAS         BaseAddressRegister[2];
    UINT32           AddressSize[2];
    UINT8            NameSpaceString[32];
    ACPI_DB_OEM_DATA OemData;
} ACPI_DBI2_IMPL;

typedef struct {
    ACPI_DBG2      DebugPortTable;
    ACPI_DBI2_IMPL DebugDevice;
} ACPI_DBG2_IMPL;

#pragma pack(pop)