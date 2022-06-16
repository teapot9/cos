#include "acpica/actbl.h"
#include "acpica/acpiosxf.h"

#include <stddef.h>

#include "acpi.h"
#include "acpica/acexcep.h"
#include <alloc.h>

ACPI_STATUS AcpiOsInitialize(void)
{
	return AE_OK;
}

ACPI_STATUS AcpiOsTerminate(void)
{
	return AE_OK;
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer(void)
{
	return (ACPI_PHYSICAL_ADDRESS) acpi_rsdp_phy();
}

ACPI_STATUS AcpiOsPredefinedOverride(
	_unused_ const ACPI_PREDEFINED_NAMES * PredefinedObject,
	ACPI_STRING * NewValue
)
{
	*NewValue = NULL;
	return AE_OK;
}

ACPI_STATUS AcpiOsTableOverride(
	_unused_ ACPI_TABLE_HEADER *ExistingTable, ACPI_TABLE_HEADER **NewTable
)
{
	*NewTable = NULL;
	return AE_OK;
}

void * AcpiOsMapMemory(
	ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length
)
{
	return mmap((void *) PhysicalAddress, Length);
}

void AcpiOsUnmapMemory(void *where, ACPI_SIZE length)
{
	vunmap(where, length);
}

ACPI_STATUS AcpiOsGetPhysicalAddress(
	void * LogicalAddress, ACPI_PHYSICAL_ADDRESS * PhysicalAddress
)
{
	if (PhysicalAddress == NULL || LogicalAddress == NULL)
		return AE_BAD_PARAMETER;
	void * paddr = virt_to_phys(LogicalAddress);
	if (paddr == NULL)
		return AE_ERROR;
	*PhysicalAddress = (ACPI_PHYSICAL_ADDRESS) paddr;
}

void * AcpiOsAllocate(ACPI_SIZE Size)
{
	return malloc(Size);
}

void AcpiOsFree(void * Memory)
{
	kfree(Memory);
}

BOOLEAN AcpiOsReadable(void *Memory, ACPI_SIZE Length)
{
	// TODO
	return TRUE;
}

BOOLEAN AcpiOsWritable(void * Memory, ACPI_SIZE Length)
{
	// TODO
	return TRUE;
}

ACPI_THREAD_ID AcpiOsGetThreadId()
{
	// TODO
	return 0;
}

ACPI_STATUS AcpiOsExecute(
	ACPI_EXECUTE_TYPE Type,
	ACPI_OSD_EXEC_CALLBACK Function,
	void * Context
)
{
	// TODO
	return AE_SUPPORT;
}

void AcpiOsSleep(UINT64 Milliseconds)
{
	// TODO
	unsigned long long i = 0;
	while (i++ < Milliseconds * 1000000);
}

void AcpiOsStall(UINT32 Microseconds)
{
	// TODO
	unsigned long long i = 0;
	while (i++ < Microseconds * 1000);
}

/*
ACPI_STATUS AcpiOsCreateMutex(ACPI_MUTEX * OutHandle)
{
	int * m = malloc(sizeof(*m));
	if (m == NULL)
		return AE_NO_MEMORY;
	*m = 0;
	*OutHandle = m;
	return AE_OK;
}

void AcpiOsDeleteMutex(ACPI_MUTEX Handle)
{
	kfree(Handle);
}

ACPI_STATUS AcpiOsAcquireMutex(ACPI_MUTEX Handle, UINT16 Timeout)
{
	int * m = Handle;
	while (handle);
	*m = 1;
	return AE_OK;
}

void AcpiOsReleaseMutex(ACPI_MUTEX Handle)
{
	int * m = Handle;
	*m = 0;
}
*/

ACPI_STATUS AcpiOsCreateSemaphore(
	UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE * OutHandle
)
{
}
