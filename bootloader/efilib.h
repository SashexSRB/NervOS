#pragma once
#include "efi.h"
#include <stdarg.h>
#include <stdbool.h>
// Helper definitions that are not in efi.h or the UEFI Spec

#define PAD0(x) ((x) < 10 ? u"0" : u"")

// -----------------
// Global Typedefs
// -----------------
//ELF Header - x86-64
typedef struct {
  struct {
    UINT8 ei_mag0;
    UINT8 ei_mag1;
    UINT8 ei_mag2;
    UINT8 ei_mag3;
    UINT8 ei_class;
    UINT8 ei_data;
    UINT8 ei_version;
    UINT8 ei_osabi;
    UINT8 ei_abiversion;
    UINT8 ei_pad[7];
  } e_ident;

  UINT16 e_type;
  UINT16 e_machine;
  UINT32 e_version;
  UINT64 e_entry; // Entry point address
  UINT64 e_phoff; // Program header offset
  UINT64 e_shoff; // Section header offset
  UINT32 e_flags;
  UINT16 e_ehsize; // ELF header size
  UINT16 e_phentsize; // Program header entry size
  UINT16 e_phnum; // Number of program header entries
  UINT16 e_shentsize; // Section header entry size
  UINT16 e_shnum; // Number of section header entries
  UINT16 e_shstrndx; // Section header string table index
} __attribute__ ((packed)) ELF_Header_64;

// ELF Program Header - x86-64
typedef struct {
  UINT32 p_type; // Type of segment
  UINT32 p_flags; // Segment flags
  UINT64 p_offset; // Offset in file
  UINT64 p_vaddr; // Virtual address in memory
  UINT64 p_paddr; // Physical address (not used)
  UINT64 p_filesz; // Size of segment in file
  UINT64 p_memsz; // Size of segment in memory
  UINT64 p_align; // Alignment of segment
} __attribute__ ((packed)) ELF_Program_Header_64;

// ELF Header e_type values
typedef enum {
  ET_EXEC = 0x2,
  ET_DYN = 0x3,
} ELF_EHEADER_TYPE;

// ELF Program Header p_type values
typedef enum {
  PT_NULL = 0x0,
  PT_LOAD = 0x1, // Loadable
} ELF_PHEADER_TYPE;

// PE Structs/types
// PE32+ COFF File Header
typedef struct {
  UINT16 Machine; // Machine type
  UINT16 NumberOfSections; // Number of sections
  UINT32 TimeDateStamp; // Time and date stamp
  UINT32 PointerToSymbolTable; // Pointer to symbol table (not used)
  UINT32 NumberOfSymbols; // Number of symbols (not used)
  UINT16 SizeOfOptionalHeader; // Size of optional header
  UINT16 Characteristics; // Characteristics flags
} __attribute__ ((packed)) PE_Coff_File_Header_64;

// COFF File Header Characteristics
typedef enum {
  IMAGE_FILE_EXECUTABLE_IMAGE = 0x0002, // Executable image
} PE_COFF_CHARACTERISTICS;

// PE32+ Optional Header
typedef struct {
  UINT16 Magic; // Magic number (0x20B for PE32+)
  UINT8 MajorLinkerVersion; // Major version of linker
  UINT8 MinorLinkerVersion; // Minor version of linker
  UINT32 SizeOfCode; // Size of code section
  UINT32 SizeOfInitializedData; // Size of initialized data section
  UINT32 SizeOfUninitializedData; // Size of uninitialized data section
  UINT32 AddressOfEntryPoint; // Entry point address
  UINT32 BaseOfCode; // Base address of code section
  UINT64 ImageBase; // Base address of image in memory
  UINT32 SectionAlignment; // Section alignment in memory
  UINT32 FileAlignment; // Section alignment in file
  UINT16 MajorOperatingSystemVersion; // Major OS version
  UINT16 MinorOperatingSystemVersion; // Minor OS version
  UINT16 MajorImageVersion; // Major image version
  UINT16 MinorImageVersion; // Minor image version
  UINT16 MajorSubsystemVersion; // Major subsystem version
  UINT16 MinorSubsystemVersion; // Minor subsystem version
  UINT32 Win32VersionValue; // Reserved, must be zero
  UINT32 SizeOfImage; // Size of image in memory
  UINT32 SizeOfHeaders; // Size of headers in file
  UINT32 CheckSum; // Checksum of image
  UINT16 Subsystem; // Subsystem type
  UINT16 DllCharacteristics; // DLL characteristics flags
  UINT64 SizeOfStackReserve; // Size of stack reserve
  UINT64 SizeOfStackCommit; // Size of stack commit
  UINT64 SizeOfHeapReserve; // Size of heap reserve
  UINT64 SizeOfHeapCommit; // Size of heap commit
  UINT32 LoaderFlags; // Loader flags (reserved, must be zero)
  UINT32 NumberOfRvaAndSizes; // Number of data directories
} __attribute__ ((packed)) PE_Optional_Header_64;

