#ifndef NEW_H
#define NEW_H
#ifdef __cplusplus

#include <stddef.h>

/* Data types */

struct nothrow_t {};
extern const nothrow_t nothrow;
enum align_val_t: size_t {};

/* C++ mandatory functions */

__attribute__((returns_nonnull)) void * operator new(size_t sz);
__attribute__((returns_nonnull)) void * operator new[](size_t sz);
void operator delete(void * ptr) noexcept;
void operator delete[](void * ptr) noexcept;

/* new */

__attribute__((returns_nonnull))
void * operator new(size_t sz, align_val_t al);
__attribute__((returns_nonnull))
void * operator new[](size_t sz, align_val_t al);

void * operator new(size_t sz, const nothrow_t &) noexcept;
void * operator new[](size_t sz, const nothrow_t &) noexcept;
void * operator new(size_t sz, align_val_t al, const nothrow_t &) noexcept;
void * operator new[](size_t sz, align_val_t al, const nothrow_t &) noexcept;

void * operator new(size_t, void * ptr) noexcept;
void * operator new[](size_t, void * ptr) noexcept;

/* delete */

void operator delete(void * ptr, align_val_t al) noexcept;
void operator delete[](void * ptr, align_val_t al) noexcept;
void operator delete(void * ptr, size_t) noexcept;
void operator delete[](void * ptr, size_t) noexcept;
void operator delete(void * ptr, size_t, align_val_t) noexcept;
void operator delete[](void * ptr, size_t, align_val_t) noexcept;

void operator delete(void * ptr, const nothrow_t &) noexcept;
void operator delete[](void * ptr, const nothrow_t &) noexcept;
void operator delete(void * ptr, align_val_t, const nothrow_t &) noexcept;
void operator delete[](void * ptr, align_val_t, const nothrow_t &) noexcept;

void operator delete(void *, void *) noexcept;
void operator delete[](void *, void *) noexcept;

#endif // __cpluspluc
#endif // NEW_H
