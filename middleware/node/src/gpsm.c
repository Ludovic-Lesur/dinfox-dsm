/*
 * gpsm.c
 *
 *  Created on: 8 jun. 2023
 *      Author: Ludo
 */

#include "gpsm.h"

#ifdef GPSM

#include "analog.h"
#include "dsm_flags.h"
#include "error.h"
#include "gps.h"
#include "gpsm_registers.h"
#include "node_register.h"
#include "node_status.h"
#include "power.h"
#include "rtc.h"
#include "swreg.h"
#include "types.h"
#include "una.h"

/*** GPSM local macros ***/

#define GPSM_TIME_TIMEOUT_SECONDS_MIN           30
#define GPSM_TIME_TIMEOUT_SECONDS_MAX           3600
#define GPSM_TIME_TIMEOUT_SECONDS_DEFAULT       120

#define GPSM_GEOLOC_TIMEOUT_SECONDS_MIN         30
#define GPSM_GEOLOC_TIMEOUT_SECONDS_MAX         3600
#define GPSM_GEOLOC_TIMEOUT_SECONDS_DEFAULT     180

#define GPSM_TP_FREQUENCY_HZ_MIN                1
#define GPSM_TP_FREQUENCY_HZ_MAX                10000000
#define GPSM_TP_FREQUENCY_HZ_DEFAULT            1

#define GPSM_TP_DUTY_CYCLE_PERCENT_MIN          0
#define GPSM_TP_DUTY_CYCLE_PERCENT_MAX          100
#define GPSM_TP_DUTY_CYCLE_PERCENT_DEFAULT      50

#ifdef GPSM_ACTIVE_ANTENNA
#define GPSM_FLAG_AAF                           0b1
#else
#define GPSM_FLAG_AAF                           0b0
#endif
#ifdef GPSM_BKEN_FORCED_HARDWARE
#define GPSM_FLAG_BKFH                          0b1
#else
#define GPSM_FLAG_BKFH                          0b0
#endif

/*** GPSM local structures ***/

/*******************************************************************/
typedef union {
    uint8_t all;
    struct {
        unsigned gps_power :1;
        unsigned tpen :1;
        unsigned pwmd :1;
        unsigned pwen :1;
    } __attribute__((scalar_storage_order("big-endian"))) __attribute__((packed));
} GPSM_flags_t;

/*******************************************************************/
typedef struct {
    GPSM_flags_t flags;
    UNA_bit_representation_t bkenst;
} GPSM_context_t;

/*** GPSM local global variables ***/

static GPSM_context_t gpsm_ctx = {
    .flags.all = 0,
    .bkenst = UNA_BIT_ERROR
};

/*** GPSM local functions ***/

/*******************************************************************/
static void _GPSM_power_control(uint8_t state) {
    // Check on transition.
    if ((state != 0) && (gpsm_ctx.flags.gps_power == 0)) {
        // Turn GPS on.
        POWER_enable(POWER_REQUESTER_ID_GPSM, POWER_DOMAIN_GPS, LPTIM_DELAY_MODE_STOP);
    }
    // Check on transition.
    if ((state == 0) && (gpsm_ctx.flags.gps_power != 0)) {
        // Turn GPS off.
        POWER_disable(POWER_REQUESTER_ID_GPSM, POWER_DOMAIN_GPS);
    }
    // Update local flag.
    gpsm_ctx.flags.gps_power = (state == 0) ? 0 : 1;
}

/*******************************************************************/
static NODE_status_t _GPSM_power_request(uint8_t state) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_control_1 = NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_CONTROL_1];
    // Check power mode.
    if ((reg_control_1 & GPSM_REGISTER_CONTROL_1_MASK_PWMD) == 0) {
        // Power managed by the node.
        if ((state == 0) && ((reg_control_1 & (GPSM_REGISTER_CONTROL_1_MASK_TTRG | GPSM_REGISTER_CONTROL_1_MASK_GTRG | GPSM_REGISTER_CONTROL_1_MASK_TPEN)) == 0)) {
            _GPSM_power_control(0);
        }
        if (state != 0) {
            _GPSM_power_control(1);
        }
    }
    else {
        // Power managed by PWEN bit.
        // Rise error only in case of turn on request while PWEN=0.
        if (((reg_control_1 & GPSM_REGISTER_CONTROL_1_MASK_PWEN) == 0) && (state != 0)) {
            status = NODE_ERROR_RADIO_POWER;
            goto errors;
        }
    }
