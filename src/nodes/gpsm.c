/*
 * gpsm.c
 *
 *  Created on: 8 jun. 2023
 *      Author: Ludo
 */

#include "gpsm.h"

#include "adc.h"
#include "dinfox.h"
#include "error.h"
#include "gpsm_reg.h"
#include "load.h"
#include "neom8n.h"
#include "power.h"
#include "node.h"

/*** GPSM local macros ***/

#define GPSM_REG_TIMEOUT_DEFAULT_VALUE						0x00B40078

#define GPSM_REG_TIMEPULSE_CONFIGURATION_0_DEFAULT_VALUE	0x00989680
#define GPSM_REG_TIMEPULSE_CONFIGURATION_1_DEFAULT_VALUE	0x00000032

/*** GPSM local structures ***/

typedef union {
	struct {
		unsigned gps_power : 1;
		unsigned tpen : 1;
		unsigned pwmd : 1;
		unsigned pwen : 1;
		unsigned bken : 1;
	};
	uint8_t all;
} GPSM_flags_t;

/*** GPSM local global variables ***/

#ifdef GPSM
static GPSM_flags_t gpsm_flags;
#endif

/*** GPSM local functions ***/

#ifdef GPSM
/*******************************************************************/
static void _GPSM_reset_analog_data(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t analog_data_1 = 0;
	uint32_t analog_data_1_mask = 0;
	// Reset fields to error value.
	DINFOX_write_field(&analog_data_1, &analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, GPSM_REG_ANALOG_DATA_1_MASK_VGPS);
	DINFOX_write_field(&analog_data_1, &analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, GPSM_REG_ANALOG_DATA_1_MASK_VANT);
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_ANALOG_DATA_1, analog_data_1_mask, analog_data_1);
	NODE_stack_error();
}
#endif

#ifdef GPSM
/*******************************************************************/
static void _GPSM_power_control(uint8_t state) {
	// Local variables.
	POWER_status_t power_status = POWER_SUCCESS;
	// Check on transition.
	if ((state != 0) && (gpsm_flags.gps_power == 0)) {
		// Turn GPS on.
		power_status = POWER_enable(POWER_DOMAIN_GPS, LPTIM_DELAY_MODE_STOP);
		POWER_stack_error();
	}
	// Check on transition.
	if ((state == 0) && (gpsm_flags.gps_power != 0)) {
		// Turn GPS off.
		power_status = POWER_disable(POWER_DOMAIN_GPS);
		POWER_stack_error();
	}
	// Update local flag.
	gpsm_flags.gps_power = (state == 0) ? 0 : 1;
}
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_power_request(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t status_control_1 = 0;
	// Get power mode.
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, &status_control_1);
	NODE_stack_error();
	// Check power mode.
	if ((status_control_1 & GPSM_REG_STATUS_CONTROL_1_MASK_PWMD) == 0) {
		// Power managed by the node.
		if ((state == 0) && ((status_control_1 & 0x00000015) == 0)) {
			// Turn GPS off.
			_GPSM_power_control(0);
		}
		if (state != 0) {
			// Turn GPS on.
			_GPSM_power_control(1);
		}
	}
	else {
		// Power managed by PWEN bit.
		// Rise error only in case of turn on request while PWEN=0.
		if (((status_control_1 & GPSM_REG_STATUS_CONTROL_1_MASK_PWEN) == 0) && (state != 0)) {
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
	NODE_status_t node_status = NODE_SUCCESS;
	NEOM8N_status_t neom8n_status = NEOM8N_SUCCESS;
	NEOM8N_time_t gps_time;
	uint32_t time_fix_duration = 0;
	uint32_t timeout = 0;
	uint32_t status_control_1 = 0;
	uint32_t status_control_1_mask = 0;
	uint32_t time_data_0 = 0;
	uint32_t time_data_0_mask = 0;
	uint32_t time_data_1 = 0;
	uint32_t time_data_1_mask = 0;
	uint32_t time_data_2 = 0;
	uint32_t time_data_2_mask = 0;
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Read timeout.
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEOUT, &timeout);
	NODE_stack_error();
	// Reset status flag.
	DINFOX_write_field(&status_control_1, &status_control_1_mask, 0b0, GPSM_REG_STATUS_CONTROL_1_MASK_TFS);
	// Perform time fix.
	neom8n_status = NEOM8N_get_time(&gps_time, DINFOX_read_field(timeout, GPSM_REG_TIMEOUT_MASK_TIME_TIMEOUT), &time_fix_duration);
	// Check status.
	if (neom8n_status == NEOM8N_SUCCESS) {
		// Fill registers with time data.
		DINFOX_write_field(&time_data_0, &time_data_0_mask, (uint32_t) DINFOX_convert_year(gps_time.year), GPSM_REG_TIME_DATA_0_MASK_YEAR);
		DINFOX_write_field(&time_data_0, &time_data_0_mask, (uint32_t) gps_time.month, GPSM_REG_TIME_DATA_0_MASK_MONTH);
		DINFOX_write_field(&time_data_0, &time_data_0_mask, (uint32_t) gps_time.date, GPSM_REG_TIME_DATA_0_MASK_DATE);
		DINFOX_write_field(&time_data_1, &time_data_1_mask, (uint32_t) gps_time.hours, GPSM_REG_TIME_DATA_1_MASK_HOUR);
		DINFOX_write_field(&time_data_1, &time_data_1_mask, (uint32_t) gps_time.minutes, GPSM_REG_TIME_DATA_1_MASK_MINUTE);
		DINFOX_write_field(&time_data_1, &time_data_1_mask, (uint32_t) gps_time.seconds, GPSM_REG_TIME_DATA_1_MASK_SECOND);
		DINFOX_write_field(&time_data_2, &time_data_2_mask, time_fix_duration, GPSM_REG_TIME_DATA_2_MASK_FIX_DURATION);
		// Update status flag.
		DINFOX_write_field(&status_control_1, &status_control_1_mask, 0b1, GPSM_REG_STATUS_CONTROL_1_MASK_TFS);
		// Write registers.
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_0, time_data_0_mask, time_data_0);
		NODE_stack_error();
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_1, time_data_1_mask, time_data_1);
		NODE_stack_error();
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_2, time_data_2_mask, time_data_2);
		NODE_stack_error();
	}
	else {
		// Do not consider timeout error.
		if (neom8n_status != NEOM8N_ERROR_TIME_TIMEOUT) {
			NEOM8N_stack_error();
		}
	}
