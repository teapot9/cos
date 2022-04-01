#ifndef CPP_H
#define CPP_H

#define _stringify(x) #x
#define stringify(x) _stringify(x)

#define typeof(x) __typeof__(x)

#define same_type(x, y) __builtin_types_compatible_p(typeof(x), typeof(y))

#endif // CPP_H
