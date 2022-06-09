#include <new.h>

#include <stddef.h>

#include <alloc.h>
#include <panic.h>

const nothrow_t nothrow;


__attribute__((returns_nonnull)) void * operator new(size_t sz)
{
	return operator new(sz, align_val_t(DEFAULT_MEMORY_ALLOC_ALIGN));
}

__attribute__((returns_nonnull)) void * operator new[](size_t sz)
{
	return operator new(sz);
}

void operator delete(void * ptr) noexcept
{
	kfree(ptr);
}

void operator delete[](void * ptr) noexcept
{
	operator delete(ptr);
}


__attribute__((returns_nonnull))
void * operator new(size_t sz, align_val_t al)
{
	void * ptr = operator new(sz, al, nothrow);
	if (ptr == NULL)
		panic("failed to allocate memory for new(%zu, %zu)", sz, al);
	return ptr;
}

__attribute__((returns_nonnull))
void * operator new[](size_t sz, align_val_t al)
{
	return operator new(sz, al);
}

void * operator new(size_t sz, const nothrow_t &) noexcept
{
	return operator
		new(sz, align_val_t(DEFAULT_MEMORY_ALLOC_ALIGN), nothrow);
}

void * operator new[](size_t sz, const nothrow_t &) noexcept
{
	return operator new(sz, nothrow);
}

void * operator new(size_t sz, align_val_t al, const nothrow_t &) noexcept
{
	return kmalloc(sz, al);
}

void * operator new[](size_t sz, align_val_t al, const nothrow_t &) noexcept
{
	return operator new(sz, al, nothrow);
}

void * operator new(size_t, void * ptr) noexcept
{
	return ptr;
}

void * operator new[](size_t, void * ptr) noexcept
{
	return ptr;
}


void operator delete(void * ptr, align_val_t) noexcept
{
	operator delete(ptr);
}

void operator delete[](void * ptr, align_val_t) noexcept
{
	operator delete(ptr);
}

void operator delete(void * ptr, size_t) noexcept
{
	operator delete(ptr);
}

void operator delete[](void * ptr, size_t) noexcept
{
	operator delete(ptr);
}

void operator delete(void * ptr, size_t, align_val_t) noexcept
{
	operator delete(ptr);
}

void operator delete[](void * ptr, size_t, align_val_t) noexcept
{
	operator delete(ptr);
}

void operator delete(void * ptr, const nothrow_t &) noexcept
{
	operator delete(ptr);
}

void operator delete[](void * ptr, const nothrow_t &) noexcept
{
	operator delete(ptr);
}

void operator delete(void * ptr, align_val_t, const nothrow_t &) noexcept
{
	operator delete(ptr);
}

void operator delete[](void * ptr, align_val_t, const nothrow_t &) noexcept
{
	operator delete(ptr);
}

void operator delete(void *, void *) noexcept
{
}

void operator delete[](void *, void *) noexcept
{
}
