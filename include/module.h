/**
 * @file module.h
 * @brief Kernel module functions
 */

#ifndef __MODULE_H
#define __MODULE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>

#include <asm/asm.h>
#include <cpp.h>

typedef int (* initcall_t)(void);
typedef int initcall_entry_t;
typedef void (* constructor_t)(void);

/**
 * @brief Define a module entry point
 * @param name Name of the module entry point
 * @param fn Entry point of the module (initcall_t)
 * @param phase Phase when the module should be initialized
 *
 * Valid phases:
 *  - early
 *  - core
 *  - device
 *  - misc
 *  - noauto
 *
 * Actions:
 *  - Declare publicly visible function
 *  - Make this function addressable
 *  - Create a pointer to this function in .initcall.* section
 *  - Check typing
 */
#define module_init(fn, phase) \
	int _##fn##_stub(void); \
	int _##fn##_stub(void) {return fn();} \
	static void * \
		__attribute__((__section__(".discard.addressable"))) \
		__attribute__((__used__)) \
		_##fn##_stub_ptr \
		= (void *) &(_##fn##_stub); \
	asm(intel( \
		".section \".initcall." #phase "\", \"a\"\n" \
		"_" #fn "_initcall:\n" \
		".long _" #fn "_stub - .\n" \
		".previous\n" \
	)); \
	static_assert(same_type(initcall_t, &fn), \
	              "Module init function has wrong type")

/**
 * @brief Kernel module informations
 * @var name Module name
 */
struct module {
	const char * const name;
};

#ifdef __cplusplus
}
#endif
#endif // __MODULE_H
