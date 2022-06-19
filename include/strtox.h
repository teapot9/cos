/**
 * @file strtox.h
 * @brief String conversion
 */

#ifndef __STRTOX_H
#define __STRTOX_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert a string into an unsigned integer
 * @param s String
 * @param base Numeric base of the number
 * @param dst Pointer where to write the converted number
 * @return errno
 */
int kstrtoull(const char * s, unsigned base, unsigned long long * dst);

/**
 * @brief Convert a string into an unsigned integer
 * @param s String
 * @param base Numeric base of the number
 * @param dst Pointer where to write the converted number
 * @return errno
 */
int kstrtoul(const char * s, unsigned base, unsigned long * dst);

/**
 * @brief Convert a string into an unsigned integer
 * @param s String
 * @param base Numeric base of the number
 * @param dst Pointer where to write the converted number
 * @return errno
 */
int kstrtou(const char * s, unsigned base, unsigned * dst);

/**
 * @brief Convert a string into an signed integer
 * @param s String
 * @param base Numeric base of the number
 * @param dst Pointer where to write the converted number
 * @return errno
 */
int kstrtoll(const char * s, unsigned base, long long * dst);

/**
 * @brief Convert a string into an signed integer
 * @param s String
 * @param base Numeric base of the number
 * @param dst Pointer where to write the converted number
 * @return errno
 */
int kstrtol(const char * s, unsigned base, long * dst);

/**
 * @brief Convert a string into an signed integer
 * @param s String
 * @param base Numeric base of the number
 * @param dst Pointer where to write the converted number
 * @return errno
 */
int kstrto(const char * s, unsigned base, int * dst);

#ifdef __cplusplus
}
#endif
#endif // __STRTOX_H