errors:
	// Clear request.
	DINFOX_write_field(&status_control_1, &status_control_1_mask, 0b0, GPSM_REG_STATUS_CONTROL_1_MASK_TTRG);
	// Write register.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, status_control_1_mask, status_control_1);
	NODE_stack_error();
	// Turn GPS off is possible.
	node_status = _GPSM_power_request(0);
	NODE_stack_error();
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_gtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	NEOM8N_status_t neom8n_status = NEOM8N_SUCCESS;
	NEOM8N_position_t gps_position;
	uint32_t geoloc_fix_duration = 0;
	uint32_t timeout = 0;
	uint32_t status_control_1 = 0;
	uint32_t status_control_1_mask = 0;
	uint32_t geoloc_data_0 = 0;
	uint32_t geoloc_data_0_mask = 0;
	uint32_t geoloc_data_1 = 0;
	uint32_t geoloc_data_1_mask = 0;
	uint32_t geoloc_data_2 = 0;
	uint32_t geoloc_data_2_mask = 0;
	uint32_t geoloc_data_3 = 0;
	uint32_t geoloc_data_3_mask = 0;
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Read timeout.
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEOUT, &timeout);
	NODE_stack_error();
	// Reset status flag.
	DINFOX_write_field(&status_control_1, &status_control_1_mask, 0b0, GPSM_REG_STATUS_CONTROL_1_MASK_GFS);
	// Perform time fix.
	neom8n_status = NEOM8N_get_position(&gps_position, DINFOX_read_field(timeout, GPSM_REG_TIMEOUT_MASK_GEOLOC_TIMEOUT), &geoloc_fix_duration);
	// Check status.
	if (neom8n_status == NEOM8N_SUCCESS) {
		// Fill registers with geoloc data.
		DINFOX_write_field(&geoloc_data_0, &geoloc_data_0_mask, (uint32_t) gps_position.lat_north_flag, GPSM_REG_GEOLOC_DATA_0_MASK_NF);
		DINFOX_write_field(&geoloc_data_0, &geoloc_data_0_mask, gps_position.lat_seconds, GPSM_REG_GEOLOC_DATA_0_MASK_SECOND);
		DINFOX_write_field(&geoloc_data_0, &geoloc_data_0_mask, (uint32_t) gps_position.lat_minutes, GPSM_REG_GEOLOC_DATA_0_MASK_MINUTE);
		DINFOX_write_field(&geoloc_data_0, &geoloc_data_0_mask, (uint32_t) gps_position.lat_degrees, GPSM_REG_GEOLOC_DATA_0_MASK_DEGREE);
		DINFOX_write_field(&geoloc_data_1, &geoloc_data_1_mask, (uint32_t) gps_position.long_east_flag, GPSM_REG_GEOLOC_DATA_1_MASK_EF);
		DINFOX_write_field(&geoloc_data_1, &geoloc_data_1_mask, gps_position.long_seconds, GPSM_REG_GEOLOC_DATA_1_MASK_SECOND);
		DINFOX_write_field(&geoloc_data_1, &geoloc_data_1_mask, (uint32_t) gps_position.long_minutes, GPSM_REG_GEOLOC_DATA_1_MASK_MINUTE);
		DINFOX_write_field(&geoloc_data_1, &geoloc_data_1_mask, (uint32_t) gps_position.long_degrees, GPSM_REG_GEOLOC_DATA_1_MASK_DEGREE);
		DINFOX_write_field(&geoloc_data_2, &geoloc_data_2_mask, gps_position.altitude, GPSM_REG_GEOLOC_DATA_2_MASK_ALTITUDE);
		DINFOX_write_field(&geoloc_data_3, &geoloc_data_3_mask, geoloc_fix_duration, GPSM_REG_GEOLOC_DATA_3_MASK_FIX_DURATION);
		// Update status flag.
		DINFOX_write_field(&status_control_1, &status_control_1_mask, 0b1, GPSM_REG_STATUS_CONTROL_1_MASK_GFS);
		// Write registers.
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_0, geoloc_data_0_mask, geoloc_data_0);
		NODE_stack_error();
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_1, geoloc_data_1_mask, geoloc_data_1);
		NODE_stack_error();
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_2, geoloc_data_2_mask, geoloc_data_2);
		NODE_stack_error();
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_3, geoloc_data_3_mask, geoloc_data_3);
		NODE_stack_error();
	}
	else {
		// Do not consider position error.
		if (neom8n_status != NEOM8N_ERROR_POSITION_TIMEOUT) {
			NEOM8N_stack_error();
		}
	}
