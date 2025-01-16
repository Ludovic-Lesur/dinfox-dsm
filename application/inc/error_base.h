/*
 * error_base.h
 *
 *  Created on: 12 mar. 2022
 *      Author: Ludo
 */

#ifndef __ERROR_BASE_H__
#define __ERROR_BASE_H__

// Peripherals.
#include "adc.h"
#include "aes.h"
#include "flash.h"
#include "i2c.h"
#include "iwdg.h"
#include "lptim.h"
#include "lpuart.h"
#include "nvm.h"
#include "rcc.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
// Utils.
#include "math.h"
#include "parser.h"
#include "string.h"
// Components.
#include "digital.h"
#include "led.h"
#include "load.h"
#include "neom8x.h"
#include "power.h"
#include "s2lp.h"
#include "sht3x.h"
// Middleware.
#include "rfe.h"
// Nodes.
#include "cli.h"
#include "node.h"
// Sigfox.
#ifdef UHFM
#include "sigfox_error.h"
#endif

/*** ERROR structures ***/

/*!******************************************************************
 * \enum ERROR_base_t
 * \brief Board error bases.
 *******************************************************************/
typedef enum {
	SUCCESS = 0,
	// Peripherals.
	ERROR_BASE_ADC = 0x0100,
	ERROR_BASE_AES = (ERROR_BASE_ADC + ADC_ERROR_BASE_LAST),
	ERROR_BASE_FLASH = (ERROR_BASE_AES + AES_ERROR_BASE_LAST),
	ERROR_BASE_I2C_SENSORS = (ERROR_BASE_FLASH + FLASH_ERROR_BASE_LAST),
	ERROR_BASE_IWDG = (ERROR_BASE_I2C_SENSORS + I2C_ERROR_BASE_LAST),
	ERROR_BASE_LPTIM = (ERROR_BASE_IWDG + IWDG_ERROR_BASE_LAST),
	ERROR_BASE_LPUART_RS485 = (ERROR_BASE_LPTIM + LPTIM_ERROR_BASE_LAST),
	ERROR_BASE_NVM = (ERROR_BASE_LPUART_RS485 + LPUART_ERROR_BASE_LAST),
	ERROR_BASE_RCC = (ERROR_BASE_NVM + NVM_ERROR_BASE_LAST),
	ERROR_BASE_RTC = (ERROR_BASE_RCC + RCC_ERROR_BASE_LAST),
	ERROR_BASE_SPI_RADIO = (ERROR_BASE_RTC + RTC_ERROR_BASE_LAST),
	ERROR_BASE_TIM_MCU_API = (ERROR_BASE_SPI_RADIO + SPI_ERROR_BASE_LAST),
	ERROR_BASE_TIM_LED_PWM = (ERROR_BASE_TIM_MCU_API + TIM_ERROR_BASE_LAST),
	ERROR_BASE_TIM_LED_DIMMING = (ERROR_BASE_TIM_LED_PWM + TIM_ERROR_BASE_LAST),
	ERROR_BASE_USART_GPS = (ERROR_BASE_TIM_LED_DIMMING + TIM_ERROR_BASE_LAST),
	// Utils.
	ERROR_BASE_MATH = (ERROR_BASE_USART_GPS + USART_ERROR_BASE_LAST),
	ERROR_BASE_PARSER = (ERROR_BASE_MATH + MATH_ERROR_BASE_LAST),
	ERROR_BASE_STRING = (ERROR_BASE_PARSER + PARSER_ERROR_BASE_LAST),
	// Components.
	ERROR_BASE_DIGITAL = (ERROR_BASE_STRING + STRING_ERROR_BASE_LAST),
	ERROR_BASE_LED = (ERROR_BASE_DIGITAL + DIGITAL_ERROR_BASE_LAST),
	ERROR_BASE_LOAD = (ERROR_BASE_LED + LED_ERROR_BASE_LAST),
	ERROR_BASE_NEOM8N = (ERROR_BASE_LOAD + LOAD_ERROR_BASE_LAST),
	ERROR_BASE_S2LP = (ERROR_BASE_NEOM8N + NEOM8X_ERROR_BASE_LAST),
	ERROR_BASE_SHT3X = (ERROR_BASE_S2LP + S2LP_ERROR_BASE_LAST),
	// Middleware.
	ERROR_BASE_ANALOG = (ERROR_BASE_SHT3X + SHT3X_ERROR_BASE_LAST),
	ERROR_BASE_CLI = (ERROR_BASE_ANALOG + ANALOG_ERROR_BASE_LAST),
	ERROR_BASE_NODE = (ERROR_BASE_CLI + CLI_ERROR_BASE_LAST),
	ERROR_BASE_POWER = (ERROR_BASE_NODE + NODE_ERROR_BASE_LAST),
	ERROR_BASE_RFE = (ERROR_BASE_POWER + POWER_ERROR_BASE_LAST),
#ifdef UHFM
	// Sigfox.
	ERROR_BASE_SIGFOX_EP_LIB = (ERROR_BASE_RFE + RFE_ERROR_BASE_LAST),
	ERROR_BASE_SIGFOX_EP_ADDON_RFP = (ERROR_BASE_SIGFOX_EP_LIB + (SIGFOX_ERROR_SOURCE_LAST * 0x0100)),
	// Last base value.
	ERROR_BASE_LAST = (ERROR_BASE_SIGFOX_EP_ADDON_RFP + 0x0100)
#else
	// Last base value.
	ERROR_BASE_LAST = (ERROR_BASE_NODE + NODE_ERROR_BASE_LAST)
#endif
} ERROR_base_t;

/*!******************************************************************
 * \enum ERROR_code_t
 * \brief Board error code type.
 *******************************************************************/
typedef uint16_t ERROR_code_t;

/*** ERROR functions ***/

/*!******************************************************************
 * \fn void ERROR_stack_init(void)
 * \brief Init error stack.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void ERROR_stack_init(void);

/*!******************************************************************
 * \fn void ERROR_stack_add(ERROR_code_t code)
 * \brief Add error to stack.
 * \param[in]  	code: Error to stack.
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void ERROR_stack_add(ERROR_code_t code);

/*!******************************************************************
 * \fn ERROR_code_t ERROR_stack_read(void)
 * \brief Read error stack.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Last error code stored.
 *******************************************************************/
ERROR_code_t ERROR_stack_read(void);

/*!******************************************************************
 * \fn uint8_t ERROR_stack_is_empty(void)
 * \brief Check if error stack is empty.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		1 if the error stack is empty, 0 otherwise.
 *******************************************************************/
uint8_t ERROR_stack_is_empty(void);

#ifdef UHFM
/*!******************************************************************
 * \fn void ERROR_import_sigfox_stack(void)
 * \brief Import the Sigfox EP lib error stack in the MCU stack.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void ERROR_import_sigfox_stack(void);
#endif

#endif /* __ERROR_BASE_H__ */
