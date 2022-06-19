#ifndef _ACPICA_INCLUDE_PLATFORM_ENV_H
#define _ACPICA_INCLUDE_PLATFORM_ENV_H

#include <kconfig.h>
#include <stdarg.h>
#include <stdint.h>

#if IS_ENABLED(CONFIG_64BIT)
# define ACPI_MACHINE_WIDTH 64
#else
# define ACPI_MACHINE_WIDTH 32
#endif

#endif // _ACPICA_INCLUDE_PLATFORM_ENV_H
