/*
 * power.c
 *
 *  Created on: 22 jul. 2023
 *      Author: Ludo
 */

#include "power.h"

#include "adc.h"
#include "gpio.h"
#include "lptim.h"
#include "mapping.h"
#include "neom8n.h"
#include "s2lp.h"
#include "spi.h"
#include "types.h"
#include "usart.h"

/*** POWER functions ***/

/*******************************************************************/
void POWER_init(void) {
	// Init power control pins.
#if ((defined LVRM) && (defined HW2_0)) || (defined BPSM)
	GPIO_configure(&GPIO_MNTR_EN, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
#ifdef GPSM
	GPIO_configure(&GPIO_GPS_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#ifdef GPSM_ACTIVE_ANTENNA
	GPIO_configure(&GPIO_ANT_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
#endif
#ifdef SM
	GPIO_configure(&GPIO_ANA_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
#ifdef UHFM
	// Radio domain.
	GPIO_configure(&GPIO_RF_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_TCXO_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
	// Disable all domains by default.
	POWER_disable(POWER_DOMAIN_ANALOG);
#ifdef GPSM
	POWER_disable(POWER_DOMAIN_GPS);
#endif
#ifdef UHFM
	POWER_disable(POWER_DOMAIN_RADIO);
#endif
}

/*******************************************************************/
POWER_status_t POWER_enable(POWER_domain_t domain, LPTIM_delay_mode_t delay_mode) {
	// Local variables.
	POWER_status_t status = POWER_SUCCESS;
	LPTIM_status_t lptim1_status = LPTIM_SUCCESS;
	uint32_t delay_ms = 0;
	// Check domain.
	switch (domain) {
	case POWER_DOMAIN_ANALOG:
#if ((defined LVRM) && (defined HW2_0)) || (defined BPSM)
		// Enable voltage dividers.
		GPIO_write(&GPIO_MNTR_EN, 1);
		// Set delay value.
		delay_ms = POWER_ON_DELAY_MS_ANALOG;
#endif
#if (defined SM) && (defined SM_AIN_ENABLE)
		// Turn analog front-end on.
		GPIO_write(&GPIO_ANA_POWER_ENABLE, 1);
		// Set delay value.
		delay_ms = POWER_ON_DELAY_MS_ANALOG;
#endif
		// Init peripherals.
		ADC1_init();
		break;
#ifdef GPSM
	case POWER_DOMAIN_GPS:
		// Turn GPS on.
		GPIO_write(&GPIO_GPS_POWER_ENABLE, 1);
#ifdef GPSM_ACTIVE_ANTENNA
		// Turn active antenna on.
		GPIO_write(&GPIO_ANT_POWER_ENABLE, 1);
#endif
		// Init peripherals.
		USART2_init();
		// Init components.
		NEOM8N_init();
		// Set delay value.
		delay_ms = POWER_ON_DELAY_MS_GPS;
		break;
#endif
#ifdef UHFM
	case POWER_DOMAIN_RADIO:
		// Turn radio on.
		GPIO_write(&GPIO_RF_POWER_ENABLE, 1);
		GPIO_write(&GPIO_TCXO_POWER_ENABLE, 1);
		// Init peripherals.
		SPI1_init();
		// Init components.
		S2LP_init();
		// Set delay value.
		delay_ms = POWER_ON_DELAY_MS_RADIO;
		break;
#endif
	default:
		status = POWER_ERROR_DOMAIN;
		goto errors;
	}
	// Power on delay.
	if (delay_ms != 0) {
		lptim1_status = LPTIM1_delay_milliseconds(delay_ms, delay_mode);
		LPTIM1_check_status(POWER_ERROR_BASE_LPTIM);
	}
errors:
	return status;
}

/*******************************************************************/
POWER_status_t POWER_disable(POWER_domain_t domain) {
	// Local variables.
	POWER_status_t status = POWER_SUCCESS;
	// Check domain.
	switch (domain) {
	case POWER_DOMAIN_ANALOG:
		// Release peripherals.
		ADC1_de_init();
#if ((defined LVRM) && (defined HW2_0)) || (defined BPSM)
		// Disable voltage dividers.
		GPIO_write(&GPIO_MNTR_EN, 0);
#endif
#if (defined SM) && (defined SM_AIN_ENABLE)
		// Turn analog front-end off.
		GPIO_write(&GPIO_ANA_POWER_ENABLE, 0);
#endif
		break;
#ifdef GPSM
	case POWER_DOMAIN_GPS:
		// Release components.
		NEOM8N_de_init();
		// Release peripherals.
		USART2_de_init();
#ifdef GPSM_ACTIVE_ANTENNA
		// Turn active antenna off.
		GPIO_write(&GPIO_ANT_POWER_ENABLE, 0);
#endif
		// Turn GPS off.
		GPIO_write(&GPIO_GPS_POWER_ENABLE, 0);
		break;
#endif
#ifdef UHFM
	case POWER_DOMAIN_RADIO:
		// Release components.
		S2LP_de_init();
		// Release peripherals.
		SPI1_de_init();
		// Turn radio off.
		GPIO_write(&GPIO_TCXO_POWER_ENABLE, 0);
		GPIO_write(&GPIO_RF_POWER_ENABLE, 0);
		break;
#endif
	default:
		status = POWER_ERROR_DOMAIN;
		goto errors;
	}
errors:
	return status;
}
