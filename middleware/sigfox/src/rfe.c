/*
 * rfe.c
 *
 *  Created on: 13 jan. 2025
 *      Author: Ludo
 */

#include "rfe.h"

#include "error.h"
#include "error_base.h"
#include "gpio.h"
#include "mcu_mapping.h"
#include "s2lp.h"
#include "sky66423.h"
#include "sx126x.h"
#include "types.h"

#ifdef UHFM

/*** RFE local macros ***/

#ifdef HW1_0
#define RFE_RX_GAIN_DB              15
#endif
#ifdef HW2_0
#define RFE_RX_LNA_GAIN_DB          SKY66423_RX_LNA_GAIN_DB
#define RFE_RX_FILTER_LOSS_DB       3
#define RFE_TX_POWER_TABLE_SIZE     (RFE_RF_OUTPUT_POWER_MAX - SX126X_OUTPUT_POWER_MAX)
#endif

/*** RFE local structures ***/

#ifdef HW2_0
/*******************************************************************/
typedef struct {
    uint8_t rx_lna_enable;
    uint8_t rx_filter_enable;
} RFE_context_t;
#endif

/*** RFE local global variables ***/

#ifdef HW2_0
static RFE_context_t rfe_ctx;
#endif

#ifdef HW2_0
static const int8_t RFE_TRANSCEIVER_TX_POWER_DBM[RFE_TX_POWER_TABLE_SIZE] = { (-10), (-9), (-8), (-7), (-6), (-4), (-2), (0), (3), (3), (3), (3) };
#endif

/*** RFE functions ***/

