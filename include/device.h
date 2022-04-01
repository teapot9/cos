#ifndef DEVICE_H
#define DEVICE_H

#include <stdarg.h>
#include <stdbool.h>

struct dev_list;

struct device_iter {
	struct dev_list * cur;
	const char * class;
	const char * type;
};

struct device {
	bool available;
	const struct module * owner;
	const char * name;
	const struct device * parent;
	const char * class;
	const char * type;
	int (*reg)(const struct device *);
	void (*unreg)(const struct device *);
	void * driver_data;
};

int device_create(
	const struct device ** dst,
	const struct module * owner,
	const struct device * parent,
	const char * class,
	const char * type,
	int (*reg)(const struct device *),
	void (*unreg)(const struct device *),
	void * driver_data,
	const char * name_fmt,
	...
);

int device_create_varg(
	const struct device ** dst,
	const struct module * owner,
	const struct device * parent,
	const char * class,
	const char * type,
	int (*reg)(const struct device *),
	void (*unreg)(const struct device *),
	void * driver_data,
	const char * name_fmt,
	va_list ap
);

void device_delete(const struct device * dev);

const struct device * device_get(const char * name);

void device_foreach_child(const struct device * dev,
                          void (*callback)(const struct device * child));

struct device_iter device_iter_init(const char * class, const char * type);

struct device * device_iter_next(struct device_iter * iter);

const struct module * core_module(void);

#endif // DEVICE_H
