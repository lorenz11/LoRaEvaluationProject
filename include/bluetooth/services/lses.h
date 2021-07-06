
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

/** @brief Set a global indicator for the ble connection
 *
 * This sets a global indicator for the ble connection
 *
 *  @param connected indicator.
 */
void set_connected(bool connected);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_LSES_H_ */
