/*
 * power.c
 *
 *  Created on: 22 jul. 2023
 *      Author: Ludo
 */

#include "power.h"

#include "analog.h"
#include "digital.h"
#include "error.h"
#include "error_base.h"
#include "gpio.h"
#include "gps.h"
#include "lptim.h"
#include "mcu_mapping.h"
#include "s2lp.h"
#include "sht3x.h"
#include "types.h"

/*** POWER local global variables ***/

static uint32_t power_domain_state[POWER_DOMAIN_LAST] = { [0 ... (POWER_DOMAIN_LAST - 1)] = 0 };

/*** POWER local functions ***/

/*******************************************************************/
#define _POWER_stack_driver_error(driver_status, driver_success, driver_error_base, power_status) { \
    if (driver_status != driver_success) { \
        ERROR_stack_add(driver_error_base + driver_status); \
        ERROR_stack_add(ERROR_BASE_POWER + power_status); \
    } \
}

/*** POWER functions ***/

/*******************************************************************/
void POWER_init(void) {
    // Local variables.
    uint8_t idx = 0;
    // Init context.
    for (idx = 0; idx < POWER_DOMAIN_LAST; idx++) {
        power_domain_state[idx] = 0;
    }
    // Init power control pins.
#if (((defined LVRM) && (defined HW2_0)) || (defined BCM) || (defined BPSM))
    GPIO_configure(&GPIO_MNTR_EN, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
#ifdef GPSM
    GPIO_configure(&GPIO_GPS_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#ifdef GPSM_ACTIVE_ANTENNA
    GPIO_configure(&GPIO_ANT_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
#endif
#ifdef SM
    GPIO_configure(&GPIO_ANALOG_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_DIGITAL_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_SENSORS_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
#ifdef UHFM
    GPIO_configure(&GPIO_TCXO_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    GPIO_configure(&GPIO_RF_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
}

/*******************************************************************/
void POWER_enable(POWER_requester_id_t requester_id, POWER_domain_t domain, LPTIM_delay_mode_t delay_mode) {
    // Local variables.
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    LPTIM_status_t lptim_status = LPTIM_SUCCESS;
#ifdef SM
    DIGITAL_status_t digital_status = DIGITAL_SUCCESS;
    SHT3X_status_t sht3x_status = SHT3X_SUCCESS;
#endif
#ifdef GPSM
    GPS_status_t gps_status = GPS_SUCCESS;
#endif
#ifdef UHFM
    S2LP_status_t s2lp_status = S2LP_SUCCESS;
    RFE_status_t rfe_status = RFE_SUCCESS;
#endif
    uint32_t delay_ms = 0;
    uint8_t action_required = 0;
    // Check parameters.
    if (requester_id >= POWER_REQUESTER_ID_LAST) {
        ERROR_stack_add(ERROR_BASE_POWER + POWER_ERROR_REQUESTER_ID);
        goto errors;
    }
    if (domain >= POWER_DOMAIN_LAST) {
        ERROR_stack_add(ERROR_BASE_POWER + POWER_ERROR_DOMAIN);
        goto errors;
    }
    action_required = ((power_domain_state[domain] == 0) ? 1 : 0);
    // Update state.
    power_domain_state[domain] |= (0b1 << requester_id);
    // Directly exit if this is not the first request.
    if (action_required == 0) goto errors;
    // Check domain.
    switch (domain) {
    case POWER_DOMAIN_ANALOG:
        // Turn analog front-end on.
#if (((defined LVRM) && (defined HW2_0)) || (defined BCM) || (defined BPSM))
        GPIO_write(&GPIO_MNTR_EN, 1);
        delay_ms = POWER_ON_DELAY_MS_ANALOG;
#endif
#if (defined SM) && (defined SM_AIN_ENABLE)
        GPIO_write(&GPIO_ANALOG_POWER_ENABLE, 1);
        delay_ms = POWER_ON_DELAY_MS_ANALOG;
#endif
        // Init attached drivers.
        analog_status = ANALOG_init();
        _POWER_stack_driver_error(analog_status, ANALOG_SUCCESS, ERROR_BASE_ANALOG, POWER_ERROR_DRIVER_ANALOG);
        break;
#ifdef SM
    case POWER_DOMAIN_DIGITAL:
        // Turn digital front-end on.
        GPIO_write(&GPIO_DIGITAL_POWER_ENABLE, 1);
        delay_ms = POWER_ON_DELAY_MS_DIGITAL;
        // Init attached drivers.
        digital_status = DIGITAL_init();
        _POWER_stack_driver_error(digital_status, DIGITAL_SUCCESS, ERROR_BASE_DIGITAL, POWER_ERROR_DRIVER_DIGITAL);
        break;
    case POWER_DOMAIN_SENSORS:
        // Turn digital sensors.
        GPIO_write(&GPIO_SENSORS_POWER_ENABLE, 1);
        delay_ms = POWER_ON_DELAY_MS_SENSORS;
        // Init attached drivers.
        sht3x_status = SHT3X_init();
        _POWER_stack_driver_error(sht3x_status, SHT3X_SUCCESS, ERROR_BASE_SHT30, POWER_ERROR_DRIVER_SHT3X);
        break;
#endif
#ifdef GPSM
    case POWER_DOMAIN_GPS:
        // Turn GPS on.
        GPIO_write(&GPIO_GPS_POWER_ENABLE, 1);
#ifdef GPSM_ACTIVE_ANTENNA
        GPIO_write(&GPIO_ANT_POWER_ENABLE, 1);
#endif
        delay_ms = POWER_ON_DELAY_MS_GPS;
        // Init attached drivers.
        gps_status = GPS_init();
        _POWER_stack_driver_error(gps_status, GPS_SUCCESS, ERROR_BASE_GPS, POWER_ERROR_DRIVER_GPS);
        break;
#endif
#ifdef UHFM
    case POWER_DOMAIN_TCXO:
        // Turn TCXO on.
        GPIO_write(&GPIO_TCXO_POWER_ENABLE, 1);
        delay_ms = POWER_ON_DELAY_MS_TCXO;
        break;
    case POWER_DOMAIN_RADIO:
        // Turn radio on.
        GPIO_write(&GPIO_RF_POWER_ENABLE, 1);
        delay_ms = POWER_ON_DELAY_MS_RADIO;
        // Init attached drivers.
        s2lp_status = S2LP_init();
        _POWER_stack_driver_error(s2lp_status, S2LP_SUCCESS, ERROR_BASE_S2LP, POWER_ERROR_DRIVER_S2LP);
        rfe_status = RFE_init();
        _POWER_stack_driver_error(rfe_status, RFE_SUCCESS, ERROR_BASE_RFE, POWER_ERROR_DRIVER_RFE);
        break;
#endif
    default:
        ERROR_stack_add(ERROR_BASE_POWER + POWER_ERROR_DOMAIN);
        goto errors;
    }
    // Power on delay.
    if (delay_ms != 0) {
        lptim_status = LPTIM_delay_milliseconds(delay_ms, delay_mode);
        _POWER_stack_driver_error(lptim_status, LPTIM_SUCCESS, ERROR_BASE_LPTIM, POWER_ERROR_DRIVER_LPTIM);
    }
errors:
    return;
}

/*******************************************************************/
void POWER_disable(POWER_requester_id_t requester_id, POWER_domain_t domain) {
    // Local variables.
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
#ifdef SM
    DIGITAL_status_t digital_status = DIGITAL_SUCCESS;
    SHT3X_status_t sht3x_status = SHT3X_SUCCESS;
#endif
#ifdef GPSM
    GPS_status_t gps_status = GPS_SUCCESS;
#endif
#ifdef UHFM
    S2LP_status_t s2lp_status = S2LP_SUCCESS;
    RFE_status_t rfe_status = RFE_SUCCESS;
#endif
    // Check parameters.
    if (requester_id >= POWER_REQUESTER_ID_LAST) {
        ERROR_stack_add(ERROR_BASE_POWER + POWER_ERROR_REQUESTER_ID);
        goto errors;
    }
    if (domain >= POWER_DOMAIN_LAST) {
        ERROR_stack_add(ERROR_BASE_POWER + POWER_ERROR_DOMAIN);
        goto errors;
    }
    if (power_domain_state[domain] == 0) goto errors;
    // Update state.
    power_domain_state[domain] &= ~(0b1 << requester_id);
    // Directly exit if this is not the last request.
    if (power_domain_state[domain] != 0) goto errors;
    // Check domain.
    switch (domain) {
    case POWER_DOMAIN_ANALOG:
        // Release attached drivers.
        analog_status = ANALOG_de_init();
        _POWER_stack_driver_error(analog_status, ANALOG_SUCCESS, ERROR_BASE_ANALOG, POWER_ERROR_DRIVER_ANALOG);
        // Turn analog front-end off.
#if (((defined LVRM) && (defined HW2_0)) || (defined BCM) || (defined BPSM))
        GPIO_write(&GPIO_MNTR_EN, 0);
#endif
#if (defined SM) && (defined SM_AIN_ENABLE)
        GPIO_write(&GPIO_ANALOG_POWER_ENABLE, 0);
#endif
        break;
#ifdef SM
    case POWER_DOMAIN_DIGITAL:
        // Release attached drivers.
        digital_status = DIGITAL_de_init();
        _POWER_stack_driver_error(digital_status, DIGITAL_SUCCESS, ERROR_BASE_DIGITAL, POWER_ERROR_DRIVER_DIGITAL);
        // Turn digital front-end off.
        GPIO_write(&GPIO_DIGITAL_POWER_ENABLE, 0);
        break;
    case POWER_DOMAIN_SENSORS:
        // Release attached drivers.
        sht3x_status = SHT3X_de_init();
        _POWER_stack_driver_error(sht3x_status, SHT3X_SUCCESS, ERROR_BASE_SHT30, POWER_ERROR_DRIVER_SHT3X);
        // Turn digital sensors off.
        GPIO_write(&GPIO_SENSORS_POWER_ENABLE, 0);
        break;
#endif
#ifdef GPSM
    case POWER_DOMAIN_GPS:
        // Release attached drivers.
        gps_status = GPS_de_init();
        _POWER_stack_driver_error(gps_status, GPS_SUCCESS, ERROR_BASE_GPS, POWER_ERROR_DRIVER_GPS);
        // Turn GPS off.
#ifdef GPSM_ACTIVE_ANTENNA
        GPIO_write(&GPIO_ANT_POWER_ENABLE, 0);
#endif
        GPIO_write(&GPIO_GPS_POWER_ENABLE, 0);
        break;
#endif
#ifdef UHFM
    case POWER_DOMAIN_TCXO:
        // Turn TCXO off.
        GPIO_write(&GPIO_TCXO_POWER_ENABLE, 0);
        break;
    case POWER_DOMAIN_RADIO:
        // Release attached drivers.
        s2lp_status = S2LP_de_init();
        _POWER_stack_driver_error(s2lp_status, S2LP_SUCCESS, ERROR_BASE_S2LP, POWER_ERROR_DRIVER_S2LP);
        rfe_status = RFE_de_init();
        _POWER_stack_driver_error(rfe_status, RFE_SUCCESS, ERROR_BASE_RFE, POWER_ERROR_DRIVER_RFE);
        // Turn radio off.
        GPIO_write(&GPIO_RF_POWER_ENABLE, 0);
        break;
#endif
    default:
        ERROR_stack_add(ERROR_BASE_POWER + POWER_ERROR_DOMAIN);
        goto errors;
    }
errors:
    return;
}

/*******************************************************************/
uint8_t POWER_get_state(POWER_domain_t domain) {
    // Local variables.
    uint8_t state = 0;
    // Check parameters.
    if (domain >= POWER_DOMAIN_LAST) {
        ERROR_stack_add(ERROR_BASE_POWER + POWER_ERROR_DOMAIN);
        goto errors;
    }
    state = (power_domain_state[domain] == 0) ? 0 : 1;
errors:
    return state;
}
