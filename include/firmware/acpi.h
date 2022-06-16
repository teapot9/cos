/**
 * @file firmware/acpi.h
 * @brief Advanced Configuration and Power Interface
 */

#ifndef FIRMWARE_ACPI_H
#define FIRMWARE_ACPI_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register RSDP address
 * @param paddr Physical address of RSDP
 */
void acpi_register_rsdp(void * paddr);

#ifdef __cplusplus
}
#endif
#endif // FIRMWARE_ACPI_H
