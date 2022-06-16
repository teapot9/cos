#ifndef LIB_BINARY_ELF_H
#define LIB_BINARY_ELF_H

#include <stdint.h>
#include <assert.h>

#include <cpp.h>

typedef void * Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

#define EI_MAG {0x7F, 'E', 'L', 'F'}
#define EV_CURRENT 1

enum ei_class {
	ELFCLASS32 = 1, // 32-bit objects
	ELFCLASS64 = 2, // 64-bit objects
};

enum ei_data {
	ELFDATA2LSB = 1, // Object file data structures are little endian
	ELFDATA2MSB = 2, // Object file data structures are big endian
};

enum ei_osabi {
	ELFOSABI_SYSV = 0, // System V ABI
	ELFOSABI_HPUX = 1, // HP-UX operating system
	ELFOSABI_STANDALONE = 255, // Standalone (embedded) application
};

struct e_ident {
	uint8_t ei_mag[4]; // File identification
	uint8_t ei_class; // File class
	uint8_t ei_data; // Data encoding
	uint8_t ei_version; // File version
	uint8_t ei_osabi; // OS/ABI identification
	uint8_t ei_abiversion; // ABI version
	uint8_t _ei_pad[7];
};
static_assert(sizeof(struct e_ident) == 16, "struct e_ident must be 16 bytes");

enum e_type {
	ET_NONE = 0, // No file type
	ET_REL = 1, // Relocatable object type
	ET_EXEC = 2, // Executable file
	ET_DYN = 3, // Shared object file
	ET_CORE = 4, // Core file
	ET_LOOS = 0xFE00, // Environment-specific use
	ET_HIOS = 0xFEFF,
	ET_LOPROC = 0xFF00, // Processor-specific use
	ET_HIPROC = 0xFFFF,
};

