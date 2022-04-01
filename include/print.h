#ifndef PRINT_H
#define PRINT_H

#include <stddef.h>
#include <stdarg.h>

#define KERN_EMERG "<\000>"
#define KERN_ALERT "<\001>"
#define KERN_CRIT "<\002>"
#define KERN_ERROR "<\003>"
#define KERN_WARNING "<\004>"
#define KERN_NOTICE "<\005>"
#define KERN_INFO "<\006>"
#define KERN_DEBUG "<\007>"

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#define pr_emerg(fmt, ...) printk(KERN_EMERG pr_fmt(fmt), __VA_ARGS__)
#define pr_alert(fmt, ...) printk(KERN_EMERG pr_fmt(fmt), __VA_ARGS__)
#define pr_crit(fmt, ...) printk(KERN_CRIT pr_fmt(fmt), __VA_ARGS__)
#define pr_err(fmt, ...) printk(KERN_ERROR pr_fmt(fmt), __VA_ARGS__)
#define pr_warn(fmt, ...) printk(KERN_WARNING pr_fmt(fmt), __VA_ARGS__)
#define pr_notice(fmt, ...) printk(KERN_NOTICE pr_fmt(fmt), __VA_ARGS__)
#define pr_info(fmt, ...) printk(KERN_INFO pr_fmt(fmt), __VA_ARGS__)
#define pr_debug(fmt, ...) printk(KERN_DEBUG pr_fmt(fmt), __VA_ARGS__)

/**
 * @brief Format and write a string to the console
 * @param fmt Format to use
 * @param ... Arguments to use for formatting
 * @return Number of characters written, excluding NUL
 */
size_t printk(const char * fmt, ...);

/**
 * @brief Format and write a string to the console
 * @param fmt Format to use
 * @param ap Arguments to use for formatting
 * @return Number of characters written, excluding NUL
 */
size_t vprintk(const char * fmt, va_list ap);

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

/**
 * @brief Get the next kernel message
 * @param ptr Current kernel message, NULL to get the first message
 * @return Pointer to the next kernel message, NULL if there is no more
 */
const char * kmsg_next(const char * ptr);

/**
 * @brief Get the string part of the current kernel message
 * @param ptr Current kernel message
 * @return String of the message, NULL if ptr is invalid
 */
const char * kmsg_get_str(const char * ptr);

#endif // PRINT_H