errors:
    return status;
}

/*******************************************************************/
static NODE_status_t _GPSM_ttrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    GPS_status_t gps_status = GPS_SUCCESS;
    GPS_acquisition_status_t gps_acquisition_status = GPS_ACQUISITION_ERROR_LAST;
    GPS_time_t gps_time;
    uint32_t time_fix_duration = 0;
    uint32_t reg_config_0 = NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_CONFIGURATION_0];
    uint32_t* reg_status_1_ptr = &(NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_STATUS_1]);
    uint32_t* reg_time_data_0_ptr = &(NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_TIME_DATA_0]);
    uint32_t* reg_time_data_1_ptr = &(NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_TIME_DATA_1]);
    uint32_t* reg_time_data_2_ptr = &(NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_TIME_DATA_2]);
    uint32_t unused_mask = 0;
    // Reset status flag.
    SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b0, GPSM_REGISTER_STATUS_1_MASK_TFS);
    // Turn GPS on.
    status = _GPSM_power_request(1);
    if (status != NODE_SUCCESS) goto errors;
    // Perform time fix.
    gps_status = GPS_get_time(&gps_time, SWREG_read_field(reg_config_0, GPSM_REGISTER_CONFIGURATION_0_MASK_TIME_TIMEOUT), &time_fix_duration, &gps_acquisition_status);
    GPS_exit_error(NODE_ERROR_BASE_GPS);
    // Check acquisition status.
    if (gps_acquisition_status == GPS_ACQUISITION_SUCCESS) {
        // Update status flag.
        SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b1, GPSM_REGISTER_STATUS_1_MASK_TFS);
        // Fill registers with time data.
        SWREG_write_field(reg_time_data_0_ptr, &unused_mask, (uint32_t) UNA_convert_year(gps_time.year), GPSM_REGISTER_TIME_DATA_0_MASK_YEAR);
        SWREG_write_field(reg_time_data_0_ptr, &unused_mask, (uint32_t) gps_time.month, GPSM_REGISTER_TIME_DATA_0_MASK_MONTH);
        SWREG_write_field(reg_time_data_0_ptr, &unused_mask, (uint32_t) gps_time.date, GPSM_REGISTER_TIME_DATA_0_MASK_DATE);
        SWREG_write_field(reg_time_data_1_ptr, &unused_mask, (uint32_t) gps_time.hours, GPSM_REGISTER_TIME_DATA_1_MASK_HOUR);
        SWREG_write_field(reg_time_data_1_ptr, &unused_mask, (uint32_t) gps_time.minutes, GPSM_REGISTER_TIME_DATA_1_MASK_MINUTE);
        SWREG_write_field(reg_time_data_1_ptr, &unused_mask, (uint32_t) gps_time.seconds, GPSM_REGISTER_TIME_DATA_1_MASK_SECOND);
        SWREG_write_field(reg_time_data_2_ptr, &unused_mask, time_fix_duration, GPSM_REGISTER_TIME_DATA_2_MASK_FIX_DURATION);
    }
    // Turn GPS off is possible.
    status = _GPSM_power_request(0);
    if (status != NODE_SUCCESS) goto errors;
errors:
    // Turn GPS off is possible.
    _GPSM_power_request(0);
    return status;
}