enum e_machine {
	EM_NONE = 0, // No machine
	EM_M32 = 1, // AT&T WE 32100
	EM_SPARC = 2, // SPARC
	EM_386 = 3, // Intel 80386
	EM_68K = 4, // Motorola 68000
	EM_88K = 5, // Motorola 88000
	EM_860 = 7, // Intel 80860
	EM_MIPS = 8, // MIPS I Architecture
	EM_S370 = 9, // IBM System/370 Processor
	EM_MIPS_RS3_LE = 10, // MIPS RS3000 Little-endian
	EM_PARISC = 15, // Hewlett-Packard PA-RISC
	EM_VPP500 = 17, // Fujitsu VPP500
	EM_SPARC32PLUS = 18, // Enhanced instruction set SPARC
	EM_960 = 19, // Intel 80960
	EM_PPC = 20, // PowerPC
	EM_PPC64 = 21, // 64-bit PowerPC
	EM_S390 = 22, // IBM System/390 Processor
	EM_V800 = 36, // NEC V800
	EM_FR20 = 37, // Fujitsu FR20
	EM_RH32 = 38, // TRW RH-32
	EM_RCE = 39, // Motorola RCE
	EM_ARM = 40, // Advanced RISC Machines ARM
	EM_ALPHA = 41, // Digital Alpha
	EM_SH = 42, // Hitachi SH
	EM_SPARCV9 = 43, // SPARC Version 9
	EM_TRICORE = 44, // Siemens TriCore embedded processor
	EM_ARC = 45, // Argonaut RISC Core, Argonaut Technologies Inc.
	EM_H8_300 = 46, // Hitachi H8/300
	EM_H8_300H = 47, // Hitachi H8/300H
	EM_H8S = 48, // Hitachi H8S
	EM_H8_500 = 49, // Hitachi H8/500
	EM_IA_64 = 50, // Intel IA-64 processor architecture
	EM_MIPS_X = 51, // Stanford MIPS-X
	EM_COLDFIRE = 52, // Motorola ColdFire
	EM_68HC12 = 53, // Motorola M68HC12
	EM_MMA = 54, // Fujitsu MMA Multimedia Accelerator
	EM_PCP = 55, // Siemens PCP
	EM_NCPU = 56, // Sony nCPU embedded RISC processor
	EM_NDR1 = 57, // Denso NDR1 microprocessor
	EM_STARCORE = 58, // Motorola Star*Core processor
	EM_ME16 = 59, // Toyota ME16 processor
	EM_ST100 = 60, // STMicroelectronics ST100 processor
	EM_TINYJ = 61, // Advanced Logic Corp. TinyJ embedded processor family
	EM_X86_64 = 62, // AMD x86-64 architecture
	EM_PDSP = 63, // Sony DSP Processor
	EM_PDP10 = 64, // Digital Equipment Corp. PDP-10
	EM_PDP11 = 65, // Digital Equipment Corp. PDP-11
	EM_FX66 = 66, // Siemens FX66 microcontroller
	EM_ST9PLUS = 67, // STMicroelectronics ST9+ 8/16 bit microcontroller
	EM_ST7 = 68, // STMicroelectronics ST7 8-bit microcontroller
	EM_68HC16 = 69, // Motorola MC68HC16 Microcontroller
	EM_68HC11 = 70, // Motorola MC68HC11 Microcontroller
	EM_68HC08 = 71, // Motorola MC68HC08 Microcontroller
	EM_68HC05 = 72, // Motorola MC68HC05 Microcontroller
	EM_SVX = 73, // Silicon Graphics SVx
	EM_ST19 = 74, // STMicroelectronics ST19 8-bit microcontroller
	EM_VAX = 75, // Digital VAX
	EM_CRIS = 76, // Axis Communications 32-bit embedded processor
	EM_JAVELIN = 77, // Infineon Technologies 32-bit embedded processor
	EM_FIREPATH = 78, // Element 14 64-bit DSP Processor
	EM_ZSP = 79, // LSI Logic 16-bit DSP Processor
	EM_MMIX = 80, // Donald Knuth's educational 64-bit processor
	EM_HUANY = 81, // Harvard University machine-independent object files
	EM_PRISM = 82, // SiTera Prism
	EM_AVR = 83, // Atmel AVR 8-bit microcontroller
	EM_FR30 = 84, // Fujitsu FR30
	EM_D10V = 85, // Mitsubishi D10V
	EM_D30V = 86, // Mitsubishi D30V
	EM_V850 = 87, // NEC v850
	EM_M32R = 88, // Mitsubishi M32R
	EM_MN10300 = 89, // Matsushita MN10300
	EM_MN10200 = 90, // Matsushita MN10200
	EM_PJ = 91, // picoJava
	EM_OPENRISC = 92, // OpenRISC 32-bit embedded processor
	EM_ARC_A5 = 93, // ARC Cores Tangent-A5
	EM_XTENSA = 94, // Tensilica Xtensa Architecture
	EM_VIDEOCORE = 95, // Alphamosaic VideoCore processor
	EM_TMM_GPP = 96, // Thompson Multimedia General Purpose Processor
	EM_NS32K = 97, // National Semiconductor 32000 series
	EM_TPC = 98, // Tenor Network TPC processor
	EM_SNP1K = 99, // Trebia SNP 1000 processor
	EM_ST200 = 100, // STMicroelectronics (www.st.com) ST200 microcontroller
};

typedef struct
{
	struct e_ident e_ident; // ELF identification
	Elf64_Half e_type; // Object file type
	Elf64_Half e_machine; // Machine type
	Elf64_Word e_version; // Object file version
	Elf64_Addr e_entry; // Entry point address
	Elf64_Off e_phoff; // Program header offset
	Elf64_Off e_shoff; // Section header offset
	Elf64_Word e_flags; // Processor-specific flags
	Elf64_Half e_ehsize; // ELF header size
	Elf64_Half e_phentsize; // Size of program header entry
	Elf64_Half e_phnum; // Number of program header entries
	Elf64_Half e_shentsize; // Size of section header entry
	Elf64_Half e_shnum; // Number of section header entries
	Elf64_Half e_shstrndx; // Section name string table index
} Elf64_Ehdr;

