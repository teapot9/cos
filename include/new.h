/**
 * @file new.h
 * @brief C++ allocation functions
 */

#ifndef __NEW_H
#define __NEW_H
#ifdef __cplusplus

#include <stddef.h>

/* Data types */

struct nothrow_t {};
extern const nothrow_t nothrow;
enum align_val_t: size_t {};

/* C++ mandatory functions */

/// Dynamic allocation of variable
__attribute__((returns_nonnull)) void * operator new(size_t sz);
/// Dynamic allocation of array
__attribute__((returns_nonnull)) void * operator new[](size_t sz);
/// Free variable
void operator delete(void * ptr) noexcept;
/// Free array
void operator delete[](void * ptr) noexcept;

/* new */

/// Allocate with alignment
__attribute__((returns_nonnull))
void * operator new(size_t sz, align_val_t al);
/// Allocate with alignment
__attribute__((returns_nonnull))
void * operator new[](size_t sz, align_val_t al);

/// Allocate, return nullpte if error
void * operator new(size_t sz, const nothrow_t &) noexcept;
/// Allocate, return nullpte if error
void * operator new[](size_t sz, const nothrow_t &) noexcept;
/// Allocate with alignment, return nullpte if error
void * operator new(size_t sz, align_val_t al, const nothrow_t &) noexcept;
/// Allocate with alignment, return nullpte if error
void * operator new[](size_t sz, align_val_t al, const nothrow_t &) noexcept;

/// Placement new
void * operator new(size_t, void * ptr) noexcept;
/// Placement new
void * operator new[](size_t, void * ptr) noexcept;

/* delete */

/// Delete with alignment
void operator delete(void * ptr, align_val_t al) noexcept;
/// Delete with alignment
void operator delete[](void * ptr, align_val_t al) noexcept;
/// Delete
void operator delete(void * ptr, size_t) noexcept;
/// Delete
void operator delete[](void * ptr, size_t) noexcept;
/// Delete with alignment
void operator delete(void * ptr, size_t, align_val_t) noexcept;
/// Delete with alignment
void operator delete[](void * ptr, size_t, align_val_t) noexcept;

/// Delete with no exception
void operator delete(void * ptr, const nothrow_t &) noexcept;
/// Delete with no exception
void operator delete[](void * ptr, const nothrow_t &) noexcept;
/// Delete with alignment and no exception
void operator delete(void * ptr, align_val_t, const nothrow_t &) noexcept;
/// Delete with alignment and no exception
void operator delete[](void * ptr, align_val_t, const nothrow_t &) noexcept;

/// Placement delete
void operator delete(void *, void *) noexcept;
/// Placement delete
void operator delete[](void *, void *) noexcept;

#endif // __cpluspluc
#endif // __NEW_H
