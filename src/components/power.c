/*
 * power.c
 *
 *  Created on: 22 jul. 2023
 *      Author: Ludo
 */

#include "power.h"

#include "gpio.h"
#include "lptim.h"
#include "mapping.h"
#include "s2lp.h"
#include "spi.h"
#include "types.h"

/*** POWER functions ***/

/*******************************************************************/
void POWER_init(void) {
	// Init power control pins.
#ifdef UHFM
	// Radio domain.
	GPIO_configure(&GPIO_RF_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_TCXO_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
	// Disable all domains by default.
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
#ifdef UHFM
	case POWER_DOMAIN_RADIO:
		// Turn power supply domain on.
		GPIO_write(&GPIO_RF_POWER_ENABLE, 1);
		GPIO_write(&GPIO_TCXO_POWER_ENABLE, 1);
		// Init peripherals.
		SPI1_init();
		// Init all components connected to the power domain.
		S2LP_init();
		// set delay value.
		delay_ms = POWER_ON_DELAY_MS_RADIO;
		break;
#endif
	default:
		status = POWER_ERROR_DOMAIN;
		goto errors;
	}
	// Power on delay.
	lptim1_status = LPTIM1_delay_milliseconds(delay_ms, delay_mode);
	LPTIM1_check_status(POWER_ERROR_BASE_LPTIM);
errors:
	return status;
}

/*******************************************************************/
POWER_status_t POWER_disable(POWER_domain_t domain) {
	// Local variables.
	POWER_status_t status = POWER_SUCCESS;
	// Check domain.
	switch (domain) {
#ifdef UHFM
	case POWER_DOMAIN_RADIO:
		// Release all components connected to the power domain.
		S2LP_de_init();
		// Release interface peripherals.
		SPI1_de_init();
		// Turn power supply domain off.
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
