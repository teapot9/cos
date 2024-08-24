/**
 * @file kconfig.h
 * @brief Kernel configuration
 */

#ifndef __KCONFIG_H
#define __KCONFIG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "generated/autoconf.h"

#define __placeholder_1 0,
#define __second_arg(x, y, ...) y
#define _is_defined(x) __is_defined(x)
#define __is_defined(x) ___is_defined(__placeholder_##x)
#define ___is_defined(x) __second_arg(x 1, 0, 0)
#define _or(x, y) __or(x, y)
#define __or(x, y) ___or(__placeholder_##x, y)
#define ___or(x, y) __second_arg(x 1, y, 0)

/**
 * @brief Test if configuration is =y
 * @param x Configuration
 * @return bool
 */
#define IS_BUILTIN(x) _is_defined(x)

/**
 * @brief Test if configuration is =m
 * @param x Configuration
 * @return bool
 */
#define IS_MODULE(x) _is_defined(x##_MODULE)

/**
 * @brief Test if configuration is =y or =m
 * @param x Configuration
 * @return bool
 */
#define IS_ENABLED(x) _or(IS_BUILTIN(x), IS_MODULE(x))

#ifdef __cplusplus
}
#endif
#endif // __KCONFIG_H
