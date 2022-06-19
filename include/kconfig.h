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

/**
 * @brief Test if configuration is =y
 * @param x Configuration
 * @return bool
 */
#define IS_BUILTIN(x) (x)

/**
 * @brief Test if configuration is =m
 * @param x Configuration
 * @return bool
 */
#define IS_MODULE(x) (!(x) && (x##_MODULE))

/**
 * @brief Test if configuration is =y or =m
 * @param x Configuration
 * @return bool
 */
#define IS_ENABLED(x) ((x) || (x##_MODULE))

#ifdef __cplusplus
}
#endif
#endif // __KCONFIG_H
