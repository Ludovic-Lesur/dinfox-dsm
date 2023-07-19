/*
 * neom8n.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __NEOM8N_H__
#define __NEOM8N_H__

#include "lptim.h"
#include "math.h"
#include "rtc.h"
#include "string.h"
#include "types.h"
#include "usart.h"

/*** NEOM8N structures ***/

typedef enum {
	NEOM8N_SUCCESS = 0,
	NEOM8N_ERROR_NULL_PARAMETER,
	NEOM8N_ERROR_TIMEOUT,
	NEOM8N_ERROR_CHECKSUM_INDEX,
	NEOM8N_ERROR_CHECKSUM,
	NEOM8N_ERROR_NMEA_FIELD_LENGTH,
	NEOM8N_ERROR_NMEA_MESSAGE,
	NEOM8N_ERROR_NMEA_NORTH_FLAG,
	NEOM8N_ERROR_NMEA_EAST_FLAG,
	NEOM8N_ERROR_NMEA_UNIT,
	NEOM8N_ERROR_TIME_INVALID,
	NEOM8N_ERROR_TIME_TIMEOUT,
	NEOM8N_ERROR_POSITION_INVALID,
	NEOM8N_ERROR_POSITION_TIMEOUT,
	NEOM8N_ERROR_TIMEPULSE_FREQUENCY,
	NEOM8N_ERROR_TIMEPULSE_DUTY_CYCLE,
	NEOM8N_ERROR_BASE_USART = 0x0100,
	NEOM8N_ERROR_BASE_LPTIM = (NEOM8N_ERROR_BASE_USART + USART_ERROR_BASE_LAST),
	NEOM8N_ERROR_BASE_RTC = (NEOM8N_ERROR_BASE_LPTIM + LPTIM_ERROR_BASE_LAST),
	NEOM8N_ERROR_BASE_STRING = (NEOM8N_ERROR_BASE_RTC + RTC_ERROR_BASE_LAST),
	NEOM8N_ERROR_BASE_LAST = (NEOM8N_ERROR_BASE_STRING + STRING_ERROR_BASE_LAST)
} NEOM8N_status_t;

#ifdef GPSM

typedef struct {
	// Latitude.
	uint8_t lat_degrees;
	uint8_t lat_minutes;
	uint32_t lat_seconds; // = (fractionnal part of minutes * 100000).
	uint8_t lat_north_flag; // 0='S', 1='N'.
	// Longitude.
	uint8_t long_degrees;
	uint8_t long_minutes;
	uint32_t long_seconds; // = (fractionnal part of minutes * 100000).
	uint8_t long_east_flag; // 0='W', 1='E'.
	// Altitude.
	uint32_t altitude;
} NEOM8N_position_t;

typedef struct {
	uint8_t active;
	uint32_t frequency_hz;
	uint8_t duty_cycle_percent;
} NEOM8N_timepulse_config_t;

/*** NEOM8N functions ***/

void NEOM8N_init(void);

void NEOM8N_set_backup(uint8_t vbckp_on);
uint8_t NEOM8N_get_backup(void);

NEOM8N_status_t NEOM8N_get_time(RTC_time_t* gps_time, uint32_t timeout_seconds, uint32_t* fix_duration_seconds);
NEOM8N_status_t NEOM8N_get_position(NEOM8N_position_t* gps_position, uint32_t timeout_seconds, uint32_t* fix_duration_seconds);

NEOM8N_status_t NEOM8N_configure_timepulse(NEOM8N_timepulse_config_t* timepulse_config);

void NEOM8N_switch_dma_buffer(uint8_t line_end_flag);

#define NEOM8N_check_status(error_base) { if (neom8n_status != NEOM8N_SUCCESS) { status = error_base + neom8n_status; goto errors; }}
#define NEOM8N_stack_error() { ERROR_stack_error(neom8n_status, NEOM8N_SUCCESS, ERROR_BASE_NEOM8N); }
#define NEOM8N_print_error() { ERROR_print_error(neom8n_status, NEOM8N_SUCCESS, ERROR_BASE_NEOM8N); }

#endif /* GPSM */

#endif /* __NEOM8N_H__ */
