/*
 * gpio_mapping.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __GPIO_MAPPING_H__
#define __GPIO_MAPPING_H__

#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "lpuart.h"
#include "mode.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

/*** GPIO MAPPING structures ***/

#ifndef GPSM
/*******************************************************************/
typedef enum {
    GPIO_TIM_CHANNEL_INDEX_LED_RED = 0,
    GPIO_TIM_CHANNEL_INDEX_LED_GREEN,
    GPIO_TIM_CHANNEL_INDEX_LED_BLUE,
    GPIO_TIM_CHANNEL_INDEX_LAST
} GPIO_tim_channel_t;
#endif

#ifdef GPSM
/*******************************************************************/
typedef enum {
    GPIO_TIM_RG_CHANNEL_INDEX_LED_RED = 0,
    GPIO_TIM_RG_CHANNEL_INDEX_LED_GREEN,
    GPIO_TIM_RG_CHANNEL_INDEX_LAST
} GPIO_tim_rg_channel_t;
#endif

#ifdef GPSM
/*******************************************************************/
typedef enum {
    GPIO_TIM_B_CHANNEL_INDEX_LED_BLUE = 0,
    GPIO_TIM_B_CHANNEL_INDEX_LAST
} GPIO_tim_b_channel_t;
#endif

/*** GPIO MAPPING global variables ***/

// RS485.
extern const LPUART_gpio_t GPIO_RS485_LPUART;
// Analog inputs.
#if ((defined BPSM) || ((defined LVRM) && (defined HW2_0)))
extern const GPIO_pin_t GPIO_MNTR_EN;
#endif
#ifdef SM
extern const GPIO_pin_t GPIO_ANALOG_POWER_ENABLE;
#endif
extern const ADC_gpio_t GPIO_ADC;
#ifdef XM_RGB_LED
// RGB LED.
#ifdef GPSM
extern const TIM_gpio_t GPIO_LED_TIM_RG;
extern const TIM_gpio_t GPIO_LED_TIM_B;
#else
extern const TIM_gpio_t GPIO_LED_TIM;
#endif
#endif
#if (((defined LVRM) && (defined HW1_0)) || (defined BPSM) || (defined DDRM) || (defined RRM))
// Load control.
extern const GPIO_pin_t GPIO_OUT_EN;
#endif
#ifdef BPSM
// Charge control and status.
extern const GPIO_pin_t GPIO_CHRG_ST;
extern const GPIO_pin_t GPIO_CHRG_EN;
#endif
#ifdef SM
// Digital front-end.
extern const GPIO_pin_t GPIO_DIGITAL_POWER_ENABLE;
extern const GPIO_pin_t GPIO_DIO0;
extern const GPIO_pin_t GPIO_DIO1;
extern const GPIO_pin_t GPIO_DIO2;
extern const GPIO_pin_t GPIO_DIO3;
// Digital sensors.
extern const GPIO_pin_t GPIO_SENSORS_POWER_ENABLE;
extern const I2C_gpio_t GPIO_SENSORS_I2C;
#endif
#if (defined LVRM) && (defined HW2_0)
// Bistable relay control.
extern const GPIO_pin_t GPIO_DC_DC_POWER_ENABLE;
extern const GPIO_pin_t GPIO_COIL_POWER_ENABLE;
extern const GPIO_pin_t GPIO_OUT_SELECT;
extern const GPIO_pin_t GPIO_OUT_CONTROL;
extern const GPIO_pin_t GPIO_MUX_SEL;
extern const GPIO_pin_t GPIO_MUX_EN;
#endif
#ifdef GPSM
// GPS.
extern const GPIO_pin_t GPIO_GPS_POWER_ENABLE;
extern const GPIO_pin_t GPIO_GPS_RESET;
extern const GPIO_pin_t GPIO_GPS_VBCKP;
extern const GPIO_pin_t GPIO_GPS_TIMEPULSE;
extern const GPIO_pin_t GPIO_ANT_POWER_ENABLE;
// USART.
extern const USART_gpio_t GPIO_GPS_USART;
// Test point.
extern const GPIO_pin_t GPIO_TP1;
#endif
#ifdef UHFM
// RF power enable.
extern const GPIO_pin_t GPIO_RF_POWER_ENABLE;
extern const GPIO_pin_t GPIO_RF_TX_ENABLE;
extern const GPIO_pin_t GPIO_RF_RX_ENABLE;
// TCXO power control.
extern const GPIO_pin_t GPIO_TCXO_POWER_ENABLE;
// Radio SPI.
extern const GPIO_pin_t GPIO_S2LP_CS;
extern const SPI_gpio_t GPIO_S2LP_SPI;
// S2LP GPIOs.
extern const GPIO_pin_t GPIO_S2LP_SDN;
extern const GPIO_pin_t GPIO_S2LP_GPIO0;
// Test points.
extern const GPIO_pin_t GPIO_TP1;
extern const GPIO_pin_t GPIO_TP2;
extern const GPIO_pin_t GPIO_TP3;
#endif

#endif /* __GPIO_MAPPING_H__ */
