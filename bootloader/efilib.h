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
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *printfCout = NULL; // Printf specific cout/cerr
EFI_SYSTEM_TABLE *st; // Systable
EFI_BOOT_SERVICES *bs; // Boot Services
EFI_RUNTIME_SERVICES *rs; // Runtime Services
EFI_HANDLE image = NULL; // Image handle
EFI_EVENT timerEvent = NULL; // Timer event for printing date/time

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
BOOLEAN printNum(UINTN number, UINT8 base, BOOLEAN isSigned, UINTN minDigits) {
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
  printfCout->OutputString(printfCout, buffer);

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

// =================
// Print formatted strings to stdout using a va_list for args
// =================
bool vprintf(CHAR16 *fmt, va_list args) {
  bool result = TRUE;
  CHAR16 cstr[2] = {0};; // place init this with memset and use = {} initializer
  
  //Initialize buffer
  cstr[0] = u'\0', cstr[1] = u'\0';

  // Print formatted string values
  for (UINTN i=0 ; fmt[i] != u'\0'; i++) {
    if (fmt[i] == u'%') {
      bool alternateForm = false;
      UINTN minFieldWidth = 0; //8/16/32/64
      UINTN precision = 0;
      UINTN lengthBits = 0;
      UINTN numPrinted = 0; // # of digits/chars printed
      UINT8 base = 0;
      bool signedNum = false;
      bool numeric = false;
      i++;

      // Check for flags
      if(fmt[i] == u'#') {
        // Alternate form
        alternateForm = true;
        i++;
      }

      // '0', '-', ...

      // Check for minimum field
      if(fmt[i] == u'*') {
        // get int arg for minFieldWidth
        minFieldWidth = va_arg(args, int);
        i++;
      } else {
        // get number literal from fmt string
        while (isDigitC16(fmt[i])) {
          minFieldWidth = (minFieldWidth * 10) + (fmt[i++] - u'0');
        }
      }

      // Check for precision/maximum field width
      if (fmt[i] == u'.') {
        i++;
        if(fmt[i] == u'*') {
          // get int arg for precision
          precision = va_arg(args, int);
          i++;
        } else {
          // Get num literal for minFieldWidth
          while (isDigitC16(fmt[i])) {
            precision = (precision * 10) + (fmt[i++] - u'0');
          }
        }
      }

      // Check for length modifiers e.g h/hh/l/ll
      if(fmt[i] == u'h') {
        i++;
        lengthBits = 16;
        if(fmt[i] == u'h') {
          i++;
          lengthBits = 8;
        }
      } else if (fmt[i] == u'l') {
        i++;
        lengthBits = 32;
        if(fmt[i] == u'l') {
          i++;
          lengthBits = 64;
        }
      }

      // Check for conversion specifier
      // Grab next arg type from input args and print it
      switch(fmt[i]) {
        case u'c': {
          if(lengthBits == 8) {
            cstr[0] = (char)va_arg(args, int); // ASCII or other 8bit
          } else {
            cstr[0] = (CHAR16)va_arg(args, int); // Assuming 16bit char16_t
          }
          printfCout->OutputString(printfCout, cstr);
        }
        break;
        case u's': {
          if(lengthBits == 8) {
            char *string = va_arg(args, char *); // %hhs: assuming 8 bit ascii
            while (*string) {
              cstr[0] = *string++;
              printfCout->OutputString(printfCout, cstr);
              numPrinted++;
              if(numPrinted == precision) break; // stop printing at max characters
            }
            // pad out with blanks by default to minimum field width
            while (numPrinted < minFieldWidth) {
              cstr[0] = u' ';
              printfCout->OutputString(printfCout, cstr);
              numPrinted++;
            }
          } else { 
            CHAR16 *string = va_arg(args, CHAR16*); // Assuming 16bit char16_t
             while (*string) {
              cstr[0] = *string++;
              printfCout->OutputString(printfCout, cstr);
              numPrinted++;
              if(numPrinted == precision) break; // stop printing at max characters
            }
            // pad out with blanks by default to minimum field width
            while (numPrinted < minFieldWidth) {
              cstr[0] = u' ';
              printfCout->OutputString(printfCout, cstr);
              numPrinted++;
            }
          }
        }
        break;
        case u'b': {
          numeric = true;
          base = 2;
          signedNum = false;
          if(alternateForm) printfCout->OutputString(printfCout, u"0b");
        }
        break;
        case u'o': {
          numeric = true;
          base = 8;
          signedNum = false;
          if(alternateForm) printfCout->OutputString(printfCout, u"0o");
        }
        break;
        case u'd': {
          numeric = true;
          base = 10;
          signedNum = true;
        }
        break;
        case u'u': {
          numeric = true;
          base = 10;
          signedNum = false;
        }
        break;
        case u'x': {
          numeric = true;
          base = 16;
          signedNum = false;
          if(alternateForm) printfCout->OutputString(printfCout, u"0x");
        }
        break;
        default:
          printfCout->OutputString(printfCout, u"Invalid format specifier: %");
          cstr[0] = fmt[i];
          printfCout->OutputString(printfCout, cstr);
          printfCout->OutputString(printfCout, u"\r\n");
          result = FALSE;
          goto end;
        break;
      }
      if(numeric) {
        // Printing a number
        UINT64 number = 0;
        switch(lengthBits) {
          case 0:
          case 32: // l
          default:
            number = va_arg(args, UINT32);
          break;
          
          case 8:
            // hh
            number = (UINT8)va_arg(args, int);
          break;
          case 16:
            number = (UINT16)va_arg(args, int);
          break;
          case 64:
            //ll
            number = va_arg(args, UINT64);
        }
        printNum(number, base, signedNum, precision);
      }
    } else {
      // Not formatted, print next char
      cstr[0] = fmt[i];
      printfCout->OutputString(printfCout, cstr);
    }
  }
end:
  va_end(args);
  return result;
} 

// =================
// printf
// =================
bool printf(CHAR16 *fmt, ...) {
  bool result = true;
  va_list args;
  va_start(args, fmt);
  result = vprintf(fmt, args);
  va_end(args);
  return result;
}

// =================
// Print error message and get a key from user, so they can acknowledge the error and it doesnt go on immediately
// =================
bool error(char *file, int line, const char *func, EFI_STATUS status, CHAR16 *fmt, ...) {
  printf(u"\r\nERROR: FILE %s, LINE %d, FUNCTION %s\r\n", file, line, func);

  // Print error code & string if applicable
  if(status > 0 && status - TOP_BIT < MAX_EFI_ERROR) printf(u"\r\nSTATUS: %x (%s)\r\n", status, EFI_ERROR_STRINGS[status - TOP_BIT]);
  
  printfCout = cerr;
  va_list args;
  va_start(args, fmt);
  bool result = printf(fmt, args); // Printf the error message to stderr

  printfCout = cout; // Reset Priintf() to stdout

  getKey(); // User will respond with input before going on
  va_end(args);
  return result;
}

#define error(...) error(__FILE__, __LINE__, __func__, __VA_ARGS__);

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
