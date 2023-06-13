/*
 * dinfox_common.h
 *
 *  Created on: 30 may. 2023
 *      Author: Ludo
 */

#ifndef __DINFOX_COMMON_H__
#define __DINFOX_COMMON_H__

#include "parser.h"
#include "string.h"
#include "types.h"

/*** DINFOX common macros ***/

#define DINFOX_NODE_ADDRESS_RANGE_R4S8CR	15

#define DINFOX_REG_SIZE_BYTES				4
#define DINFOX_REG_MASK_ALL					0xFFFFFFFF
#define DINFOX_REG_MASK_NONE				0x00000000

#define DINFOX_TIME_UNIT_SIZE_BITS			2
#define DINFOX_TIME_VALUE_SIZE_BITS			6

#define DINFOX_TEMPERATURE_SIGN_SIZE_BITS	1
#define DINFOX_TEMPERATURE_VALUE_SIZE_BITS	7
#define DINFOX_TEMPERATURE_ERROR_VALUE		0x7F

#define DINFOX_HUMIDITY_ERROR_VALUE			0xFF

#define DINFOX_VOLTAGE_UNIT_SIZE_BITS		1
#define DINFOX_VOLTAGE_VALUE_SIZE_BITS		15
#define DINFOX_VOLTAGE_ERROR_VALUE			0xFFFF

#define DINFOX_CURRENT_UNIT_SIZE_BITS		2
#define DINFOX_CURRENT_VALUE_SIZE_BITS		14
#define DINFOX_CURRENT_ERROR_VALUE			0xFFFF

/*** DINFOX common structures ***/

typedef enum {
	DINFOX_BOARD_ID_LVRM = 0,
	DINFOX_BOARD_ID_BPSM,
	DINFOX_BOARD_ID_DDRM,
	DINFOX_BOARD_ID_UHFM,
	DINFOX_BOARD_ID_GPSM,
	DINFOX_BOARD_ID_SM,
	DINFOX_BOARD_ID_DIM,
	DINFOX_BOARD_ID_RRM,
	DINFOX_BOARD_ID_DMM,
	DINFOX_BOARD_ID_MPMCM,
	DINFOX_BOARD_ID_R4S8CR,
	DINFOX_BOARD_ID_LAST,
	DINFOX_BOARD_ID_ERROR
} DINFOX_board_id_t;

typedef enum {
	DINFOX_NODE_ADDRESS_DMM = 0x00,
	DINFOX_NODE_ADDRESS_DIM = 0x01,
	DINFOX_NODE_ADDRESS_BPSM_START = 0x08,
	DINFOX_NODE_ADDRESS_UHFM_START = (DINFOX_NODE_ADDRESS_BPSM_START + 4),
	DINFOX_NODE_ADDRESS_GPSM_START = (DINFOX_NODE_ADDRESS_UHFM_START + 4),
	DINFOX_NODE_ADDRESS_SM_START = (DINFOX_NODE_ADDRESS_GPSM_START + 4),
	DINFOX_NODE_ADDRESS_RRM_START = (DINFOX_NODE_ADDRESS_SM_START + 4),
	DINFOX_NODE_ADDRESS_MPMCM_START = (DINFOX_NODE_ADDRESS_RRM_START + 4),
	DINFOX_NODE_ADDRESS_LVRM_START = (DINFOX_NODE_ADDRESS_MPMCM_START + 4),
	DINFOX_NODE_ADDRESS_DDRM_START = (DINFOX_NODE_ADDRESS_LVRM_START + 16),
	DINFOX_NODE_ADDRESS_LBUS_LAST = (DINFOX_NODE_ADDRESS_DDRM_START + 16),
	DINFOX_NODE_ADDRESS_R4S8CR_START = 0x70,
	DINFOX_NODE_ADDRESS_BROADCAST = (DINFOX_NODE_ADDRESS_R4S8CR_START + DINFOX_NODE_ADDRESS_RANGE_R4S8CR)
} DINFOX_address_t;

