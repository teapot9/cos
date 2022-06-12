/**
 * @file unicode.h
 * @brief Unicode conversion
 */

#ifndef UNICODE_H
#define UNICODE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Decode the unicode code of a UTF-encoded character
 * @param src String to read the character from
 * @param code Pointer to where the unicode code will be written
 * @return Number of character read from `src`
 */
size_t utf_to_unicode(const char * src, uint32_t * code);

/**
 * @brief Decode the unicode code of a UTF8-encoded character
 * @param src String to read the character from
 * @param code Pointer to where the unicode code will be written
 * @return Number of character read from `src`
 */
size_t utf8_to_unicode(const uint8_t * src, uint32_t * code);

/**
 * @brief Encode a unicode code into UTF16 characters
 * @param dst String to write the encoded characters to
 * @param code Unicode code to encode
 * @param len Write at most `len` characters, including NUL
 * @return Number of characters written to `dst`, excluding NUL
 */
size_t unicode_to_utf16(uint16_t * dst, uint32_t code, size_t len);

/**
 * @brief Convert a UTF-encoded string into a UTF16-encoded one
 * @param dst String where to write the converted string
 * @param src String to convert
 * @param len Write at most `len` characters, including NUL
 * @param Number of characters written to `dst`, excluding NUL
 */
size_t utf_to_utf16(uint16_t * dst, const char * src, size_t len);

/**
 * @brief Convert a UTF8-encoded string into a UTF16-encoded one
 * @param dst String where to write the converted string
 * @param src String to convert
 * @param len Write at most `len` characters, including NUL
 * @param Number of characters written to `dst`, excluding NUL
 */
size_t utf8_to_utf16(uint16_t * dst, const uint8_t * src, size_t len);

/**
 * @brief Convert a UTF-encoded string into a UTF16-encoded one
 * @param dst String where to write the converted string
 * @param src String to convert
 * @param len Write at most `len` characters, including NUL
 * @param eol String to replace an end-of-line '\n' character with
 * @param Number of characters written to `dst`
 * @return Number of characters written, excluding NUL
 */
size_t utf_to_utf16_eol(uint16_t * dst, const char * src, size_t len, uint16_t * eol);

/**
 * @brief Convert a UTF8-encoded string into a UTF16-encoded one
 * @param dst String where to write the converted string
 * @param src String to convert
 * @param len Write at most `len` characters, including NUL
 * @param eol String to replace an end-of-line '\n' character with
 * @return Number of characters written, excluding NUL
 */
size_t utf8_to_utf16_eol(uint16_t * dst, const uint8_t * src, size_t len, uint16_t * eol);

/**
 * @brief Decode the unicode code of a UTF16-encoded character
 * @param src String to read the character from
 * @param code Pointer to where the unicode code will be written
 * @return Number of character read from `src`
 */
size_t utf16_to_unicode(const uint16_t * src, uint32_t * code);

/**
 * @brief Encode a unicode code into UTF8 characters
 * @param dst String to write the encoded characters to
 * @param code Unicode code to encode
 * @param len Write at most `len` characters, including NUL
 * @return Number of characters written to `dst`, excluding NUL
 */
size_t unicode_to_utf8(uint8_t * dst, uint32_t code, size_t len);

/**
 * @brief Convert a UTF16-encoded string into a UTF8-encoded one
 * @param dst String where to write the converted string
 * @param src String to convert
 * @param len Write at most `len` bytes, including NUL
 * @return Number of character written, excluding NUL
 */
size_t utf16_to_utf8(uint8_t * dst, const uint16_t * src, size_t len);

/**
 * @brief Convert a UTF16-encoded string into a UTF-encoded one
 * @param dst String where to write the converted string
 * @param src String to convert
 * @param len Write at most `len` characters, including NUL
 * @param Number of characters written to `dst`, excluding NUL
 */
size_t utf16_to_utf(char * dst, const uint16_t * src, size_t len);

/**
 * @brief Copy an UTF16-encoded string
 * @param dst String where to write the copy
 * @param src String to copy from
 * @param len Write at most `len` characters, including NUL
 * @return Number of characters written, excluding NUL
 */
size_t utf16_copy(uint16_t * dst, const uint16_t * src, size_t len);

#ifdef __cplusplus
}
#endif
#endif // UNICODE_H
