#define pr_fmt(fmt) "ubsan: " fmt

#include "ubsan.h"

#include <stdbool.h>

#include <printk.h>
#include <printf.h>
#include <task.h>

#define VALUE_BUFFER 32
#define UBSAN_RECURSE_MAX 3
#define ubsan_abort() (*get_call_count() >= UBSAN_RECURSE_MAX)

static int early_call_count = 0;

const char * type_check_str(unsigned char n)
{
	static const char * const str[] = {
		"load of",
		"store to",
		"reference binding to",
		"member access in",
		"member call to",
		"constructor call to",
		"downcast pointer to",
		"downcast reference to",
		"upcast to",
		"upcast to virtual",
		"non null assign to",
		"dynamic operation",
	};
	static const unsigned count = sizeof(str) / sizeof(*str);
	return n >= count ? "unknown" : str[n];
}

const char * implicit_conversion_str(unsigned char n)
{
	static const char * const str[] = {
		"integer truncation",
		"unsigned integer truncation",
		"signed integer truncation",
		"integer sign change",
		"signed integer truncation or sign change",
	};
	static const unsigned count = sizeof(str) / sizeof(*str);
	return n >= count ? "unknown" : str[n];
}

static inline int * get_call_count(void)
{
#ifdef BOOTLOADER
	return &early_call_count;
#else
	struct thread * t = thread_current();
	return t == NULL ? &early_call_count : &t->ubsan;
#endif
}

static void ubsan_start(const struct source_location * loc, const char * desc)
{
	(*get_call_count())++;
	pr_err("undefined behavior: %s in %s:%d:%d\n",
	       desc, loc->filename, loc->line, loc->column);
}

static void ubsan_end(void)
{
	(*get_call_count())--;
}

static bool value_signed(const struct type_descriptor * type)
{
	return type->type_kind == TK_INTEGER && type->type_info & 1;
}

static unsigned value_bit_width(const struct type_descriptor * type)
{
	if (type->type_kind == TK_INTEGER)
		return 1 << (type->type_info >> 1);
	else
		return 0;
}

static uintmax_t value_uint(const value_handle val,
                            const struct type_descriptor * type)
{
	uintmax_t v;
	unsigned bits = value_bit_width(type);
	if (bits <= sizeof(type) * 8) {
		// inline
		v = (uintptr_t) val;
	} else {
		if (bits <= 8)
			v = *(uint8_t *) val;
		else if (bits <= 16)
			v = *(uint16_t *) val;
		else if (bits <= 32)
			v = *(uint32_t *) val;
		else if (bits <= 64)
			v = *(uint64_t *) val;
		else
			v = -1;
	}
	return v;
}

static intmax_t value_int(const value_handle val,
                          const struct type_descriptor * type)
{
	return (intmax_t) value_uint(val, type);
}

static void value_str(char * dst, size_t len, const value_handle val,
                      const struct type_descriptor * type)
{
	if (type->type_kind != TK_INTEGER) {
		snprintf(dst, len, "unknown [%p]", val);
		return;
	}

	bool is_signed = value_signed(type);;

	if (is_signed)
		snprintf(dst, len, "%jd", value_int(val, type));
	else
		snprintf(dst, len, "0x%jx", value_uint(val, type));
}

static void ubsan_overflow(
	const char * op,
	struct type_descriptor * lhs_type, struct type_descriptor * rhs_type,
	value_handle lhs, value_handle rhs
)
{
	char lhs_str[VALUE_BUFFER];
	char rhs_str[VALUE_BUFFER];
	value_str(lhs_str, sizeof(lhs_str), lhs, lhs_type);
	value_str(rhs_str, sizeof(rhs_str), rhs, rhs_type);
	pr_err("%s %s %s overflowed (%s %s %s)\n", lhs_str, op, rhs_str,
	       lhs_type->type_name, op, lhs_type->type_name);
}

void __ubsan_handle_type_mismatch_v1(
	struct type_mismatch_data * data, value_handle ptr
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "type mismatch");

	const char * err = type_check_str(data->type_check_kind);
	char ptr_str[VALUE_BUFFER];
	value_str(ptr_str, sizeof(ptr_str), ptr, data->type);
	pr_err("%s %s [aligned %p] (%s)\n", err, ptr_str,
	       data->log_alignment, data->type->type_name);

	ubsan_end();
}

