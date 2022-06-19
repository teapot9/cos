/**
 * @file setup.h
 * @brief Early initialization
 */

#ifndef __SETUP_H
#define __SETUP_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdnoreturn.h>

#include <kconfig.h>

struct memlist;
struct memmap;

/**
 * @brief Kernel architecture-independant entry point
 */
void kernel_main(void);

/**
 * @brief Call early initialization functions
 */
void kernel_initcalls_early(void);

/**
 * @brief Call initialization functions
 */
void kernel_initcalls(void);

/**
 * @brief Initialize physical memory manager
 * @param newmap Kernel physical memory map
 * @return errno
 */
int pmm_init(struct memmap * newmap);

#if IS_ENABLED(CONFIG_SERIAL_EARLY_DEBUG)
/**
 * @brief Initialize serial devices
 * @return errno
 */
int serial_init(void);
#endif

/**
 * @brief Initialize kernel process (PID 0)
 * @return errno
 */
int process_pid0(void);

/**
 * @brief Build effective kernel command-line
 * @param firmware_cmdline Callback to get cmdline from firmware, the string
 * 	will be freed with `kfree()`
 * @return errno
 *
 * This concatenate the built-in cmdline with the firmware cmdline
 * and make it available through the `kernel_cmdline` symbol.
 */
int cmdline_init(const char * (* firmware_cmdline)(void));

/**
 * @brief Kernel memory manager (heap) initialization
 */
void kmm_init(void);

/**
 * @brief Initialize virtual memory manager
 * @return errno
 */
int vmm_init(void);

/**
 * @brief Enable paging
 */
void vmm_enable_paging(void);

#ifdef __cplusplus
}
#endif
#endif // __SETUP_H