errors:
	// Clear request.
	DINFOX_write_field(&status_control_1, &status_control_1_mask, 0b0, GPSM_REG_STATUS_CONTROL_1_MASK_GTRG);
	// Write register.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, status_control_1_mask, status_control_1);
	NODE_stack_error();
	// Turn GPS off is possible.
	node_status = _GPSM_power_request(0);
	NODE_stack_error();
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
static NODE_status_t _GPSM_tpen_callback(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	NEOM8N_status_t neom8n_status = NEOM8N_SUCCESS;
	NEOM8N_timepulse_config_t timepulse_config;
	uint32_t timepulse_configuration_0 = 0;
	uint32_t timepulse_configuration_1 = 0;
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Read registers.
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_0, &timepulse_configuration_0);
	NODE_stack_error();
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_1, &timepulse_configuration_1);
	NODE_stack_error();
	// Set parameters.
	timepulse_config.active = state;
	timepulse_config.frequency_hz = DINFOX_read_field(timepulse_configuration_0, GPSM_REG_TIMEPULSE_CONFIGURATION_0_MASK_FREQUENCY);
	timepulse_config.duty_cycle_percent = (uint8_t) DINFOX_read_field(timepulse_configuration_1, GPSM_REG_TIMEPULSE_CONFIGURATION_1_MASK_DUTY_CYCLE);
	// Set timepulse.
	neom8n_status = NEOM8N_configure_timepulse(&timepulse_config);
	NEOM8N_stack_error();
errors:
	// Turn GPS off is possible.
	node_status = _GPSM_power_request(0);
	NODE_stack_error();
	return status;
}
#endif

/*** GPSM functions ***/

#ifdef GPSM
/*******************************************************************/
void GPSM_init_registers(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	// Init flags.
	gpsm_flags.all = 0;
	// Load default values.
	_GPSM_reset_analog_data();
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEOUT, DINFOX_REG_MASK_ALL, GPSM_REG_TIMEOUT_DEFAULT_VALUE);
	NODE_stack_error();
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_0, DINFOX_REG_MASK_ALL, GPSM_REG_TIMEPULSE_CONFIGURATION_0_DEFAULT_VALUE);
	NODE_stack_error();
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_1, DINFOX_REG_MASK_ALL, GPSM_REG_TIMEPULSE_CONFIGURATION_1_DEFAULT_VALUE);
	NODE_stack_error();
}
#endif

