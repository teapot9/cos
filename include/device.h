/**
 * @file device.h
 * @brief Device data structure
 */

#ifndef DEVICE_H
#define DEVICE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>

struct dev_list;

/**
 * @brief Device iterator structure
 */
struct device_iter {
	/// Current device
	struct dev_list * cur;
	/// Filter over device class
	const char * class;
	/// Filter over device type
	const char * type;
};

/**
 * @brief Device
 */
struct device {
	/// Device is available (if false, no functionality is available)
	bool available;
	/// Module owning the device
	const struct module * owner;
	/// Name in kernel
	const char * name;
	/// Parent device
	const struct device * parent;
	/// Class of device
	const char * class;
	/// Type of device
	const char * type;
	/// Callback after the device has been created
	int (*reg)(const struct device *);
	/// Callback before removing the device
	void (*unreg)(const struct device *);
	/// Driver-specific data
	void * driver_data;
};

/**
 * @brief Create a new device
 * @param dst If not NULL, a pointer to the created device will be stored
 * @param owner Owner of the device
 * @param parent Parent device
 * @param class Device class
 * @param type Device type
 * @param reg Called at the end of device creation
 * @param unreg Calleed before deleting the device
 * @param driver_data Driver-specific data
 * @param name_fmt printf-like format string for device name
 * @return errno
 */
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

/**
 * @brief Create a new device
 * @param dst If not NULL, a pointer to the created device will be stored
 * @param owner Owner of the device
 * @param parent Parent device
 * @param class Device class
 * @param type Device type
 * @param reg Called at the end of device creation
 * @param unreg Calleed before deleting the device
 * @param driver_data Driver-specific data
 * @param name_fmt printf-like format string for device name
 * @param ap Varargs
 * @return errno
 */
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

/**
 * @brief Delete a device
 * @param dev Device
 */
void device_delete(const struct device * dev);

/**
 * @brief Get device by name
 * @param name Name
 * @return Pointer to found device (NULL if not found)
 */
const struct device * device_get(const char * name);

/**
 * @brief Call a function over each of a device's children
 * @param dev Device
 * @param callback Function to call
 */
void device_foreach_child(const struct device * dev,
                          void (*callback)(const struct device * child));

/**
 * @brief Iterate over devices
 * @param class Filter over device class
 * @param type Filter over device type
 * @return Device iterator structure
 */
struct device_iter device_iter_init(const char * class, const char * type);

/**
 * @brief Next device in iterator
 * @param iter Device iterator
 * @return Next device pointer
 */
struct device * device_iter_next(struct device_iter * iter);

/**
 * @brief Get kernel core module
 * @return struct module for kernel core components
 */
const struct module * core_module(void);

#ifdef __cplusplus
}
#endif
#endif // DEVICE_H