// Optional Header Characteristics
typedef enum {
  IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE = 0x0040, // DLL can be relocated at load time
} PE_OPTIONAL_HEADER_CHARACTERISTICS;

// PE32+ Section Headers - Immediately follows the optional header
typedef struct {
  UINT64 Name; // Section name (offset to string table)
  UINT32 VirtualSize; // Size of section in memory
  UINT32 VirtualAddress; // Virtual address of section in memory
  UINT32 SizeOfRawData; // Size of section in file
  UINT32 PointerToRawData; // Offset to section data in file
  UINT32 PointerToRelocations; // Offset to relocations (not used)
  UINT32 PointerToLinenumbers; // Offset to line numbers (not used)
  UINT16 NumberOfRelocations; // Number of relocations (not used)
  UINT16 NumberOfLinenumbers; // Number of line numbers (not used)
  UINT32 Characteristics; // Section characteristics flags
} __attribute__ ((packed)) PE_Section_Header_64;

typedef struct {
  UINTN size;
  EFI_MEMORY_DESCRIPTOR *map;
  UINTN key;
  UINTN descriptorSize;
  UINT32 descriptorVersion;
} MemoryMapInfo;

// EFI Configuration Table GUIDs and string names
typedef struct {
  EFI_GUID guid;
  char16_t *string;
} EFI_Guid_String;

// Default
EFI_Guid_String configTableGuidsAndStrings[] = {
  {EFI_ACPI_TABLE_GUID,   u"EFI_ACPI_TABLE_GUID"},
  {ACPI_TABLE_GUID,       u"ACPI_TABLE_GUID"},
  {SAL_SYSTEM_TABLE_GUID, u"SAL_SYSTEM_TABLE_GUID"},
  {SMBIOS_TABLE_GUID,     u"SMBIOS_TABLE_GUID"},
  {SMBIOS3_TABLE_GUID,    u"SMBIOS3_TABLE_GUID"},
  {MPS_TABLE_GUID,        u"MPS_TABLE_GUID"},
};

// General ACPI description header
typedef struct {
  char signature[4];
  UINT32 length;
  UINT8 revision;
  UINT8 checksum;
  char OEMID[6];
  char OEMTableID[8];
  UINT32 OEMRevision;
  char creatorID;
  UINT32 creatorRevision;
} ACPI_TABLE_HEADER;

// Example Kernel Parameters
typedef struct {
  MemoryMapInfo *mMap; // Get memory map to fill this out
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE gopMode;
  EFI_RUNTIME_SERVICES *RuntimeServices;
  EFI_CONFIGURATION_TABLE *ConfigurationTable;
  UINTN NumberOfTableEntries;
} KernelParameters;

// Kernel Entry Point Typedef
typedef void EFIAPI (*EntryPoint)(KernelParameters);

// KEY:
// GDT = Global Descriptor Table
// LDT = Local Descriptor Table
// TSS = Task State Segment
 
// Use this for GDTR with assembly "LGDT" instruction
typedef struct {
  UINT16 limit;
  UINT64 base;
} __attribute__((packed)) DescriptorRegister;

// Descriptor e.g an array of these is used for the GDT/LDT
typedef struct {
  union {
    UINT64 value;
    struct {
      UINT64 limit_15_0:  16;
      UINT64 base_15_0:   16;
      UINT64 base_23_16:  8;
      UINT64 type:        4;
      UINT64 s:           1;
      UINT64 dpl:         2;
      UINT64 p:           1;
      UINT64 limit_19_16: 4;
      UINT64 avl:         1;
      UINT64 l:           1;
      UINT64 d_b:         1;
      UINT64 g:           1;
      UINT64 base31_24:   8;
    };
  };
} X86_64_Descriptor;

// TSS/LDT Descriptor 64bit
typedef struct {
  X86_64_Descriptor descriptor;
  UINT64 base63_32;
  UINT32 zero;
} TSS_LDT_Descriptor;

