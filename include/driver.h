#ifndef DRIVER_H
#define DRIVER_H

/**
 * @brief Driver information structure
 * @var name String containing the name of the driver
 * @var init Initialization function
 */
struct driver {
	const char * const name;
	void (* const init)(void);
};

#endif // DRIVER_H