typedef enum {
	DINFOX_REG_ACCESS_READ_ONLY = 0,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_LAST
} DINFOX_register_access_t;

typedef enum {
	DINFOX_TIME_UNIT_SECOND = 0,
	DINFOX_TIME_UNIT_MINUTE,
	DINFOX_TIME_UNIT_HOUR,
	DINFOX_TIME_UNIT_DAY
} DINFOX_time_unit_t;

typedef union {
	uint8_t representation;
	struct {
		unsigned value : DINFOX_TIME_VALUE_SIZE_BITS;
		DINFOX_time_unit_t unit : DINFOX_TIME_UNIT_SIZE_BITS;
	} __attribute__((scalar_storage_order("little-endian"))) __attribute__((packed));
} DINFOX_time_t;

typedef enum {
	DINFOX_TEMPERATURE_SIGN_POSITIVE = 0,
	DINFOX_TEMPERATURE_SIGN_NEGATIVE
} DINFOX_temperature_sign_t;

typedef union {
	uint8_t representation;
	struct {
		unsigned value : DINFOX_TEMPERATURE_VALUE_SIZE_BITS;
		DINFOX_temperature_sign_t sign : DINFOX_TEMPERATURE_SIGN_SIZE_BITS;
	} __attribute__((scalar_storage_order("little-endian"))) __attribute__((packed));
} DINFOX_temperature_t;

typedef enum {
	DINFOX_VOLTAGE_UNIT_MV = 0,
	DINFOX_VOLTAGE_UNIT_DV
} DINFOX_voltage_unit_t;

typedef union {
	uint16_t representation;
	struct {
		unsigned value : DINFOX_VOLTAGE_VALUE_SIZE_BITS;
		DINFOX_voltage_unit_t unit : DINFOX_VOLTAGE_UNIT_SIZE_BITS;
	} __attribute__((scalar_storage_order("little-endian"))) __attribute__((packed));
} DINFOX_voltage_t;

typedef enum {
	DINFOX_CURRENT_UNIT_UA = 0,
	DINFOX_CURRENT_UNIT_DMA,
	DINFOX_CURRENT_UNIT_MA,
	DINFOX_CURRENT_UNIT_DA
} DINFOX_current_unit_t;

typedef union {
	uint8_t representation;
	struct {
		unsigned value : DINFOX_CURRENT_VALUE_SIZE_BITS;
		DINFOX_current_unit_t unit : DINFOX_CURRENT_UNIT_SIZE_BITS;
	} __attribute__((scalar_storage_order("little-endian"))) __attribute__((packed));
} DINFOX_current_t;

/*** DINFOX common functions ***/

uint8_t DINFOX_get_field_offset(uint32_t field_mask);
uint32_t DINFOX_get_field_value(uint32_t reg_value, uint32_t field_mask);

PARSER_status_t DINFOX_parse_register(PARSER_context_t* parser_ctx, char_t separator, uint32_t* reg_value);
STRING_status_t DINFOX_register_to_string(uint32_t reg_value, char_t* str);

uint8_t DINFOX_convert_seconds(uint32_t time_seconds);
uint32_t DINFOX_get_seconds(uint8_t dinfox_time);

uint8_t DINFOX_convert_degrees(int8_t temperature_degrees);
int8_t DINFOX_get_degrees(uint8_t dinfox_temperature);

uint16_t DINFOX_convert_mv(uint32_t voltage_mv);
uint32_t DINFOX_get_mv(uint16_t dinfox_voltage);

uint16_t DINFOX_convert_ua(uint32_t current_ua);
uint32_t DINFOX_get_ua(uint16_t dinfox_current);

uint8_t DINFOX_convert_dbm(int16_t rf_power_dbm);
int16_t DINFOX_get_dbm(uint8_t dinfox_rf_power);

#endif /* __DINFOX_COMMON_H__ */
