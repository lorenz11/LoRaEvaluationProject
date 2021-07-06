
#ifndef ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_LSES_H_
#define ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_LSES_H_

/**
 * @brief Lora Send Evaluation Service (LSES)
 * @defgroup bt_lses Lora Send Evaluation Service (LSES)
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
 *  @param commstat indicator for successful send/receive.
 *
 *  @return Zero in case of success and error code in case of error.
 */
int bt_lses_notify_obs(uint16_t commstat);

#ifdef __cplusplus
}
#endif

bool bt_lses_connectedddd;

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_LSES_H_ */