// TSS Structure - TSS Descriptor points to this structure in the GDT
typedef struct {
  UINT32 reserved_1;
  UINT32 RSP0_lower;
  UINT32 RSP0_upper;
  UINT32 RSP1_lower;
  UINT32 RSP1_upper;
  UINT32 RSP2_lower;
  UINT32 RSP2_upper;
  UINT32 reserved_2;
  UINT32 reserved_3;
  UINT32 IST1_lower;
  UINT32 IST1_upper;
  UINT32 IST2_lower;
  UINT32 IST2_upper;
  UINT32 IST3_lower;
  UINT32 IST3_upper;
  UINT32 IST4_lower;
  UINT32 IST4_upper;
  UINT32 IST5_lower;
  UINT32 IST5_upper;
  UINT32 IST6_lower;
  UINT32 IST6_upper;
  UINT32 IST7_lower;
  UINT32 IST7_upper;
  UINT32 reserved_4;
  UINT32 reserved_5;
  UINT16 reserved_6;
  UINT16 io_map_base;
} TSS;

// Example GDT
typedef struct {
  X86_64_Descriptor null;
  X86_64_Descriptor kernelCode64;
  X86_64_Descriptor kernelData64;
  X86_64_Descriptor userCode64;
  X86_64_Descriptor userData64;
  X86_64_Descriptor kernelCode32;
  X86_64_Descriptor kernelData32;
  X86_64_Descriptor userCode32;
  X86_64_Descriptor userData32;
  TSS_LDT_Descriptor tss;
} GDT;

// -----------------
//  Global vars
// -----------------
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout = NULL; // Console out
EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin = NULL; // Console out
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cerr = NULL; // Console error
EFI_SYSTEM_TABLE *st; // Systable
EFI_BOOT_SERVICES *bs; // Boot Services
EFI_RUNTIME_SERVICES *rs; // Runtime Services
EFI_HANDLE image = NULL; // Image handle
EFI_EVENT timerEvent = NULL; // Timer event for printing date/time

// -----------------
//  Global macros
// -----------------
#define ARR_SIZE(x) (sizeof (x) / sizeof (x)[0])
#define BOOL_TO_YN(b) ((b) ? u"Yes" : u"No")

// -----------------
//  Global constants
// -----------------
#define DEFAULT_FG_COLOR EFI_RED
#define DEFAULT_BG_COLOR EFI_BLACK
#define HL_FG_COLOR EFI_BLACK
#define HL_BG_COLOR EFI_LIGHTGRAY
#define px_LGRAY { 0xEE, 0xEE, 0xEE, 0x00 }
#define px_BLACK { 0x00, 0x00, 0x00, 0x00 }

#define PHYSICAL_PAGE_ADDR_MASK 0x000FFFFFFFFFF000 // 52 bits physical addr limit, lowest 12 are for flags only
#define PAGE_SIZE 4096

#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20B // PE32+ magic number

// Kernel start address in higher memory (64bit)
#define KERNEL_START_ADDR 0xFFFFFFFF80000000

typedef struct {
  UINT64 entries[512];
} pageTable;

// Page flags: bits 11-0
enum {
  PRESENT = (1 << 0),
  RW =(1 << 1),
  USER = (1 << 2)
};

// -----------------
// Mouse drawing stuff
// -----------------
// Mouse cursor buffer 8x8
EFI_GRAPHICS_OUTPUT_BLT_PIXEL cursorBuffer[] = {
    px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, // Line 1
    px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, // Line 2
    px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, px_BLACK, px_BLACK, // Line 3
    px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, px_BLACK, // Line 4
    px_LGRAY, px_LGRAY, px_BLACK, px_LGRAY, px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, // Line 5
    px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, px_LGRAY, px_LGRAY, px_LGRAY, px_BLACK, // Line 6
    px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, px_BLACK, px_LGRAY, px_LGRAY, px_LGRAY, // Line 7
    px_LGRAY, px_LGRAY, px_BLACK, px_BLACK, px_BLACK, px_BLACK, px_LGRAY, px_LGRAY, // Line 8
};
// Buffer to save FB data at cursor position
EFI_GRAPHICS_OUTPUT_BLT_PIXEL savedBuffer[8*8] = {0};

// =================
// memset for compling with clang/gcc (set len bytes of dst memory with int c) UNCOMMENT FOR CLANG
// =================
VOID *memset(VOID *dst, UINT8 c, UINTN len) {
  UINT8 *p = dst;
  for(UINTN i = 0; i < len; i++) {
    p[i] = c;
  }
  return dst;
}

