/*
 * rfe.h
 *
 *  Created on: 13 jan. 2025
 *      Author: Ludo
 */

#ifndef __RFE_H__
#define __RFE_H__

#include "types.h"
#ifndef S2LP_DRIVER_DISABLE_FLAGS_FILE
#include "s2lp_driver_flags.h"
#endif
#include "error.h"
#include "s2lp.h"
#ifndef SIGFOX_EP_DISABLE_FLAGS_FILE
#include "sigfox_ep_flags.h"
#endif

/*** RFE structures ***/

/*!******************************************************************
 * \enum RFE_status_t
 * \brief Radio front-end driver error codes.
 *******************************************************************/
typedef enum {
    // Driver errors.
    RFE_SUCCESS = 0,
    RFE_ERROR_PATH,
    // Low level drivers errors.
    RFE_ERROR_BASE_S2LP = ERROR_BASE_STEP,
    // Last base value.
    RFE_ERROR_BASE_LAST = (RFE_ERROR_BASE_S2LP + S2LP_ERROR_BASE_LAST)
} RFE_status_t;

/*!******************************************************************
 * \enum RFE_path_t
 * \brief Radio front-end paths list.
 *******************************************************************/
typedef enum {
    RFE_PATH_NONE = 0,
    RFE_PATH_TX,
#ifdef SIGFOX_EP_BIDIRECTIONAL
    RFE_PATH_RX,
#endif
    RFE_PATH_LAST
} RFE_path_t;

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
 * \fn RFE_status_t RFE_set_path(RFE_path_t radio_path)
 * \brief Select active radio path.
 * \param[in]   radio_path: Radio line to select.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
RFE_status_t RFE_set_path(RFE_path_t radio_path);

#if ((defined SIGFOX_EP_BIDIRECTIONAL) && !(defined S2LP_DRIVER_DISABLE))
/*!******************************************************************
 * \fn RFE_status_t RFE_get_rssi(S2LP_rssi_t rssi_type, int16_t* rssi_dbm)
 * \brief Get calibrated RSSI at board connector.
 * \param[in]   rssi_type: RSSI type to read.
 * \param[out]  rssi_dbm: Pointer to signed 16-bits value that will contain the RSSI in dBm.
 * \retval      Function execution status.
 *******************************************************************/
RFE_status_t RFE_get_rssi(S2LP_rssi_t rssi_type, int16_t* rssi_dbm);
#endif

/*******************************************************************/
#define RFE_exit_error(base) { ERROR_check_exit(rfe_status, RFE_SUCCESS, base) }

/*******************************************************************/
#define RFE_stack_error(base) { ERROR_check_stack(rfe_status, RFE_SUCCESS, base) }

/*******************************************************************/
#define RFE_stack_exit_error(base, code) { ERROR_check_stack_exit(rfe_status, RFE_SUCCESS, base, code) }

#endif /* __RFE_H__ */
