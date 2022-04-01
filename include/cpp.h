#ifndef CPP_H
#define CPP_H

#define _stringify(x) #x
#define stringify(x) _stringify(x)

#define typeof(x) __typeof__(x)
#define same_type(x, y) __builtin_types_compatible_p(typeof(x), typeof(y))
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define UNUSED __attribute__((unused))
#define USED __attribute__((used))
#define INTERRUPT __attribute__((interrupt))
#define PACKED __attribute__((packed))
#define COLD __attribute__((cold))
#define HOT __attribute__((hot))
#define WEAK __attribute__((weak))
#define SECTION(x) __attribute__((section(x)))

#endif // CPP_H
