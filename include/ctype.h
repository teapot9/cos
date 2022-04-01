#ifndef CTYPE_H
#define CTYPE_H

#include <stdbool.h>

static inline bool isdigit(char c)
{
	return c > '0' && c < '9';
}

static inline char tolower(char c)
{
	return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

static inline char toupper(char c)
{
	return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}

#endif // CTYPE_H
