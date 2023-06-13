/*
 * dinfox_common.c
 *
 *  Created on: 30 may. 2023
 *      Author: Ludo
 */

#include "dinfox_common.h"

#include "math.h"
#include "parser.h"
#include "string.h"
#include "types.h"

/*** DINFOX TYPES local macros ***/

#define DINFOX_SECONDS_PER_MINUTE	60
#define DINFOX_MINUTES_PER_HOUR		60
#define DINFOX_HOURS_PER_DAY		24

#define DINFOX_MV_PER_DV			100

#define DINFOX_UA_PER_DMA			100
#define DINFOX_DMA_PER_MA			10
#define DINFOX_MA_PER_DA			100

#define DINFOX_RF_POWER_OFFSET		174

/*** DINFOX TYPES functions ***/

/* RETURN THE OFFSET OF A FIELD MASK.
 * @param reg_mask:	Register mask.
 * @return shift:	Position of the first bit 1.
 */
uint8_t DINFOX_get_field_offset(uint32_t field_mask) {
	// Local variables.
	uint8_t offset = 0;
	// Compute shift according to mask.
	for (offset=0 ; offset<(8 * DINFOX_REG_SIZE_BYTES) ; offset++) {
		if ((field_mask & (0b1 << offset)) != 0) {
			break;
		}
	}
	return offset;
}

/* GET FIELD VALUE FROM REGISTER.
 * @param reg_value:	Register value.
 * @param field_mask:	Field mask.
 * @return field_value:	Field value.
 */
uint32_t DINFOX_get_field_value(uint32_t reg_value, uint32_t field_mask) {
	// Isolate field.
	return ((reg_value & field_mask) >> DINFOX_get_field_offset(field_mask));
}

/* CONVERT A BYTE ARRAY TO 32 BITS INTEGER.
 * @param data:				Byte array to convert.
 * @param data_size_bytes:	Size of the input byte array.
 * @return value:			32 bits result.
 */
PARSER_status_t DINFOX_parser_register(PARSER_context_t* parser_ctx, uint32_t* reg_value) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_SUCCESS;
	uint8_t reg_bytes[DINFOX_REG_SIZE_BYTES];
	uint8_t reg_size_bytes = 0;
	uint8_t idx = 0;
	// Parse register as byte array.
	parser_status = PARSER_get_byte_array(parser_ctx, STRING_CHAR_NULL, DINFOX_REG_SIZE_BYTES, 0, (uint8_t*) reg_bytes, &reg_size_bytes);
	if (parser_status != PARSER_SUCCESS) goto errors;
	// Convert byte array to 32 bits value.
	(*reg_value) = 0;
	for (idx=0 ; idx<reg_size_bytes ; idx++) {
		(*reg_value) |= (reg_bytes[idx] >> (8 * (reg_size_bytes - 1 - idx)));
	}
errors:
	return parser_status;
}

/* CONVERT A REGISTER VALUE INTO THE SHORTEST HEXADECIMAL STRING.
 * @param regvalue:	Register value to convert.
 * @param str:		Destination string.
 * @return status:	Function execution status.
 */
STRING_status_t DINFOX_register_to_string(uint32_t reg_value, char_t* str) {
	// Local variables.
	STRING_status_t status = STRING_SUCCESS;
	uint8_t byte = 0;
	uint8_t idx = 0;
	// Convert 32-bits value to byte array.
	for (idx=0 ; idx<DINFOX_REG_SIZE_BYTES ; idx++) {
		// Compute byte.
		byte = (uint8_t) ((reg_value >> (8 * (DINFOX_REG_SIZE_BYTES - 1 - idx))) & 0xFF);
		// Check value.
		if (byte != 0) {
			status = STRING_value_to_string((int32_t) byte, STRING_FORMAT_HEXADECIMAL, 0, &(str[2 * idx]));
			if (status != STRING_SUCCESS) break;
		}
	}
	str[2 * idx] = STRING_CHAR_NULL;
	return status;
}

/* CONVERT SECONDS TO DINFOX TIME REPRESENTATION.
 * @param time_seconds:	Time in seconds.
 * @return dinfox_time:	DINFox duration representation.
 */
