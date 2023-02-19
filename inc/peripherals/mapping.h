/*
 * mapping.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "gpio.h"
#include "gpio_reg.h"

// ADC inputs.
#if (defined LVRM) || (defined BPSM) || (defined DDRM)
static const GPIO_pin_t GPIO_ADC1_IN0 =			(GPIO_pin_t) {GPIOA, 0, 0, 0};
#endif
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
static const GPIO_pin_t GPIO_ADC1_IN4 =			(GPIO_pin_t) {GPIOA, 0, 4, 0};
#endif
#if (defined LVRM) || (defined BPSM) || (defined RRM)
static const GPIO_pin_t GPIO_ADC1_IN6 =			(GPIO_pin_t) {GPIOA, 0, 6, 0};
#endif
#if (defined DDRM) || (defined RRM) || (defined UHFM)
static const GPIO_pin_t GPIO_ADC1_IN7 =			(GPIO_pin_t) {GPIOA, 0, 7, 0};
#endif
#ifdef BPSM
// Monitoring enable.
static const GPIO_pin_t GPIO_MNTR_EN =			(GPIO_pin_t) {GPIOA, 0, 1, 0};
// Charge control and status.
static const GPIO_pin_t GPIO_CHRG_ST=			(GPIO_pin_t) {GPIOA, 0, 8, 0};
static const GPIO_pin_t GPIO_CHRG_EN =			(GPIO_pin_t) {GPIOA, 0, 9, 0};
#endif
#ifdef SM
// Analog power enable.
static const GPIO_pin_t GPIO_ANA_POWER_ENABLE =	(GPIO_pin_t) {GPIOA, 0, 8, 0};
// Analog inputs
static const GPIO_pin_t GPIO_AIN0 =				(GPIO_pin_t) {GPIOA, 0, 5, 0};
static const GPIO_pin_t GPIO_AIN1 =				(GPIO_pin_t) {GPIOA, 0, 6, 0};
static const GPIO_pin_t GPIO_AIN2 =				(GPIO_pin_t) {GPIOA, 0, 7, 0};
static const GPIO_pin_t GPIO_AIN3 =				(GPIO_pin_t) {GPIOB, 1, 0, 0};
// Digital power enable.
static const GPIO_pin_t GPIO_DIG_POWER_ENABLE =	(GPIO_pin_t) {GPIOB, 1, 3, 0};
// Digital inputs.
static const GPIO_pin_t GPIO_DIO0 =				(GPIO_pin_t) {GPIOA, 0, 0, 0};
static const GPIO_pin_t GPIO_DIO1 =				(GPIO_pin_t) {GPIOA, 0, 1, 0};
static const GPIO_pin_t GPIO_DIO2 =				(GPIO_pin_t) {GPIOA, 0, 9, 0};
static const GPIO_pin_t GPIO_DIO3 =				(GPIO_pin_t) {GPIOA, 0, 10, 0};
// Sensors power enable.
static const GPIO_pin_t GPIO_SEN_POWER_ENABLE =	(GPIO_pin_t) {GPIOA, 0, 15, 0};
// I2C.
static const GPIO_pin_t GPIO_I2C1_SCL =			(GPIO_pin_t) {GPIOB, 1, 6, 1}; // AF1 = I2C1_SCL.
static const GPIO_pin_t GPIO_I2C1_SDA =			(GPIO_pin_t) {GPIOB, 1, 7, 1}; // AF1 = I2C1_SDA.
#endif
// Output enable.
#ifdef LVRM
static const GPIO_pin_t GPIO_OUT_EN =			(GPIO_pin_t) {GPIOA, 0, 7, 0};
#endif
#ifdef BPSM
static const GPIO_pin_t GPIO_OUT_EN =			(GPIO_pin_t) {GPIOA, 0, 5, 0};
#endif
#ifdef DDRM
static const GPIO_pin_t GPIO_OUT_EN =			(GPIO_pin_t) {GPIOA, 0, 6, 0};
#endif
#ifdef RRM
static const GPIO_pin_t GPIO_OUT_EN =			(GPIO_pin_t) {GPIOA, 0, 0, 0};
#endif
#ifdef UHFM
// SPI1.
static const GPIO_pin_t GPIO_SPI1_SCK = 			(GPIO_pin_t) {GPIOB, 1, 3, 0};
static const GPIO_pin_t GPIO_SPI1_MISO = 			(GPIO_pin_t) {GPIOB, 1, 4, 0};
static const GPIO_pin_t GPIO_SPI1_MOSI = 			(GPIO_pin_t) {GPIOB, 1, 5, 0};
static const GPIO_pin_t GPIO_S2LP_CS = 				(GPIO_pin_t) {GPIOA, 0, 15, 0};
// RF power enable.
static const GPIO_pin_t GPIO_RF_POWER_ENABLE =		(GPIO_pin_t) {GPIOB, 1, 8, 0};
static const GPIO_pin_t GPIO_RF_TX_ENABLE =			(GPIO_pin_t) {GPIOB, 1, 7, 0};
static const GPIO_pin_t GPIO_RF_RX_ENABLE =			(GPIO_pin_t) {GPIOB, 1, 6, 0};
// TCXO power control.
static const GPIO_pin_t GPIO_TCXO_POWER_ENABLE =	(GPIO_pin_t) {GPIOA, 0, 8, 0};
// S2LP GPIOs.
static const GPIO_pin_t GPIO_S2LP_SDN =				(GPIO_pin_t) {GPIOA, 0, 9, 0};
static const GPIO_pin_t GPIO_S2LP_GPIO0 =			(GPIO_pin_t) {GPIOA, 0, 11, 0};
// Test points.
static const GPIO_pin_t GPIO_TP1 =					(GPIO_pin_t) {GPIOA, 0, 0, 0};
static const GPIO_pin_t GPIO_TP2 =					(GPIO_pin_t) {GPIOA, 0, 5, 0};
static const GPIO_pin_t GPIO_TP3 =					(GPIO_pin_t) {GPIOA, 0, 12, 0};
#endif
// LPUART1 (RS485).
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
static const GPIO_pin_t GPIO_LPUART1_TX =		(GPIO_pin_t) {GPIOB, 1, 6, 6}; // AF6 = LPUART1_TX.
static const GPIO_pin_t GPIO_LPUART1_RX =		(GPIO_pin_t) {GPIOB, 1, 7, 6}; // AF6 = LPUART1_RX.

#endif
#if (defined SM) || (defined UHFM)
static const GPIO_pin_t GPIO_LPUART1_TX =		(GPIO_pin_t) {GPIOA, 0, 2, 6}; // AF6 = LPUART1_TX.
static const GPIO_pin_t GPIO_LPUART1_RX =		(GPIO_pin_t) {GPIOA, 0, 3, 6}; // AF6 = LPUART1_RX.
#endif
static const GPIO_pin_t GPIO_LPUART1_DE =		(GPIO_pin_t) {GPIOB, 1, 1, 4}; // AF4 = LPUART1_DE.
#if (defined LVRM) || (defined DDRM) || (defined RRM)
static const GPIO_pin_t GPIO_LPUART1_NRE =		(GPIO_pin_t) {GPIOA, 0, 9, 0};
#endif
#ifdef BPSM
static const GPIO_pin_t GPIO_LPUART1_NRE =		(GPIO_pin_t) {GPIOA, 0, 10, 0};
#endif
#ifdef SM
static const GPIO_pin_t GPIO_LPUART1_NRE =		(GPIO_pin_t) {GPIOA, 0, 4, 0};
#endif
#ifdef UHFM
static const GPIO_pin_t GPIO_LPUART1_NRE =		(GPIO_pin_t) {GPIOB, 1, 2, 0};
#endif
// RGB LED.
#ifdef LVRM
static const GPIO_pin_t GPIO_LED_RED =			(GPIO_pin_t) {GPIOA, 0, 10, 5}; // AF5 = TIM2_CH3.
static const GPIO_pin_t GPIO_LED_GREEN =		(GPIO_pin_t) {GPIOA, 0, 1, 2}; // AF2 = TIM2_CH2.
static const GPIO_pin_t GPIO_LED_BLUE =			(GPIO_pin_t) {GPIOA, 0, 5, 5}; // AF5 = TIM2_CH1.
#endif
#ifdef DDRM
static const GPIO_pin_t GPIO_LED_RED =			(GPIO_pin_t) {GPIOA, 0, 10, 5}; // AF5 = TIM2_CH3.
static const GPIO_pin_t GPIO_LED_GREEN =		(GPIO_pin_t) {GPIOA, 0, 5, 5}; // AF5 = TIM2_CH1.
static const GPIO_pin_t GPIO_LED_BLUE =			(GPIO_pin_t) {GPIOA, 0, 1, 2}; // AF2 = TIM2_CH2.
#endif
#ifdef RRM
static const GPIO_pin_t GPIO_LED_RED =			(GPIO_pin_t) {GPIOA, 0, 1, 2}; // AF2 = TIM2_CH2.
static const GPIO_pin_t GPIO_LED_GREEN =		(GPIO_pin_t) {GPIOA, 0, 10, 5}; // AF5 = TIM2_CH3.
static const GPIO_pin_t GPIO_LED_BLUE =			(GPIO_pin_t) {GPIOA, 0, 5, 5}; // AF5 = TIM2_CH1.
#endif
// Programming.
static const GPIO_pin_t GPIO_SWDIO =			(GPIO_pin_t) {GPIOA, 0, 13, 0};
static const GPIO_pin_t GPIO_SWCLK =			(GPIO_pin_t) {GPIOA, 0, 14, 0};

#endif /* __MAPPING_H__ */
