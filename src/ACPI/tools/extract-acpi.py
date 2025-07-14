import argparse
from ctypes import *

IMAGE_DOS_SIGNATURE = 0x5A4D  # "MZ"
IMAGE_FILE_MACHINE_AARCH64 = 0xAA64

class IMAGE_DOS_HEADER(LittleEndianStructure):
    _fields_ = [
        ('e_magic', c_uint16),
        ('e_cblp', c_uint16),
        ('e_cp', c_uint16),
        ('e_crlc', c_uint16),
        ('e_cparhdr', c_uint16),
        ('e_minalloc', c_uint16),
        ('e_maxalloc', c_uint16),
        ('e_ss', c_uint16),
        ('e_sp', c_uint16),
        ('e_csum', c_uint16),
        ('e_ip', c_uint16),
        ('e_cs', c_uint16),
        ('e_lfarlc', c_uint16),
        ('e_ovno', c_uint16),
        ('e_res', c_uint16 * 4),
        ('e_oemid', c_uint16),
        ('e_oeminfo', c_uint16),
        ('e_res2', c_uint16 * 10),
        ('e_lfanew', c_int32),
    ]


class IMAGE_FILE_HEADER(LittleEndianStructure):
    _fields_ = [
        ('Machine', c_uint16),
        ('NumberOfSections', c_uint16),
        ('TimeDateStamp', c_uint32),
        ('PointerToSymbolTable', c_uint32),
        ('NumberOfSymbols', c_uint32),
        ('SizeOfOptionalHeader', c_uint16),
        ('Characteristics', c_uint16),
    ]


class IMAGE_DATA_DIRECTORY(LittleEndianStructure):
    _fields_ = [
        ('VirtualAddress', c_uint32),
        ('Size', c_uint32),
    ]


class IMAGE_OPTIONAL_HEADER32(LittleEndianStructure):
    _fields_ = [
        ('Magic', c_uint16),
        ('MajorLinkerVersion', c_uint8),
        ('MinorLinkerVersion', c_uint8),
        ('SizeOfCode', c_uint32),
        ('SizeOfInitializedData', c_uint32),
        ('SizeOfUninitializedData', c_uint32),
        ('AddressOfEntryPoint', c_uint32),
        ('BaseOfCode', c_uint32),
        ('BaseOfData', c_uint32),
        ('ImageBase', c_uint32),
        ('SectionAlignment', c_uint32),
        ('FileAlignment', c_uint32),
        ('MajorOperatingSystemVersion', c_uint16),
        ('MinorOperatingSystemVersion', c_uint16),
        ('MajorImageVersion', c_uint16),
        ('MinorImageVersion', c_uint16),
        ('MajorSubsystemVersion', c_uint16),
        ('MinorSubsystemVersion', c_uint16),
        ('Win32VersionValue', c_uint32),
        ('SizeOfImage', c_uint32),
        ('SizeOfHeaders', c_uint32),
        ('CheckSum', c_uint32),
        ('Subsystem', c_uint16),
        ('DllCharacteristics', c_uint16),
        ('SizeOfStackReserve', c_uint32),
        ('SizeOfStackCommit', c_uint32),
        ('SizeOfHeapReserve', c_uint32),
        ('SizeOfHeapCommit', c_uint32),
        ('LoaderFlags', c_uint32),
        ('NumberOfRvaAndSizes', c_uint32),
        ('DataDirectory', IMAGE_DATA_DIRECTORY * 16),
    ]


class IMAGE_NT_HEADERS32(LittleEndianStructure):
    _fields_ = [
        ('Signature', c_uint32),
        ('FileHeader', IMAGE_FILE_HEADER),
        ('OptionalHeader', IMAGE_OPTIONAL_HEADER32),
    ]


class IMAGE_SECTION_HEADER(LittleEndianStructure):
    _fields_ = [
        ('Name', c_char * 8),
        ('VirtualSize', c_uint32),
        ('VirtualAddress', c_uint32),
        ('SizeOfRawData', c_uint32),
        ('PointerToRawData', c_uint32),
        ('PointerToRelocations', c_uint32),
        ('PointerToLinenumbers', c_uint32),
        ('NumberOfRelocations', c_uint16),
        ('NumberOfLinenumbers', c_uint16),
        ('Characteristics', c_uint32),
    ]


class ACPI_HEADER(LittleEndianStructure):
    _fields_ = [
        ('Signature', c_uint32),
        ('Length', c_uint32),
        ('Revision', c_uint8),
        ('Checksum', c_uint8),
        ('OEMID', c_uint8 * 6),
        ('OEMTableID', c_uint64),
        ('OEMRevision', c_uint32),
        ('CreatorID', c_uint32),
        ('CreatorRevision', c_uint32),
    ]


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('input')
    parser.add_argument('-o', '--output', required=True)
    return parser.parse_args()

def read_struct(buffer, offset, cls):
    size = sizeof(cls)
    return cast((c_char * size).from_buffer_copy(buffer[offset:offset+size]), POINTER(cls)).contents

def main(input, output):
    with open(input, 'rb') as f:
        data = f.read()

    dos_header = read_struct(data, 0, IMAGE_DOS_HEADER)
    if dos_header.e_magic != IMAGE_DOS_SIGNATURE:
        raise Exception(f"Invalid DOS signature: {hex(dos_header.e_magic)}")

    nt_offset = dos_header.e_lfanew
    nt_headers = read_struct(data, nt_offset, IMAGE_NT_HEADERS32)

    machine = nt_headers.FileHeader.Machine
    if machine != IMAGE_FILE_MACHINE_AARCH64:
        raise Exception(f"Unexpected machine type: {hex(machine)}")

    section_offset = nt_offset + 4 + sizeof(IMAGE_FILE_HEADER) + nt_headers.FileHeader.SizeOfOptionalHeader
    found = False

    for i in range(nt_headers.FileHeader.NumberOfSections):
        section = read_struct(data, section_offset + i * sizeof(IMAGE_SECTION_HEADER), IMAGE_SECTION_HEADER)
        name = section.Name.decode('ascii', errors='ignore').rstrip('\x00')

        if name == '.data':
            table_offset = section.PointerToRawData
            table = read_struct(data, table_offset, ACPI_HEADER)

            if table.Length > section.SizeOfRawData:
                raise Exception("Corrupt ACPI table!")

            size = min(section.VirtualSize, section.SizeOfRawData)

            with open(output, 'wb') as out:
                out.write(data[table_offset:table_offset + size])

            print(f"ACPI table extracted to {output}")
            found = True
            break

    if not found:
        raise Exception("ACPI table not found in .data section")

if __name__ == "__main__":
    main(**vars(parse_args()))