/*******************************************************************/
static NODE_status_t _GPSM_gtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    GPS_status_t gps_status = GPS_SUCCESS;
    GPS_acquisition_status_t gps_acquisition_status = GPS_ACQUISITION_ERROR_LAST;
    GPS_position_t gps_position;
    uint32_t geoloc_fix_duration = 0;
    uint32_t reg_config_0 = NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_CONFIGURATION_0];
    uint32_t* reg_status_1_ptr = &(NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_STATUS_1]);
    uint32_t* reg_geoloc_data_0_ptr = &(NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_GEOLOC_DATA_0]);
    uint32_t* reg_geoloc_data_1_ptr = &(NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_GEOLOC_DATA_1]);
    uint32_t* reg_geoloc_data_2_ptr = &(NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_GEOLOC_DATA_2]);
    uint32_t* reg_geoloc_data_3_ptr = &(NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_GEOLOC_DATA_3]);
    uint32_t unused_mask = 0;
    // Reset status flag.
    SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b0, GPSM_REGISTER_STATUS_1_MASK_GFS);
    // Turn GPS on.
    status = _GPSM_power_request(1);
    if (status != NODE_SUCCESS) goto errors;
    // Perform time fix.
    gps_status = GPS_get_position(&gps_position, SWREG_read_field(reg_config_0, GPSM_REGISTER_CONFIGURATION_0_MASK_GEOLOC_TIMEOUT), &geoloc_fix_duration, &gps_acquisition_status);
    GPS_exit_error(NODE_ERROR_BASE_GPS);
    // Check acquisition status.
    if (gps_acquisition_status == GPS_ACQUISITION_SUCCESS) {
        // Update status flag.
        SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b1, GPSM_REGISTER_STATUS_1_MASK_GFS);
        // Fill registers with geoloc data.
        SWREG_write_field(reg_geoloc_data_0_ptr, &unused_mask, (uint32_t) gps_position.lat_north_flag, GPSM_REGISTER_GEOLOC_DATA_0_MASK_NF);
        SWREG_write_field(reg_geoloc_data_0_ptr, &unused_mask, gps_position.lat_seconds, GPSM_REGISTER_GEOLOC_DATA_0_MASK_SECOND);
        SWREG_write_field(reg_geoloc_data_0_ptr, &unused_mask, (uint32_t) gps_position.lat_minutes, GPSM_REGISTER_GEOLOC_DATA_0_MASK_MINUTE);
        SWREG_write_field(reg_geoloc_data_0_ptr, &unused_mask, (uint32_t) gps_position.lat_degrees, GPSM_REGISTER_GEOLOC_DATA_0_MASK_DEGREE);
        SWREG_write_field(reg_geoloc_data_1_ptr, &unused_mask, (uint32_t) gps_position.long_east_flag, GPSM_REGISTER_GEOLOC_DATA_1_MASK_EF);
        SWREG_write_field(reg_geoloc_data_1_ptr, &unused_mask, gps_position.long_seconds, GPSM_REGISTER_GEOLOC_DATA_1_MASK_SECOND);
        SWREG_write_field(reg_geoloc_data_1_ptr, &unused_mask, (uint32_t) gps_position.long_minutes, GPSM_REGISTER_GEOLOC_DATA_1_MASK_MINUTE);
        SWREG_write_field(reg_geoloc_data_1_ptr, &unused_mask, (uint32_t) gps_position.long_degrees, GPSM_REGISTER_GEOLOC_DATA_1_MASK_DEGREE);
        SWREG_write_field(reg_geoloc_data_2_ptr, &unused_mask, gps_position.altitude, GPSM_REGISTER_GEOLOC_DATA_2_MASK_ALTITUDE);
        SWREG_write_field(reg_geoloc_data_3_ptr, &unused_mask, geoloc_fix_duration, GPSM_REGISTER_GEOLOC_DATA_3_MASK_FIX_DURATION);
    }
    // Turn GPS off is possible.
    status = _GPSM_power_request(0);
    if (status != NODE_SUCCESS) goto errors;
errors:
    // Turn GPS off is possible.
    _GPSM_power_request(0);
    return status;
}

