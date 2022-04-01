#include <device.h>
#include "device.h"

#include <errno.h>
#include <stddef.h>
#include <stdbool.h>

#include <string.h>
#include <mm.h>
#include <print.h>

static struct dev_list * devices = NULL;

static bool is_class(const char * class)
{
#define _is_class(str) (!strcmp(str, class))
	if (
		_is_class("firmware")
		|| _is_class("graphics")
		|| _is_class("virtual")
		|| _is_class("tty")
	)
		return true;
	return false;
}

static struct dev_list ** find_prec_dev(const struct device * dev)
{
	struct dev_list ** pcur = &devices;
	while (*pcur != NULL && (*pcur)->dev != dev)
		pcur = &(*pcur)->next;
	return pcur;
}

static inline bool is_reg(const struct device * dev)
{
	return *find_prec_dev(dev) != NULL;
}

static int add_dev(struct device * dev)
{
	struct dev_list * node = kmalloc(sizeof(*devices));
	if (node == NULL) {
		pr_err("Cannot alocate memory for struct dev_list\n",
		       0);
		return -ENOMEM;
	}
	node->dev = dev;
	if (devices == NULL) {
		node->next = NULL;
		devices = node;
	} else {
		node->next = devices;
		devices = node;
	}
	dev->available = true;
	return 0;
}

static int del_dev(const struct device * dev)
{
	struct dev_list ** pcur = find_prec_dev(dev);
	if (*pcur == NULL)
		return -ENOENT;
	(*pcur)->dev->available = false;
	*pcur = (*pcur)->next;
	kfree(dev);
	return 0;
}

/* public: device.h */
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
)
{
	va_list ap;
	va_start(ap, name_fmt);
	int ret = device_create_varg(dst, owner, parent, class, type, reg,
				     unreg, driver_data, name_fmt, ap);
	va_end(ap);
	return ret;
}

/* public: device.h */
int device_create_varg(
	const struct device ** dst,
	const struct module * owner,
	const struct device * parent,
	const char * class,
	const char * type,
	int (*reg)(const struct device * dev),
	void (*unreg)(const struct device * dev),
	void * driver_data,
	const char * name_fmt,
	va_list ap
)
{
	int err;
	if (dst != NULL)
		*dst = NULL;

	if (owner == NULL)
		return -EINVAL;
	if (name_fmt == NULL)
		return -EINVAL;
	if (parent != NULL && !is_reg(parent))
		return -ENOENT;
	if (!is_class(class))
		return -EINVAL;
	if (type == NULL)
		return -EINVAL;

	char * name;
	if (!vasprintf(&name, name_fmt, ap))
		return -ENOMEM;
	if (device_get(name) != NULL) {
		kfree(name);
		return -EBUSY;
	}

	struct device * dev = kmalloc(sizeof(*dev));
	if (dev == NULL) {
		kfree(name);
		return -ENOMEM;
	}
	dev->available = false;
	dev->owner = owner;
	dev->name = name;
	dev->parent = parent;
	dev->class = class;
	dev->type = type;
	dev->reg = reg;
	dev->unreg = unreg;
	dev->driver_data = driver_data;

	if ((err = add_dev(dev))) {
		kfree(dev);
		return err;
	}

	if (dev->reg != NULL && (err = dev->reg(dev))) {
		del_dev(dev);
		return err;
	}
	if (dst != NULL)
		*dst = dev;
	return 0;
}

/* public: device.h */
void device_delete(const struct device * dev)
{
	int err;

	if ((err = del_dev(dev)))
		pr_err("Failed to delete device %s, errno = %d\n",
		       dev->name, err);

	device_foreach_child(dev, device_delete);

	if (dev->unreg != NULL)
		dev->unreg(dev);
}

/* public: device.h */
const struct device * device_get(const char * name)
{
	struct dev_list * cur = devices;
	while (cur != NULL && strcmp(cur->dev->name, name))
		cur = cur->next;

	if (strcmp(cur->dev->name, name))
		return NULL;
	return cur->dev;
}

/* public: device.h */
void device_foreach_child(const struct device * dev,
                          void (*callback)(const struct device * child))
{
	struct dev_list * cur = devices;
	while (cur != NULL) {
		if (cur->dev->parent == dev)
			callback(cur->dev);
		cur = cur->next;
	}
}

/* public: device.h */
struct device_iter device_iter_init(const char * class, const char * type)
{
	return (struct device_iter) {
		.cur = devices, .class = class, .type = type
	};
}

/* public: device.h */
struct device * device_iter_next(struct device_iter * iter)
{
	struct dev_list * cur = iter->cur;
	while(cur != NULL
	      && (iter->class == NULL || strcmp(cur->dev->class, iter->class))
	      && (iter->type == NULL || strcmp(cur->dev->type, iter->type)))
		cur = cur->next;
	if (cur == NULL)
		return NULL;
	iter->cur = cur->next;
	return cur->dev;
}
