/**
 * @file my_lbs.c
 * @brief LED Button Service
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <zephyr/types.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "my_lbs.h"

LOG_MODULE_REGISTER(LBS_Service, LOG_LEVEL_DBG);

/*
 * UUID
 */

#define UUID_LBS BT_UUID_DECLARE_128(UUID_LBS_VAL)

/// @brief BUTTON Characteristic UUID
#define UUID_LBS_BUTTON_VAL \
    BT_UUID_128_ENCODE(0x00001524, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define UUID_LBS_BUTTON BT_UUID_DECLARE_128(UUID_LBS_BUTTON_VAL)

/// @brief LED Characteristic UUID
#define UUID_LBS_LED_VAL \
    BT_UUID_128_ENCODE(0x00001525, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define UUID_LBS_LED BT_UUID_DECLARE_128(UUID_LBS_LED_VAL)

/// @brief MYSENSOR Characteristic UUID
#define UUID_LBS_MYSENSOR_VAL \
    BT_UUID_128_ENCODE(0x00001526, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define UUID_LBS_MYSENSOR BT_UUID_DECLARE_128(UUID_LBS_MYSENSOR_VAL)


/// @brief BUTTON Characteristic read status
// TODO: Modify
static struct lbs_button_status button_state;
/// @brief BUTTON Characteristic indication flag
static bool indicate_button_enabled;
static struct bt_gatt_indicate_params indicate_button_params;

/// @brief MYSENSOR Characteristic notification flag
static bool notify_mysensor_enabled;


/// @brief service callbacks
static struct lbs_cb lbs_cb;


/**
 * Update BUTTON indication flag.
 */
static void button_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    indicate_button_enabled = (value == BT_GATT_CCC_INDICATE);
    LOG_DBG("BUTTON indication flag: %d", indicate_button_enabled);
}

static void button_indicate_callback(struct bt_conn *conn, struct bt_gatt_indicate_params *params, uint8_t err)
{
	LOG_DBG("Indication BUTTON Characteristic %s", err != 0U ? "fail" : "success");
}

/**
 * Update MYSENSOR notification flag.
 */
static void mysensor_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    notify_mysensor_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_DBG("MYSENSOR notification flag: %d", notify_mysensor_enabled);
}

/**
 * Callback application function triggered by writing to LED Characteristic.
 */
static ssize_t write_led(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags)
{
	LOG_DBG("Attribute write led, handle: %u, conn: %p", attr->handle, (const void *)conn);

    // TODO: Check length
    if (len != 1) {
        LOG_ERR("Write led: Incorrect data length(%u)", len);
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    // TODO: Check offset
    if (offset != 0) {
        LOG_ERR("Write led: Incorrect data offset(%u)", offset);
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

    // TODO: Modify callback
    if (lbs_cb.led_write_cb) {
        int ret = lbs_cb.led_write_cb(buf, len, offset);
        if (ret != 0) {
            LOG_ERR("Write led: callback error happen: %d", ret);
            return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
        }
    }

    return len;
}

/**
 * Callback application function triggered by reading BUTTON Characteristic.
 */
static ssize_t read_button(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset)
{
    // TODO: Modify data
    LOG_DBG("Attribute read button, handle: %u, conn: %p", attr->handle, (const void *)conn);

    // TODO: Modify callback
    if (lbs_cb.button_read_cb) {
        int ret = lbs_cb.button_read_cb(buf, len, offset, &button_state);
        if (ret != 0) {
            LOG_ERR("Read button: callback error happen: %d", ret);
            return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
        }
        return bt_gatt_attr_read(
            conn, attr, buf, len, offset, button_state.serialized, sizeof(button_state.serialized));
    }

    return 0;
}

// LBS Service Declaration
BT_GATT_SERVICE_DEFINE(
    lbs_svc,
    BT_GATT_PRIMARY_SERVICE(UUID_LBS),

    // BUTTON Characteristic
    BT_GATT_CHARACTERISTIC(
        // UUID
        UUID_LBS_BUTTON,
        // Properties
        BT_GATT_CHRC_READ | BT_GATT_CHRC_INDICATE,
        // Permissions
        BT_GATT_PERM_READ,
        // Characteristic Attribute read callback
        read_button,
        // Characteristic Attribute write callback
        NULL,
        // Characteristic Attribute user data(TODO: modify)
        NULL
    ),

    // BUTTON Client Characteristic Configuration Descriptor
    BT_GATT_CCC(
        button_ccc_cfg_changed,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
    ),

    // LED Characteristic
    BT_GATT_CHARACTERISTIC(
        // UUID
        UUID_LBS_LED,
        // Properties
        BT_GATT_CHRC_WRITE,
        // Permissions
        BT_GATT_PERM_WRITE,
        // Characteristic Attribute read callback
        NULL,
        // Characteristic Attribute write callback
        write_led,
        // Characteristic Attribute user data(TODO: modify)
        NULL
    ),

    // MYSENSOR Characteristic
    BT_GATT_CHARACTERISTIC(
        // UUID
        UUID_LBS_MYSENSOR,
        // Properties
        BT_GATT_CHRC_NOTIFY,
        // Permissions
        BT_GATT_PERM_NONE,
        // Characteristic Attribute read callback
        NULL,
        // Characteristic Attribute write callback
        NULL,
        // Characteristic Attribute user data(TODO: modify)
        NULL
    ),

    // MYSENSOR Client Characteristic Configuration Descriptor
    BT_GATT_CCC(
        mysensor_ccc_cfg_changed,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
    ),
);


/*
 * Functions
 */

int lbs_init(struct lbs_cb *callbacks)
{
    lbs_cb = *callbacks;

    // TODO: add your code

    return 0;
}

/// @brief lbs_send_button_indicate sends the value by indication through button characteristic.
// TODO: Modifying parameters
int lbs_send_button_indicate(const uint8_t *data, uint16_t len)
{
    if (!indicate_button_enabled) {
        LOG_ERR("lbs_send_button_indicate: indicate not enabled.");
        return -EACCES;
    }

    // TODO: Modify
    indicate_button_params.attr = &lbs_svc.attrs[2];
    indicate_button_params.func = button_indicate_callback;
    indicate_button_params.destroy = NULL;
    indicate_button_params.data = data;
    indicate_button_params.len = len;
    int ret = bt_gatt_indicate(NULL, &indicate_button_params);
    if (ret != 0) {
        LOG_ERR("lbs_send_button_indicate: fail bt_gatt_indicate(ret=%d).", ret);
    }
    return ret;
}
/// @brief lbs_send_mysensor_notify sends the value by notification through mysensor characteristic.
// TODO: Modifying parameters
int lbs_send_mysensor_notify(const uint8_t *data, uint16_t len)
{
    if (!notify_mysensor_enabled) {
        LOG_ERR("lbs_send_mysensor_notify: notification not enabled.");
        return -EACCES;
    }

    // TODO: Modify
    int ret = bt_gatt_notify(
        NULL,
        &lbs_svc.attrs[7],
        data,
        len);
    if (ret != 0) {
        LOG_ERR("lbs_send_mysensor_notify: fail bt_gatt_notify(ret=%d).", ret);
    }
    return ret;
}

