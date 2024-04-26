/*
 * mapping.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "gpio.h"

/*** MAPPING global variables ***/

// ADC inputs.
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined GPSM)
extern const GPIO_pin_t GPIO_ADC1_IN0;
#endif
#if ((defined LVRM) && (defined HW2_0)) || (defined GPSM)
extern const GPIO_pin_t GPIO_ADC1_IN1;
#endif
#if ((defined LVRM) && (defined HW1_0)) || (defined BPSM) || (defined DDRM) || (defined RRM)
extern const GPIO_pin_t GPIO_ADC1_IN4;
#endif
#if (defined LVRM) || (defined BPSM) || (defined RRM)
extern const GPIO_pin_t GPIO_ADC1_IN6;
#endif
#if (defined DDRM) || (defined RRM) || (defined UHFM)
extern const GPIO_pin_t GPIO_ADC1_IN7;
#endif
// Monitoring enable.
#if (defined LVRM) && (defined HW2_0)
extern const GPIO_pin_t GPIO_MNTR_EN;
#endif
// Charge control and status.
#ifdef BPSM
extern const GPIO_pin_t GPIO_MNTR_EN;
extern const GPIO_pin_t GPIO_CHRG_ST;
extern const GPIO_pin_t GPIO_CHRG_EN;
#endif
// Analog front-end.
#ifdef SM
extern const GPIO_pin_t GPIO_ANA_POWER_ENABLE;
extern const GPIO_pin_t GPIO_AIN0;
extern const GPIO_pin_t GPIO_AIN1;
extern const GPIO_pin_t GPIO_AIN2;
extern const GPIO_pin_t GPIO_AIN3;
#endif
// Digital front-end.
#ifdef SM
extern const GPIO_pin_t GPIO_DIG_POWER_ENABLE;
extern const GPIO_pin_t GPIO_DIO0;
extern const GPIO_pin_t GPIO_DIO1;
extern const GPIO_pin_t GPIO_DIO2;
extern const GPIO_pin_t GPIO_DIO3;
#endif
// Digital sensors.
#ifdef SM
extern const GPIO_pin_t GPIO_SEN_POWER_ENABLE;
extern const GPIO_pin_t GPIO_I2C1_SCL;
extern const GPIO_pin_t GPIO_I2C1_SDA;
#endif
// Load control.
#if (defined LVRM) && (defined HW1_0)
extern const GPIO_pin_t GPIO_OUT_EN;
#endif
#if (defined LVRM) && (defined HW2_0)
extern const GPIO_pin_t GPIO_DC_DC_POWER_ENABLE;
extern const GPIO_pin_t GPIO_COIL_POWER_ENABLE;
extern const GPIO_pin_t GPIO_OUT_SELECT;
extern const GPIO_pin_t GPIO_OUT_CONTROL;
#endif
#ifdef BPSM
extern const GPIO_pin_t GPIO_OUT_EN;
#endif
#ifdef DDRM
extern const GPIO_pin_t GPIO_OUT_EN;
#endif
#ifdef RRM
extern const GPIO_pin_t GPIO_OUT_EN;
#endif
// Radio.
#ifdef UHFM
extern const GPIO_pin_t GPIO_SPI1_SCK;
extern const GPIO_pin_t GPIO_SPI1_MISO;
extern const GPIO_pin_t GPIO_SPI1_MOSI;
extern const GPIO_pin_t GPIO_S2LP_CS;
// RF power enable.
extern const GPIO_pin_t GPIO_RF_POWER_ENABLE;
extern const GPIO_pin_t GPIO_RF_TX_ENABLE;
extern const GPIO_pin_t GPIO_RF_RX_ENABLE;
// TCXO power control.
extern const GPIO_pin_t GPIO_TCXO_POWER_ENABLE;
// S2LP GPIOs.
extern const GPIO_pin_t GPIO_S2LP_SDN;
extern const GPIO_pin_t GPIO_S2LP_GPIO0;
#endif
// GPS.
#ifdef GPSM
extern const GPIO_pin_t GPIO_GPS_POWER_ENABLE;
extern const GPIO_pin_t GPIO_GPS_RESET;
extern const GPIO_pin_t GPIO_GPS_VBCKP;
extern const GPIO_pin_t GPIO_GPS_TIMEPULSE;
extern const GPIO_pin_t GPIO_ANT_POWER_ENABLE;
extern const GPIO_pin_t GPIO_USART2_TX;
extern const GPIO_pin_t GPIO_USART2_RX;
#endif
// LPUART1 (RS485).
#if ((defined LVRM) && (defined HW1_0)) || (defined BPSM) || (defined DDRM) || (defined RRM)
extern const GPIO_pin_t GPIO_LPUART1_TX;
extern const GPIO_pin_t GPIO_LPUART1_RX;

#endif
#if ((defined LVRM) && (defined HW2_0)) || (defined SM) || (defined UHFM) || (defined GPSM)
extern const GPIO_pin_t GPIO_LPUART1_TX;
extern const GPIO_pin_t GPIO_LPUART1_RX;
#endif
extern const GPIO_pin_t GPIO_LPUART1_DE;
#if ((defined LVRM) && (defined HW1_0)) || (defined DDRM) || (defined RRM)
extern const GPIO_pin_t GPIO_LPUART1_NRE;
#endif
#ifdef BPSM
extern const GPIO_pin_t GPIO_LPUART1_NRE;
#endif
#if (defined SM) || (defined GPSM)
extern const GPIO_pin_t GPIO_LPUART1_NRE;
#endif
#ifdef UHFM
extern const GPIO_pin_t GPIO_LPUART1_NRE;
#endif
#if (defined LVRM) && (defined HW2_0)
extern const GPIO_pin_t GPIO_LPUART1_NRE;
#endif
// RGB LED.
#if (defined LVRM) && (defined HW1_0)
extern const GPIO_pin_t GPIO_LED_RED;
extern const GPIO_pin_t GPIO_LED_GREEN;
extern const GPIO_pin_t GPIO_LED_BLUE;
#endif
#if (defined LVRM) && (defined HW2_0)
extern const GPIO_pin_t GPIO_LED_RED;
extern const GPIO_pin_t GPIO_LED_GREEN;
extern const GPIO_pin_t GPIO_LED_BLUE;
#endif
#ifdef DDRM
extern const GPIO_pin_t GPIO_LED_RED;
extern const GPIO_pin_t GPIO_LED_GREEN;
extern const GPIO_pin_t GPIO_LED_BLUE;
#endif
#ifdef RRM
extern const GPIO_pin_t GPIO_LED_RED;
extern const GPIO_pin_t GPIO_LED_GREEN;
extern const GPIO_pin_t GPIO_LED_BLUE;
#endif
#ifdef GPSM
extern const GPIO_pin_t GPIO_LED_RED;
extern const GPIO_pin_t GPIO_LED_GREEN;
extern const GPIO_pin_t GPIO_LED_BLUE;
#endif
// MUX control.
#if (defined LVRM) && (defined HW2_0)
extern const GPIO_pin_t GPIO_MUX_SEL;
extern const GPIO_pin_t GPIO_MUX_EN;
#endif
// Test points.
#ifdef UHFM
extern const GPIO_pin_t GPIO_TP1;
extern const GPIO_pin_t GPIO_TP2;
extern const GPIO_pin_t GPIO_TP3;
#endif
#ifdef GPSM
extern const GPIO_pin_t GPIO_TP1;
#endif
// Programming.
extern const GPIO_pin_t GPIO_SWDIO;
extern const GPIO_pin_t GPIO_SWCLK;

#endif /* __MAPPING_H__ */
