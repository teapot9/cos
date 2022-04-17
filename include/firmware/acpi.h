#ifndef FIRMWARE_ACPI_H
#define FIRMWARE_ACPI_H
#ifdef __cplusplus
extern "C" {
#endif

void acpi_register_rsdp(void * paddr);

#ifdef __cplusplus
}
#endif
#endif // FIRMWARE_ACPI_H
