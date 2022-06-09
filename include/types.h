#ifndef TYPES_H
#define TYPES_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include <kconfig.h>

typedef size_t pid_t;
typedef size_t tid_t;

#if IS_ENABLED(CONFIG_64BIT)
typedef uint64_t uintn_t;
#else
typedef uint32_t uintn_t;
#endif

#ifdef __cplusplus
}
#endif
#endif // TYPES_H
