#ifndef ASM_H
#define ASM_H

/**
 * @brief Assembly macro helpers
 * asm(): Inline assembly
 * intel(): Wrapper to use intel assembly, independently of the default
 * assembly language
 * att(): Wrapper to use AT&T assembly, independently of the default assembly
 * language
 */
#ifndef _DEFAULT_ASM
# if defined(CONFIG_ASM_DEFAULT_INTEL)
#  define _DEFAULT_ASM ".intel_syntax noprefix\n"
# elif defined(CONFIG_ASM_DEFAULT_ATT)
#  define _DEFAULT_ASM ".att_syntax\n"
# elif defined(CONFIG_ASM_DEFAULT_CUSTOM)
#  define _DEFAULT_ASM CONFIG_ASM_DEFAULT_CUSTOM
# else
#  error CONFIG_ASM_DEFAULT is not defined
# endif
#endif
#define asm __asm__
#define intel(code) ".intel_syntax noprefix\n" code _DEFAULT_ASM
#define att(code) ".att_syntax\n" code _DEFAULT_ASM

#endif // ASM_H