#ifdef GPSM
/*******************************************************************/
NODE_status_t GPSM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Check address.
	switch (reg_addr) {
	case GPSM_REG_ADDR_STATUS_CONTROL_1:
		// GPS backup voltage.
		DINFOX_write_field(&reg_value, &reg_mask, (uint32_t) (NEOM8N_get_backup()), GPSM_REG_STATUS_CONTROL_1_MASK_BKEN);
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	// Write register.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_mask, reg_value);
	NODE_stack_error();
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
NODE_status_t GPSM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t pwmd = 0;
	uint32_t pwen = 0;
	uint32_t tpen = 0;
	uint32_t bken = 0;
	uint32_t reg_value = 0;
	uint32_t new_reg_value = 0;
	uint32_t new_reg_mask = 0;
	// Read register.
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
	NODE_stack_error();
	// Check address.
	switch (reg_addr) {
	case GPSM_REG_ADDR_STATUS_CONTROL_1:
		// Read bits.
		tpen = DINFOX_read_field(reg_value, GPSM_REG_STATUS_CONTROL_1_MASK_TPEN);
		pwmd = DINFOX_read_field(reg_value, GPSM_REG_STATUS_CONTROL_1_MASK_PWMD);
		pwen = DINFOX_read_field(reg_value, GPSM_REG_STATUS_CONTROL_1_MASK_PWEN);
		bken = DINFOX_read_field(reg_value, GPSM_REG_STATUS_CONTROL_1_MASK_BKEN);
		// Check TTRG bit.
		if (DINFOX_read_field(reg_value, GPSM_REG_STATUS_CONTROL_1_MASK_TTRG) != 0) {
			// Start GPS time fix.
			status = _GPSM_ttrg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
		// Check GTRG bit.
		if (DINFOX_read_field(reg_value, GPSM_REG_STATUS_CONTROL_1_MASK_GTRG) != 0) {
			// Start GPS geolocation fix.
			status = _GPSM_gtrg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
		// Check TPEN bit change.
		if (tpen != gpsm_flags.tpen) {
			// Start timepulse.
			status = _GPSM_tpen_callback(tpen);
			if (status != NODE_SUCCESS) {
				// Clear request.
				DINFOX_write_field(&new_reg_value, &new_reg_mask, gpsm_flags.tpen, GPSM_REG_STATUS_CONTROL_1_MASK_TPEN);
				goto errors;
			}
			// Update local flag.
			gpsm_flags.tpen = tpen;
		}
		// Check PWMD bit change.
		if ((pwmd != 0) && (gpsm_flags.pwmd == 0)) {
			// Apply PWEN bit.
			_GPSM_power_control(pwen);
			// Update local flag.
			gpsm_flags.pwmd = pwmd;
		}
		// Check PWMD bit change.
		if ((pwmd == 0) && (gpsm_flags.pwmd != 0)) {
			// Try turning GPS off.
			node_status = _GPSM_power_request(0);
			NODE_stack_error();
			// Update local flag.
			gpsm_flags.pwmd = pwmd;
		}
		// Check PWEN bit change.
		if ((pwmd != 0) && (pwen != gpsm_flags.pwen)) {
			// Apply PWEN bit.
			_GPSM_power_control(pwen);
			// Update local flag.
			gpsm_flags.pwen = pwen;
		}
		// Check BKEN bit change.
		if (bken != gpsm_flags.bken) {
			// Start timepulse.
			NEOM8N_set_backup(bken);
			// Update local flag.
			gpsm_flags.bken = bken;
		}
		break;
	case GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_0:
	case GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_1:
		// Update timepulse signal if running.
		if (gpsm_flags.tpen != 0) {
			// Start timepulse with new settings.
			status = _GPSM_tpen_callback(1);
			if (status != NODE_SUCCESS) goto errors;
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}
#endif

#ifdef GPSM
/*******************************************************************/
NODE_status_t GPSM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	uint32_t analog_data_1 = 0;
	uint32_t analog_data_1_mask = 0;
	// Reset results.
	_GPSM_reset_analog_data();
	// Turn GPS on.
	status = _GPSM_power_request(1);
	if (status != NODE_SUCCESS) goto errors;
	// Perform analog measurements.
	power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_SLEEP);
	POWER_stack_error();
	adc1_status = ADC1_perform_measurements();
	ADC1_stack_error();
	power_status = POWER_disable(POWER_DOMAIN_ANALOG);
	POWER_stack_error();
	// Update parameter.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// GPS voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VGPS_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_1, &analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), GPSM_REG_ANALOG_DATA_1_MASK_VGPS);
		}
		// Active antenna voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VANT_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_1, &analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), GPSM_REG_ANALOG_DATA_1_MASK_VANT);
		}
		// Write register.
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_ANALOG_DATA_1, analog_data_1_mask,analog_data_1);
		NODE_stack_error();
	}
errors:
	// Turn GPS off is possible.
	node_status = _GPSM_power_request(0);
	NODE_stack_error();
	return status;
}
#endif