enum sh_type {
	SHT_NULL = 0, // Marks an unused section header
	SHT_PROGBITS = 1, // Contains information defined by the program
	SHT_SYMTAB = 2, // Contains a linker symbol table
	SHT_STRTAB = 3, // Contains a string table
	SHT_RELA = 4, // Contains “Rela” type relocation entries
	SHT_HASH = 5, // Contains a symbol hash table
	SHT_DYNAMIC = 6, // Contains dynamic linking tables
	SHT_NOTE = 7, // Contains note information
	SHT_NOBITS = 8, // Contains uninitialized space;
	                // does not occupy any space in the file
	SHT_REL = 9, // Contains “Rel” type relocation entries
	SHT_SHLIB = 10, // Reserved
	SHT_DYNSYM = 11, // Contains a dynamic loader symbol table
	SHT_LOOS = 0x60000000, // Environment-specific use
	SHT_HIOS = 0x6FFFFFFF,
	SHT_LOPROC = 0x70000000, // Processor-specific use
	SHT_HIPROC = 0x7FFFFFFF,
};

enum sh_flags {
	SHF_WRITE = 0x1, // Section contains writable data
	SHF_ALLOC = 0x2, // Section is allocated in memory image of program
	SHF_EXECINSTR = 0x4, // Section contains executable instructions
};

typedef struct
{
	Elf64_Word sh_name; // Section name
	Elf64_Word sh_type; // Section type
	Elf64_Xword sh_flags; // Section attributes
	Elf64_Addr sh_addr; // Virtual address in memory
	Elf64_Off sh_offset; // Offset in file
	Elf64_Xword sh_size; // Size of section
	Elf64_Word sh_link; // Link to other section
	Elf64_Word sh_info; // Miscellaneous information
	Elf64_Xword sh_addralign; // Address alignment boundary
	Elf64_Xword sh_entsize; // Size of entries, if section has table
} Elf64_Shdr;

enum st_info_binding {
	STB_LOCAL = 0, // Not visible outside the object file
	STB_GLOBAL = 1, // Global symbol, visible to all object files
	STB_WEAK = 2, // Global scope, but with lower precedence than global symbols
	STB_LOOS = 10, // Environment-specific use
	STB_HIOS = 12,
	STB_LOPROC = 13, // Processor-specific use
	STB_HIPROC = 15,
};

enum st_info_type {
	STT_NOTYPE = 0, // No type specified (e.g., an absolute symbol)
	STT_OBJECT = 1, // Data object
	STT_FUNC = 2, // Function entry point
	STT_SECTION = 3, // Symbol is associated with a section
	STT_FILE = 4, // Source file associated with the object file
	STT_LOOS = 10, // Environment-specific use
	STT_HIOS = 12,
	STT_LOPROC = 13, // Processor-specific use
	STT_HIPROC = 15,
};

struct _packed_ st_info {
	unsigned type : 4;
	unsigned binding : 4;
};
static_assert(sizeof(struct st_info) == 1, "struct st_info must be 1 byte");

typedef struct
{
	Elf64_Word st_name; // Symbol name
	struct st_info st_info; // Type and Binding attributes
	unsigned char st_other; // Reserved
	Elf64_Half st_shndx; // Section table index
	Elf64_Addr st_value; // Symbol value
	Elf64_Xword st_size; // Size of object (e.g., common)
} Elf64_Sym;

typedef struct
{
	Elf64_Addr r_offset; // Address of reference
	Elf64_Xword r_info; // Symbol index and type of relocation
} Elf64_Rel;

typedef struct
{
	Elf64_Addr r_offset; // Address of reference
	Elf64_Xword r_info; // Symbol index and type of relocation
	Elf64_Sxword r_addend; // Constant part of expression
} Elf64_Rela;

enum p_type {
	PT_NULL = 0, // Unused entry
	PT_LOAD = 1, // Loadable segment
	PT_DYNAMIC = 2, // Dynamic linking tables
	PT_INTERP = 3, // Program interpreter path name
	PT_NOTE = 4, // Note sections
	PT_SHLIB = 5, // Reserved
	PT_PHDR = 6, // Program header table
	PT_LOOS = 0x60000000, // Environment-specific use
	PT_HIOS = 0x6FFFFFFF,
	PT_LOPROC = 0x70000000, // Processor-specific use
	PT_HIPROC = 0x7FFFFFFF,
};

