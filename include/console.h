#ifndef CONSOLE_H
#define CONSOLE_H

struct driver;

// Use a struct kmsg {int lvl; int categ; char * msg};

// Update console at driver initialization

/**
 * @brief Add a console to the list of consoles to write to
 * @param id Console identifier
 * @param update Function to update the console
 * @param reset Function to reset the console
 * @return 0 on success, non-zero on failure
 */
int console_add(void * id, void (* update)(void * id),
                void (* reset)(void * id));

/**
 * @brief Remove a console from the list of consoles to write to
 * @param id Console identifier
 * @return 0 on success, non-zero on failure
 */
int console_remove(void * id);

/**
 * @brief Update consoles text
 */
void console_update(void);

#endif // CONSOLE_H