uint8_t DINFOX_convert_seconds(uint32_t time_seconds) {
	// Local variables.
	DINFOX_time_t dinfox_time;
	uint32_t value = time_seconds;
	// Select unit.
	if (value < (0b1 << DINFOX_TIME_VALUE_SIZE_BITS)) {
		dinfox_time.unit = DINFOX_TIME_UNIT_SECOND;
	}
	else {
		value /= DINFOX_SECONDS_PER_MINUTE;
		if (value < (0b1 << DINFOX_TIME_VALUE_SIZE_BITS)) {
			dinfox_time.unit = DINFOX_TIME_UNIT_MINUTE;
		}
		else {
			value /= DINFOX_MINUTES_PER_HOUR;
			if (value < (0b1 << DINFOX_TIME_VALUE_SIZE_BITS)) {
				dinfox_time.unit = DINFOX_TIME_UNIT_HOUR;
			}
			else {
				value /= DINFOX_HOURS_PER_DAY;
				dinfox_time.unit = DINFOX_TIME_UNIT_DAY;
			}
		}
	}
	dinfox_time.value = value;
	return (dinfox_time.representation);
}

/* CONVERT DINFOX TIME REPRESENTATION TO SECONDS.
 * @param dinfox_time:		DINFox duration representation.
 * @return time_seconds:	Time in seconds.
 */
uint32_t DINFOX_get_seconds(uint8_t dinfox_time) {
	// Local variables.
	uint32_t time_seconds = 0;
	uint8_t local_dinfox_time = dinfox_time;
	DINFOX_time_unit_t unit = DINFOX_TIME_UNIT_SECOND;
	uint8_t value = 0;
	// Parse field.
	unit = ((DINFOX_time_t*) &local_dinfox_time) -> unit;
	value = ((DINFOX_time_t*) &local_dinfox_time) -> value;
	// Compute seconds.
	switch (unit) {
	case DINFOX_TIME_UNIT_SECOND:
		time_seconds = (uint32_t) value;
		break;
	case DINFOX_TIME_UNIT_MINUTE:
		time_seconds = (uint32_t) (DINFOX_SECONDS_PER_MINUTE * value);
		break;
	case DINFOX_TIME_UNIT_HOUR:
		time_seconds = (uint32_t) (DINFOX_MINUTES_PER_HOUR * DINFOX_SECONDS_PER_MINUTE * value);
		break;
	default:
		time_seconds = (uint32_t) (DINFOX_HOURS_PER_DAY * DINFOX_MINUTES_PER_HOUR * DINFOX_SECONDS_PER_MINUTE * value);
		break;
	}
	return time_seconds;
}

/* CONVERT DEGREES TO DINFOX TEMPERATURE REPRESENTATION.
 * @param temperature_degrees:	Temperature in degreees.
 * @return dinfox_temperature:	DINFox temperature representation.
 */
uint8_t DINFOX_convert_degrees(int8_t temperature_degrees) {
	// Local variables.
	uint32_t dinfox_temperature = 0;
	int32_t temp_degrees = (int32_t) temperature_degrees;
	// DINFox representation is equivalent to one complement.
	MATH_one_complement(temp_degrees, DINFOX_TEMPERATURE_VALUE_SIZE_BITS, &dinfox_temperature);
	return ((uint8_t) dinfox_temperature);
}

/* CONVERT DINFOX TEMPERATURE REPRESENTATION TO DEGREES.
 * @param dinfox_temperature:	DINFox temperature representation.
 * @return temperature_degrees:	Temperature in degreees.
 */
int8_t DINFOX_get_degrees(uint8_t dinfox_temperature) {
	// Local variables.
	int8_t temperature_degrees = 0;
	uint8_t local_dinfox_temperature = dinfox_temperature;
	uint8_t sign = 0;
	uint8_t value = 0;
	// Parse sign and value.
	sign = (uint32_t) (((DINFOX_temperature_t*) &local_dinfox_temperature) -> sign);
	value = (uint32_t) (((DINFOX_temperature_t*) &local_dinfox_temperature) -> value);
	// Check sign.
	temperature_degrees = (sign == DINFOX_TEMPERATURE_SIGN_POSITIVE) ? (value) : ((-1) * value);
	return temperature_degrees;
}

/* CONVERT MV TO DINFOX VOLTAGE REPRESENTATION.
 * @param voltage_mv:		Votage in mV.
 * @return dinfox_voltage:	DINFox voltage representation.
 */
uint16_t DINFOX_convert_mv(uint32_t voltage_mv) {
	// Local variables.
	DINFOX_voltage_t dinfox_voltage;
	// Select format.
	if (voltage_mv < (0b1 << DINFOX_VOLTAGE_VALUE_SIZE_BITS)) {
		dinfox_voltage.unit = DINFOX_VOLTAGE_UNIT_MV;
		dinfox_voltage.value = voltage_mv;
	}
	else {
		dinfox_voltage.unit = DINFOX_VOLTAGE_UNIT_DV;
		dinfox_voltage.value = (voltage_mv / 100);
	}
	return (dinfox_voltage.representation);
}

