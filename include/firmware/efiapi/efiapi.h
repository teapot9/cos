#ifndef __FIRMWARE_EFIAPI_EFIAPI_H
#define __FIRMWARE_EFIAPI_EFIAPI_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <kconfig.h>

#ifndef EFIABI
#define EFIABI __attribute__ ((ms_abi))
#endif

/* EFI base types */

#if IS_ENABLED(CONFIG_X86_64)
typedef uint64_t efi_uintn;
typedef int64_t efi_intn;
#elif IS_ENABLED(CONFIG_X86_32)
typedef uint32_t efi_uintn;
typedef int32_t efi_intn;
#else
#error Unsupported architecture
#endif

typedef uint8_t efi_bool;

/* EFI status */

#if IS_ENABLED(CONFIG_X86_64)
# define EFI_UINTN_HIGH_BIT (1UL << 63)
#elif IS_ENABLED(CONFIG_X86_32)
# define EFI_UINTN_HIGH_BIT (1U << 31)
#else
# error Architecture must be 32 or 64 bits
#endif

typedef efi_uintn efi_status_t;

#define EFI_SUCCESS (0)
#define EFI_LOAD_ERROR (1 | EFI_UINTN_HIGH_BIT)
#define EFI_INVALID_PARAMETER (2 | EFI_UINTN_HIGH_BIT)
#define EFI_UNSUPPORTED (3 | EFI_UINTN_HIGH_BIT)
#define EFI_BAD_BUFFER_SIZE (4 | EFI_UINTN_HIGH_BIT)
#define EFI_BUFFER_TOO_SMALL (5 | EFI_UINTN_HIGH_BIT)
#define EFI_NOT_READY (6 | EFI_UINTN_HIGH_BIT)
#define EFI_DEVICE_ERROR (7 | EFI_UINTN_HIGH_BIT)
#define EFI_WRITE_PROTECTED (8 | EFI_UINTN_HIGH_BIT)
#define EFI_OUT_OF_RESOURCES (9 | EFI_UINTN_HIGH_BIT)
#define EFI_VOLUME_CORRUPTED (10 | EFI_UINTN_HIGH_BIT)
#define EFI_VOLUME_FULL (11 | EFI_UINTN_HIGH_BIT)
#define EFI_NO_MEDI (12 | EFI_UINTN_HIGH_BIT)
#define EFI_MEDIA_CHANGED (13 | EFI_UINTN_HIGH_BIT)
#define EFI_NOT_FOUND (14 | EFI_UINTN_HIGH_BIT)
#define EFI_ACCESS_DENIED (15 | EFI_UINTN_HIGH_BIT)
#define EFI_NO_RESPONSE (16 | EFI_UINTN_HIGH_BIT)
#define EFI_NO_MAPPING (17 | EFI_UINTN_HIGH_BIT)
#define EFI_TIMEOUT (18 | EFI_UINTN_HIGH_BIT)
#define EFI_NOT_STARTED (19 | EFI_UINTN_HIGH_BIT)
#define EFI_ALREADY_STARTED (20 | EFI_UINTN_HIGH_BIT)
#define EFI_ABORTED (21 | EFI_UINTN_HIGH_BIT)
#define EFI_ICMP_ERROR (22 | EFI_UINTN_HIGH_BIT)
#define EFI_TFTP_ERROR (23 | EFI_UINTN_HIGH_BIT)
#define EFI_PROTOCOL_ERROR (24 | EFI_UINTN_HIGH_BIT)
#define EFI_INCOMPATIBLE_VERSION (25 | EFI_UINTN_HIGH_BIT)
#define EFI_SECURITY_VIOLATION (26 | EFI_UINTN_HIGH_BIT)
#define EFI_CRC_ERROR (27 | EFI_UINTN_HIGH_BIT)
#define EFI_END_OF_MEDIA (28 | EFI_UINTN_HIGH_BIT)
#define EFI_END_OF_FILE (29 | EFI_UINTN_HIGH_BIT)
#define EFI_INVALID_LANGUAGE (30 | EFI_UINTN_HIGH_BIT)
#define EFI_COMPROMISED_DATA (31 | EFI_UINTN_HIGH_BIT)
#define EFI_IP_ADDRESS_CONFLICT (32 | EFI_UINTN_HIGH_BIT)
#define EFI_HTTP_ERROR (33 | EFI_UINTN_HIGH_BIT)
#define EFI_WARN_UNKNOWN_GLYPH (1)
#define EFI_WARN_DELETE_FAILURE (2)
#define EFI_WARN_WRITE_FAILURE (3)
#define EFI_WARN_BUFFER_TOO_SMALL (4)
#define EFI_WARN_STALE_DATA (5)
#define EFI_WARN_FILE_SYSTEM (6)

#define EFI_ERROR(code) ((code) && (EFI_UINTN_HIGH_BIT) != 0)

/* Other EFI definitions */

typedef void * efi_handle_t;

typedef struct {
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t data4[8];
} efi_guid_t;

/* EFI table header */

typedef struct {
	uint64_t signature;
	uint32_t revision;
	uint32_t header_size;
	uint32_t crc32;
	uint32_t reserved;
} efi_table_header_t;

/* Memory definitions */

#define EFI_MEMORY_UC 0x0000000000000001
#define EFI_MEMORY_WC 0x0000000000000002
#define EFI_MEMORY_WT 0x0000000000000004
#define EFI_MEMORY_WB 0x0000000000000008
#define EFI_MEMORY_UCE 0x0000000000000010
#define EFI_MEMORY_WP 0x0000000000001000
#define EFI_MEMORY_RP 0x0000000000002000
#define EFI_MEMORY_XP 0x0000000000004000
#define EFI_MEMORY_NV 0x0000000000008000
#define EFI_MEMORY_MORE_RELIABLE 0x0000000000010000
#define EFI_MEMORY_RO 0x0000000000020000
#define EFI_MEMORY_RUNTIME 0x8000000000000000

typedef uint64_t efi_physical_address_t;

typedef uint64_t efi_virtual_address_t;

typedef enum {
	EFI_RESERVED_MEMORY_TYPE,
	EFI_LOADER_CODE,
	EFI_LOADER_DATA,
	EFI_BOOT_SERVICES_CODE,
	EFI_BOOT_SERVICES_DATA,
	EFI_RUNTIME_SERVICES_CODE,
	EFI_RUNTIME_SERVICES_DATA,
	EFI_CONVENTIONAL_MEMORY,
	EFI_UNUSABLE_MEMORY,
	EFI_ACPIRECLAIM_MEMORY,
	EFI_ACPIMEMORY_NVS,
	EFI_MEMORY_MAPPED_IO,
	EFI_MEMORY_MAPPED_IOPORT_SPACE,
	EFI_PAL_CODE,
	EFI_PERSISTENT_MEMORY,
	EFI_MAX_MEMORY_TYPE,
} efi_memory_type_t;

typedef struct {
	uint32_t type;
	efi_physical_address_t physical_start;
	efi_virtual_address_t virtual_start;
	uint64_t number_of_pages;
	uint64_t attribute;
} efi_memory_descriptor_t;

typedef struct {
	uint32_t version;
	uint32_t number_of_entries;
	uint32_t descriptor_size;
	uint32_t _reserved;
} efi_memory_attributes_table;

#ifdef __cplusplus
}
#endif
#endif // __FIRMWARE_EFIAPI_EFIAPI_H
