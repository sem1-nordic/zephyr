/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/bluetooth/bluetooth.h>
#include "common.h"

extern enum bst_result_t bst_result;

CREATE_FLAG(is_scanned);

#define NUM_ITERATIONS 10

void device_scanned(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
		  struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];
	int err;

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	printk("Device found: %s (RSSI %d)\n", addr_str, rssi);

	printk("Stopping scan\n");
	err = bt_le_scan_stop();
	if (err != 0) {
		FAIL("Could not stop scan: %d");
		return;
	}

	SET_FLAG(is_scanned);
}

static void test_main_scan(void)
{
	int err;

	for (int i = 0; i < NUM_ITERATIONS; i++) {

		err = bt_enable(NULL);
		if (err != 0) {
			FAIL("Bluetooth discover failed (err %d)\n", err);
		}
		printk("Bluetooth initialized\n");

		UNSET_FLAG(is_scanned);

		err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, device_scanned);
		if (err != 0) {
			FAIL("Scanning failed to start (err %d)\n", err);
		}

		printk("Scanning successfully started\n");

		WAIT_FOR_FLAG(is_scanned);

		err = bt_disable();
		if (err != 0) {
			FAIL("Bluetooth disable failed (err %d)\n", err);
		}
		printk("Bluetooth successfully disabled\n");
	}

	PASS("GATT client Passed\n");
}

static const struct bst_test_instance test_scanner[] = {
	{
		.test_id = "scanner",
		.test_post_init_f = test_init,
		.test_tick_f = test_tick,
		.test_main_f = test_main_scan
	},
	BSTEST_END_MARKER
};

struct bst_test_list *test_scanner_install(struct bst_test_list *tests)
{
	return bst_add_tests(tests, test_scanner);
}

static void test_main_adv(void)
{
	int err;
	const struct bt_data ad[] = {
		BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR))
	};

	for (int i = 0; i < NUM_ITERATIONS; i++) {
		struct bt_le_adv_param adv;

		err = bt_enable(NULL);
		if (err != 0) {
			FAIL("Bluetooth init failed (err %d)\n", err);
			return;
		}

		printk("Bluetooth initialized\n");

		int id_1;
		bt_id_get(NULL, &id_1);
		printk("ID: %i\n", id_1);

		int id = bt_id_create(NULL, NULL);
		if (id < 0) {
			FAIL("ID create (id %d)\n", id);
		}

		adv.id = id;
		adv.sid = 0;
		adv.secondary_max_skip = 0;
		adv.options = BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME;
		adv.interval_min = BT_GAP_ADV_FAST_INT_MIN_2;
		adv.interval_max = BT_GAP_ADV_FAST_INT_MAX_2;
		adv.peer = NULL;

		err = bt_le_adv_start(&adv, ad, ARRAY_SIZE(ad), NULL, 0);
		if (err != 0) {
			FAIL("Advertising failed to start (err %d)\n", err);
			return;
		}

		printk("Advertising successfully started\n");

		k_sleep(K_MSEC(10));

		err = bt_le_adv_stop();
		if (err != 0) {
			FAIL("Advertising failed to stop (err %d)\n", err);
			return;
		}

		err = bt_id_delete(id);
		if (err != 0) {
			FAIL("Enable failed (err %d)\n", err);
		}

		err = bt_disable();
		if (err != 0) {
			FAIL("Bluetooth disable failed (err %d)\n", err);
			return;
		}

		printk("Bluetooth disabled\n");
	}

	PASS("GATT server passed\n");
}

static const struct bst_test_instance test_advertiser[] = {
	{
		.test_id = "advertiser",
		.test_post_init_f = test_init,
		.test_tick_f = test_tick,
		.test_main_f = test_main_adv
	},
	BSTEST_END_MARKER
};

struct bst_test_list *test_advertiser_install(struct bst_test_list *tests)
{
	return bst_add_tests(tests, test_advertiser);
}