/* CONVERT DINFOX VOLTAGE REPRESENTATION TO MV.
 * @param dinfox_voltage:	DINFox voltage representation.
 * @return voltage_mv:		Votage in mV.
 */
uint32_t DINFOX_get_mv(uint16_t dinfox_voltage) {
	// Local variables.
	uint32_t voltage_mv = 0;
	uint16_t local_dinfox_voltage = dinfox_voltage;
	DINFOX_voltage_unit_t unit = DINFOX_VOLTAGE_UNIT_MV;
	uint16_t value = 0;
	// Parse field.
	unit = ((DINFOX_voltage_t*) &local_dinfox_voltage) -> unit;
	value = ((DINFOX_voltage_t*) &local_dinfox_voltage) -> value;
	// Compute mV.
	voltage_mv = (unit == DINFOX_VOLTAGE_UNIT_MV) ? ((uint32_t) value) : ((uint32_t) (value / DINFOX_MV_PER_DV));
	return voltage_mv;
}

/* CONVERT UA TO DINFOX CURRENT REPRESENTATION.
 * @param current_ua:		Current in uA.
 * @return dinfox_current:	DINFox current representation.
 */
uint16_t DINFOX_convert_ua(uint32_t current_ua) {
	// Local variables.
	DINFOX_current_t dinfox_current;
	uint32_t value = current_ua;
	// Select unit.
	if (value < (0b1 << DINFOX_CURRENT_VALUE_SIZE_BITS)) {
		dinfox_current.unit = DINFOX_CURRENT_UNIT_UA;
	}
	else {
		value /= DINFOX_UA_PER_DMA;
		if (value < (0b1 << DINFOX_CURRENT_VALUE_SIZE_BITS)) {
			dinfox_current.unit = DINFOX_CURRENT_UNIT_DMA;
		}
		else {
			value /= DINFOX_DMA_PER_MA;
			if (value < (0b1 << DINFOX_CURRENT_VALUE_SIZE_BITS)) {
				dinfox_current.unit = DINFOX_CURRENT_UNIT_MA;
			}
			else {
				value /= DINFOX_MA_PER_DA;
				dinfox_current.unit = DINFOX_CURRENT_UNIT_DA;
			}
		}
	}
	dinfox_current.value = value;
	return (dinfox_current.representation);
}

/* CONVERT DINFOX CURRENT REPRESENTATION TO UA.
 * @param dinfox_current:	DINFox current representation.
 * @return current_ua:		Current in uA.
 */
uint32_t DINFOX_get_ua(uint16_t dinfox_current) {
	// Local variables.
	uint32_t current_ua = 0;
	uint8_t local_dinfox_current = dinfox_current;
	DINFOX_time_unit_t unit = DINFOX_TIME_UNIT_SECOND;
	uint8_t value = 0;
	// Parse field.
	unit = ((DINFOX_time_t*) &local_dinfox_current) -> unit;
	value = ((DINFOX_time_t*) &local_dinfox_current) -> value;
	// Compute seconds.
	switch (unit) {
	case DINFOX_CURRENT_UNIT_UA:
		current_ua = (uint32_t) value;
		break;
	case DINFOX_CURRENT_UNIT_DMA:
		current_ua = (uint32_t) (DINFOX_UA_PER_DMA * value);
		break;
	case DINFOX_CURRENT_UNIT_MA:
		current_ua = (uint32_t) (DINFOX_UA_PER_DMA * DINFOX_DMA_PER_MA * value);
		break;
	default:
		current_ua = (uint32_t) (DINFOX_UA_PER_DMA * DINFOX_DMA_PER_MA * DINFOX_MA_PER_DA * value);
		break;
	}
	return current_ua;
}

/* CONVERT DBM TO DINFOX RF POWER REPRESENTATION.
 * @param rf_power_dbm:		RF power in dBm.
 * @return dinfox_rf_power:	DINFox RF power representation.
 */
uint8_t DINFOX_convert_dbm(int16_t rf_power_dbm) {
	return ((uint8_t) (rf_power_dbm + DINFOX_RF_POWER_OFFSET));
}

/* CONVERT DINFOX RF POWER REPRESENTATION TO DBM.
 * @param dinfox_rf_power:	DINFox RF power representation.
 * @return rf_power_dbm:	RF power in dBm.
 */
int16_t DINFOX_get_dbm(uint8_t dinfox_rf_power) {
	return ((uint16_t) (dinfox_rf_power - DINFOX_RF_POWER_OFFSET));
}
