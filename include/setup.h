#ifndef SETUP_H
#define SETUP_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdnoreturn.h>

#include <kconfig.h>

struct memlist;
struct memmap;

void kernel_main(void);

void kernel_initcalls_early(void);

void kernel_initcalls(void);

int pmm_init(struct memmap * newmap);

#if IS_ENABLED(CONFIG_SERIAL_EARLY_DEBUG)
int serial_init(void);
#endif

int process_pid0(void);

/* Save cmdline (dynamically allocated buffer) in dst */
int cmdline_init(const char * (* firmware_cmdline)(void));

void kmm_init(void);

/* Compiler tests
 * These functions returns COMPILER_TEST_VALUE on success, and expects
 * COMPILER_TEST_VALUE as sole argument.
 * If error, failed test line is returned.
 */
#if IS_ENABLED(CONFIG_DEBUG)

#define COMPILER_TEST_VALUE -9999

int compiler_test_cxx(int value);

#endif // CONFIG_DEBUG

int vmm_init(void);
void vmm_enable_paging(void);

#ifdef __cplusplus
}
#endif
#endif // SETUP_H
