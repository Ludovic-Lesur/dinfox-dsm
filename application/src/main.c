/*
 * main.c
 *
 *  Created on: 05 feb. 2022
 *      Author: Ludo
 */

// Peripherals.
#include "exti.h"
#include "gpio.h"
#include "iwdg.h"
#include "lptim.h"
#include "mcu_mapping.h"
#include "nvic.h"
#include "nvic_priority.h"
#include "pwr.h"
#include "rcc.h"
#include "rtc.h"
#include "types.h"
// Utils.
#include "error.h"
// Middleware.
#include "cli.h"
#include "node.h"
#include "power.h"
// Applicative.
#include "error_base.h"
#include "xm_flags.h"

/*** MAIN local functions ***/

/*******************************************************************/
static void _XM_init_hw(void) {
    // Local variables.
    RCC_status_t rcc_status = RCC_SUCCESS;
    RTC_status_t rtc_status = RTC_SUCCESS;
    NODE_status_t node_status = NODE_SUCCESS;
    CLI_status_t cli_status = CLI_SUCCESS;
#ifndef XM_DEBUG
    IWDG_status_t iwdg_status = IWDG_SUCCESS;
#endif
    // Init error stack
    ERROR_stack_init();
    // Init memory.
    NVIC_init();
    // Init power module and clock tree.
    PWR_init();
    rcc_status = RCC_init(NVIC_PRIORITY_CLOCK);
    RCC_stack_error(ERROR_BASE_RCC);
    // Init GPIOs.
    GPIO_init();
    EXTI_init();
#ifndef XM_DEBUG
    // Start independent watchdog.
    iwdg_status = IWDG_init();
    IWDG_stack_error(ERROR_BASE_IWDG);
    IWDG_reload();
#endif
    // High speed oscillator.
    rcc_status = RCC_switch_to_hsi();
    RCC_stack_error(ERROR_BASE_RCC);
#ifdef GPSM
    // Calibrate clocks.
    rcc_status = RCC_calibrate_internal_clocks(NVIC_PRIORITY_CLOCK_CALIBRATION);
    RCC_stack_error(ERROR_BASE_RCC);
#endif
    // Init RTC.
    rtc_status = RTC_init(NULL, NVIC_PRIORITY_RTC);
    RTC_stack_error(ERROR_BASE_RTC);
    // Init delay timer.
    LPTIM_init(NVIC_PRIORITY_DELAY);
    // Init components.
    POWER_init();
    // Init node layer.
    node_status = NODE_init();
    NODE_stack_error(ERROR_BASE_NODE);
    cli_status = CLI_init();
    CLI_stack_error(ERROR_BASE_CLI);
}

/*** MAIN function ***/

/*******************************************************************/
int main(void) {
    // Local variables.
    NODE_status_t node_status = NODE_SUCCESS;
    CLI_status_t cli_status = CLI_SUCCESS;
    // Init board.
    _XM_init_hw();
    // Main loop.
    while (1) {
        IWDG_reload();
#ifndef XM_DEBUG
        // Enter sleep or stop mode depending on node state.
        if (NODE_get_state() == NODE_STATE_IDLE) {
            PWR_enter_stop_mode();
        }
        else {
            PWR_enter_sleep_mode();
        }
        IWDG_reload();
#endif
        // Perform node tasks.
        node_status = NODE_process();
        NODE_stack_error(ERROR_BASE_NODE);
        // Perform command task.
        cli_status = CLI_process();
        CLI_stack_error(ERROR_BASE_CLI);
    }
}
