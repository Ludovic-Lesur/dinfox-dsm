/*
 * dinfox_types.h
 *
 *  Created on: 30 may. 2023
 *      Author: Ludo
 */

#ifndef __DINFOX_TYPES_H__
#define __DINFOX_TYPES_H__

#include "types.h"

/*** DINFOX TYPES macros ***/

#define DINFOX_REG_SIZE_BYTES				4
#define DINFOX_REG_MASK_ALL					0xFFFFFFFF
#define DINFOX_REG_MASK_NONE				0x00000000

#define DINFOX_TIME_UNIT_SIZE_BITS			2
#define DINFOX_TIME_VALUE_SIZE_BITS			6

#define DINFOX_TEMPERATURE_SIGN_SIZE_BITS	1
#define DINFOX_TEMPERATURE_VALUE_SIZE_BITS	7

#define DINFOX_VOLTAGE_UNIT_SIZE_BITS		1
#define DINFOX_VOLTAGE_VALUE_SIZE_BITS		15

#define DINFOX_CURRENT_UNIT_SIZE_BITS		2
#define DINFOX_CURRENT_VALUE_SIZE_BITS		14

/*** DINFOX TYPES structures ***/

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
		DINFOX_temperature_sign_t unit : DINFOX_TEMPERATURE_SIGN_SIZE_BITS;
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

/*** DINFOX TYPES functions ***/

uint8_t DINFOX_TYPES_convert_seconds(uint32_t time_seconds);
uint32_t DINFOX_TYPES_get_seconds(uint8_t dinfox_time);

uint8_t DINFOX_TYPES_convert_degrees(int8_t temperature_degrees);
int8_t DINFOX_TYPES_get_degrees(uint8_t dinfox_temperature);

uint16_t DINFOX_TYPES_convert_mv(uint32_t voltage_mv);
uint32_t DINFOX_TYPES_get_mv(uint16_t dinfox_voltage);

uint16_t DINFOX_TYPES_convert_ua(uint32_t current_ua);
uint32_t DINFOX_TYPES_get_ua(uint16_t dinfox_current);

uint8_t DINFOX_TYPES_convert_dbm(int16_t rf_power_dbm);
int16_t DINFOX_TYPES_get_dbm(uint8_t dinfox_rf_power);

#endif /* __DINFOX_TYPES_H__ */
