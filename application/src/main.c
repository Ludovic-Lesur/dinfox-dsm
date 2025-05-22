/*
 * main.c
 *
 *  Created on: 05 feb. 2022
 *      Author: Ludo
 */

// Peripherals.
#include "exti.h"
#ifdef MPMCM
#include "fpu.h"
#endif
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
#include "dsm_flags.h"
#include "error_base.h"

/*** MAIN local structures ***/

#ifdef MPMCM
/*******************************************************************/
typedef struct {
    volatile uint8_t rtc_wakeup_timer_flag;
} DSM_context_t;
#endif

/*** MAIN local global variables ***/

#ifdef MPMCM
static DSM_context_t dsm_ctx = {
    .rtc_wakeup_timer_flag = 0
};
#endif

/*** MAIN local functions ***/

#ifdef MPMCM
/*******************************************************************/
static void _DSM_rtc_wakeup_timer_irq_callback(void) {
    dsm_ctx.rtc_wakeup_timer_flag = 1;
}
#endif

/*******************************************************************/
static void _DSM_init_hw(void) {
    // Local variables.
    RCC_status_t rcc_status = RCC_SUCCESS;
    RTC_status_t rtc_status = RTC_SUCCESS;
    LPTIM_status_t lptim_status = LPTIM_SUCCESS;
    NODE_status_t node_status = NODE_SUCCESS;
    CLI_status_t cli_status = CLI_SUCCESS;
#ifndef DSM_DEBUG
    IWDG_status_t iwdg_status = IWDG_SUCCESS;
#endif
    // Init error stack
    ERROR_stack_init();
    // Init memory.
    NVIC_init();
#ifdef MPMCM
    // Enable FPU.
    FPU_init();
#endif
    // Init power module and clock tree.
    PWR_init();
    rcc_status = RCC_init(NVIC_PRIORITY_CLOCK);
    RCC_stack_error(ERROR_BASE_RCC);
    // Init GPIOs.
    GPIO_init();
    POWER_init();
    EXTI_init();
#ifndef DSM_DEBUG
    // Start independent watchdog.
    iwdg_status = IWDG_init();
    IWDG_stack_error(ERROR_BASE_IWDG);
    IWDG_reload();
#endif
#ifndef MPMCM
    // High speed oscillator.
    rcc_status = RCC_switch_to_hsi();
    RCC_stack_error(ERROR_BASE_RCC);
#endif
#if ((defined GPSM) || (defined MPMCM))
    // Calibrate clocks.
    rcc_status = RCC_calibrate_internal_clocks(NVIC_PRIORITY_CLOCK_CALIBRATION);
    RCC_stack_error(ERROR_BASE_RCC);
#endif
    // Init RTC.
#ifdef MPMCM
    rtc_status = RTC_init(&_DSM_rtc_wakeup_timer_irq_callback, NVIC_PRIORITY_RTC);
#else
    rtc_status = RTC_init(NULL, NVIC_PRIORITY_RTC);
#endif
    RTC_stack_error(ERROR_BASE_RTC);
    // Init delay timer.
    lptim_status = LPTIM_init(NVIC_PRIORITY_DELAY);
    LPTIM_stack_error(ERROR_BASE_LPTIM);
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
    _DSM_init_hw();
    // Main loop.
    while (1) {
        IWDG_reload();
#ifndef DSM_DEBUG
        // Enter sleep or stop mode depending on node state.
        if (NODE_get_state() == NODE_STATE_IDLE) {
#ifdef MPMCM
            PWR_enter_deepsleep_mode(PWR_DEEPSLEEP_MODE_STOP_1);
#else
            PWR_enter_deepsleep_mode(PWR_DEEPSLEEP_MODE_STOP);
#endif
        }
        else {
            PWR_enter_sleep_mode(PWR_SLEEP_MODE_NORMAL);
        }
        IWDG_reload();
#endif
#ifdef MPMCM
        // Check RTC flag.
        if (dsm_ctx.rtc_wakeup_timer_flag != 0) {
            // Clear flag.
            dsm_ctx.rtc_wakeup_timer_flag = 0;
            // Call tick second function.
            node_status = NODE_tick_second();
            NODE_stack_error(ERROR_BASE_NODE);
        }
#endif
        // Perform node tasks.
        node_status = NODE_process();
        NODE_stack_error(ERROR_BASE_NODE);
        // Perform command task.
        cli_status = CLI_process();
        CLI_stack_error(ERROR_BASE_CLI);
    }
}
