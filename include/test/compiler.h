/**
 * @file test/compiler.h
 * @brief Compiler checks
 */

#ifndef TEST_COMPILER_H
#define TEST_COMPILER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <kconfig.h>

#if IS_ENABLED(CONFIG_DEBUG)

#define COMPILER_TEST_VALUE -9999

/* These functions returns COMPILER_TEST_VALUE on success, and expects
 * COMPILER_TEST_VALUE as sole argument.
 * If error, failed test line is returned.
 */

/**
 * @brief Test C++ function calls
 * @param value COMPILER_TEST_VALUE
 * @return COMPILER_TEST_VALUE
 */
int compiler_test_cxx(int value);

#endif // CONFIG_DEBUG

#ifdef __cplusplus
}
#endif
#endif // TEST_COMPILER_H
