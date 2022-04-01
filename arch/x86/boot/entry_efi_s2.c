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
#include <mm.h>
#include <mm/early.h>
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
#include <mm/vmemmap.h>
#include "../kernel/idt.h"

noreturn void entry_efi_s2(const struct entry_efi_data * info)
{
	int err;

#ifdef CONFIG_DEBUG
	//bool tmp = false;
	//while (!tmp);
#endif

#ifdef CONFIG_SERIAL_EARLY_DEBUG
	/* Serial debug */
	serial_init();
#endif

	/* Create GDT & IDT */
	cpu_reg(NULL);

	/* Create process 0 */
	err = process_new(NULL, NULL);

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
	vmemmap_print(info->vmemmap, "vmemmap");
	vmm_init(info->vmemmap);
	pr_info("Early init: VMM\n", 0);
	int a = 0;
	int b = 2333/a;

	/* Clear bootloader memory */
	struct memlist_elt * cur;
	list_foreach (cur, info->bootmem->l.first) {
		err = memmap_update(&memmap, cur->addr, cur->size,
		                    MEMORY_TYPE_FREE, 0);
		if (err)
			pr_warn("cannot free bootloader memory at "
			        "%p (%zu bytes), errno = %d\n",
			        cur->addr, cur->size, err);
	}

	/* EFI GOP */
	err = gop_init(info->gop);
	if (err)
		pr_err("failed to initialize EFI GOP, errno = %d\n", err);
	else
		pr_info("Early init: EFI GOP\n", 0);

	/* cmdline */
	kernel_cmdline = info->cmdline;
	if (err)
		panic("failed to get cmdline, errno = %d", err);
	pr_info("Early init: cmdline: %s\n", kernel_cmdline);

	/* Thread 0 */
	int main_pid = process_new(NULL, kernel_main);
	if (main_pid > 0)
		main_pid = -EINVAL;
	if (main_pid)
		panic("Failed to create kernel process, errno = %d",
		      main_pid);

	/* CPU 0 */
	err = cpu_reg(NULL);
	if (err)
		panic("Failed CPU0 initialization, errno = %d", err);
	pr_info("Early init: CPU0\n", 0);

	/* Start scheduling + trampoline */
	sched_enable();
	cpu_start();
}
