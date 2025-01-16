/*
 * rfe.c
 *
 *  Created on: 13 jan. 2025
 *      Author: Ludo
 */

#include "rfe.h"

#include "error.h"
#include "gpio.h"
#include "gpio_mapping.h"
#include "s2lp.h"
#include "types.h"

/*** RFE local macros ***/

#define RFE_RX_GAIN_DB	12

/*** RFE functions ***/

/*******************************************************************/
RFE_status_t RFE_init(void) {
    // Local variables.
    RFE_status_t status = RFE_SUCCESS;
    // Configure GPIOs.
    GPIO_configure(&GPIO_RF_TX_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_RF_RX_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_write(&GPIO_RF_TX_ENABLE, 0);
    GPIO_write(&GPIO_RF_RX_ENABLE, 0);
    return status;
}

/*******************************************************************/
RFE_status_t RFE_de_init(void) {
    // Local variables.
    RFE_status_t status = RFE_SUCCESS;
    // Set all pins to output low.
    status = RFE_set_path(RFE_PATH_NONE);
    if (status != RFE_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
RFE_status_t RFE_set_path(RFE_path_t radio_path) {
    // Local variables.
    RFE_status_t status = RFE_SUCCESS;
    // Reset channels.
    GPIO_write(&GPIO_RF_TX_ENABLE, 0);
    GPIO_write(&GPIO_RF_RX_ENABLE, 0);
    // Select channel.
    switch (radio_path) {
    case RFE_PATH_NONE:
        // Already done by previous reset.
        break;
    case RFE_PATH_TX:
        GPIO_write(&GPIO_RF_TX_ENABLE, 1);
        break;
#ifdef BIDIRECTIONAL
    case RFE_PATH_RX:
        GPIO_write(&GPIO_RF_RX_ENABLE, 1);
        break;
#endif
    default:
        status = RFE_ERROR_PATH;
        goto errors;
    }
errors:
    return status;
}

#ifdef BIDIRECTIONAL
/*******************************************************************/
RFE_status_t RFE_get_rssi(S2LP_rssi_t rssi_type, int16_t* rssi_dbm) {
    // Local variables.
    RFE_status_t status = RFE_SUCCESS;
    S2LP_status_t s2lp_status = S2LP_SUCCESS;
    // Read raw RSSI.
    s2lp_status = S2LP_get_rssi(rssi_type, rssi_dbm);
    S2LP_exit_error(RFE_ERROR_BASE_S2LP);
    // Apply calibration gain.
    (*rssi_dbm) -= RFE_RX_GAIN_DB;
errors:
    return status;
}
#endif
