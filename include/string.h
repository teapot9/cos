/**
 * @file string.h
 * @brief String operations
 */

#ifndef STRING_H
#define STRING_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Calculate the length of a string
 * @param s NUL-terminated string to read
 * @return Length of the string
 */
size_t strlen(const char * s);

/**
 * @brief Copy a string into a newly allocated buffer
 * @param s Source
 * @return String (allocated with kmalloc)
 */
char * strdup(const char * s);

/**
 * @brief Compare strings
 * @param s1 First string to read
 * @param s2 Second string to read
 * @return Zero if the strings are identical, s1 - s2 if they are different
 */
int strcmp(const char * s1, const char * s2);

/**
 * @brief Compare strings
 * @param s1 First string to read
 * @param s2 Second string to read
 * @param len Compare up to `len` bytes
 * @return Zero if the strings are identical, s1 - s2 if they are different
 */
int strncmp(const char * s1, const char * s2, size_t len);

/**
 * @brief Copy a string
 * @param dst Destination
 * @param src Source string to copy
 * @return Number of characters written, excluding NUL
 */
size_t strcpy(char * dst, const char * src);

/**
 * @brief Copy a string
 * @param dst Destination
 * @param src Source string to copy
 * @param len Copy at most `len` bytes
 * @return Number of characters written, excluding NUL
 */
size_t strncpy(char * dst, const char * src, size_t len);

/**
 * @brief Copy memory area
 * @param dst Destination
 * @param src Source
 * @param n Size in bytes
 */
void memcpy(void * dst, void * src, size_t len);

/**
 * @brief Write a byte to a memory area
 * @param dst Destination
 * @param data Data to write
 * @param len Number of bytes to write
 */
void memset(void * dst, uint8_t data, size_t len);

/**
 * @brief Compare memory buffers
 * @param p1 First buffer
 * @param p2 Second buffer
 * @param len How many bytes to compare
 * @return If identical, zero; otherwise difference between the last bytes
 */
int memcmp(const void * p1, const void * p2, size_t len);

#ifdef __cplusplus
}
#endif
#endif // STRING_H
