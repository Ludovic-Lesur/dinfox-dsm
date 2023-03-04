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
#include "i2c.h"
#include "iwdg.h"
#include "lptim.h"
#include "lpuart.h"
#include "nvm.h"
#include "rcc.h"
#include "rtc.h"
#include "spi.h"
// Components.
#include "digital.h"
#include "led.h"
#include "s2lp.h"
#include "sht3x.h"
// Utils.
#include "math.h"
#include "parser.h"
#include "string.h"
// Nodes.
#include "lbus.h"

typedef enum {
	SUCCESS = 0,
	ERROR_REGISTER_ADDRESS,
	ERROR_REGISTER_READ_ONLY,
	ERROR_REGISTER_UNSUPPORTED,
	ERROR_NODE_ADDRESS,
	ERROR_DL_PAYLOAD_UNAVAILABLE,
	// Peripherals.
	ERROR_BASE_ADC1 = 0x0100,
	ERROR_BASE_I2C1 = (ERROR_BASE_ADC1 + ADC_ERROR_BASE_LAST),
	ERROR_BASE_IWDG = (ERROR_BASE_I2C1 + I2C_ERROR_BASE_LAST),
	ERROR_BASE_LPTIM1 = (ERROR_BASE_IWDG + IWDG_ERROR_BASE_LAST),
	ERROR_BASE_LPUART1 = (ERROR_BASE_LPTIM1 + LPTIM_ERROR_BASE_LAST),
	ERROR_BASE_NVM = (ERROR_BASE_LPUART1 + LPUART_ERROR_BASE_LAST),
	ERROR_BASE_RCC = (ERROR_BASE_NVM + NVM_ERROR_BASE_LAST),
	ERROR_BASE_RTC = (ERROR_BASE_RCC + RCC_ERROR_BASE_LAST),
	ERROR_BASE_SPI1 = (ERROR_BASE_RTC + RTC_ERROR_BASE_LAST),
	// Components.
	ERROR_BASE_DIGITAL = (ERROR_BASE_SPI1 + SPI_ERROR_BASE_LAST),
	ERROR_BASE_LED = (ERROR_BASE_DIGITAL + DIGITAL_ERROR_BASE_LAST),
	ERROR_BASE_S2LP = (ERROR_BASE_LED + LED_ERROR_BASE_LAST),
	ERROR_BASE_SHT3X = (ERROR_BASE_S2LP + S2LP_ERROR_BASE_LAST),
	// Utils.
	ERROR_BASE_MATH = (ERROR_BASE_SHT3X + SHT3X_ERROR_BASE_LAST),
	ERROR_BASE_PARSER = (ERROR_BASE_MATH + MATH_ERROR_BASE_LAST),
	ERROR_BASE_STRING = (ERROR_BASE_PARSER + PARSER_ERROR_BASE_LAST),
	// Nodes.
	ERROR_BASE_LBUS = (ERROR_BASE_STRING + STRING_ERROR_BASE_LAST),
	// Sigfox.
	ERROR_BASE_SIGFOX = (ERROR_BASE_LBUS + LBUS_ERROR_BASE_LAST),
	// Last index.
	ERROR_BASE_LAST = (ERROR_BASE_SIGFOX + 0x4000)
} ERROR_t;

/*** ERROR functions ***/

void ERROR_stack_init(void);
void ERROR_stack_add(ERROR_t code);
ERROR_t ERROR_stack_read(void);
uint8_t ERROR_stack_is_empty(void);

#define ERROR_status_check(status, success, error_base) { \
	if (status != success) { \
		ERROR_stack_add(error_base + status); \
	} \
}

#define ERROR_status_check_print(status, success, error_base) { \
	if (status != success) { \
		_AT_BUS_print_error(error_base + status); \
		goto errors; \
	} \
}

#endif /* __ERROR_H__ */
