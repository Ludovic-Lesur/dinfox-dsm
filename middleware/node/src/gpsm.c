/*
 * gpsm.c
 *
 *  Created on: 8 jun. 2023
 *      Author: Ludo
 */

#include "gpsm.h"

#include "analog.h"
#include "error.h"
#include "gps.h"
#include "gpsm_registers.h"
#include "node.h"
#include "power.h"
#include "rtc.h"
#include "swreg.h"
#include "una.h"

/*** GPSM local structures ***/

/*******************************************************************/
typedef union {
	struct {
		unsigned gps_power : 1;
		unsigned tpen : 1;
		unsigned pwmd : 1;
		unsigned pwen : 1;
	};
	uint8_t all;
} GPSM_flags_t;

/*******************************************************************/
typedef struct {
	GPSM_flags_t flags;
	UNA_bit_representation_t bkenst;
} GPSM_context_t;

/*** GPSM local global variables ***/

#ifdef GPSM
static GPSM_context_t gpsm_ctx;
#endif

/*** GPSM local functions ***/

#ifdef GPSM
/*******************************************************************/
static void _GPSM_load_fixed_configuration(void) {
	// Local variables.
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Active antenna flag.
#ifdef GPSM_ACTIVE_ANTENNA
	SWREG_write_field(&reg_value, &reg_mask, 0b1, GPSM_REGISTER_CONFIGURATION_0_MASK_AAF);
#else
	SWREG_write_field(&reg_value, &reg_mask, 0b0, GPSM_REGISTER_CONFIGURATION_0_MASK_AAF);
#endif
	// Backup output control mode.
#ifdef GPSM_BKEN_FORCED_HARDWARE
	SWREG_write_field(&reg_value, &reg_mask, 0b1, GPSM_REGISTER_CONFIGURATION_0_MASK_BKFH);
#else
	SWREG_write_field(&reg_value, &reg_mask, 0b0, GPSM_REGISTER_CONFIGURATION_0_MASK_BKFH);
#endif
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_CONFIGURATION_0, reg_value, reg_mask);
}
#endif

#ifdef GPSM
/*******************************************************************/
static void _GPSM_load_dynamic_configuration(void) {
	// Local variables.
	uint8_t reg_addr = 0;
	uint32_t reg_value = 0;
	// Load configuration registers from NVM.
	for (reg_addr=GPSM_REGISTER_ADDRESS_CONFIGURATION_1 ; reg_addr<GPSM_REGISTER_ADDRESS_STATUS_1 ; reg_addr++) {
		// Read NVM.
		NODE_read_nvm(reg_addr, &reg_value);
		// Write register.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, UNA_REGISTER_MASK_ALL);
	}
}
#endif

#ifdef GPSM
/*******************************************************************/
static void _GPSM_reset_analog_data(void) {
	// Local variables.
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	// Reset fields to error value.
	SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, GPSM_REGISTER_ANALOG_DATA_1_MASK_VGPS);
	SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, GPSM_REGISTER_ANALOG_DATA_1_MASK_VANT);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
}
#endif