// =================
// memcpy for compiling with clang/gcc (set len bytes of dst memory from src) UNCOMMENT FOR CLANG
// =================
VOID *memcpy(VOID *dst, VOID *src, UINTN len) {
  UINT8 *p = dst;
  UINT8 *q = src;
  for(UINTN i = 0; i < len; i++) {
    p[i] = q[i];
  }
  return dst;
}

// ================
// memcmp Compare up to len bytes of m1 and m2, stop at first point that they dont equal.
// ================
INTN memcmp(VOID *m1, VOID *m2, UINTN len) {
  UINT8 *p = m1;
  UINT8 *q = m2;
  for(UINTN i = 0; i < len; i++) {
    if(p[i] != q[i]) return (INTN)p[i] - (INTN)q[i];
  }
  return 0;
}

// =================
// strcmp Compare two strings, stop at first point that they dont equal.
// =================
INTN strcmp(char *s1, char *s2) {
  UINTN i = 0;
  while (s1 && s2 && *s1 && *s2) {
    if(s1[i] != s2[i]) break;
    i++;
  }
  return (INTN)(s1[i]) - (INTN)(s2[i]);
}

// =================
// strLen
// =================
UINTN strLen(char *str) {
  UINTN len = 0;
  while(*str) {
    len++;
    str++;
  }
  return len;
}

// =================
// strLen
// =================
UINTN strLenC16(CHAR16 *str) {
  UINTN len = 0;
  while(*str) {
    len++;
    str++;
  }
  return len;
}

// ======================================================
// (CHAR16) strrev:
//  Reverse a string.
//  Returns reversed string.
// ======================================================
CHAR16 *strRevC16(CHAR16 *s) {
    if (!s) return s;

    CHAR16 *start = s, *end = s + strLenC16(s)-1;
    while (start < end) {
        CHAR16 temp = *end;  // Swap
        *end-- = *start;
        *start++ = temp;
    }

    return s;
}

CHAR16 *strCatC16(CHAR16 *dst, CHAR16 *src) {
    CHAR16 *s = dst;
    while (*s) s++;             // Go until null terminator
    while (*src) *s++ = *src++; // Copy src to dst at null position
    *s = u'\0';                 // Null terminate new string
    return dst; 
}


// =================
// Substring function
// =================
char *substr(char *haystack, char *needle) {
  if(!needle) return haystack;

  char *p = haystack;
  while(*p) {
    if(*p == *needle) {
      if(!memcmp(p, needle, strLen(needle))) return p;
    }
    p++;
  }
  return NULL;
}

// =================
// isDigit
// =================
BOOLEAN isDigit(char c) {
  return (c >= '0' && c <= '9');
}

// =================
// isDigit CHAR16
// =================
BOOLEAN isDigitC16(CHAR16 c) {
  return (c >= u'0' && c <= u'9');
}

// =================
// isHexDigit
// =================
BOOLEAN isHexDigitC16(CHAR16 c) {
  return (c >= u'0' && c <= u'9') ||
         (c >= u'a' && c <= u'f') ||
         (c >= u'A' && c <= u'F');
}

// =================
// Copy string CHAR16 strcpy
// =================
CHAR16 *strcpy_u16(CHAR16 *dst, CHAR16 *src) {
  if(!dst) return NULL;
  if(!src) return dst;

  CHAR16 *result = dst;
  while(*src) {
    *dst++ = *src++;
  }
  *dst = u'\0'; // Null terminate

  return result;
}

// =================
// Compare string CHAR16 strcmp
// =================
INTN strcmp_u16(CHAR16 *s1, CHAR16 *s2, UINTN len) {
  if(len == 0) return 0;
  while(len > 0 && *s1 && *s2 && *s1 == *s2) {
    s1++;
    s2++;
    len--;
  }
  return *s1 - *s2;
}

// =================
// Compare string CHAR16 strrchr
// =================
CHAR16 *strrchr_u16(CHAR16 *str, CHAR16 c) {
  CHAR16 *result = NULL;

  while (*str) {
    if(*str == c) result = str;
    str++;
  }

  return result;
}

// =================
// concatenate string CHAR16 strcat
// =================
CHAR16 *strcat_u16(CHAR16 *dst, CHAR16 *src) {
  CHAR16 *s = dst;

  while(*s) s++; // Go until null terminator
  while(*src) *s++ = *src++;

  *s = u'\0';
  return dst;
}

