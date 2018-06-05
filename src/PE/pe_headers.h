/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#pragma once
#pragma pack(push, 1)

#define MACHINE_X86_64 0x8664
#define MACHINE_X86 0x14c

#define OPT_HEADER_MAGIC_X86_64 0x20b
#define OPT_HEADER_MAGIC_X86    0x10b

#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER 0x2000
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE 0x0040

#define IMAGE_DIRECTORY_ENTRY_EXPORT	0
#define IMAGE_DIRECTORY_ENTRY_IMPORT	1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE	2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION	3
#define IMAGE_DIRECTORY_ENTRY_SECURITY	4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC	5
#define IMAGE_DIRECTORY_ENTRY_DEBUG		6
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE	7
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR	8	
#define IMAGE_DIRECTORY_ENTRY_TLS		9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG	10	
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT	11	
#define IMAGE_DIRECTORY_ENTRY_IAT			12	
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT	13	
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR	14

#define IMAGE_DIRECTORY_ENTRY_MIN IMAGE_DIRECTORY_ENTRY_EXPORT
#define IMAGE_DIRECTORY_ENTRY_MAX IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR

#define IMAGE_REL_BASED_DIR64 10
#define IMAGE_REL_BASED_ABSOLUTE 0
#define IMAGE_REL_BASED_HIGH 1
#define IMAGE_REL_BASED_LOW 2
#define IMAGE_REL_BASED_HIGHLOW 3

typedef struct _IMAGE_DATA_DIRECTORY 
{
    uint32_t VirtualAddress;
    uint32_t Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_OPTIONAL_HEADER 
{
    uint16_t Magic; //OPT_HEADER
    uint8_t MajorLinkerVersion;
    uint8_t MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;
    uint32_t BaseOfCode;
    uint32_t BaseOfData;
    uint32_t ImageBase;
    uint32_t SectionAlignment;
    uint32_t FileAlignment;
    uint16_t MajorOperatingSystemVersion;
    uint16_t MinorOperatingSystemVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;
    uint32_t Win32VersionValue;
    uint32_t SizeOfImage;
    uint32_t SizeOfHeaders;
    uint32_t CheckSum;
    uint16_t Subsystem;
    uint16_t DllCharacteristics;
    uint32_t SizeOfStackReserve;
    uint32_t SizeOfStackCommit;
    uint32_t SizeOfHeapReserve;
    uint32_t SizeOfHeapCommit;
    uint32_t LoaderFlags;
    uint32_t NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[20];
} IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER64
{
    uint16_t Magic;
    uint8_t MajorLinkerVersion;
    uint8_t MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;
    uint32_t BaseOfCode;
    uint64_t ImageBase;
    uint32_t SectionAlignment;
    uint32_t FileAlignment;
    uint16_t MajorOperatingSystemVersion;
    uint16_t MinorOperatingSystemVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;
    uint32_t Win32VersionValue;
    uint32_t SizeOfImage;
    uint32_t SizeOfHeaders;
    uint32_t CheckSum;
    uint16_t Subsystem;
    uint16_t DllCharacteristics;
    uint64_t SizeOfStackReserve;
    uint64_t SizeOfStackCommit;
    uint64_t SizeOfHeapReserve;
    uint64_t SizeOfHeapCommit;
    uint32_t LoaderFlags;
    uint32_t NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[20];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

typedef struct _IMAGE_FILE_HEADER 
{
    uint16_t Machine;
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_SECTION_HEADER 
{
    uint8_t  Name[8];
    union {
        uint32_t PhysicalAddress;
        uint32_t VirtualSize;
    } Misc;
    uint32_t VirtualAddress;
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLinenumbers;
    uint16_t  NumberOfRelocations;
    uint16_t  NumberOfLinenumbers;
    uint32_t Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef struct _IMAGE_EXPORT_DIRECTORY 
{
    long Characteristics;
    long TimeDateStamp;
    short MajorVersion;
    short MinorVersion;
    long Name;
    long Base;
    long NumberOfFunctions;
    long NumberOfNames;
    long AddressOfFunctions;
    long AddressOfNames;
    long AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY, * PIMAGE_EXPORT_DIRECTORY;

typedef struct _IMAGE_IMPORT_BY_NAME 
{
    short Hint;
    char Name[1];
} *PIMAGE_IMPORT_BY_NAME, IMAGE_IMPORT_BY_NAME;

typedef struct _IMAGE_IMPORT_DESCRIPTOR 
{
    long *OriginalFirstThunk;
    long TimeDateStamp;
    long ForwarderChain;
    long Name;
    long *FirstThunk;
} *PIMAGE_IMPORT_DESCRIPTOR, IMAGE_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_BASE_RELOCATION 
{
    uint32_t   VirtualAddress;
    uint32_t   SizeOfBlock;
} IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;

typedef struct _IMAGE_NT_HEADERS 
{
    //uint32_t Signature;  //PE
    uint8_t Signature[2][2]; //P E | 00 00 
    IMAGE_FILE_HEADER FileHeader;
    union
    {
        IMAGE_OPTIONAL_HEADER OptionalHeader;
        IMAGE_OPTIONAL_HEADER64 OptionalHeader64;
    };
} IMAGE_NT_HEADERS, *PIMAGE_NT_HEADERS;

typedef struct
{
    uint16_t	offset : 12;
    uint16_t	type : 4;
} IMAGE_RELOC, *PIMAGE_RELOC;


typedef struct _DOS_HEADER
{
    uint8_t signature[2];// = { 'M', 'Z' };
    uint16_t lastsize;
    uint16_t nblocks;
    uint16_t nreloc;
    uint16_t hdrsize;
    uint16_t minalloc;
    uint16_t maxalloc;
    uint16_t ss; // 2 uint8_t value
    uint16_t sp; // 2 uint8_t value
    uint16_t checksum;
    uint16_t ip; // 2 uint8_t value
    uint16_t cs; // 2 uint8_t value
    uint16_t relocpos;
    uint16_t noverlay;
    uint16_t reserved1[4];
    uint16_t oem_id;
    uint16_t oem_info;
    uint16_t reserved2[10];
    uint32_t e_lfanew; // Offset to the 'PE\0\0' signature relative to the beginning of the file
} *PDOS_HEADER, DOS_HEADER;

#pragma pack(pop)

#define HD_PE_HEADERS