#define pr_fmt(fmt) "acpi: " fmt

#include <firmware/acpi.h>
#include "acpi.h"

#include <errno.h>
#include <stddef.h>

#include <string.h>
#include <print.h>
#include <module.h>
#include <alloc.h>
#include <mm/early.h>
#include <mm/map.h>
#include <kconfig.h>

#if IS_ENABLED(CONFIG_64BIT)
# define UINT_PTR uint64_t
#else
# define UINT_PTR uint32_t
#endif

static void * phy_rsdp = NULL;
static struct memmap_list * acpi_map = NULL;
static bool is_init = false;

void * acpi_rsdp_phy(void)
{
	return phy_rsdp;
}

void acpi_register_rsdp(void * paddr)
{
	phy_rsdp = paddr;
	pr_debug("registered RSDP at %p\n", phy_rsdp);
}

static int map(void)
{
	struct memblock cur = {.addr = NULL, .size = 0};

	while (pmm_acpi_iter(&cur)) {
		void * vaddr = mmap(cur.addr, cur.size);
		if (vaddr == NULL) {
			pr_err("failed to map memory %p -> %p [%zuB]\n",
			       cur.addr, vaddr, cur.size);
			return -ENOMEM;
		}

		int err = memmap_add(&acpi_map, cur.addr, vaddr, cur.size);
		if (err) {
			pr_err("failed to save memory mapping "
			       "%p -> %p [%zuB]\n", cur.addr, vaddr, cur.size);
			return err;
		}
	}

	return 0;
}

static void * translate(void * ptr)
{
	return memmap_translate(&acpi_map, ptr);
}

static uint8_t csum_bytes(void * ptr, size_t len)
{
	uint8_t * uptr = ptr;
	uint8_t sum = 0;
	for (size_t i = 0; i < len; sum += uptr[i++]);
	return sum;
}

static bool table_csum(struct description_header * table)
{
	return !csum_bytes(table, table->length);
}

static int parse_rsdp(void)
{
	union rsdp * rsdp = acpi_rsdp();
	if (rsdp == NULL) {
		pr_err("rsdp: %p -> %p: invalid address\n", phy_rsdp, rsdp);
		return -EINVAL;
	}

	// Verify signature and checksums
	if (strncmp(rsdp->rsdp1.sig, ACPI_RSDP_SIG, 8)) {
		pr_err("rsdp: invalid signature\n", 0);
		return -EINVAL;
	}
	if (csum_bytes(&rsdp->rsdp1, sizeof(rsdp->rsdp1))) {
		pr_err("rsdp: invalid checksum\n", 0);
		return -EINVAL;
	}
	if (acpi_version() >= ACPI_VERSION_2
	    && csum_bytes(&rsdp->rsdp2, rsdp->rsdp2.length)) {
		pr_err("rsdp: invalid extended checksum\n", 0);
		return -EINVAL;
	}

	pr_debug("rsdp: ACPI %d available\n", acpi_version());
	return 0;
}

static int parse_tables(void)
{
	struct description_header * cur;
	size_t i = 0;
	while ((cur = acpi_get_table(i++)) != NULL) {
		pr_debug("found table %.4s v%d [%.6s:%.8s] (%.4s)\n",
			 cur->sig, cur->rev,
			 cur->oem_id, cur->oem_table_id, cur->creator_id);
	}
	return 0;
}

static int acpi_init(void)
{
	int err;
	if (is_init)
		return 0;
	pr_debug("init\n", 0);

	err = map();
	if (err)
		return err;

	err = parse_rsdp();
	if (err)
		return err;

	err = parse_tables();
	if (err)
		return err;

	is_init = true;
	return 0;
}
module_init(acpi_init, device);

union rsdp * acpi_rsdp(void)
{
	return translate(phy_rsdp);
}

enum acpi_version acpi_version(void)
{
	union rsdp * rsdp = acpi_rsdp();
	if (rsdp == NULL)
		return ACPI_VERSION_UNDEFINED;
	if (rsdp->rsdp1.rev >= 2)
		return ACPI_VERSION_2;
	else
		return ACPI_VERSION_1;
}

struct description_header * acpi_get_table(size_t index)
{
	union rsdp * rsdp = acpi_rsdp();
	if (rsdp == NULL)
		return NULL;

	if (acpi_version() >= ACPI_VERSION_2) {
		struct xsdt * xsdt =
			translate((void *) rsdp->rsdp2.phy_xsdt_ptr);
		if (xsdt != NULL)
			return translate(
				(void *) (UINT_PTR) xsdt->phy_entry_ptr[index]
			);
	}

	struct rsdt * rsdt =
		translate((void *) (UINT_PTR) rsdp->rsdp1.phy_rsdt_ptr);
	if (rsdt != NULL)
		return translate(
			(void *) (UINT_PTR) rsdt->phy_entry_ptr[index]
		);

	pr_err("sdt: table %zu does not exists\n", index);
	return NULL;
}
