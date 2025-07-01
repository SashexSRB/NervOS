#pragma once
#include "efi.h"
#include <stdarg.h>
#include <stdbool.h>
// Helper definitions that are not in efi.h or the UEFI Spec

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

// Example Kernel Parameters
typedef struct {
  MemoryMapInfo *mMap; // Get memory map to fill this out
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE gopMode;
} KernelParameters;

// -----------------
//  Global vars
// -----------------
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cout = NULL; // Console out
EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin = NULL; // Console out
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *cerr = NULL; // Console error
EFI_BOOT_SERVICES *bs; // Boot Services
EFI_RUNTIME_SERVICES *rs; // Runtime Services
EFI_HANDLE image = NULL; // Image handle
EFI_EVENT timerEvent = NULL; // Timer event for printing date/time

// =================
// Set global vars
// =================
void initGlbVars(EFI_HANDLE handle, EFI_SYSTEM_TABLE *systable) {
  cout = systable->ConOut;
  cin = systable->ConIn;
  //cerr = systable->StdErr; // TODO: Research why it doesnt work in emulation
  cerr = cout; // Temporary
  bs = systable->BootServices;
  rs = systable->RuntimeServices;
  image = handle;
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
// isDigit
// =================
BOOLEAN isDigit(char c) {
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
// Print an number to stderr
// =================
BOOLEAN eprintNum(UINTN number, UINT8 base, BOOLEAN isSigned) {
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

  switch(base) {
    case 2:
      // binary
      buffer[i++] = u'b';
      buffer[i++] = u'0';
      break;
    case 8:
      // octal
      buffer[i++] = u'o';
      buffer[i++] = u'0';
      break;
    case 10: 
      // decimal
      if (negative) buffer[i++] = u'-';
      break;
    case 16:
      // hexadecimal
      buffer[i++] = u'x';
      buffer[i++] = u'0';
      break;
    default:
      // Maybe invalid base, but we'll go with it (no processing)
      break;
  }
  
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
  cerr->OutputString(cerr, buffer);

  return TRUE;
}

// =================
// Print an number to stdout
// =================
BOOLEAN printNum(UINTN number, UINT8 base, BOOLEAN isSigned) {
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

  switch(base) {
    case 2:
      // binary
      buffer[i++] = u'b';
      buffer[i++] = u'0';
      break;
    case 8:
      // octal
      buffer[i++] = u'o';
      buffer[i++] = u'0';
      break;
    case 10: 
      // decimal
      if (negative) buffer[i++] = u'-';
      break;
    case 16:
      // hexadecimal
      buffer[i++] = u'x';
      buffer[i++] = u'0';
      break;
    default:
      // Maybe invalid base, but we'll go with it (no processing)
      break;
  }
  
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
  cout->OutputString(cout, buffer);

  return TRUE;
}

// =================
// Print formatted strings to stderr
// =================
BOOLEAN eprintf(CHAR16 *fmt, va_list args) {
  BOOLEAN result = TRUE;
  CHAR16 cstr[2]; // place init this with memset and use = {} initializer
  
  //Initialize buffer
  cstr[0] = u'\0', cstr[1] = u'\0';

  // Print formatted string values
  for (UINTN i=0 ; fmt[i] != u'\0'; i++) {
    if (fmt[i] == u'%') {
      i++;
      // Grab next arg type from input args and print it
      switch(fmt[i]) {
        case u'c': {
          cstr[0] = va_arg(args, int);
          cerr->OutputString(cerr, cstr);
        }
        break;
        case u's': {
          //printf("%s", string) - Print CHAR16 string
          CHAR16 *string = va_arg(args, CHAR16 *);
          cerr->OutputString(cerr, string);
        }
        break;
        case u'b': {
          UINTN number = va_arg(args, UINTN);
          eprintNum(number, 2, FALSE);
        }
        break;
        case u'o': {
          UINTN number = va_arg(args, UINTN);
          eprintNum(number, 8, FALSE);
        }
        break;
        case u'd': {
          //printf("%d", number_int32) - Print INT32
          INT32 number = va_arg(args, INT32);
          eprintNum(number, 10, TRUE);
        }
        break;
        case u'x': {
          //printf("%d", number_int32) - Print INT32
          UINTN number = va_arg(args, UINTN);
          eprintNum(number, 16, FALSE);
        }
        break;
        case u'u': {
          UINT32 number = va_arg(args, UINT32);
          eprintNum(number, 10, FALSE);
        }
        break;
        default:
          cerr->OutputString(cerr, u"Invalid format specifier: %");
          cstr[0] = fmt[i];
          cerr->OutputString(cerr, cstr);
          cerr->OutputString(cerr, u"\r\n");
          result = FALSE;
          goto end;
        break;
      }
    } else {
      // Not formatted, print next char
      cstr[0] = fmt[i];
      cerr->OutputString(cerr, cstr);
    }
  }
end:
  return result;
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
// Print formatted strings to stdout
// =================
BOOLEAN printf(CHAR16 *fmt, ...) {
  BOOLEAN result = TRUE;
  CHAR16 cstr[2]; // place init this with memset and use = {} initializer
  va_list args;
  va_start(args, fmt);
  
  //Initialize buffer
  cstr[0] = u'\0', cstr[1] = u'\0';

  // Print formatted string values
  for (UINTN i=0 ; fmt[i] != u'\0'; i++) {
    if (fmt[i] == u'%') {
      i++;
      // Grab next arg type from input args and print it
      switch(fmt[i]) {
        case u'c': {
          cstr[0] = va_arg(args, int);
          cout->OutputString(cout, cstr);
        }
        break;
        case u's': {
          CHAR16 *string = va_arg(args, CHAR16*);
          cout->OutputString(cout, string);
        }
        break;
        case u'b': {
          UINTN number = va_arg(args, UINTN);
          printNum(number, 2, FALSE);
        }
        break;
        case u'o': {
          UINTN number = va_arg(args, UINTN);
          printNum(number, 8, FALSE);
        }
        break;
        case u'd': {
          INT32 number = va_arg(args, INT32);
          printNum(number, 10, TRUE);
        }
        break;
        case u'x': {
          UINTN number = va_arg(args, UINTN);
          printNum(number, 16, FALSE);
        }
        break;
        case u'u': {
          UINT32 number = va_arg(args, UINT32);
          printNum(number, 10, FALSE);
        }
        break;
        default:
          cout->OutputString(cout, u"Invalid format specifier: %");
          cstr[0] = fmt[i];
          cout->OutputString(cout, cstr);
          cout->OutputString(cout, u"\r\n");
          result = FALSE;
          goto end;
        break;
      }
    } else {
      // Not formatted, print next char
      cstr[0] = fmt[i];
      cout->OutputString(cout, cstr);
    }
  }
end:
  va_end(args);
  return result;
} 


// =================
// Print error message and get a key from user, so they can acknowledge the error and it doesnt go on immediately
// =================
BOOLEAN error(CHAR16 *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  BOOLEAN result = eprintf(fmt, args); // Printf the error message to stderr

  getKey(); // User will respond with input before going on

  va_end(args);
  return result;
}
