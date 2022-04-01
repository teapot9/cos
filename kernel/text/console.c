#include <console.h>

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>

#define CONSOLE_EARLY_COUNT 4

struct driver;

struct console_id {
	void * id;
	void (* update)(void * id);
	void (* reset)(void * id);
};

static struct console_id early_consoles[CONSOLE_EARLY_COUNT];
static size_t console_count = 0;
static struct console_id * consoles = NULL;

int console_add(void * id, void (* update)(void * id),
                void (* reset)(void * id))
{
	static bool first_call = true;

	if (first_call) {
		first_call = false;
		console_count = 0;
		consoles = early_consoles;
	}

	if (consoles == early_consoles
	    && console_count >= CONSOLE_EARLY_COUNT)
		return -ENOMEM;
	if (consoles != early_consoles)
		return -ENOMEM;

	consoles[console_count].id = id;
	consoles[console_count].update = update;
	consoles[console_count].reset = reset;
	console_count++;
	return 0;
}

int console_remove(void * id)
{
	size_t i = 0;

	while (i < console_count && consoles[i].id != id)
		i++;
	if (i >= console_count)
		return -ENOENT;

	while (i < --console_count)
		consoles[i] = consoles[i + 1];
	return 0;
}

void console_reset(void)
{
	if (consoles == NULL)
		return;

	for (size_t i = 0; i < console_count; i++)
		consoles[i].reset(consoles[i].id);
}

void console_update(void)
{
	if (consoles == NULL)
		return;

	for (size_t i = 0; i < console_count; i++)
		consoles[i].update(consoles[i].id);
}
