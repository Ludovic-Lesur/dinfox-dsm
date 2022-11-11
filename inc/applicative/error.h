/*
 * error.h
 *
 *  Created on: Mar 12, 2022
 *      Author: Ludo
 */

#ifndef __ERROR_H__
#define __ERROR_H__

// Peripherals.
#include "adc.h"
#include "iwdg.h"
#include "nvm.h"
#include "rtc.h"
// Utils.
#include "math.h"
#include "parser.h"
#include "string.h"

typedef enum {
	SUCCESS = 0,
	// Peripherals.
	ERROR_BASE_ADC1 = 0x0100,
	ERROR_BASE_IWDG = (ERROR_BASE_ADC1 + ADC_ERROR_BASE_LAST),
	ERROR_BASE_NVM = (ERROR_BASE_IWDG + IWDG_ERROR_BASE_LAST),
	ERROR_BASE_RTC = (ERROR_BASE_NVM + NVM_ERROR_BASE_LAST),
	// Utils.
	ERROR_BASE_MATH = (ERROR_BASE_RTC + RTC_ERROR_BASE_LAST),
	ERROR_BASE_PARSER = (ERROR_BASE_MATH + MATH_ERROR_BASE_LAST),
	ERROR_BASE_STRING = (ERROR_BASE_PARSER + PARSER_ERROR_BASE_LAST),
	// Last index.
	ERROR_BASE_LAST
} ERROR_t;

/*** ERROR functions ***/

#define ERROR_status_check_print(status, success, error_base) { \
	if (status != success) { \
		_AT_print_status(error_base + status); \
		goto errors; \
	} \
}

#endif /* __ERROR_H__ */