#ifdef GPSM
/*******************************************************************/
static void _GPSM_power_control(uint8_t state) {
	// Check on transition.
	if ((state != 0) && (gpsm_ctx.flags.gps_power == 0)) {
		// Turn GPS on.
		POWER_enable(POWER_REQUESTER_ID_NODE, POWER_DOMAIN_GPS, LPTIM_DELAY_MODE_STOP);
	}
	// Check on transition.
	if ((state == 0) && (gpsm_ctx.flags.gps_power != 0)) {
		// Turn GPS off.
		POWER_disable(POWER_REQUESTER_ID_NODE, POWER_DOMAIN_GPS);
	}
	// Update local flag.
	gpsm_ctx.flags.gps_power = (state == 0) ? 0 : 1;
}
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_power_request(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t reg_control_1 = 0;
	// Get power mode.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_CONTROL_1, &reg_control_1);
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
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_ttrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	GPS_status_t gps_status = GPS_SUCCESS;
	GPS_acquisition_status_t gps_acquisition_status = GPS_ACQUISITION_ERROR_LAST;
	GPS_time_t gps_time;
	uint32_t time_fix_duration = 0;
	uint32_t reg_timeout = 0;
	uint32_t reg_status_1 = 0;
	uint32_t reg_status_1_mask = 0;
	uint32_t reg_time_data_0 = 0;
	uint32_t reg_time_data_0_mask = 0;
	uint32_t reg_time_data_1 = 0;
	uint32_t reg_time_data_1_mask = 0;
	uint32_t reg_time_data_2 = 0;
	uint32_t reg_time_data_2_mask = 0;
	// Read status and timeout.
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_STATUS_1, &reg_status_1);
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_CONFIGURATION_1, &reg_timeout);
    // Reset status flag.
    SWREG_write_field(&reg_status_1, &reg_status_1_mask, 0b0, GPSM_REGISTER_STATUS_1_MASK_TFS);
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Perform time fix.
	gps_status = GPS_get_time(&gps_time, SWREG_read_field(reg_timeout, GPSM_REGISTER_CONFIGURATION_1_MASK_TIME_TIMEOUT), &time_fix_duration, &gps_acquisition_status);
	GPS_exit_error(NODE_ERROR_BASE_GPS);
	// Check acquisition status.
	if (gps_acquisition_status == GPS_ACQUISITION_SUCCESS) {
        // Update status flag.
        SWREG_write_field(&reg_status_1, &reg_status_1_mask, 0b1, GPSM_REGISTER_STATUS_1_MASK_TFS);
        // Fill registers with time data.
        SWREG_write_field(&reg_time_data_0, &reg_time_data_0_mask, (uint32_t) UNA_convert_year(gps_time.year), GPSM_REGISTER_TIME_DATA_0_MASK_YEAR);
        SWREG_write_field(&reg_time_data_0, &reg_time_data_0_mask, (uint32_t) gps_time.month, GPSM_REGISTER_TIME_DATA_0_MASK_MONTH);
        SWREG_write_field(&reg_time_data_0, &reg_time_data_0_mask, (uint32_t) gps_time.date, GPSM_REGISTER_TIME_DATA_0_MASK_DATE);
        SWREG_write_field(&reg_time_data_1, &reg_time_data_1_mask, (uint32_t) gps_time.hours, GPSM_REGISTER_TIME_DATA_1_MASK_HOUR);
        SWREG_write_field(&reg_time_data_1, &reg_time_data_1_mask, (uint32_t) gps_time.minutes, GPSM_REGISTER_TIME_DATA_1_MASK_MINUTE);
        SWREG_write_field(&reg_time_data_1, &reg_time_data_1_mask, (uint32_t) gps_time.seconds, GPSM_REGISTER_TIME_DATA_1_MASK_SECOND);
        SWREG_write_field(&reg_time_data_2, &reg_time_data_2_mask, time_fix_duration, GPSM_REGISTER_TIME_DATA_2_MASK_FIX_DURATION);
        // Write registers.
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_TIME_DATA_0, reg_time_data_0, reg_time_data_0_mask);
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_TIME_DATA_1, reg_time_data_1, reg_time_data_1_mask);
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_TIME_DATA_2, reg_time_data_2, reg_time_data_2_mask);
	}
	// Turn GPS off is possible.
	status = _GPSM_power_request(0);
	if (status != NODE_SUCCESS) goto errors;
