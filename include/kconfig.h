#ifndef KCONFIG_H
#define KCONFIG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "generated/autoconf.h"

#define IS_BUILTIN(x) (x)
#define IS_MODULE(x) (!(x) && (x##_MODULE))
#define IS_ENABLED(x) ((x) || (x##_MODULE))

#ifdef __cplusplus
}
#endif
#endif // KCONFIG_H