// =================
// Print an number to stdout
// =================
BOOLEAN printNum(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *stream, UINTN number, UINT8 base, BOOLEAN isSigned, UINTN minDigits) {
  const CHAR16 *digits = u"0123456789ABCDEF";
  CHAR16 buffer[24]; // enough for UINTN_MAX (UINT64_MAX) + sign
  UINTN i = 0;
  BOOLEAN negative = FALSE;

  if(base > 16) {
    cerr->OutputString(cerr, u"Invalid Base Specified!\r\n");
    return FALSE;
  } // Invalid base

  // only use and print neg numbers if decimal is signed. 
  if(base == 10 && isSigned && (INTN)number < 0) {
    number = -(INTN)number; // Get absolute value of correct signed value to get digits to print
    negative = TRUE;
  }

  do {
    buffer[i++] = digits[number % base];
    number /= base;
  } while(number > 0);

  // Pad with 0s
  while (i < minDigits) buffer[i++] = u'0'; 

  // Print negative sign for decimal
  if (base == 10 && negative) buffer[i++] = u'-';
 
  // null terminate string first
  buffer[i--] = u'\0';

  // reverse buffer before printing
  for (UINTN j = 0; j < i; j++, i--) {
    // swap digits
    UINTN temp = buffer[i];
    buffer[i] = buffer[j];
    buffer[j] = temp;
  }

  // print number string
  stream->OutputString(stream, buffer);

  return TRUE;
}

// =================
// Get key from user
// =================
EFI_INPUT_KEY getKey(void) {
    EFI_EVENT events[1];
    EFI_INPUT_KEY key;
    key.ScanCode = 0;
    key.UnicodeChar = u'\0';

    events[0] = cin->WaitForKey;
    UINTN idx = 0;
    bs->WaitForEvent(1, events, &idx);

    if(idx == 0) cin->ReadKeyStroke(cin, &key);
    return key;
}

// ==========================================
// (CHAR16) Add integer as string to buffer
// ==========================================
BOOLEAN addIntToBufC16(UINTN number, UINT8 base, BOOLEAN signed_num, UINTN min_digits, CHAR16 *buf, UINTN *buf_idx) {
    const CHAR16 *digits = u"0123456789ABCDEF";
    CHAR16 buffer[24];  // Hopefully enough for UINTN_MAX (UINT64_MAX) + sign character
    UINTN i = 0;
    BOOLEAN negative = FALSE;

    if (base > 16) {
        cerr->OutputString(cerr, u"Invalid base specified!\r\n");
        return FALSE;    // Invalid base
    }

    // Only use and print negative numbers if decimal and signed True
    if (base == 10 && signed_num && (INTN)number < 0) {
       number = -(INTN)number;  // Get absolute value of correct signed value to get digits to print
       negative = TRUE;
    }

    do {
       buffer[i++] = digits[number % base];
       number /= base;
    } while (number > 0);

    while (i < min_digits) buffer[i++] = u'0'; // Pad with 0s

    // Add negative sign for decimal numbers
    if (base == 10 && negative) buffer[i++] = u'-';

    // NULL terminate string
    buffer[i--] = u'\0';

    // Reverse buffer to read left to right
    strRevC16(buffer);

    // Add number string to input buffer for printing
    for (CHAR16 *p = buffer; *p; p++) {
        buf[*buf_idx] = *p;
        *buf_idx += 1;
    }
    return TRUE;
}

