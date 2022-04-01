#ifndef CONFIG_H
#define CONFIG_H

#include "generated/autoconf.h"

#define IS_BUILTIN(x) (x)
#define IS_MODULE(x) (!(x) && (x##_MODULE))
#define IS_ENABLED(x) ((x) || (x##_MODULE))

#endif // CONFIG_H