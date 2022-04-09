#ifndef LIB_COMPILER_UBSAN_H
#define LIB_COMPILER_UBSAN_H

#include <stdbool.h>
#include <stdint.h>

typedef void * value_handle;
typedef value_handle uptr;

enum type_kind {
	TK_INTEGER = 0x0000,
	TK_FLOAT = 0x0001,
	TK_UNKNOWN = 0xFFFF,
};

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
  struct source_location loc;
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

/* ubsan callbacks */

void __ubsan_handle_type_mismatch_v1(
	struct type_mismatch_data * data, value_handle ptr
);

void __ubsan_handle_alignment_assumption(
	struct alignment_assumption_data * data, uintptr_t ptr,
	uintptr_t alignment, uintptr_t offset
);

void __ubsan_handle_negate_overflow(
	struct overflow_data * data, value_handle old_val
);

void __ubsan_handle_divrem_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
);

void __ubsan_handle_shift_out_of_bounds(
	struct shift_out_of_bound_data * data,
	value_handle lhs, value_handle rhs
);

void __ubsan_handle_out_of_bounds(
	struct out_of_bounds_data * data, value_handle index
);

void __ubsan_handle_builtin_unreachable(
	struct unreachable_data * data
);

void __ubsan_handle_missing_return(
	struct unreachable_data * data
);

void __ubsan_handle_vla_bound_not_positive(
	struct vla_bound_data * data, value_handle bound
);

void __ubsan_handle_float_cast_overflow(
	void * data, value_handle from
);

void __ubsan_handle_load_invalid_value(
	struct invalid_value_data * data, value_handle val
);

void __ubsan_handle_implicit_conversion(
	struct implicit_conversion_data * data,
	value_handle src, value_handle dst
);

void __ubsan_handle_invalid_builtin(
	struct invalid_builtin_data * data
);

void __ubsan_handle_invalid_objc_cast(
	struct invalid_obj_c_cast * data, value_handle ptr
);

void __ubsan_handle_nonnull_return_v1(
	struct non_null_return_data * data, struct source_location * loc
);

void __ubsan_handle_nullability_return_v1(
	struct non_null_return_data * data, struct source_location * loc
);

void __ubsan_handle_nonnull_arg(
	struct non_null_arg_data * data
);

void __ubsan_handle_nullability_arg(
	struct non_null_arg_data * data
);

void __ubsan_handle_pointer_overflow(
	struct pointer_overflow_data * data,
	void * base, void * result
);

void __ubsan_handle_cfi_bad_type(
	struct cfi_check_fail_data * data, value_handle vtable,
	bool valid_vtable, struct report_options opts
);

void __ubsan_handle_cfi_check_fail(
	struct cfi_check_fail_data * data, value_handle val,
	uptr valid_vtable
);

void __ubsan_handle_add_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
);

void __ubsan_handle_sub_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
);

void __ubsan_handle_mul_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
);

#endif // LIB_COMPILER_UBSAN_H
