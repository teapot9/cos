#include "entry_efi.h"

#include <errno.h>
#include <stddef.h>
#include <stdnoreturn.h>

#include <sched.h>
#include <task.h>
#include "../../../kernel/task.h"
#include <cpu.h>
#include <asm/asm.h>
#include <console.h>
#include <firmware/efistub.h>
#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/boot.h>
#include <firmware/efiapi/loaded_image.h>
#include <firmware/efiapi/system_table.h>
#include <firmware/efiapi/console.h>
#include <fonts.h>
#include <alloc.h>
#include <mm/vmm.h>
#include <print.h>
#include <debug.h>
#include <string.h>
#include <unicode.h>
#include <video/fb.h>
#include <asm/io.h>
#include <platform_setup.h>
#include <setup.h>
#include <cmdline.h>
#include <panic.h>
#include <mm/memmap.h>
#include "../kernel/idt.h"
#include <elf.h>
#include "../mm/debug.h"
#include <kconfig.h>

__attribute__ ((constructor)) void foo(void)
{
	int a = 0;
}

noreturn void entry_efi_s2(const struct entry_efi_data * info)
{
	int err;
	kmm_init();

#if IS_ENABLED(CONFIG_SERIAL_EARLY_DEBUG)
	/* Serial debug */
	serial_init();
	pr_info("Early init: serial\n", 0);
#endif

	/* Create GDT & IDT */
	err = cpu_reg(NULL);
	if (err)
		panic("Failed CPU0 initialization, errno = %d", err);
	pr_info("Early init: CPU0\n", 0);

	/* EFI stub */
	err = efistub_init(info->efistub);
	if (err)
		pr_crit("failed to initialize EFI stub, errno = %d\n", err);
	else
		pr_info("Early init: EFI stub\n", 0);

	/* pmm */
	memmap_print(info->pmemmap, "pmemmap");
	pmm_init(info->pmemmap);
	pr_info("Early init: PMM\n", 0);

	/* vmm */
	err = vmm_init();
	if (err)
		panic("could not initialize VMM, errno = %d\n", err);
#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("===== start of dump current VMM =====\n", 0);
	dump_vmm_current();
	pr_debug("===== end of dump current VMM =====\n", 0);
	pr_debug("===== start of dump new VMM =====\n", 0);
	dump_vmm((void *) &kernel_cr3);
	pr_debug("===== end of dump new VMM =====\n", 0);
#endif
	pr_info("Early init: VMM initialized\n", 0);

	/* Clear bootloader memory: TODO */
#if 0
	struct memlist_elt * cur;
	list_foreach (cur, info->bootmem->l.first) {
		err = memmap_update(&memmap, cur->addr, cur->size,
		                    MEMORY_TYPE_FREE, 0);
		if (err)
			pr_warn("cannot free bootloader memory at "
			        "%p (%zu bytes), errno = %d\n",
			        cur->addr, cur->size, err);
	}
#endif

#if IS_BUILTIN(CONFIG_FB_EFIGOP)
	/* Save GOP data */
	struct gop * gop = gop_save(info->gop);
#endif

	/* cmdline */
	kernel_cmdline = strdup(info->cmdline);
	if (kernel_cmdline == NULL)
		panic("failed to get cmdline, errno = %d", err);
	pr_info("Early init: cmdline: %s\n", kernel_cmdline);

	/* load vmm */
	vmm_enable_paging();
	pr_info("Early init: VMM loaded\n", 0);

	/* Create process 0 */
	err = process_new(NULL, NULL);
	if (err < 0)
		panic("could not create kernel process PID0\n");
	if (err > 0)
		panic("invalid process created: PID0 != PID%d\n", err);

	/* Thread 0 */
	int main_pid = kthread_new(kernel_main);
	if (main_pid > 0)
		main_pid = -EINVAL;
	if (main_pid)
		panic("Failed to create kernel process, errno = %d",
		      main_pid);

#if IS_BUILTIN(CONFIG_FB_EFIGOP)
	/* EFI GOP */
	err = gop_init(gop);
	if (err)
		pr_err("failed to initialize EFI GOP, errno = %d\n", err);
	else
		pr_info("Early init: EFI GOP\n", 0);
#endif

	/* Start scheduling + trampoline */
	sched_enable();
	cpu_start();
}
