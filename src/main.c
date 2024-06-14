/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic mesh light sample
 */
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/dk_prov.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include "model_handler.h"
#include "provisioner_stage.h"
#include "smp_bt.h"


static uint8_t dev_uuid[16] = { 0 };

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	.unprovisioned_beacon = provisioner_unprovisioned_beacon_callback,
	.node_added = provisioner_node_added_callback,
	.oob_info = BT_MESH_PROV_OOB_CERTIFICATE | BT_MESH_PROV_OOB_RECORDS,
	.static_val = "testtesttesttes",
	.static_val_len = 15,
};


void bt_set_unique_uuid(){
	size_t id_len = hwinfo_get_device_id(dev_uuid, sizeof(dev_uuid));

	if (!IS_ENABLED(CONFIG_BT_MESH_DK_LEGACY_UUID_GEN)) {
		/* If device ID is shorter than UUID size, fill rest of buffer with
		 * inverted device ID.
		 */
		for (size_t i = id_len; i < sizeof(dev_uuid); i++) {
			dev_uuid[i] = dev_uuid[i % id_len] ^ 0xff;
		}
	}

	dev_uuid[6] = (dev_uuid[6] & BIT_MASK(4)) | BIT(6);
	dev_uuid[8] = (dev_uuid[8] & BIT_MASK(6)) | BIT(7);
}


static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	dk_leds_init();
	dk_buttons_init(NULL);

	bt_set_unique_uuid();

	err = bt_mesh_init(&prov, model_handler_init());
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT | BT_MESH_PROV_REMOTE);

	printk("Mesh initialized\n");

	if (IS_ENABLED(CONFIG_SOC_SERIES_NRF52X) && IS_ENABLED(CONFIG_MCUMGR_TRANSPORT_BT)) {
		err = smp_dfu_init();
		if (err) {
			printk("Unable to initialize DFU (err %d)\n", err);
		}
	}
}

int main(void)
{
	int err;

	printk("Initializing...\n");
	const struct device *dev;
	int ret;
#ifdef CONFIG_BOARD_NRF52840DONGLE_NRF52840
	dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);
	if (!device_is_ready(dev)) {
		printk("CDC ACM device not ready\n");
		return 0;
	}

	ret = usb_enable(NULL);
#endif
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}

	provisioner_search_for_unprovisioned_devices();

	return 0;
}