// ========================================================================
// (CHAR16) Fill formatted string buffer with printf() format conversions
// ========================================================================
bool formatStringC16(CHAR16 *buf, CHAR16 *fmt, va_list args) {
  bool result = true;
  CHAR16 charstr[2] = {0};    
  UINTN buf_idx = 0;

  for (UINTN i = 0; fmt[i] != u'\0'; i++) {
    if (fmt[i] == u'%') {
      bool alternate_form = false;
      UINTN min_field_width = 0;
      UINTN precision = 0;
      UINTN length_bits = 0;  
      UINTN num_printed = 0;      // # of digits/chars printed for numbers or strings
      UINT8 base = 0;
      bool input_precision = false;
      bool signed_num   = false;
      bool int_num      = false;
      bool double_num   = false;
      bool left_justify = false;  // Left justify text from '-' flag instead of default right justify
      bool space_flag   = false;
      bool plus_flag    = false;
      CHAR16 padding_char = ' ';  // '0' or ' ' depending on flags
      i++;

      // Check for flags
      while (true) {
        switch (fmt[i]) {
            case u'#':
              // Alternate form
              alternate_form = true;
              i++;
            continue;

            case u'0':
              // 0-pad numbers on the left, unless '-' or precision is also defined
              padding_char = '0'; 
              i++;
            continue;

            case u' ':
              // Print a space before positive signed number conversion or empty string
              //   number conversions
              space_flag = true;
              if (plus_flag) space_flag = false;  // Plus flag '+' overrides space flag
              i++;
            continue;

            case u'+':
              // Always print +/- before a signed number conversion
              plus_flag = true;
              if (space_flag) space_flag = false; // Plus flag '+' overrides space flag
              i++;
            continue;

            case u'-':
                  left_justify = true;
                  i++;
            continue;

            default:
            break;
        }
        break; // No more flags
      }

      // Check for minimum field width e.g. in "8.2" this would be 8
      if (fmt[i] == u'*') {
        // Get int argument for min field width
        min_field_width = va_arg(args, int);
        i++;
      } else {
        // Get number literal from format string
        while (isDigitC16(fmt[i])) 
          min_field_width = (min_field_width * 10) + (fmt[i++] - u'0');
      }

      // Check for precision/maximum field width e.g. in "8.2" this would be 2
      if (fmt[i] == u'.') {
        input_precision = true; 
        i++;
        if (fmt[i] == u'*') {
          // Get int argument for precision
          precision = va_arg(args, int);
          i++;
        } else {
          // Get number literal from format string
          while (isDigitC16(fmt[i])) 
            precision = (precision * 10) + (fmt[i++] - u'0');
        }
      }

      // Check for Length modifiers e.g. h/hh/l/ll
      if (fmt[i] == u'h') {
        i++;
        length_bits = 16;       // h
        if (fmt[i] == u'h') {
          i++;
          length_bits = 8;    // hh
        }
      } else if (fmt[i] == u'l') {
        i++;
        length_bits = 32;       // l
        if (fmt[i] == u'l') {
          i++;
          length_bits = 64;    // ll
        }
      }

      // Check for conversion specifier
      switch (fmt[i]) {
        case u'c': {
          // Print CHAR16 value; printf("%c", char)
          if (length_bits == 8)
            charstr[0] = (char)va_arg(args, int);   // %hhc "ascii" or other 8 bit char
          else
            charstr[0] = (CHAR16)va_arg(args, int); // Assuming 16 bit char16_t

          // Only add non-null characters, to not end string early
          if (charstr[0]) buf[buf_idx++] = charstr[0];    
        }
        break;

        case u's': {
          // Print CHAR16 string; printf("%s", string)
          if (length_bits == 8) {
            char *string = va_arg(args, char*);         // %hhs; Assuming 8 bit ascii chars
            while (*string) {
              buf[buf_idx++] = *string++;
              if (++num_printed == precision) break;  // Stop printing at max characters
            }

          } else {
            CHAR16 *string = va_arg(args, CHAR16*);     // Assuming 16 bit char16_t
            while (*string) {
              buf[buf_idx++] = *string++;
              if (++num_printed == precision) break;  // Stop printing at max characters
            }
          }
        }
        break;

        case u'd': {
          // Print INT32; printf("%d", number_int32)
          int_num = true;
          base = 10;
          signed_num = true;
        }
        break;

        case u'x': {
          // Print hex UINTN; printf("%x", number_uintn)
          int_num = true;
          base = 16;
          signed_num = false;
          if (alternate_form) {
            buf[buf_idx++] = u'0';
            buf[buf_idx++] = u'x';
          }
        }
        break;

        case u'u': {
          // Print UINT32; printf("%u", number_uint32)
          int_num = true;
          base = 10;
          signed_num = false;
        }
        break;

        case u'b': {
          // Print UINTN as binary; printf("%b", number_uintn)
          int_num = true;
          base = 2;
          signed_num = false;
          if (alternate_form) {
            buf[buf_idx++] = u'0';
            buf[buf_idx++] = u'b';
          }
        }
        break;

        case u'o': {
          // Print UINTN as octal; printf("%o", number_uintn)
          int_num = true;
          base = 8;
          signed_num = false;
          if (alternate_form) {
            buf[buf_idx++] = u'0';
            buf[buf_idx++] = u'o';
          }
        }
        break;

        case u'f': {
          // Print INTN rounded float value
          double_num = true;
          signed_num = true;
          base = 10;
          if (!input_precision) precision = 6;    // Default decimal places to print
        }
        break;

        default:
          strcpy_u16(buf, u"Invalid format specifier: %");
          charstr[0] = fmt[i];
          strCatC16(buf, charstr);
          strCatC16(buf, u"\r\n");
          result = false;
          goto end;
        break;
      }

      if (int_num) {
        // Number conversion: Integer
        UINT64 number = 0;
        switch (length_bits) {
          case 0:
          case 32: 
          default:
            // l
            number = va_arg(args, UINT32);
            if (signed_num) number = (INT32)number;
          break;

          case 8:
            // hh
            number = (UINT8)va_arg(args, int);
            if (signed_num) number = (INT8)number;
          break;

          case 16:
            // h
            number = (UINT16)va_arg(args, int);
            if (signed_num) number = (INT16)number;
          break;

          case 64:
            // ll
            number = va_arg(args, UINT64);
            if (signed_num) number = (INT64)number;
          break;
        }

        // Add space before positive number for ' ' flag
        if (space_flag && signed_num && (INTN)number >= 0) buf[buf_idx++] = u' ';    

        // Add sign +/- before signed number for '+' flag
        if (plus_flag && signed_num) buf[buf_idx++] = (INTN)number >= 0 ? u'+' : u'-';

        addIntToBufC16(number, base, signed_num, precision, buf, &buf_idx);
      }

      if (double_num) {
        // Number conversion: Float/Double
        double number = va_arg(args, double);
        INTN whole_num = 0;

        // Get digits before decimal point
        whole_num = (INTN)number;
        if (whole_num < 0) whole_num = -whole_num;

        UINTN num_digits = 0;
        do {
          num_digits++; 
          whole_num /= 10;
        } while (whole_num > 0);

        // Add digits to write buffer
        addIntToBufC16(number, base, signed_num, num_digits, buf, &buf_idx);

        // Print decimal digits equal to precision value, 
        //   if precision is explicitly 0 then do not print
        if (!input_precision || precision != 0) {
          buf[buf_idx++] = u'.';      // Add decimal point

          if (number < 0.0) number = -number; // Ensure number is positive
          whole_num = (INTN)number;
          number -= whole_num;                // Get only decimal digits
          signed_num = FALSE;                 // Don't print negative sign for decimals

          // Move precision # of decimal digits before decimal point 
          //   using base 10, number = number * 10^precision
          for (UINTN i = 0; i < precision; i++)
            number *= 10;

          // Add digits to write buffer
          addIntToBufC16(number, base, signed_num, precision, buf, &buf_idx);
        }
      }

      // Flags are defined such that 0 is overruled by left justify and precision
      if (padding_char == u'0' && (left_justify || precision > 0))
        padding_char = u' ';

      // Add padding depending on flags (0 or space) and left/right justify
      INTN diff = min_field_width - buf_idx;
      if (diff > 0) {
        if (left_justify) {
          // Append padding to minimum width, always spaces
          while (diff--) buf[buf_idx++] = u' ';   
        } else {
          // Right justify
          // Copy buffer to end of buffer
          INTN dst = min_field_width-1, src = buf_idx-1;
          while (src >= 0)  buf[dst--] = buf[src--];  // e.g. "TEST\0\0" -> "TETEST"

          // Overwrite beginning of buffer with padding
          dst = (int_num && alternate_form) ? 2 : 0;  // Skip 0x/0b/0o/... prefix
          while (diff--) buf[dst++] = padding_char;   // e.g. "TETEST" -> "  TEST"
        }
      }
    } else {
      // Not formatted string, print next character
      buf[buf_idx++] = fmt[i];
    }
  }

  end:
  buf[buf_idx] = u'\0'; 
  va_end(args);
  return result;
}