void __ubsan_handle_negate_overflow(
	struct overflow_data * data, value_handle old_val
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "negate overflow");

	char old_val_str[VALUE_BUFFER];
	value_str(old_val_str, sizeof(old_val_str), old_val, data->type);
	pr_err("%s negation of %s is overflowing\n", data->type->type_name,
	       old_val_str);

	ubsan_end();
}

void __ubsan_handle_divrem_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "divrem overflow");

	ubsan_overflow("%", data->type, data->type, lhs, rhs);

	ubsan_end();
}

void __ubsan_handle_shift_out_of_bounds(
	struct shift_out_of_bound_data * data,
	value_handle lhs, value_handle rhs
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "shift out of bounds");

	char lhs_str[VALUE_BUFFER];
	char rhs_str[VALUE_BUFFER];
	value_str(lhs_str, sizeof(lhs_str), lhs, data->lhs_type);
	value_str(rhs_str, sizeof(rhs_str), rhs, data->rhs_type);
	pr_err("%s shifted %s\n", lhs_str, rhs_str);

	ubsan_end();
}

void __ubsan_handle_out_of_bounds(
	struct out_of_bounds_data * data, value_handle index
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "out of bounds");

	char index_str[VALUE_BUFFER];
	value_str(index_str, sizeof(index_str), index, data->index_type);
	pr_err("%s array: %s is out of range\n", data->array_type->type_name,
	       index_str);

	ubsan_end();
}

void __ubsan_handle_builtin_unreachable(
	struct unreachable_data * data
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "builtin unreachable");
	ubsan_end();
}

void __ubsan_handle_load_invalid_value(
	struct invalid_value_data * data, value_handle val
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "load invalid value");

	char val_str[VALUE_BUFFER];
	value_str(val_str, sizeof(val_str), val, data->type);
	pr_err("loading invalid %s (%s)\n", val_str,
	       data->type->type_name);

	ubsan_end();
}

void __ubsan_handle_pointer_overflow(
	struct pointer_overflow_data * data,
	void * base, void * result
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "pointer overflow");

	pr_err("%p overflowed to %p\n", base, result);

	ubsan_end();
}

void __ubsan_handle_add_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "add overflow");

	ubsan_overflow("+", data->type, data->type, lhs, rhs);

	ubsan_end();
}

void __ubsan_handle_sub_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "add overflow");

	ubsan_overflow("-", data->type, data->type, lhs, rhs);

	ubsan_end();
}

void __ubsan_handle_mul_overflow(
	struct overflow_data * data, value_handle lhs, value_handle rhs
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "add overflow");

	ubsan_overflow("*", data->type, data->type, lhs, rhs);

	ubsan_end();
}

void __ubsan_handle_alignment_assumption(
	struct alignment_assumption_data * data, uintptr_t ptr,
	uintptr_t alignment, uintptr_t offset
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "alignment assumption");

	pr_err("address %p[%p] to %s was expected to be %p"
	       " aligned but is not\n",
	       ptr, offset, data->type->type_name, alignment);
	pr_err("assumption defined in %s:%d:%d\n",
	       data->assumption_loc.filename, data->assumption_loc.line,
	       data->assumption_loc.column);

	ubsan_end();
}

void __ubsan_handle_implicit_conversion(
	struct implicit_conversion_data * data,
	value_handle src, value_handle dst
)
{
	if (ubsan_abort())
		return;
	ubsan_start(&data->loc, "implicit conversion");

	const char * kind = implicit_conversion_str(data->kind);
	char src_str[VALUE_BUFFER];
	char dst_str[VALUE_BUFFER];
	value_str(src_str, sizeof(src_str), src, data->from_type);
	value_str(dst_str, sizeof(dst_str), dst, data->from_type);
	pr_err("%s: from %s of type %s to %s of type %s\n", kind, src_str,
	       data->from_type->type_name, dst_str, data->to_type->type_name);

	ubsan_end();
}

void __ubsan_handle_nonnull_return_v1(
	struct non_null_return_data * data, struct source_location * loc
)
{
	if (ubsan_abort())
		return;
	ubsan_start(loc, "non-null return");

	pr_err("NULL pointer returned from function that should not\n", 0);
	pr_err("non-null attribute in %s:%d:%d\n", data->attr_loc.filename,
	       data->attr_loc.line, data->attr_loc.column);

	ubsan_end();
}
