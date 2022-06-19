/**
 * @file printf.h
 * @brief String formatting functions
 */

#ifndef __PRINTF_H
#define __PRINTF_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdarg.h>

/* String manipulation */

/**
 * @brief Format a string to a allocated buffer
 * @param dst Where to store the pointer to the allocated buffer
 * @param fmt Format to use
 * @param ... Arguments to use for formatting
 * @return Number of characters written, excluding NUL
 */
size_t asprintf(char ** dst, const char * fmt, ...);

/**
 * @brief Format a string to a allocated buffer
 * @param dst Where to store the pointer to the allocated buffer
 * @param fmt Format to use
 * @param ap Arguments to use for formatting
 * @return Number of characters written, excluding NUL
 */
size_t vasprintf(char ** dst, const char * fmt, va_list ap);

/**
 * @brief Format a string to a buffer
 * @param dst Buffer to write into
 * @param fmt Format to use
 * @param ... Arguments to use for formatting
 * @return Number of characters written, excluding NUL
 */
size_t sprintf(char * dst, const char * fmt, ...);

/**
 * @brief Format a string to a buffer
 * @param dst Buffer to write into
 * @param len Write at most `len` characters, including NUL
 * @param fmt Format to use
 * @param ... Arguments to use for formatting
 * @return Number of characters written, excluding NUL
 */
size_t snprintf(char * dst, size_t len, const char * fmt, ...);

/**
 * @brief Format a string to a buffer
 * @param dst Buffer to write into
 * @param fmt Format to use
 * @param ap Arguments to use for formatting
 * @return Number of characters written, excluding NUL
 */
size_t vsprintf(char * dst, const char * fmt, va_list ap);

/**
 * @brief Format a string to a buffer
 * @param dst Buffer to write into
 * @param len Write at most `len` characters, including NUL
 * @param fmt Format to use
 * @param ap Arguments to use for formatting
 * @return Number of characters written, excluding NUL
 */
size_t vsnprintf(char * dst, size_t len, const char * fmt, va_list ap);

#ifdef __cplusplus
}
#endif
#endif // __PRINTF_H
