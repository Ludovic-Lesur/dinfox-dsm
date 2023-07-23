/*
 * rtc.h
 *
 *  Created on: 16 aug. 2020
 *      Author: Ludo
 */

#ifndef __RTC_H__
#define __RTC_H__

#include "types.h"

/*** RTC macros ***/

// RTC wake-up timer period.
// Warning: this value must be lower than the watchdog period (~27s).
#define RTC_WAKEUP_PERIOD_SECONDS	10

/*** RTC structures ***/

/*!******************************************************************
 * \enum RTC_status_t
 * \brief RTC driver error codes.
 *******************************************************************/
typedef enum {
	RTC_SUCCESS = 0,
	RTC_ERROR_NULL_PARAMETER,
	RTC_ERROR_INITIALIZATION_MODE,
	RTC_ERROR_WAKEUP_TIMER_RUNNING,
	RTC_ERROR_WAKEUP_TIMER_REGISTER_ACCESS,
	RTC_ERROR_BASE_LAST = 0x0100
} RTC_status_t;

/*** RTC functions ***/

/*!******************************************************************
 * \fn void RTC_reset(void)
 * \brief Reset RTC peripheral.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void RTC_reset(void);

/*!******************************************************************
 * \fn RTC_status_t RTC_init(void)
 * \brief Init RTC peripheral.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
RTC_status_t RTC_init(void);

/*!******************************************************************
 * \fn RTC_status_t RTC_start_wakeup_timer(uint32_t delay_seconds)
 * \brief Start RTC wakeup timer.
 * \param[in]  	period_seconds: Timer period in seconds.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
RTC_status_t RTC_start_wakeup_timer(uint16_t period_seconds);

/*!******************************************************************
 * \fn RTC_status_t RTC_stop_wakeup_timer(void)
 * \brief Stop RTC wakeup timer.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
RTC_status_t RTC_stop_wakeup_timer(void);

/*!******************************************************************
 * \fn volatile uint8_t RTC_get_wakeup_timer_flag(void)
 * \brief Read RTC wakeup timer interrupt flag.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		RTC wakeup timer flag.
 *******************************************************************/
volatile uint8_t RTC_get_wakeup_timer_flag(void);

/*!******************************************************************
 * \fn void RTC_clear_wakeup_timer_flag(void)
 * \brief Clear RTC wakeup timer interrupt flag.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void RTC_clear_wakeup_timer_flag(void);

/*******************************************************************/
#define RTC_check_status(error_base) { if (rtc_status != RTC_SUCCESS) { status = error_base + rtc_status; goto errors; } }

/*******************************************************************/
#define RTC_stack_error() { ERROR_stack_error(rtc_status, RTC_SUCCESS, ERROR_BASE_RTC); }

/*******************************************************************/
#define RTC_print_error() { ERROR_stack_error(rtc_status, RTC_SUCCESS, ERROR_BASE_RTC); }

#endif /* __RTC_H__ */
