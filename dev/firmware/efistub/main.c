#define pr_fmt(fmt) "efistub: " fmt

#include <firmware/efistub.h>
#include "efistub.h"
#include <platform_setup.h>

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>

#include <firmware/efiapi/loaded_image.h>
#include <device.h>
#include <module.h>
#include <printk.h>

const struct module efistub_mod = {
	.name = "efistub",
};
const struct device * efiboot_dev = NULL;

static bool is_init = false;
static struct efistub_data stub = {
	.image_handle = NULL, .system_table = NULL, .image_proto = NULL,
};

static efi_loaded_image_protocol_t * get_image(
	efi_handle_t handle, const efi_system_table_t * system_table
)
{
	efi_status_t status;
	efi_guid_t image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	efi_loaded_image_protocol_t * image = NULL;

	status = system_table->boot_services->handle_protocol(
		handle, &image_guid, (void *) &image
	);
	if (status != EFI_SUCCESS) {
		pr_err("Failed to get EFI loaded image protocol handle "
		       "(error %d)\n", status);
		return NULL;
	}
	return image;
}

#ifdef BOOT
static void early_setup(
	efi_handle_t image_handle,
	const efi_system_table_t * system_table
)
#else
static void early_setup(
	efi_handle_t image_handle,
	const efi_system_table_t * system_table,
	const efi_loaded_image_protocol_t * image_proto
)
#endif
{
	if (efistub_is_init())
		return;
	stub.image_handle = image_handle;
	stub.system_table = system_table;
#ifdef BOOT
	stub.image_proto = get_image(image_handle, system_table);
#else
	stub.image_proto = image_proto;
#endif
}

/* public: platform_setup.h */
#ifdef BOOT
int efistub_init(
	struct efistub_data ** data,
	efi_handle_t image_handle,
	const efi_system_table_t * system_table
)
#else
int efistub_init(
	struct efistub_data * data
)
#endif
{
	if (data == NULL)
		return -EINVAL;
	int ret;
	if (efistub_is_init())
		return 0;

#ifdef BOOT
	early_setup(image_handle, system_table);
	*data = &stub;
#else
	early_setup(data->image_handle, data->system_table,
	            data->image_proto);
#endif

	if (stub.image_handle == NULL) {
		pr_err("No EFI image handle provided\n", 0);
		return -EINVAL;
	}
	if (stub.system_table == NULL) {
		pr_err("No EFI system table provided\n", 0);
		return -EINVAL;
	}
	if (stub.image_proto == NULL)
		pr_err("No EFI loaded image protocol available\n", 0);

	ret = device_create(&efiboot_dev, &efistub_mod, NULL, "platform", "api",
	                    NULL, NULL, NULL, "efiboot");
	if (ret)
		return ret;

	is_init = true;

	efistub_parse_configuration_table();
	pr_err("kernel loaded at %p\n", efistub_image_proto()->image_base);

	return 0;
}

bool efistub_is_init(void)
{
	return is_init;
}

/* public: firmware/efistub.h */
efi_handle_t efistub_image_handle(void)
{
	return efistub_is_init() ? stub.image_handle : NULL;
}

/* public: firmware/efistub.h */
const efi_system_table_t * efistub_system_table(void)
{
	return efistub_is_init() ? stub.system_table : NULL;
}

/* public: firmware/efistub.h */
const efi_loaded_image_protocol_t * efistub_image_proto(void)
{
	return efistub_is_init() ? stub.image_proto : NULL;
}
