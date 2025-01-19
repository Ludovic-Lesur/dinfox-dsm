/*
 * digital.c
 *
 *  Created on: 15 dec. 2024
 *      Author: Ludo
 */

#include "digital.h"

#include "error.h"
#include "gpio.h"
#include "gpio_mapping.h"
#include "types.h"
#include "xm_flags.h"

#ifdef SM

/*** DIGITAL local global variables ***/

static const GPIO_pin_t* DIGITAL_CHANNEL_GPIO[DIGITAL_CHANNEL_LAST] = { &GPIO_DIO0, &GPIO_DIO1, &GPIO_DIO2, &GPIO_DIO3 };

/*** DIGITAL functions ***/

/*******************************************************************/
DIGITAL_status_t DIGITAL_init(void) {
    // Local variables.
    DIGITAL_status_t status = DIGITAL_SUCCESS;
    uint8_t channel = 0;
    // Configure digital inputs.
    for (channel = 0; channel < DIGITAL_CHANNEL_LAST; channel++) {
        GPIO_configure(DIGITAL_CHANNEL_GPIO[channel], GPIO_MODE_INPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    }
    return status;
}

/*******************************************************************/
DIGITAL_status_t DIGITAL_de_init(void) {
    // Local variables.
    DIGITAL_status_t status = DIGITAL_SUCCESS;
    uint8_t channel = 0;
    // Release digital inputs.
    for (channel = 0; channel < DIGITAL_CHANNEL_LAST; channel++) {
        GPIO_configure(DIGITAL_CHANNEL_GPIO[channel], GPIO_MODE_ANALOG, GPIO_TYPE_OPEN_DRAIN, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    }
    return status;
}

/*******************************************************************/
DIGITAL_status_t DIGITAL_read_channel(DIGITAL_channel_t channel, uint8_t* state) {
    // Local variables.
    DIGITAL_status_t status = DIGITAL_SUCCESS;
    // Check parameters.
    if (channel >= DIGITAL_CHANNEL_LAST) {
        status = DIGITAL_ERROR_CHANNEL;
        goto errors;
    }
    if (state == NULL) {
        status = DIGITAL_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Read GPIO.
    (*state) = GPIO_read(DIGITAL_CHANNEL_GPIO[channel]);
errors:
    return status;
}

#endif /* SM */
