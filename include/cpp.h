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
#define ISR_AVAILABLE __attribute__((no_caller_saved_registers))
#define ISR_DEFINE __attribute__((interrupt))

#define _IDENT(x) x
#define _PATH(x, y) stringify(_IDENT(x)_IDENT(y))
#define ARCHDIR(x) _PATH(ARCHSRC, x)

#define _REP0(x)
#define _REP1(x) _REP0(x) x
#define _REP2(x) _REP1(x) x
#define _REP3(x) _REP2(x) x
#define _REP4(x) _REP3(x) x
#define _REP5(x) _REP4(x) x
#define _REP6(x) _REP5(x) x
#define _REP7(x) _REP6(x) x
#define _REP8(x) _REP7(x) x
#define _REP9(x) _REP8(x) x
#define _REP10(x) _REP9(x) x
#define REP(hundred, ten, unit, x) \
	_REP##hundred(_REP10(REP10(x))) \
	_REP##ten(_REP10(x)) \
	_REP##unit(x)

#define _FORLOOP_1(f, x) f(0, x)
#define _FORLOOP_2(f, x) _FORLOOP_1(f, x), f(1, x)
#define _FORLOOP_3(f, x) _FORLOOP_2(f, x), f(2, x)
#define _FORLOOP_4(f, x) _FORLOOP_3(f, x), f(3, x)
#define _FORLOOP_5(f, x) _FORLOOP_4(f, x), f(4, x)
#define _FORLOOP_6(f, x) _FORLOOP_5(f, x), f(5, x)
#define _FORLOOP_7(f, x) _FORLOOP_6(f, x), f(6, x)
#define _FORLOOP_8(f, x) _FORLOOP_7(f, x), f(7, x)
#define _FORLOOP_9(f, x) _FORLOOP_8(f, x), f(8, x)
#define _FORLOOP_10(f, x) _FORLOOP_9(f, x), f(9, x)
#define _FORLOOP_IN(f, x, n) _FORLOOP_##n(f, x)
#define FORLOOP(func, x, size) _FORLOOP_IN(func, x, size)

#define _ARRAY_1(x) x
#define _ARRAY_2(x) x, _ARRAY_1(x)
#define _ARRAY_3(x) x, _ARRAY_2(x)
#define _ARRAY_4(x) x, _ARRAY_3(x)
#define _ARRAY_5(x) x, _ARRAY_4(x)
#define _ARRAY_6(x) x, _ARRAY_5(x)
#define _ARRAY_7(x) x, _ARRAY_6(x)
#define _ARRAY_8(x) x, _ARRAY_7(x)
#define _ARRAY_9(x) x, _ARRAY_8(x)
#define _ARRAH_H0(x)
#define _ARRAY_H1(x) x, _ARRAY_9(x),
#define _ARRAY_H2(x) _ARRAY_H1(x) _ARRAY_H1(x)
#define _ARRAY_H3(x) _ARRAY_H1(x) _ARRAY_H2(x)
#define _ARRAY_H4(x) _ARRAY_H1(x) _ARRAY_H3(x)
#define _ARRAY_H5(x) _ARRAY_H1(x) _ARRAY_H4(x)
#define _ARRAY_H6(x) _ARRAY_H1(x) _ARRAY_H5(x)
#define _ARRAY_H7(x) _ARRAY_H1(x) _ARRAY_H6(x)
#define _ARRAY_H8(x) _ARRAY_H1(x) _ARRAY_H7(x)
#define _ARRAY_H9(x) _ARRAY_H1(x) _ARRAY_H8(x)
#define _ARRAY_H10(x) _ARRAY_H1(x) _ARRAY_H9(x)
#define _ARRAY(ten, unit, value) _ARRAY_H##ten(value) _ARRAY_##unit(value)
#define ARRAY(ten, unit, value) _ARRAY(ten, unit, value)

#endif // CPP_H
