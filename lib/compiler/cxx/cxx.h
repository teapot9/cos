#ifndef LIB_COMPILER_CXX_H
#define LIB_COMPILER_CXX_H

#include <stdint.h>
#include <stddef.h>
#include <stdnoreturn.h>

#include <cpp.h>

noreturn void __cxa_pure_virtual();
noreturn void __cxa_deleted_virtual();

int __cxa_guard_acquire(int64_t * guard_object);
void __cxa_guard_release(int64_t * guard_object);
void __cxa_guard_abort(int64_t * guard_object);

extern void * __dso_handle;
int __cxa_atexit(UNUSED void (* f)(void *), UNUSED void * p, UNUSED void * d);
void __cxa_finalize(UNUSED void * d);

#if 0 // not implemented
char* __cxa_demangle(const char * mangled_name,
                     char * buf, size_t * n, int * status);
#endif

#if 0 // not needed
void * __cxa_vec_new(
	size_t element_count, size_t element_size, size_t padding_size,
	void (* constructor)(void * this), void (* destructor)(void * this)
);

void * __cxa_vec_new2(
	size_t element_count, size_t element_size, size_t padding_size,
	void (* constructor)(void * this), void (* destructor)(void * this),
	void * (* alloc)(size_t size), void (* dealloc)(void * obj)
);

void * __cxa_vec_new3(
	size_t element_count, size_t element_size, size_t padding_size,
	void (* constructor)(void * this), void (* destructor)(void * this),
	void * (* alloc)(size_t size), void(* dealloc)(void * obj, size_t size)
);

void __cxa_throw_bad_array_new_length(void);

void __cxa_vec_ctor(
	void * array_address, size_t element_count, size_t element_size,
	void (* constructor)(void * this), void (* destructor)(void * this)
);

void __cxa_vec_dtor(
	void * array_address, size_t element_count, size_t element_size,
	void (* destructor)(void * this)
);

void __cxa_vec_cleanup(
	void * array_address, size_t element_count, size_t element_size,
	void (* destructor)(void * this)
);

void __cxa_vec_delete(
	void * array_address, size_t element_size, size_t padding_size,
	void (* destructor)(void * this)
);

void __cxa_vec_delete2(
	void * array_address, size_t element_size, size_t padding_size,
	void (* destructor)(void * this), void (* dealloc)(void * obj)
);

void __cxa_vec_delete3(
	void * array_address, size_t element_size, size_t padding_size,
	void (* destructor)(void * this),
	void (* dealloc)(void * obj, size_t size)
);

void __cxa_vec_cctor(
	void * dest_array, void * src_array,
	size_t element_count, size_t element_size,
	void (* constructor)(void * destination, void * source),
	void (* destructor)(void *)
);

typedef struct {
	  Elf64_Word	pi_pri;
	  Elf64_Addr	pi_addr;
} Elf64_Priority_Init;
void __cxa_priority_init(ElfXX_Priority_Init * pi,int cnt);
#endif

#endif // LIB_COMPILER_CXX_H
