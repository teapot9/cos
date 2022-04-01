#define pr_fmt(fmt) "ubsan: " fmt

#include "ubsan.h"

#include <stdbool.h>

#include <print.h>

void __ubsan_handle_type_mismatch_v1(
	struct type_mismatch_data * data, value_handle ptr
)
{
	return;
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_alignment_assumption(
	struct alignment_assumption_data * data, value_handle ptr,
	value_handle alignment, value_handle offset
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_negate_overflow(
	struct overflow_data * data, value_handle old_Val
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_divrem_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_shift_out_of_bounds(
	struct shift_out_of_bound_data * data,
	value_handle lhs, value_handle rhs
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_out_of_bounds(
	struct out_of_bounds_data * data, value_handle index
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_builtin_unreachable(
	struct unreachable_data * data
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_missing_return(
	struct unreachable_data * data
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_vla_bound_not_positive(
	struct vla_bound_data * data, value_handle bound
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_float_cast_overflow(
	void * data, value_handle from
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_load_invalid_value(
	struct invalid_value_data * data, value_handle val
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_implicit_conversion(
	struct implicit_conversion_data * data,
	value_handle src, value_handle dst
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_invalid_builtin(
	struct invalid_builtin_data * data
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_invalid_objc_cast(
	struct invalid_obj_c_cast * data, value_handle ptr
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_nonnull_return_v1(
	struct non_null_return_data * data, struct source_location * loc
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_nullability_return_v1(
	struct non_null_return_data * data, struct source_location * loc
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_nonnull_arg(
	struct non_null_arg_data * data
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_nullability_arg(
	struct non_null_arg_data * data
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_pointer_overflow(
	struct pointer_overflow_data * data,
	value_handle base, value_handle result
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_cfi_bad_type(
	struct cfi_check_fail_data * data, value_handle vtable,
	bool valid_vtable, struct report_options opts
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_cfi_check_fail(
	struct cfi_check_fail_data * data, value_handle val,
	uptr valid_vtable
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_add_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_sub_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
)
{
	pr_err("undefined behavior: %s\n", __func__);
}

void __ubsan_handle_mul_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
)
{
	pr_err("undefined behavior: %s\n", __func__);
}
