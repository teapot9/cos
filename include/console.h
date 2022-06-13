/**
 * @file console.h
 * @brief System console devices
 */

#ifndef CONSOLE_H
#define CONSOLE_H
#ifdef __cplusplus
extern "C" {
#endif

struct device;

/**
 * @brief Register a new console
 * @param dev Parent device
 * @param update Callback to update console
 * @param clear Callback to clear console
 * @param enable Callback to enable console
 * @param disable Callback to disable console
 * @return errno
 */
int console_reg(
	const struct device * dev,
	void (*update)(const struct device *),
	void (*clear)(const struct device *),
	int (*enable)(const struct device *),
	void (*disable)(const struct device *)
);

/**
 * @brief Update consoles
 */
void console_update(void);

/**
 * @brief Clear consoles
 */
void console_clear(void);

/**
 * @brief Reset consoles
 */
void console_reset(void);

#ifdef __cplusplus
}
#endif
#endif // CONSOLE_H
