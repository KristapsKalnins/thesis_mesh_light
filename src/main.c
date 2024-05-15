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
#include "model_handler.h"
#include "provisioner_stage.h"
#include "smp_bt.h"


static const uint8_t dev_uuid[16] = { 0xdd, 0xdd, 0xab, 0xac, 0xca, 0xde };

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	.unprovisioned_beacon = provisioner_unprovisioned_beacon_callback,
	.node_added = provisioner_node_added_callback,
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	dk_leds_init();
	dk_buttons_init(NULL);

	err = bt_mesh_init(&prov, model_handler_init());
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

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

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}

	provisioner_search_for_unprovisioned_devices();

	return 0;
}
