#ifndef STRTOX_H
#define STRTOX_H
#ifdef __cplusplus
extern "C" {
#endif

int kstrtoull(const char * s, unsigned base, unsigned long long * dst);
int kstrtoul(const char * s, unsigned base, unsigned long * dst);
int kstrtou(const char * s, unsigned base, unsigned * dst);

int kstrtoll(const char * s, unsigned base, long long * dst);
int kstrtol(const char * s, unsigned base, long * dst);
int kstrto(const char * s, unsigned base, int * dst);

#ifdef __cplusplus
}
#endif
#endif // STRTOX_H