// =================
// Print formatted strings to stdout using a va_list for args
// =================
bool vfprintf(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *stream, CHAR16 *fmt, va_list args) {
  CHAR16 buf[1024];   // Format string buffer for % strings
  if (!formatStringC16(buf, fmt, args)) return false;
  return !EFI_ERROR(stream->OutputString(stream, buf));
} 

// =================
// printf
// =================
bool printf(CHAR16 *fmt, ...) {
  bool result = true;
  va_list args;
  va_start(args, fmt);
  result = vfprintf(cout, fmt, args);
  va_end(args);
  return result;
}

// =================
// Print error message and get a key from user, so they can acknowledge the error and it doesnt go on immediately
// =================
bool error(char *file, int line, const char *func, EFI_STATUS status, CHAR16 *fmt, ...) {

  printf(u"\r\nERROR: FILE %hhs, LINE %d, FUNCTION %hhs\r\n", file, line, func);

  // Print error code & string if applicable
  if(status > 0 && status - TOP_BIT < MAX_EFI_ERROR) printf(u"STATUS: %#llx (%s)\r\n", status, EFI_ERROR_STRINGS[status - TOP_BIT]);
  
  va_list args;
  va_start(args, fmt);
  bool result = vfprintf(cerr, fmt, args); // Printf the error message to stderr

  getKey(); // User will respond with input before going on
  va_end(args);
  return result;
}