enum p_flags {
	PF_X = 0x1, // Execute permission
	PF_W = 0x2, // Write permission
	PF_R = 0x4, // Read permission
};

typedef struct
{
	Elf64_Word p_type; // Type of segment
	Elf64_Word p_flags; // Segment attributes
	Elf64_Off p_offset; // Offset in file
	Elf64_Addr p_vaddr; // Virtual address in memory
	Elf64_Addr p_paddr; // Reserved
	Elf64_Xword p_filesz; // Size of segment in file
	Elf64_Xword p_memsz; // Size of segment in memory
	Elf64_Xword p_align; // Alignment of segment
} Elf64_Phdr;

enum d_tag {
	DT_NULL = 0, // Marks the end of the dynamic array
	DT_NEEDED = 1, // The string table offset of the name of a
	               // needed library.
	DT_PLTRELSZ = 2, // Total size, in bytes, of the relocation entries
	                 // associated with the procedure linkage table.
	DT_PLTGOT = 3, // Contains an address associated with the linkage
	               // table. The specific meaning of this field
	               // is processor-dependent.
	DT_HASH = 4, // Address of the symbol hash table, described below.
	DT_STRTAB = 5, // Address of the dynamic string table.
	DT_SYMTAB = 6, // Address of the dynamic symbol table.
	DT_RELA = 7, // Address of a relocation table with Elf64_Rela entries.
	DT_RELASZ = 8, // Total size, in bytes, of the DT_RELA relocation table.
	DT_RELAENT = 9, // Size, in bytes, of each DT_RELA relocation entry.
	DT_STRSZ = 10, // Total size, in bytes, of the string table.
	DT_SYMENT = 11, // Size, in bytes, of each symbol table entry.
	DT_INIT = 12, // Address of the initialization function.
	DT_FINI = 13, // Address of the termination function.
	DT_SONAME = 14, // The string table offset of the name of this
	                // shared object.
	DT_RPATH = 15, // The string table offset of a shared library
	               // search path string.
	DT_SYMBOLIC = 16, // The presence of this dynamic table entry modifies
	                  // the symbol resolution algorithm for references
	                  // within the library. Symbols defined within the
	                  // library are used to resolve references before the
	                  // dynamic linker searches the usual search path.
	DT_REL = 17, // Address of a relocation table with Elf64_Rel entries.
	DT_RELSZ = 18, // Total size, in bytes, of the DT_REL relocation table.
	DT_RELENT = 19, // Size, in bytes, of each DT_REL relocation entry.
	DT_PLTREL = 20, // Type of relocation entry used for the procedure
	                // linkage table. The d_val member contains either
	                // DT_REL or DT_RELA.
	DT_DEBUG = 21, // Reserved for debugger use.
	DT_TEXTREL = 22, // The presence of this dynamic table entry signals
	                 // that the relocation table contains relocations
	                 // for a non-writable segment.
	DT_JMPREL = 23, // Address of the relocations associated with the
	                // procedure linkage table.
	DT_BIND_NOW = 24, // The presence of this dynamic table entry signals
	                  // that the dynamic loader should process all
	                  // relocations for this object before transferring
	                  // control to the program.
	DT_INIT_ARRAY = 25, // Pointer to an array of pointers to
	                    // initialization functions.
	DT_FINI_ARRAY = 26, // Pointer to an array of pointers to
	                    // termination functions.
	DT_INIT_ARRAYSZ = 27, // Size, in bytes, of the array of
	                      // initialization functions.
	DT_FINI_ARRAYSZ = 28, // Size, in bytes, of the array of
	                      // termination functions.
	DT_LOOS = 0x60000000, // a range of dynamic table tags that are
	                      // reserved for environment-specific use.
	DT_HIOS = 0x6FFFFFFF,
	DT_LOPROC = 0x70000000, // a range of dynamic table tags that are
	                        // reserved for processor-specific use.
	DT_HIPROC = 0x7FFFFFFF,
};

typedef struct
{
	Elf64_Sxword d_tag;
	union {
		Elf64_Xword d_val;
		Elf64_Addr d_ptr;
	} d_un;
} Elf64_Dyn;

#endif // LIB_BINARY_ELF_H