errors:
    // Update status.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_STATUS_1, reg_status_1, UNA_REGISTER_MASK_ALL);
	// Turn GPS off is possible.
	_GPSM_power_request(0);
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_gtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	GPS_status_t gps_status = GPS_SUCCESS;
    GPS_acquisition_status_t gps_acquisition_status = GPS_ACQUISITION_ERROR_LAST;
	GPS_position_t gps_position;
	uint32_t geoloc_fix_duration = 0;
	uint32_t reg_timeout = 0;
	uint32_t reg_status_1 = 0;
	uint32_t reg_status_1_mask = 0;
	uint32_t reg_geoloc_data_0 = 0;
	uint32_t reg_geoloc_data_0_mask = 0;
	uint32_t reg_geoloc_data_1 = 0;
	uint32_t reg_geoloc_data_1_mask = 0;
	uint32_t reg_geoloc_data_2 = 0;
	uint32_t reg_geoloc_data_2_mask = 0;
	uint32_t reg_geoloc_data_3 = 0;
	uint32_t reg_geoloc_data_3_mask = 0;
	// Read status and timeout.
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_STATUS_1, &reg_status_1);
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_CONFIGURATION_1, &reg_timeout);
    // Reset status flag.
    SWREG_write_field(&reg_status_1, &reg_status_1_mask, 0b0, GPSM_REGISTER_STATUS_1_MASK_GFS);
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Perform time fix.
	gps_status = GPS_get_position(&gps_position, SWREG_read_field(reg_timeout, GPSM_REGISTER_CONFIGURATION_1_MASK_GEOLOC_TIMEOUT), &geoloc_fix_duration, &gps_acquisition_status);
	GPS_exit_error(NODE_ERROR_BASE_GPS);
	// Check acquisition status.
    if (gps_acquisition_status == GPS_ACQUISITION_SUCCESS) {
        // Update status flag.
        SWREG_write_field(&reg_status_1, &reg_status_1_mask, 0b1, GPSM_REGISTER_STATUS_1_MASK_GFS);
        // Fill registers with geoloc data.
        SWREG_write_field(&reg_geoloc_data_0, &reg_geoloc_data_0_mask, (uint32_t) gps_position.lat_north_flag, GPSM_REGISTER_GEOLOC_DATA_0_MASK_NF);
        SWREG_write_field(&reg_geoloc_data_0, &reg_geoloc_data_0_mask, gps_position.lat_seconds, GPSM_REGISTER_GEOLOC_DATA_0_MASK_SECOND);
        SWREG_write_field(&reg_geoloc_data_0, &reg_geoloc_data_0_mask, (uint32_t) gps_position.lat_minutes, GPSM_REGISTER_GEOLOC_DATA_0_MASK_MINUTE);
        SWREG_write_field(&reg_geoloc_data_0, &reg_geoloc_data_0_mask, (uint32_t) gps_position.lat_degrees, GPSM_REGISTER_GEOLOC_DATA_0_MASK_DEGREE);
        SWREG_write_field(&reg_geoloc_data_1, &reg_geoloc_data_1_mask, (uint32_t) gps_position.long_east_flag, GPSM_REGISTER_GEOLOC_DATA_1_MASK_EF);
        SWREG_write_field(&reg_geoloc_data_1, &reg_geoloc_data_1_mask, gps_position.long_seconds, GPSM_REGISTER_GEOLOC_DATA_1_MASK_SECOND);
        SWREG_write_field(&reg_geoloc_data_1, &reg_geoloc_data_1_mask, (uint32_t) gps_position.long_minutes, GPSM_REGISTER_GEOLOC_DATA_1_MASK_MINUTE);
        SWREG_write_field(&reg_geoloc_data_1, &reg_geoloc_data_1_mask, (uint32_t) gps_position.long_degrees, GPSM_REGISTER_GEOLOC_DATA_1_MASK_DEGREE);
        SWREG_write_field(&reg_geoloc_data_2, &reg_geoloc_data_2_mask, gps_position.altitude, GPSM_REGISTER_GEOLOC_DATA_2_MASK_ALTITUDE);
        SWREG_write_field(&reg_geoloc_data_3, &reg_geoloc_data_3_mask, geoloc_fix_duration, GPSM_REGISTER_GEOLOC_DATA_3_MASK_FIX_DURATION);
        // Write registers.
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_GEOLOC_DATA_0, reg_geoloc_data_0, reg_geoloc_data_0_mask);
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_GEOLOC_DATA_1, reg_geoloc_data_1, reg_geoloc_data_1_mask);
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_GEOLOC_DATA_2, reg_geoloc_data_2, reg_geoloc_data_2_mask);
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_GEOLOC_DATA_3, reg_geoloc_data_3, reg_geoloc_data_3_mask);
    }
	// Turn GPS off is possible.
	status = _GPSM_power_request(0);
	if (status != NODE_SUCCESS) goto errors;
