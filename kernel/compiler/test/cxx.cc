#include <test/compiler.h>

#include <new.h>
#include <setup.h>

#include <errno.h>

#define TOTAL_COUNT_INIT 10
#define DEFAULT_ID 0

namespace test_ns {

class test_class {
	static int _total_count;
	int _id;
public:
	static const int & total_count();

	test_class(void);
	test_class(int id);
	~test_class(void);

	const int & id(void) const;
	int & id(void);
};

int test_class::_total_count = TOTAL_COUNT_INIT;

const int & test_class::total_count()
{
	return test_class::_total_count;
}

test_class::test_class(void): test_class(DEFAULT_ID)
{
}

test_class::test_class(int id): _id(id)
{
	_total_count++;
}

test_class::~test_class(void)
{
	_total_count--;
}

const int & test_class::id(void) const
{
	return _id;
}

int & test_class::id(void)
{
	return _id;
}

}

static int cxx_test(int value)
{
	if (value != COMPILER_TEST_VALUE)
		return __LINE__;

	if (test_ns::test_class::total_count() != TOTAL_COUNT_INIT)
		return __LINE__;

	test_ns::test_class obj1(99);
	if (obj1.total_count() != TOTAL_COUNT_INIT + 1)
		return __LINE__;
	if (obj1.id() != 99)
		return __LINE__;
	obj1.id() = 9;
	if (obj1.id() != 9)
		return __LINE__;

	test_ns::test_class * obj2 = new(nothrow) test_ns::test_class(1000);
	if (obj2 == NULL)
		return __LINE__;
	if (obj2->total_count() != TOTAL_COUNT_INIT + 2)
		return __LINE__;
	if (obj2->id() != 1000)
		return __LINE__;
	obj2->id() = 2000;
	if (obj2->id() != 2000)
		return __LINE__;
	delete obj2;
	if (obj1.total_count() != TOTAL_COUNT_INIT + 1)
		return __LINE__;

	test_ns::test_class obj3;
	if (obj3.id() != DEFAULT_ID)
		return __LINE__;

	return value;
}

extern "C" {

/* public: setup.h */
int compiler_test_cxx(int value)
{
	return cxx_test(value);
}

}