#define error(...) error(__FILE__, __LINE__, __func__, __VA_ARGS__);

// =================
// Get a number from the user with a getKey loop
// =================
BOOLEAN getNumber(INTN *number) {
  EFI_INPUT_KEY key = {0};
  if (!number) return false; // Passed in NULL pointer
  *number = 0;
  do {
    key = getKey();
    if(key.ScanCode == SCANCODE_ESC) return FALSE; // user wants to leave
    if(isDigitC16(key.UnicodeChar)) {
      *number = (*number * 10) + (key.UnicodeChar - u'0');
      printf(u"%c", key.UnicodeChar);
    }
  } while (key.UnicodeChar != u'\r');
  return TRUE;
}

// =================
// Get a hex number from user with a getKey loop
// =================
BOOLEAN getHex(UINTN *number) {
  EFI_INPUT_KEY key = {0};
  if (!number) return false; // Passed in NULL pointer
  *number = 0;
  do {
    key = getKey();
    if(key.ScanCode == SCANCODE_ESC) return FALSE; // user wants to leave
    if(isDigitC16(key.UnicodeChar)) {
      *number = (*number * 16) + (key.UnicodeChar - u'0');
      printf(u"%c", key.UnicodeChar);
    } else if (key.UnicodeChar >= u'a' && key.UnicodeChar <= u'f') {
      *number = (*number * 16) + (key.UnicodeChar - u'a' + 10);
      printf(u"%c", key.UnicodeChar);
    } else if (key.UnicodeChar >= u'A' && key.UnicodeChar <= u'F') {
      *number = (*number * 16) + (key.UnicodeChar - u'A' + 10);
      printf(u"%c", key.UnicodeChar);
    } 
  } while (key.UnicodeChar != u'\r');
  return TRUE;
}

// =================
// Print a GUID value
// =================
VOID printGuid(EFI_GUID guid) {
  UINT8 *p = (UINT8 *)&guid;
  printf(
    u"{%x,%x,%x,%x,%x,{%x,%x,%x,%x,%x,%x}}",
    *(UINT32 *)&p[0], 
    *(UINT16 *)&p[4],
    *(UINT16 *)&p[6],
    (UINTN)p[8], (UINTN)p[9], (UINTN)p[10],(UINTN)p[11],
    (UINTN)p[12],(UINTN)p[13],(UINTN)p[14],(UINTN)p[15]
  );
}

// =================
// Printing DateTime
// =================
VOID EFIAPI printDateTime(IN EFI_EVENT event, IN VOID *Context) {
  (VOID)event; // Suppress compiler warnings

  // Timer context will be the textmode screen bounds
  typedef struct {
    UINT32 rows, cols;
  } TimerContext;

  TimerContext context = *(TimerContext *)Context;

  // Save current cursor pos before printing
  UINT32 saveCol = cout->Mode->CursorColumn, saveRow = cout->Mode->CursorRow;

  // Get datetime
  EFI_TIME time = {0};
  EFI_TIME_CAPABILITIES capabilities = {0};

  // get current datetime
  rs->GetTime(&time, &capabilities);

  // Move cursor to print in lower right corner
  cout->SetCursorPosition(cout, context.cols - 20, context.rows -1);

  // Print current Datetime
  printf(u"%u-%s%u-%s%u %s%u:%s%u:%s%u",
    time.Year,
    PAD0(time.Month), time.Month,
    PAD0(time.Day),   time.Day,
    PAD0(time.Hour),  time.Hour,
    PAD0(time.Minute),time.Minute,
    PAD0(time.Second),time.Second
  );

  // Restore cursor pos
  cout->SetCursorPosition(cout, saveCol, saveRow);
}