errors:
    // Update status.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_STATUS_1, reg_status_1, UNA_REGISTER_MASK_ALL);
	// Turn GPS off is possible.
	_GPSM_power_request(0);
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_tpen_callback(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	GPS_status_t gps_status = GPS_SUCCESS;
	GPS_timepulse_configuration_t timepulse_config;
	uint32_t reg_timepulse_configuration_0 = 0;
	uint32_t reg_timepulse_configuration_1 = 0;
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Read registers.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_CONFIGURATION_2, &reg_timepulse_configuration_0);
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_CONFIGURATION_3, &reg_timepulse_configuration_1);
	// Set parameters.
	timepulse_config.active = state;
	timepulse_config.frequency_hz = SWREG_read_field(reg_timepulse_configuration_0, GPSM_REGISTER_CONFIGURATION_2_MASK_TP_FREQUENCY);
	timepulse_config.duty_cycle_percent = (uint8_t) SWREG_read_field(reg_timepulse_configuration_1, GPSM_REGISTER_CONFIGURATION_3_MASK_TP_DUTY_CYCLE);
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
#endif

/*** GPSM functions ***/

#ifdef GPSM
/*******************************************************************/
NODE_status_t GPSM_init_registers(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef XM_NVM_FACTORY_RESET
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Timeouts.
	SWREG_write_field(&reg_value, &reg_mask, GPSM_TIME_TIMEOUT_SECONDS,   GPSM_REGISTER_CONFIGURATION_1_MASK_TIME_TIMEOUT);
	SWREG_write_field(&reg_value, &reg_mask, GPSM_GEOLOC_TIMEOUT_SECONDS, GPSM_REGISTER_CONFIGURATION_1_MASK_GEOLOC_TIMEOUT);
	NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REGISTER_ADDRESS_CONFIGURATION_1, reg_value, reg_mask);
	// Timepulse settings.
	reg_value = 0;
	reg_mask = 0;
	SWREG_write_field(&reg_value, &reg_mask, GPSM_TIMEPULSE_FREQUENCY_HZ, GPSM_REGISTER_CONFIGURATION_2_MASK_TP_FREQUENCY);
	NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REGISTER_ADDRESS_CONFIGURATION_2, reg_value, reg_mask);
	reg_value = 0;
	reg_mask = 0;
	SWREG_write_field(&reg_value, &reg_mask, GPSM_TIMEPULSE_DUTY_CYCLE, GPSM_REGISTER_CONFIGURATION_3_MASK_TP_DUTY_CYCLE);
	NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REGISTER_ADDRESS_CONFIGURATION_3, reg_value, reg_mask);
#endif
	// Init flags.
	gpsm_ctx.flags.all = 0;
	// Read init state.
	GPSM_update_register(GPSM_REGISTER_ADDRESS_STATUS_1);
	// Load default values.
	_GPSM_load_fixed_configuration();
	_GPSM_load_dynamic_configuration();
	_GPSM_reset_analog_data();
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
NODE_status_t GPSM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Check address.
	switch (reg_addr) {
	case GPSM_REGISTER_ADDRESS_STATUS_1:
		// Timepulse enable.
		SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) gpsm_ctx.flags.tpen), GPSM_REGISTER_STATUS_1_MASK_TPST);
		// Power enable.
		SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) gpsm_ctx.flags.pwen), GPSM_REGISTER_STATUS_1_MASK_PWST);
		// Backup enable.
#ifdef GPSM_BKEN_FORCED_HARDWARE
		gpsm_ctx.bkenst = UNA_BIT_FORCED_HARDWARE;
#else
		gpsm_ctx.bkenst = (GPS_get_backup_voltage() == 0) ? UNA_BIT_0 : UNA_BIT_1;
#endif
		SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) gpsm_ctx.bkenst), GPSM_REGISTER_STATUS_1_MASK_BKENST);
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	// Write register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, reg_mask);
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
NODE_status_t GPSM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
#ifndef GPSM_BKEN_FORCED_HARDWARE
	GPS_status_t gps_status = GPS_SUCCESS;
	UNA_bit_representation_t bken = 0;
