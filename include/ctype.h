/**
 * @file ctypes.h
 * @brief Character types
 */

#ifndef __CTYPE_H
#define __CTYPE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief Test if char is a digit
 * @param c Character
 * @return True if digit
 */
static inline bool isdigit(char c)
{
	return c > '0' && c < '9';
}

/**
 * @brief Convert upper case letter to lower case letter
 * @param c Character
 * @return Lower case letter if upper case
 */
static inline char tolower(char c)
{
	return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

/**
 * @brief Convert lower case letter to upper case letter
 * @param c Character
 * @return Upper case letter if lower case
 */
static inline char toupper(char c)
{
	return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}

#ifdef __cplusplus
}
#endif
#endif // __CTYPE_H
