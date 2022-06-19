#ifndef _KERNEL_DEVICE_H
#define _KERNEL_DEVICE_H

struct dev_list {
	struct device * dev;
	struct dev_list * next;
};

#endif // _KERNEL_DEVICE_H
