
#ifndef ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_LRES_H_
#define ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_LRES_H_

/**
 * @brief Lora Receive Evaluation Service (LRES)
 * @defgroup bt_lres Lora Receive Evaluation Service (LRES)
 * @ingroup bluetooth
 * @{
 *
 * [Experimental] Users should note that the APIs can change
 * as a part of ongoing development.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Notify LoRa communication events.
 *
 * This will send a GATT notification to all current subscribers.
 *
 *  @param data indicator for successful send/receive.
 *
 *  @return Zero in case of success and error code in case of error.
 */
int bt_lres_notify(const void *data, uint8_t type_of_notification);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_LRES_H_ */
