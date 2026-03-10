/*
 * rfe.h
 *
 *  Created on: 13 jan. 2025
 *      Author: Ludo
 */

#ifndef __RFE_H__
#define __RFE_H__

#include "types.h"
#include "error.h"
#include "s2lp.h"
#include "sky66423.h"
#include "sx126x.h"

/*** RFE macros ***/

#ifdef HW1_0
#define RFE_RF_FREQUENCY_HZ_MIN         S2LP_RF_FREQUENCY_HZ_MIN
#define RFE_RF_FREQUENCY_HZ_MAX         S2LP_RF_FREQUENCY_HZ_MAX
#define RFE_RF_OUTPUT_POWER_DBM_MIN     S2LP_RF_OUTPUT_POWER_DBM_MIN
#define RFE_RF_OUTPUT_POWER_DBM_MAX     S2LP_RF_OUTPUT_POWER_DBM_MAX
#endif
#ifdef HW2_0
#define RFE_RF_FREQUENCY_HZ_MIN         SX126X_RF_FREQUENCY_HZ_MIN
#define RFE_RF_FREQUENCY_HZ_MAX         SX126X_RF_FREQUENCY_HZ_MAX
#define RFE_RF_OUTPUT_POWER_DBM_MIN     SX126X_RF_OUTPUT_POWER_DBM_MIN
#define RFE_RF_OUTPUT_POWER_DBM_MAX     SKY66423_RF_OUTPUT_POWER_DBM_MAX
#endif

/*** RFE structures ***/

/*!******************************************************************
 * \enum RFE_status_t
 * \brief Radio front-end driver error codes.
 *******************************************************************/
typedef enum {
    // Driver errors.
    RFE_SUCCESS = 0,
    RFE_ERROR_NULL_PARAMETER,
    RFE_ERROR_TX_POWER_RANGE,
    RFE_ERROR_PATH,
    RFE_ERROR_PA_FREQUENCY_RANGE,
    RFE_ERROR_LNA_FREQUENCY_RANGE,
    // Low level drivers errors.
    RFE_ERROR_BASE_S2LP = ERROR_BASE_STEP,
    RFE_ERROR_BASE_SX126X = (RFE_ERROR_BASE_S2LP + S2LP_ERROR_BASE_LAST),
    // Last base value.
    RFE_ERROR_BASE_LAST = (RFE_ERROR_BASE_SX126X + SX126X_ERROR_BASE_LAST)
} RFE_status_t;

#ifdef UHFM

/*!******************************************************************
 * \enum RFE_path_t
 * \brief Radio front-end paths list.
 *******************************************************************/
typedef enum {
    RFE_PATH_NONE = 0,
    RFE_PATH_TX,
    RFE_PATH_RX,
    RFE_PATH_LAST
} RFE_path_t;

/*!******************************************************************
 * \enum RFE_rssi_type_t
 * \brief Radio RSSI types.
 *******************************************************************/
typedef enum {
    RFE_RSSI_TYPE_RUN,
    RFE_RSSI_TYPE_SYNC_WORD,
    RFE_RSSI_TYPE_LAST
} RFE_rssi_type_t;

/*!******************************************************************
 * \struct RFE_configuration_t
 * \brief Radio front-end configuration.
 *******************************************************************/
typedef struct {
    RFE_path_t path;
    int8_t expected_tx_power_dbm;
#ifdef HW2_0
    uint32_t rf_frequency_hz;
    uint8_t rx_lna_enable;
    uint8_t rx_filter_enable;
#endif
} RFE_configuration_t;

/*** RFE functions ***/

/*!******************************************************************
 * \fn RFE_status_t RFE_init(void)
 * \brief Init radio front-end interface.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
RFE_status_t RFE_init(void);

/*!******************************************************************
 * \fn RFE_status_t RFE_de_init(void)
 * \brief Release radio front-end interface.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
RFE_status_t RFE_de_init(void);

/*!******************************************************************
 * \fn RFE_status_t RFE_set_path(RFE_configuration_t* configuration, int8_t* transceiver_tx_power_dbm)
 * \brief Select active radio path.
 * \param[in]   configuration: Pointer to the RF front-end configuration structure.
 * \param[out]  transceiver_tx_power_dbm: Effective TX power to program on the transceiver.
 * \retval      Function execution status.
 *******************************************************************/
RFE_status_t RFE_set_path(RFE_configuration_t* configuration, int8_t* transceiver_tx_power_dbm);

/*!******************************************************************
 * \fn RFE_status_t RFE_get_rssi(RFE_rssi_type_t rssi_type, int16_t* rssi_dbm)
 * \brief Get calibrated RSSI at board connector.
 * \param[in]   rssi_type: RSSI type to read.
 * \param[out]  rssi_dbm: Pointer to signed 16-bits value that will contain the RSSI in dBm.
 * \retval      Function execution status.
 *******************************************************************/
RFE_status_t RFE_get_rssi(RFE_rssi_type_t rssi_type, int16_t* rssi_dbm);

/*******************************************************************/
#define RFE_exit_error(base) { ERROR_check_exit(rfe_status, RFE_SUCCESS, base) }

/*******************************************************************/
#define RFE_stack_error(base) { ERROR_check_stack(rfe_status, RFE_SUCCESS, base) }

/*******************************************************************/
#define RFE_stack_exit_error(base, code) { ERROR_check_stack_exit(rfe_status, RFE_SUCCESS, base, code) }

#endif /* UHFM */

#endif /* __RFE_H__ */
