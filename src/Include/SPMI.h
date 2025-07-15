#pragma once

typedef
NTSTATUS
(*PREAD) (
    PVOID Context,
    UINT8 Sid,
    UINT16 Addr,
    UINT8* Data,
    UINT8 DataLen
    );

typedef
NTSTATUS
(*PWRITE) (
    PVOID Context,
    UINT8 Sid,
    UINT16 Addr,
    UINT8* Data,
    UINT8 DataLen
    );

#define SPMI_SYMBOLIC_NAME_STRING L"\\DosDevices\\SPMI"

// {5BD94292-BA06-4AA0-96DA-80148B254A4E}
DEFINE_GUID(GUID_SPMI_INTERFACE,
    0x5bd94292, 0xba06, 0x4aa0, 0x96, 0xda, 0x80, 0x14, 0x8b, 0x25, 0x4a, 0x4e);

typedef struct _SPMI_INTERFACE
{
    INTERFACE Interface;
    PREAD Read;
    PWRITE Write;
} SPMI_INTERFACE, *PSPMI_INTERFACE;