/*
 * exti.c
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#include "exti.h"

#include "exti_reg.h"
#include "gpio.h"
#include "mapping.h"
#include "nvic.h"
#include "rcc_reg.h"
#include "syscfg_reg.h"
#include "types.h"
#ifdef UHFM
#include "manuf/rf_api.h"
#endif

/*** EXTI local macros ***/

#define EXTI_RTSR_FTSR_RESERVED_INDEX	18
#define EXTI_RTSR_FTSR_MAX_INDEX		22

/*** EXTI local global variables ***/

static EXTI_gpio_irq_cb exti_gpio_irq_callbacks[GPIO_PINS_PER_PORT];

/*** EXTI local functions ***/

#ifdef UHFM
/*******************************************************************/
void __attribute__((optimize("-O0"))) EXTI4_15_IRQHandler(void) {
	// S2LP GPIO0 (PA11).
	if (((EXTI -> PR) & (0b1 << (GPIO_S2LP_GPIO0.pin))) != 0) {
		// Set applicative flag.
		if ((((EXTI -> IMR) & (0b1 << (GPIO_S2LP_GPIO0.pin))) != 0) && (exti_gpio_irq_callbacks[GPIO_S2LP_GPIO0.pin] != NULL)) {
			// Execute callback.
			exti_gpio_irq_callbacks[GPIO_S2LP_GPIO0.pin]();
		}
		// Clear flag.
		EXTI -> PR |= (0b1 << (GPIO_S2LP_GPIO0.pin));
	}
}
#endif

/*******************************************************************/
static void _EXTI_set_trigger(EXTI_trigger_t trigger, uint8_t line_idx) {
	// Select triggers.
	switch (trigger) {
	// Rising edge only.
	case EXTI_TRIGGER_RISING_EDGE:
		EXTI -> RTSR |= (0b1 << line_idx); // Rising edge enabled.
		EXTI -> FTSR &= ~(0b1 << line_idx); // Falling edge disabled.
		break;
	// Falling edge only.
	case EXTI_TRIGGER_FALLING_EDGE:
		EXTI -> RTSR &= ~(0b1 << line_idx); // Rising edge disabled.
		EXTI -> FTSR |= (0b1 << line_idx); // Falling edge enabled.
		break;
	// Both edges.
	case EXTI_TRIGGER_ANY_EDGE:
		EXTI -> RTSR |= (0b1 << line_idx); // Rising edge enabled.
		EXTI -> FTSR |= (0b1 << line_idx); // Falling edge enabled.
		break;
	// Unknown configuration.
	default:
		break;
	}
	// Clear flag.
	EXTI -> PR |= (0b1 << line_idx);
}

/*** EXTI functions ***/

/*******************************************************************/
void EXTI_init(void) {
	// Local variables.
	uint8_t idx = 0;
	// Enable peripheral clock.
	RCC -> APB2ENR |= (0b1 << 0); // SYSCFEN='1'.
	// Mask all sources by default.
	EXTI -> IMR = 0;
	// Clear all flags.
	EXTI_clear_all_flags();
	// Reset callbacks.
	for (idx=0 ; idx<GPIO_PINS_PER_PORT ; idx++) {
		exti_gpio_irq_callbacks[idx] = NULL;
	}
#ifdef UHFM
	NVIC_set_priority(NVIC_INTERRUPT_EXTI_4_15, 0);
#endif
}

/*******************************************************************/
void EXTI_configure_gpio(const GPIO_pin_t* gpio, EXTI_trigger_t trigger, EXTI_gpio_irq_cb irq_callback) {
	// Select GPIO port.
	SYSCFG -> EXTICR[((gpio -> pin) / 4)] &= ~(0b1111 << (4 * ((gpio -> pin) % 4)));
	SYSCFG -> EXTICR[((gpio -> pin) / 4)] |= ((gpio -> port_index) << (4 * ((gpio -> pin) % 4)));
	// Set mask.
	EXTI -> IMR |= (0b1 << ((gpio -> pin))); // IMx='1'.
	// Select triggers.
	_EXTI_set_trigger(trigger, (gpio -> pin));
	// Register callback.
	exti_gpio_irq_callbacks[gpio -> pin] = irq_callback;
}

/*******************************************************************/
void EXTI_configure_line(EXTI_line_t line, EXTI_trigger_t trigger) {
	// Set mask.
	EXTI -> IMR |= (0b1 << line); // IMx='1'.
	// Select triggers.
	if ((line != EXTI_RTSR_FTSR_RESERVED_INDEX) || (line <= EXTI_RTSR_FTSR_MAX_INDEX)) {
		_EXTI_set_trigger(trigger, line);
	}
}

/*******************************************************************/
void EXTI_clear_flag(EXTI_line_t line) {
	// Clear flag.
	EXTI -> PR |= line; // PIFx='1'.
}

/*******************************************************************/
void EXTI_clear_all_flags(void) {
	// Clear all flags.
	EXTI -> PR |= 0x007BFFFF; // PIFx='1'.
}
