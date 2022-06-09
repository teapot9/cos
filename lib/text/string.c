#define pr_fmt(fmt) "text: " fmt

#include <string.h>

#include <stddef.h>
#include <stdint.h>

#include <alloc.h>

/* public: string.h */
size_t strlen(const char * s)
{
	size_t len = 0;

	while (s[len])
		len++;
	return len;
}

/* public: string.h */
char * strdup(const char * s)
{
	size_t len = strlen(s);
	size_t alloc = (len + 1) * sizeof(char);
	char * new = malloc(alloc);
	if (new == NULL)
		return NULL;
	strncpy(new, s, alloc);
	return new;
}

/* public: string.h */
int strcmp(const char * s1, const char * s2)
{
	return strncmp(s1, s2, SIZE_MAX);
}

/* public: string.h */
int strncmp(const char * s1, const char * s2, size_t len)
{
	size_t pos = 0;

	while (pos < len - 1
	       && s1[pos] == s2[pos]
	       && s1[pos] != 0
	       && s2[pos] != 0)
		pos++;
	return s1[pos] - s2[pos];
}

/* public: string.h */
size_t strcpy(char * dst, const char * src)
{
	return strncpy(dst, src, SIZE_MAX);
}

/* public: string.h */
size_t strncpy(char * dst, const char * src, size_t len)
{
	size_t copied = 0;

	while (copied < len && src[copied]) {
		dst[copied] = src[copied];
		copied++;
	}
	if (copied < len)
		dst[copied] = 0;
	return copied;
}

/* public: string.h */
void memcpy(void * dst, void * src, size_t len)
{
	uint8_t * bdst = dst;
	uint8_t * bsrc = src;

	for (size_t i = 0; i < len; i++)
		bdst[i] = bsrc[i];
}

/* public: string.h */
void memset(void * dst, uint8_t data, size_t len)
{
	uint8_t * bdst = dst;

	for (size_t i = 0; i < len; i++)
		bdst[i] = data;
}

/* public: string.h */
int memcmp(const void * p1, const void * p2, size_t len)
{
	size_t pos = 0;

	while (pos < len - 1 && ((uint8_t *) p1)[pos] == ((uint8_t *) p2)[pos])
		pos++;
	return ((uint8_t *) p1)[pos] - ((uint8_t *) p2)[pos];
}
