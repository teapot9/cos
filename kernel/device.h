#ifndef KERNEL_DEVICE_H
#define KERNEL_DEVICE_H

struct dev_list {
	struct device * dev;
	struct dev_list * next;
};

#endif // KERNEL_DEVICE_H
