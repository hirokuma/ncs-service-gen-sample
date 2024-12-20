/**
 * @file my_lbs.h
 * @brief LED Button Service
 */

#ifndef MY_LBS_H_
#define MY_LBS_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

#include <zephyr/bluetooth/uuid.h>

/*
 * UUID
 *
 *  LBS Service:
 *      00001523-1212-efde-1523-785feabcd123
 *  BUTTON Characteristic UUID:
 *      00001524-1212-efde-1523-785feabcd123
 *  LED Characteristic UUID:
 *      00001525-1212-efde-1523-785feabcd123
 *  MYSENSOR Characteristic UUID:
 *      00001526-1212-efde-1523-785feabcd123
 */

/// @brief LBS Service UUID
#define UUID_LBS_VAL \
    BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)


/*
 * Types
 */


/// @brief Read BUTTON Characteristic callback data
// TODO: Modifying members
struct lbs_button_status {
    // TODO: add your parameters...

    // serialized data for Read request
    uint8_t serialized[1];
};

/// @brief Read callback type for BUTTON Characteristic.
// TODO: Modifying parameters
typedef int (*lbs_button_read_cb_t)(const void *data, uint16_t len, uint16_t offset, struct lbs_button_status *newState);

/// @brief Write callback type for LED Characteristic.
// TODO: Modifying parameters
typedef int (*lbs_led_write_cb_t)(const void *data, uint16_t len, uint16_t offset);


/// @brief Read MYSENSOR Characteristic callback data
// TODO: Modifying members
struct lbs_mysensor_status {
    // TODO: add your parameters...

    // serialized data for Read request
    uint8_t serialized[4];
};

/// @brief Callback struct used by the LBS Service.
struct lbs_cb {
    lbs_button_read_cb_t button_read_cb;
    lbs_led_write_cb_t led_write_cb;
};


/*
 * Functions
 */

/// @brief Initialize the LBS Service.
int lbs_init(const struct lbs_cb *callbacks);

/// @brief lbs_send_button_indicate sends the value by indication through button characteristic.
// TODO: Modifying parameters
int lbs_send_button_indicate(const uint8_t *data, uint16_t len);

/// @brief lbs_send_mysensor_notify sends the value by notification through mysensor characteristic.
// TODO: Modifying parameters
int lbs_send_mysensor_notify(const uint8_t *data, uint16_t len);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MY_LBS_H_