/*******************************************************************/
static NODE_status_t _GPSM_tpen_callback(uint8_t state) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    GPS_status_t gps_status = GPS_SUCCESS;
    GPS_timepulse_configuration_t timepulse_config;
    uint32_t reg_config_0 = NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_CONFIGURATION_0];
    uint32_t reg_config_1 = NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_CONFIGURATION_1];
    // Turn GPS on.
    status = _GPSM_power_request(1);
    if (status != NODE_SUCCESS) goto errors;
    // Set parameters.
    timepulse_config.active = state;
    timepulse_config.frequency_hz = SWREG_read_field(reg_config_0, GPSM_REGISTER_CONFIGURATION_1_MASK_TP_FREQUENCY);
    timepulse_config.duty_cycle_percent = (uint8_t) SWREG_read_field(reg_config_1, GPSM_REGISTER_CONFIGURATION_2_MASK_TP_DUTY_CYCLE);
    // Set timepulse.
    gps_status = GPS_set_timepulse(&timepulse_config);
    GPS_exit_error(NODE_ERROR_BASE_GPS);
    // Turn GPS off is possible.
    status = _GPSM_power_request(0);
    if (status != NODE_SUCCESS) goto errors;
errors:
    // Turn GPS off is possible.
    _GPSM_power_request(0);
    return status;
}

/*** GPSM functions ***/

/*******************************************************************/
NODE_status_t GPSM_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Init context.
    gpsm_ctx.flags.all = 0;
    gpsm_ctx.bkenst = UNA_BIT_ERROR;
    return status;
}

/*******************************************************************/
void GPSM_init_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case GPSM_REGISTER_ADDRESS_FLAGS_1:
        SWREG_write_field(reg_value, &unused_mask, GPSM_FLAG_AAF,  GPSM_REGISTER_FLAGS_1_MASK_AAF);
        SWREG_write_field(reg_value, &unused_mask, GPSM_FLAG_BKFH, GPSM_REGISTER_FLAGS_1_MASK_BKFH);
        break;
#ifdef DSM_NVM_FACTORY_RESET
    case GPSM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_write_field(reg_value, &unused_mask, GPSM_TIME_TIMEOUT_SECONDS,   GPSM_REGISTER_CONFIGURATION_0_MASK_TIME_TIMEOUT);
        SWREG_write_field(reg_value, &unused_mask, GPSM_GEOLOC_TIMEOUT_SECONDS, GPSM_REGISTER_CONFIGURATION_0_MASK_GEOLOC_TIMEOUT);
        break;
    case GPSM_REGISTER_ADDRESS_CONFIGURATION_1:
        SWREG_write_field(reg_value, &unused_mask, GPSM_TIMEPULSE_FREQUENCY_HZ, GPSM_REGISTER_CONFIGURATION_1_MASK_TP_FREQUENCY);
        break;
    case GPSM_REGISTER_ADDRESS_CONFIGURATION_2:
        SWREG_write_field(reg_value, &unused_mask, GPSM_TIMEPULSE_DUTY_CYCLE, GPSM_REGISTER_CONFIGURATION_2_MASK_TP_DUTY_CYCLE);
        break;
#endif
    default:
        break;
    }
}

/*******************************************************************/
void GPSM_refresh_register(uint8_t reg_addr) {
    // Local variables.
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case GPSM_REGISTER_ADDRESS_STATUS_1:
#ifdef GPSM_BKEN_FORCED_HARDWARE
        gpsm_ctx.bkenst = UNA_BIT_FORCED_HARDWARE;
#else
        gpsm_ctx.bkenst = (GPS_get_backup_voltage() == 0) ? UNA_BIT_0 : UNA_BIT_1;
#endif
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) gpsm_ctx.flags.tpen), GPSM_REGISTER_STATUS_1_MASK_TPST);
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) gpsm_ctx.flags.pwen), GPSM_REGISTER_STATUS_1_MASK_PWST);
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) gpsm_ctx.bkenst), GPSM_REGISTER_STATUS_1_MASK_BKENST);
        break;
    default:
        // Nothing to do for other registers.
        break;
    }
}