/*******************************************************************/
RFE_status_t RFE_init(void) {
    // Local variables.
    RFE_status_t status = RFE_SUCCESS;
    // Configure GPIOs.
#ifdef HW1_0
    GPIO_configure(&GPIO_RF_TX_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_RF_RX_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
#ifdef HW2_0
    GPIO_configure(&GPIO_RF_TX_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_RF_PA_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_RF_FLT_DISABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_RF_SKY_CTX, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_RF_SKY_CPS, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_RF_SKY_PATH, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_RF_SW_V1, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_RF_SW_V2, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_LED_TX, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_LED_RX, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    // Init context.
    rfe_ctx.rx_lna_enable = 1;
    rfe_ctx.rx_filter_enable = 1;
#endif
    return status;
}

/*******************************************************************/
RFE_status_t RFE_de_init(void) {
    // Local variables.
    RFE_status_t status = RFE_SUCCESS;
    RFE_status_t rfe_status = RFE_SUCCESS;
    RFE_configuration_t rfe_config;
    // Reset path.
    rfe_config.path = RFE_PATH_NONE;
    rfe_config.expected_tx_power_dbm = 0;
#ifdef HW2_0
    rfe_config.rf_frequency_hz = 0;
    rfe_config.rx_lna_enable = 0;
    rfe_config.rx_filter_enable = 0;
#endif
    // Set all pins to output low.
    rfe_status = RFE_set_path(&rfe_config, NULL);
    RFE_stack_error(ERROR_BASE_RFE);
    return status;
}

/*******************************************************************/
RFE_status_t RFE_set_path(RFE_configuration_t* configuration, int8_t* transceiver_tx_power_dbm) {
    // Local variables.
    RFE_status_t status = RFE_SUCCESS;
    // Check parameters.
    if (configuration == NULL) {
        status = RFE_ERROR_NULL_PARAMETER;
        goto errors;
    }
    if (((configuration->expected_tx_power_dbm) < RFE_RF_OUTPUT_POWER_MIN) || ((configuration->expected_tx_power_dbm) > RFE_RF_OUTPUT_POWER_MAX)) {
        status = RFE_ERROR_TX_POWER_RANGE;
        goto errors;
    }
    // Set same output power by default.
    if (transceiver_tx_power_dbm != NULL) {
       (*transceiver_tx_power_dbm) = (configuration->expected_tx_power_dbm);
    }
    // Select channel.
    switch (configuration->path) {
    case RFE_PATH_NONE:
#ifdef HW1_0
        GPIO_write(&GPIO_RF_TX_ENABLE, 0);
        GPIO_write(&GPIO_RF_RX_ENABLE, 0);
#endif
#ifdef HW2_0
        GPIO_write(&GPIO_RF_TX_ENABLE, 0);
        GPIO_write(&GPIO_RF_PA_ENABLE, 0);
        GPIO_write(&GPIO_RF_FLT_DISABLE, 0);
        GPIO_write(&GPIO_RF_SKY_CTX, 0);
        GPIO_write(&GPIO_RF_SKY_CPS, 0);
        GPIO_write(&GPIO_RF_SKY_PATH, 0);
        GPIO_write(&GPIO_RF_SW_V1, 0);
        GPIO_write(&GPIO_RF_SW_V2, 0);
        GPIO_write(&GPIO_LED_TX, 0);
        GPIO_write(&GPIO_LED_RX, 0);
        // Update context.
        rfe_ctx.rx_lna_enable = 1;
        rfe_ctx.rx_filter_enable = 1;
#endif
        break;
    case RFE_PATH_TX:
#ifdef HW1_0
        GPIO_write(&GPIO_RF_TX_ENABLE, 1);
#endif
#ifdef HW2_0
        // Check expected power.
        if ((configuration->expected_tx_power_dbm) > SX126X_OUTPUT_POWER_MAX) {
            // Check PA frequency range.
            if (((configuration->rf_frequency_hz) < SKY66423_RF_FREQUENCY_HZ_MIN) || ((configuration->rf_frequency_hz) > SKY66423_RF_FREQUENCY_HZ_MAX)) {
                status = RFE_ERROR_PA_FREQUENCY_RANGE;
                goto errors;
            }
            // Enable PA.
            GPIO_write(&GPIO_RF_PA_ENABLE, 1);
            GPIO_write(&GPIO_RF_SKY_CPS, 1);
            GPIO_write(&GPIO_RF_SW_V1, 1);
            GPIO_write(&GPIO_RF_SW_V2, 0);
            // Update transceiver output power.
            if (transceiver_tx_power_dbm != NULL) {
               (*transceiver_tx_power_dbm) = RFE_TRANSCEIVER_TX_POWER_DBM[(configuration->expected_tx_power_dbm) - SX126X_OUTPUT_POWER_MAX - 1];
            }
        }
        else {
            // Bypass PA.
            GPIO_write(&GPIO_RF_PA_ENABLE, 0);
            GPIO_write(&GPIO_RF_SKY_CPS, 0);
            GPIO_write(&GPIO_RF_SW_V1, 0);
            GPIO_write(&GPIO_RF_SW_V2, 1);
        }
        // Enable TX.
        GPIO_write(&GPIO_RF_TX_ENABLE, 1);
        GPIO_write(&GPIO_RF_FLT_DISABLE, 1);
        GPIO_write(&GPIO_RF_SKY_CTX, 1);
        GPIO_write(&GPIO_RF_SKY_PATH, 1);
        GPIO_write(&GPIO_LED_TX, 1);
        GPIO_write(&GPIO_LED_RX, 0);
#endif
        break;
    case RFE_PATH_RX:
#ifdef HW1_0
        GPIO_write(&GPIO_RF_RX_ENABLE, 1);
#endif
#ifdef HW2_0
        // Check LNA configuration.
        if ((configuration->rx_lna_enable) != 0) {
            // Check LNA frequency range.
            if (((configuration->rf_frequency_hz) < SKY66423_RF_FREQUENCY_HZ_MIN) || ((configuration->rf_frequency_hz) > SKY66423_RF_FREQUENCY_HZ_MAX)) {
                status = RFE_ERROR_LNA_FREQUENCY_RANGE;
                goto errors;
            }
            // Enable LNA.
            GPIO_write(&GPIO_RF_SKY_CTX, 0);
            GPIO_write(&GPIO_RF_SKY_CPS, 0);
            GPIO_write(&GPIO_RF_SKY_PATH, 0);
        }
        else {
            // Bypass LNA.
            GPIO_write(&GPIO_RF_SKY_CTX, 1);
            GPIO_write(&GPIO_RF_SKY_CPS, 0);
            GPIO_write(&GPIO_RF_SKY_PATH, 1);
        }
        // Check filter configuration.
        if ((configuration->rx_filter_enable) != 0) {
            // Enable filter.
            GPIO_write(&GPIO_RF_FLT_DISABLE, 0);
            GPIO_write(&GPIO_RF_SW_V1, 1);
            GPIO_write(&GPIO_RF_SW_V2, 1);
        }
        else {
            // Bypass filter.
            GPIO_write(&GPIO_RF_FLT_DISABLE, 1);
            GPIO_write(&GPIO_RF_SW_V1, 1);
            GPIO_write(&GPIO_RF_SW_V2, 0);
        }
        // Enable RX.
        GPIO_write(&GPIO_RF_TX_ENABLE, 0);
        GPIO_write(&GPIO_RF_PA_ENABLE, 0);
        GPIO_write(&GPIO_LED_TX, 0);
        GPIO_write(&GPIO_LED_RX, 1);
        // Update context.
        rfe_ctx.rx_lna_enable = (configuration->rx_lna_enable);
        rfe_ctx.rx_filter_enable = (configuration->rx_filter_enable);
#endif
        break;
    default:
        status = RFE_ERROR_PATH;
        goto errors;
    }
errors:
    return status;
}

/*******************************************************************/
RFE_status_t RFE_get_rssi(RFE_rssi_type_t rssi_type, int16_t* rssi_dbm) {
    // Local variables.
    RFE_status_t status = RFE_SUCCESS;
#ifdef HW1_0
    S2LP_status_t s2lp_status = S2LP_SUCCESS;
    // Read RSSI.
    s2lp_status = S2LP_get_rssi((S2LP_rssi_t) rssi_type, rssi_dbm);
    S2LP_exit_error(RFE_ERROR_BASE_S2LP);
    // Compensate LNA gain.
    (*rssi_dbm) -= RFE_RX_GAIN_DB;
#endif
#ifdef HW2_0
    SX126X_status_t sx126x_status = SX126X_SUCCESS;
    // Read RSSI.
    sx126x_status = SX126X_get_rssi((SX126X_rssi_t) rssi_type, rssi_dbm);
    SX126X_exit_error(RFE_ERROR_BASE_SX126X);
    // Compensate filter loss.
    if (rfe_ctx.rx_filter_enable != 0) {
        (*rssi_dbm) += RFE_RX_FILTER_LOSS_DB;
    }
    // Compensate LNA gain.
    if (rfe_ctx.rx_lna_enable != 0) {
        (*rssi_dbm) -= RFE_RX_LNA_GAIN_DB;
    }
#endif
errors:
    return status;
}

#endif /* UHFM */
