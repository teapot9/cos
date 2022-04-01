#ifndef LIB_COMPILER_UBSAN_H
#define LIB_COMPILER_UBSAN_H

#include <stdbool.h>
#include <stdint.h>

typedef void * value_handle;
typedef value_handle uptr;

struct source_location {
	const char * filename;
	uint32_t line;
	uint32_t column;
};

struct type_descriptor {
	uint16_t type_kind;
	uint16_t type_info;
	char type_name[1];
};

struct type_mismatch_data {
  struct source_location Loc;
  const struct type_descriptor * type;
  uintptr_t log_alignment;
  unsigned char type_check_kind;
};

struct alignment_assumption_data {
  struct source_location loc;
  struct source_location assumption_loc;
  struct type_descriptor * type;
};

struct overflow_data {
	struct source_location loc;
	struct type_descriptor * type;
};

struct shift_out_of_bound_data {
	struct source_location loc;
	struct type_descriptor * lhs_type;
	struct type_descriptor * rhs_type;
};

struct out_of_bounds_data {
	struct source_location loc;
	struct type_descriptor * array_type;
	struct type_descriptor * index_type;
};

struct unreachable_data {
	struct source_location loc;
};

struct vla_bound_data {
	struct source_location loc;
	struct type_descriptor * type;
};

struct invalid_value_data {
	struct source_location loc;
	struct type_descriptor * type;
};

struct implicit_conversion_data {
	struct source_location loc;
	struct type_descriptor * from_type;
	struct type_descriptor * to_type;
	unsigned char kind;
};

struct invalid_builtin_data {
	struct source_location loc;
	unsigned char kind;
};

struct invalid_obj_c_cast {
	struct source_location loc;
	struct type_descriptor * expected_type;
};

struct non_null_return_data {
	struct source_location attr_loc;
};

struct non_null_arg_data {
	struct source_location loc;
	struct source_location attr_loc;
	int arg_index;
};

struct pointer_overflow_data {
	struct source_location loc;
};

enum cfi_type_check_kind {
	CFITCK_VCall,
	CFITCK_NVCall,
	CFITCK_DerivedCast,
	CFITCK_UnrelatedCast,
	CFITCK_ICall,
	CFITCK_NVMFCall,
	CFITCK_VMFCall,
};

struct cfi_check_fail_data {
	enum cfi_type_check_kind check_kind;
	struct source_location loc;
	struct type_descriptor * type;
};

struct report_options {
	bool from_unrecoverable_handler;
	uptr pc;
	uptr bp;
};

#endif // LIB_COMPILER_UBSAN_H
