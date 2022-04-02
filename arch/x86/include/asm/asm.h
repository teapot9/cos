#ifndef ASM_H
#define ASM_H

#include <kconfig.h>

/**
 * @brief Assembly macro helpers
 * asm(): Inline assembly
 * intel(): Wrapper to use intel assembly, independently of the default
 * assembly language
 * att(): Wrapper to use AT&T assembly, independently of the default assembly
 * language
 *
 * Usage: `asm(intel("mov bx, dx\n") : "=b" (out) : "d" (in));`
 */
#if IS_ENABLED(CONFIG_ASM_DEFAULT_INTEL)
# define _DEFAULT_ASM ".intel_syntax noprefix\n"
#elif IS_ENABLED(CONFIG_ASM_DEFAULT_ATT)
# define _DEFAULT_ASM ".att_syntax\n"
#else
# error No default assembler selected
#endif
#define asm __asm__
#define intel(code) ".intel_syntax noprefix\n" code _DEFAULT_ASM
#define att(code) ".att_syntax\n" code _DEFAULT_ASM

#endif // ASM_H
