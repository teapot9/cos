/**
 * @file printk.h
 * @brief Kernel logging
 */

#ifndef PRINTK_H
#define PRINTK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdarg.h>

#include <cpp.h>

/* Log levels */

#define KERN_EMERG "<\000>"
#define KERN_ALERT "<\001>"
#define KERN_CRIT "<\002>"
#define KERN_ERROR "<\003>"
#define KERN_WARNING "<\004>"
#define KERN_NOTICE "<\005>"
#define KERN_INFO "<\006>"
#define KERN_DEBUG "<\007>"

/* Helper macros */

#ifndef pr_fmt
/**
 * @brief Called by pr_*(), allow string processing before call to printk
 * @param fmt Format string used for pr_*()
 *
 * Example: `#define pr_fmt(fmt) "print: " fmt` will add "print: " before
 * each line logged with a pr_*() helper.
 */
#define pr_fmt(fmt) fmt
#endif

/**
 * @brief Print emergency-level message
 * @param fmt Format string
 * @param ... Arguments matching format string
 */
#define pr_emerg(fmt, ...) printk(KERN_EMERG pr_fmt(fmt), __VA_ARGS__)

/**
 * @brief Print alert-level message
 * @param fmt Format string
 * @param ... Arguments matching format string
 */
#define pr_alert(fmt, ...) printk(KERN_EMERG pr_fmt(fmt), __VA_ARGS__)

/**
 * @brief Print critical-level message
 * @param fmt Format string
 * @param ... Arguments matching format string
 */
#define pr_crit(fmt, ...) printk(KERN_CRIT pr_fmt(fmt), __VA_ARGS__)

/**
 * @brief Print error-level message
 * @param fmt Format string
 * @param ... Arguments matching format string
 */
#define pr_err(fmt, ...) printk(KERN_ERROR pr_fmt(fmt), __VA_ARGS__)

/**
 * @brief Print warning-level message
 * @param fmt Format string
 * @param ... Arguments matching format string
 */
#define pr_warn(fmt, ...) printk(KERN_WARNING pr_fmt(fmt), __VA_ARGS__)

/**
 * @brief Print notice-level message
 * @param fmt Format string
 * @param ... Arguments matching format string
 */
#define pr_notice(fmt, ...) printk(KERN_NOTICE pr_fmt(fmt), __VA_ARGS__)

/**
 * @brief Print information-level message
 * @param fmt Format string
 * @param ... Arguments matching format string
 */
#define pr_info(fmt, ...) printk(KERN_INFO pr_fmt(fmt), __VA_ARGS__)

/**
 * @brief Print debug-level message
 * @param fmt Format string
 * @param ... Arguments matching format string
 */
#define pr_debug(fmt, ...) printk(KERN_DEBUG pr_fmt(fmt), __VA_ARGS__)

/* Kernel log functions */

/**
 * @brief Format and write a string to the console
 * @param fmt Format to use
 * @param ... Arguments to use for formatting
 * @return Number of characters written, excluding NUL
 */
size_t printk(const char * fmt, ...) ISR_AVAILABLE;

/**
 * @brief Format and write a string to the console
 * @param fmt Format to use
 * @param ap Arguments to use for formatting
 * @return Number of characters written, excluding NUL
 */
size_t vprintk(const char * fmt, va_list ap) ISR_AVAILABLE;

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

#ifdef __cplusplus
}
#endif
#endif // PRINTK_H