/*******************************************************************/
NODE_status_t GPSM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t generic_u32 = 0;
    // Check address.
    switch (reg_addr) {
    case GPSM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_secure_field(
            GPSM_REGISTER_CONFIGURATION_0_MASK_TIME_TIMEOUT,,,
            < GPSM_TIME_TIMEOUT_SECONDS_MIN,
            > GPSM_TIME_TIMEOUT_SECONDS_MAX,
            GPSM_TIME_TIMEOUT_SECONDS_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        SWREG_secure_field(
            GPSM_REGISTER_CONFIGURATION_0_MASK_GEOLOC_TIMEOUT,,,
            < GPSM_GEOLOC_TIMEOUT_SECONDS_MIN,
            > GPSM_GEOLOC_TIMEOUT_SECONDS_MAX,
            GPSM_GEOLOC_TIMEOUT_SECONDS_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    case GPSM_REGISTER_ADDRESS_CONFIGURATION_1:
        SWREG_secure_field(
            GPSM_REGISTER_CONFIGURATION_1_MASK_TP_FREQUENCY,,,
            < GPSM_TP_FREQUENCY_HZ_MIN,
            > GPSM_TP_FREQUENCY_HZ_MAX,
            GPSM_TP_FREQUENCY_HZ_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    case GPSM_REGISTER_ADDRESS_CONFIGURATION_2:
        SWREG_secure_field(
            GPSM_REGISTER_CONFIGURATION_2_MASK_TP_DUTY_CYCLE,,,
            > GPSM_TP_DUTY_CYCLE_PERCENT_MAX,
            > GPSM_TP_DUTY_CYCLE_PERCENT_MAX,
            GPSM_TP_DUTY_CYCLE_PERCENT_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    default:
        break;
    }
    return status;
}

/*******************************************************************/
NODE_status_t GPSM_process_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
#ifndef GPSM_BKEN_FORCED_HARDWARE
    GPS_status_t gps_status = GPS_SUCCESS;
    UNA_bit_representation_t bken = 0;
#endif
    UNA_bit_representation_t pwmd = 0;
    UNA_bit_representation_t pwen = 0;
    UNA_bit_representation_t tpen = 0;
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case GPSM_REGISTER_ADDRESS_CONFIGURATION_1:
    case GPSM_REGISTER_ADDRESS_CONFIGURATION_2:
        // Update timepulse signal if running.
        if (gpsm_ctx.flags.tpen != 0) {
            // Start timepulse with new settings.
            status = _GPSM_tpen_callback(1);
            if (status != NODE_SUCCESS) goto errors;
        }
        break;
    case GPSM_REGISTER_ADDRESS_CONTROL_1:
        // TTRG.
        if ((reg_mask & GPSM_REGISTER_CONTROL_1_MASK_TTRG) != 0) {
            // Read bit.
            if (SWREG_read_field((*reg_ptr), GPSM_REGISTER_CONTROL_1_MASK_TTRG) != 0) {
                // Clear request.
                SWREG_write_field(reg_ptr, &unused_mask, 0b0, GPSM_REGISTER_CONTROL_1_MASK_TTRG);
                // Start GPS time fix.
                status = _GPSM_ttrg_callback();
                if (status != NODE_SUCCESS) goto errors;
            }
        }
        // GTRG.
        if ((reg_mask & GPSM_REGISTER_CONTROL_1_MASK_GTRG) != 0) {
            // Read bit.
            if (SWREG_read_field((*reg_ptr), GPSM_REGISTER_CONTROL_1_MASK_GTRG) != 0) {
                // Clear request.
                SWREG_write_field(reg_ptr, &unused_mask, 0b0, GPSM_REGISTER_CONTROL_1_MASK_GTRG);
                // Start GPS geolocation fix.
                status = _GPSM_gtrg_callback();
                if (status != NODE_SUCCESS) goto errors;
            }
        }
        // TPEN.
        if ((reg_mask & GPSM_REGISTER_CONTROL_1_MASK_TPEN) != 0) {
            // Read bit.
            tpen = SWREG_read_field((*reg_ptr), GPSM_REGISTER_CONTROL_1_MASK_TPEN);
            // Compare to current state.
            if (tpen != gpsm_ctx.flags.tpen) {
                // Start timepulse.
                status = _GPSM_tpen_callback(tpen);
                if (status != NODE_SUCCESS) {
                    // Clear request.
                    SWREG_write_field(reg_ptr, &unused_mask, gpsm_ctx.flags.tpen, GPSM_REGISTER_CONTROL_1_MASK_TPEN);
                    goto errors;
                }
                // Update local flag.
                gpsm_ctx.flags.tpen = tpen;
            }
        }
        // PWMD.
        if ((reg_mask & GPSM_REGISTER_CONTROL_1_MASK_TPEN) != 0) {
            // Read bit.
            pwmd = SWREG_read_field((*reg_ptr), GPSM_REGISTER_CONTROL_1_MASK_PWMD);
            // Check PWMD bit change.
            if ((pwmd != 0) && (gpsm_ctx.flags.pwmd == 0)) {
                // Apply PWEN bit.
                _GPSM_power_control(pwen);
                // Update local flag.
                gpsm_ctx.flags.pwmd = pwmd;
            }
            // Check PWMD bit change.
            if ((pwmd == 0) && (gpsm_ctx.flags.pwmd != 0)) {
                // Try turning GPS off.
                status = _GPSM_power_request(0);
                if (status != NODE_SUCCESS) goto errors;
                // Update local flag.
                gpsm_ctx.flags.pwmd = pwmd;
            }
        }
        // PWEN.
        if ((reg_mask & GPSM_REGISTER_CONTROL_1_MASK_PWEN) != 0) {
            // Check control mode.
            if (pwmd != 0) {
                // Read bit.
                pwen = SWREG_read_field((*reg_ptr), GPSM_REGISTER_CONTROL_1_MASK_PWEN);
                // Compare to current state.
                if (pwen != gpsm_ctx.flags.pwen) {
                    // Apply PWEN bit.
                    _GPSM_power_control(pwen);
                    // Update local flag.
                    gpsm_ctx.flags.pwen = pwen;
                }
            }
            else {
                status = NODE_ERROR_FORCED_SOFTWARE;
                goto errors;
            }
        }
        // BKEN.
        if ((reg_mask & GPSM_REGISTER_CONTROL_1_MASK_BKEN) != 0) {
            // Check pin mode.
#ifdef GPSM_BKEN_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
            // Read bit.
            bken = SWREG_read_field((*reg_ptr), GPSM_REGISTER_CONTROL_1_MASK_BKEN);
            // Compare to current state.
            if (bken != gpsm_ctx.bkenst) {
                // Set backup voltage.
                gps_status = GPS_set_backup_voltage(bken);
                GPS_exit_error(NODE_ERROR_BASE_GPS);
            }
#endif
        }
        break;
    default:
        // Nothing to do for other registers.
        break;
    }
errors:
    GPSM_refresh_register(GPSM_REGISTER_ADDRESS_STATUS_1);
    return status;
}

/*******************************************************************/
NODE_status_t GPSM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    uint32_t* reg_analog_data_1_ptr = &(NODE_RAM_REGISTER[GPSM_REGISTER_ADDRESS_ANALOG_DATA_1]);
    int32_t adc_data = 0;
    uint32_t unused_mask = 0;
    // Reset data.
    (*reg_analog_data_1_ptr) = NODE_REGISTER[GPSM_REGISTER_ADDRESS_ANALOG_DATA_1].error_value;
    // Turn GPS on.
    status = _GPSM_power_request(1);
    if (status != NODE_SUCCESS) goto errors;
    // GPS voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VGPS_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(adc_data), GPSM_REGISTER_ANALOG_DATA_1_MASK_VGPS);
    // Active antenna voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VANT_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(adc_data), GPSM_REGISTER_ANALOG_DATA_1_MASK_VANT);
    // Turn GPS off is possible.
    status = _GPSM_power_request(0);
    if (status != NODE_SUCCESS) goto errors;
errors:
    _GPSM_power_request(0);
    return status;
}

#endif /* GPSM */
