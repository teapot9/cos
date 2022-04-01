#include <strtox.h>

#include <errno.h>
#include <stddef.h>
#include <stdbool.h>

#include <ctype.h>

static bool isdigit_base(char c, unsigned b)
{
	return (c >= '0' && c <= '9' && (b == 10 || b == 16))
		|| (tolower(c) >= 'a' && tolower(c) <= 'f' && b == 16);
}

static unsigned strto_digit(char c, unsigned b)
{
	if (!isdigit_base(c, b))
		return 0;
	switch (b) {
	case 10:
		return c - '0';
	case 16:
		return (c >= '0' && c <= '9') ? c - '0' : tolower(c) - 'a';
	default:
		return 0;
	}
}

int kstrtoull(const char * s, unsigned base, unsigned long long * dst)
{
	if (dst == NULL || s == NULL || !isdigit_base('0', base))
		return -EINVAL;

	int i = 0;
	unsigned long long ret = 0;
	while (isdigit_base(s[i], base)) {
		ret *= base;
		ret += strto_digit(s[i++], base);
	}
	*dst = ret;
	return i;
}

int kstrtoul(const char * s, unsigned base, unsigned long * dst)
{
	unsigned long long tmp;
	int ret = kstrtoull(s, base, &tmp);
	if (ret >= 0)
		*dst = tmp;
	return ret;
}

int kstrtou(const char * s, unsigned base, unsigned * dst)
{
	unsigned long long tmp;
	int ret = kstrtoull(s, base, &tmp);
	if (ret >= 0)
		*dst = tmp;
	return ret;
}

int kstrtoll(const char * s, unsigned base, long long * dst)
{
	if (s == NULL)
		return -EINVAL;

	bool neg = false;
	if (*s == '-')
		neg = true;

	unsigned long long tmp;
	int ret = kstrtoull(neg ? s + 1 : s, base, &tmp);
	if (ret >= 0) {
		*dst = neg ? -tmp : tmp;
		ret++;
	}
	return ret;
}

int kstrtol(const char * s, unsigned base, long * dst)
{
	long long tmp;
	int ret = kstrtoll(s, base, &tmp);
	if (ret >= 0)
		*dst = tmp;
	return ret;
}

int kstrto(const char * s, unsigned base, int * dst)
{
	long long tmp;
	int ret = kstrtoll(s, base, &tmp);
	if (ret >= 0)
		*dst = tmp;
	return ret;
}