#endif
	UNA_bit_representation_t pwmd = 0;
	UNA_bit_representation_t pwen = 0;
	UNA_bit_representation_t tpen = 0;
	uint32_t reg_value = 0;
	uint32_t new_reg_value = 0;
	uint32_t new_reg_mask = 0;
	// Read register.
	status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
	if (status != NODE_SUCCESS) goto errors;
	// Check address.
	switch (reg_addr) {
	case GPSM_REGISTER_ADDRESS_CONFIGURATION_1:
		// Store new value in NVM.
		if (reg_mask != 0) {
			NODE_write_nvm(reg_addr, reg_value);
		}
		break;
	case GPSM_REGISTER_ADDRESS_CONFIGURATION_2:
	case GPSM_REGISTER_ADDRESS_CONFIGURATION_3:
		// Store new value in NVM.
		if (reg_mask != 0) {
			NODE_write_nvm(reg_addr, reg_value);
		}
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
			if (SWREG_read_field(reg_value, GPSM_REGISTER_CONTROL_1_MASK_TTRG) != 0) {
				// Clear request.
				NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_CONTROL_1, 0b0, GPSM_REGISTER_CONTROL_1_MASK_TTRG);
				// Start GPS time fix.
				status = _GPSM_ttrg_callback();
				if (status != NODE_SUCCESS) goto errors;
			}
		}
		// GTRG.
		if ((reg_mask & GPSM_REGISTER_CONTROL_1_MASK_GTRG) != 0) {
			// Read bit.
			if (SWREG_read_field(reg_value, GPSM_REGISTER_CONTROL_1_MASK_GTRG) != 0) {
				// Clear request.
				NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_CONTROL_1, 0b0, GPSM_REGISTER_CONTROL_1_MASK_GTRG);
				// Start GPS geolocation fix.
				status = _GPSM_gtrg_callback();
				if (status != NODE_SUCCESS) goto errors;
			}
		}
		// TPEN.
		if ((reg_mask & GPSM_REGISTER_CONTROL_1_MASK_TPEN) != 0) {
			// Read bit.
			tpen = SWREG_read_field(reg_value, GPSM_REGISTER_CONTROL_1_MASK_TPEN);
			// Compare to current state.
			if (tpen != gpsm_ctx.flags.tpen) {
				// Start timepulse.
				status = _GPSM_tpen_callback(tpen);
				if (status != NODE_SUCCESS) {
					// Clear request.
					SWREG_write_field(&new_reg_value, &new_reg_mask, gpsm_ctx.flags.tpen, GPSM_REGISTER_CONTROL_1_MASK_TPEN);
					goto errors;
				}
				// Update local flag.
				gpsm_ctx.flags.tpen = tpen;
			}
		}
		// PWMD.
		if ((reg_mask & GPSM_REGISTER_CONTROL_1_MASK_TPEN) != 0) {
			// Read bit.
			pwmd = SWREG_read_field(reg_value, GPSM_REGISTER_CONTROL_1_MASK_PWMD);
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
				pwen = SWREG_read_field(reg_value, GPSM_REGISTER_CONTROL_1_MASK_PWEN);
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
			bken = SWREG_read_field(reg_value, GPSM_REGISTER_CONTROL_1_MASK_BKEN);
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
	// Update status register.
	GPSM_update_register(GPSM_REGISTER_ADDRESS_STATUS_1);
	// Update checked register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, new_reg_value, new_reg_mask);
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
NODE_status_t GPSM_mtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	ANALOG_status_t analog_status = ANALOG_SUCCESS;
	int32_t adc_data = 0;
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	// Reset results.
	_GPSM_reset_analog_data();
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
    // GPS voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VGPS_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) UNA_convert_mv(adc_data), GPSM_REGISTER_ANALOG_DATA_1_MASK_VGPS);
    // Active antenna voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VANT_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) UNA_convert_mv(adc_data), GPSM_REGISTER_ANALOG_DATA_1_MASK_VANT);
    // Write register.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
    // Turn GPS off is possible.
    status = _GPSM_power_request(0);
    if (status != NODE_SUCCESS) goto errors;
errors:
	_GPSM_power_request(0);
	return status;
}
#endif
