/*
 * node.c
 *
 *  Created on: 28 may. 2023
 *      Author: Ludo
 */

#include "node.h"

#include "bcm.h"
#include "bcm_registers.h"
#include "bpsm.h"
#include "bpsm_registers.h"
#include "common.h"
#include "common_registers.h"
#include "ddrm.h"
#include "ddrm_registers.h"
#include "dsm_flags.h"
#include "error.h"
#include "error_base.h"
#include "gpsm.h"
#include "gpsm_registers.h"
#include "led.h"
#include "lvrm.h"
#include "lvrm_registers.h"
#include "mpmcm.h"
#include "mpmcm_registers.h"
#include "node_register.h"
#include "node_status.h"
#include "nvm.h"
#include "nvm_address.h"
#include "power.h"
#include "pwr.h"
#include "rtc.h"
#include "rrm.h"
#include "rrm_registers.h"
#include "sm.h"
#include "sm_registers.h"
#include "swreg.h"
#include "uhfm.h"
#include "uhfm_registers.h"
#include "una.h"

/*** NODE local macros ***/

#ifdef DSM_IOUT_INDICATOR
#ifdef BCM
#define NODE_IOUT_CHANNEL_INPUT_VOLTAGE         ANALOG_CHANNEL_VSRC_MV
#define NODE_IOUT_CHANNEL                       ANALOG_CHANNEL_ISTR_UA
#else
#define NODE_IOUT_CHANNEL_INPUT_VOLTAGE         ANALOG_CHANNEL_VIN_MV
#define NODE_IOUT_CHANNEL                       ANALOG_CHANNEL_IOUT_UA
#endif
#define NODE_IOUT_MEASUREMENTS_PERIOD_SECONDS   60
#define NODE_IOUT_INDICATOR_PERIOD_SECONDS      10
#define NODE_IOUT_INDICATOR_RANGE               7
#define NODE_IOUT_INDICATOR_POWER_THRESHOLD_MV  6000
#define NODE_IOUT_INDICATOR_BLINK_DURATION_US   2000000
#endif

/*** NODE local structures ***/

#ifdef DSM_IOUT_INDICATOR
/*******************************************************************/
typedef struct {
    int32_t threshold_ua;
    LED_color_t led_color;
} NODE_iout_indicator_t;
#endif


/*******************************************************************/
typedef struct {
    uint8_t internal_access;
#ifdef DSM_IOUT_INDICATOR
    uint32_t iout_measurements_next_time_seconds;
    uint32_t iout_indicator_next_time_seconds;
    int32_t input_voltage_mv;
    int32_t iout_ua;
#endif
} NODE_context_t;

/*** NODE local global variables ***/

#ifdef DSM_IOUT_INDICATOR
static const NODE_iout_indicator_t NODE_IOUT_INDICATOR[NODE_IOUT_INDICATOR_RANGE] = {
    {0,       LED_COLOR_GREEN},
    {50000,   LED_COLOR_YELLOW},
    {500000,  LED_COLOR_RED},
    {1000000, LED_COLOR_MAGENTA},
    {2000000, LED_COLOR_BLUE},
    {3000000, LED_COLOR_CYAN},
    {4000000, LED_COLOR_WHITE}
};
#endif

static NODE_context_t node_ctx = {
    .internal_access = 0,
#ifdef DSM_IOUT_INDICATOR
    .iout_measurements_next_time_seconds = 0,
    .iout_indicator_next_time_seconds = 0,
    .input_voltage_mv = 0,
    .iout_ua = 0,
#endif
};

/*** NODE local functions ***/

#ifdef DSM_IOUT_INDICATOR
/*******************************************************************/
static NODE_status_t _NODE_iout_measurement(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    // Turn analog front-end on.
    POWER_enable(POWER_REQUESTER_ID_NODE, POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
    // Check input voltage.
    analog_status = ANALOG_convert_channel(NODE_IOUT_CHANNEL_INPUT_VOLTAGE, &(node_ctx.input_voltage_mv));
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    // Enable RGB LED only if power input supplies the board.
    if (node_ctx.input_voltage_mv > NODE_IOUT_INDICATOR_POWER_THRESHOLD_MV) {
        // Read output current.
        analog_status = ANALOG_convert_channel(NODE_IOUT_CHANNEL, &(node_ctx.iout_ua));
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    }
errors:
    POWER_disable(POWER_REQUESTER_ID_NODE, POWER_DOMAIN_ANALOG);
    return status;
}
#endif

#ifdef DSM_IOUT_INDICATOR
/*******************************************************************/
static NODE_status_t _NODE_iout_indicator(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    LED_status_t led_status = LED_SUCCESS;
    LED_color_t led_color;
    uint8_t idx = NODE_IOUT_INDICATOR_RANGE;
    // Enable RGB LED only if power input supplies the board.
    if (node_ctx.input_voltage_mv > NODE_IOUT_INDICATOR_POWER_THRESHOLD_MV) {
        // Compute LED color according to output current..
        do {
            idx--;
            // Get range and corresponding color.
            led_color = NODE_IOUT_INDICATOR[idx].led_color;
            if (node_ctx.iout_ua >= NODE_IOUT_INDICATOR[idx].threshold_ua) break;
        }
        while (idx > 0);
        // Blink LED.
        led_status = LED_start_single_blink(NODE_IOUT_INDICATOR_BLINK_DURATION_US, led_color);
        LED_exit_error(NODE_ERROR_BASE_LED);
    }
errors:
    return status;
}
#endif

#ifndef DSM_NVM_FACTORY_RESET
/*******************************************************************/
static NODE_status_t _NODE_load_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    NVM_status_t nvm_status = NVM_SUCCESS;
#ifndef MPMCM
    uint8_t nvm_byte = 0;
    uint8_t idx = 0;
#endif
#ifdef MPMCM
    nvm_status = NVM_read_word((NVM_ADDRESS_UNA_REGISTERS + reg_addr), reg_value);
    NVM_exit_error(NODE_ERROR_BASE_NVM);
#else
    // Reset output.
    (*reg_value) = 0;
    // Byte loop.
    for (idx = 0; idx < UNA_REGISTER_SIZE_BYTES; idx++) {
        // Read NVM.
        nvm_status = NVM_read_byte((NVM_ADDRESS_UNA_REGISTERS + (reg_addr << 2) + idx), &nvm_byte);
        NVM_exit_error(NODE_ERROR_BASE_NVM);
        // Update output value.
        (*reg_value) |= ((uint32_t) nvm_byte) << (idx << 3);
    }
#endif
errors:
    return status;
}
#endif

/*******************************************************************/
static NODE_status_t _NODE_store_register(uint8_t reg_addr) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    NVM_status_t nvm_status = NVM_SUCCESS;
#ifdef MPMCM
    nvm_status = NVM_write_word((NVM_ADDRESS_UNA_REGISTERS + reg_addr), NODE_RAM_REGISTER[reg_addr]);
    NVM_exit_error(NODE_ERROR_BASE_NVM);
#else
    uint8_t nvm_byte = 0;
    uint8_t idx = 0;
    // Byte loop.
    for (idx = 0; idx < UNA_REGISTER_SIZE_BYTES; idx++) {
        // Compute byte.
        nvm_byte = (uint8_t) (((NODE_RAM_REGISTER[reg_addr]) >> (idx << 3)) & 0x000000FF);
        // Write NVM.
        nvm_status = NVM_write_byte((NVM_ADDRESS_UNA_REGISTERS + (reg_addr << 2) + idx), nvm_byte);
        NVM_exit_error(NODE_ERROR_BASE_NVM);
    }
#endif
errors:
    return status;
}

/*******************************************************************/
static void _NODE_refresh_register(uint8_t reg_addr) {
    // Refresh registers.
    COMMON_refresh_register(reg_addr);
    NODE_REFRESH_REGISTER(reg_addr);
}

/*******************************************************************/
static NODE_status_t _NODE_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Secure register.
    status = COMMON_secure_register(reg_addr, new_reg_value, reg_mask, reg_value);
    status = NODE_SECURE_REGISTER(reg_addr, new_reg_value, reg_mask, reg_value);
    return status;
}

/*******************************************************************/
static NODE_status_t _NODE_process_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Process register.
    status = COMMON_process_register(reg_addr, reg_mask);
    status = NODE_PROCESS_REGISTER(reg_addr, reg_mask);
    return status;
}

/*** NODE functions ***/

/*******************************************************************/
NODE_status_t NODE_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    NODE_status_t node_status = NODE_SUCCESS;
    uint32_t init_reg_value = 0;
    uint8_t reg_addr = 0;
#ifdef DSM_RGB_LED
    LED_status_t led_status = LED_SUCCESS;
#endif
#ifdef MPMCM
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    MEASURE_status_t measure_status = MEASURE_SUCCESS;
#endif
#ifdef MPMCM_LINKY_TIC_ENABLE
    TIC_status_t tic_status = TIC_SUCCESS;
#endif
#endif
    // Init context.
    node_ctx.internal_access = 1;
#ifdef DSM_IOUT_INDICATOR
    node_ctx.iout_measurements_next_time_seconds = 0;
    node_ctx.iout_indicator_next_time_seconds = 0;
    node_ctx.input_voltage_mv = 0;
    node_ctx.iout_ua = 0;
#endif
    // Init registers.
    for (reg_addr = 0; reg_addr < NODE_REGISTER_ADDRESS_LAST; reg_addr++) {
        // Check reset value.
        switch (NODE_REGISTER[reg_addr].reset_value) {
        case UNA_REGISTER_RESET_VALUE_STATIC:
            // Init to error value.
            init_reg_value = NODE_REGISTER[reg_addr].error_value;
            break;
#ifndef DSM_NVM_FACTORY_RESET
        case UNA_REGISTER_RESET_VALUE_NVM:
            // Read NVM.
            node_status = _NODE_load_register(reg_addr, &init_reg_value);
            NODE_stack_error(ERROR_BASE_NODE);
            break;
#endif
        default:
            // Init to default value.
            init_reg_value = 0x00000000;
            break;
        }
        // Override with specific initialization value.
        COMMON_init_register(reg_addr, &init_reg_value);
        NODE_INIT_REGISTER(reg_addr, &init_reg_value);
        // Write initial value.
        node_status = NODE_write_register(reg_addr, init_reg_value, UNA_REGISTER_MASK_ALL);
        NODE_stack_error(ERROR_BASE_NODE);
    }
#ifdef DSM_LOAD_CONTROL
    LOAD_init();
#endif
#ifdef DSM_RGB_LED
    led_status = LED_init();
    LED_stack_error(ERROR_BASE_LED);
#endif
#ifdef MPMCM
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    measure_status = MEASURE_init();
    MEASURE_stack_error(ERROR_BASE_MEASURE);
#endif
#ifdef MPMCM_LINKY_TIC_ENABLE
    tic_status = TIC_init();
    TIC_stack_error(ERROR_BASE_TIC);
#endif
#endif
    // Init specific driver.
    status = NODE_INIT();
    // Disable internal access.
    node_ctx.internal_access = 0;
    return status;
}

/*******************************************************************/
NODE_status_t NODE_de_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef MPMCM
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    MEASURE_status_t measure_status = MEASURE_SUCCESS;
#endif
#ifdef MPMCM_LINKY_TIC_ENABLE
    TIC_status_t tic_status = TIC_SUCCESS;
#endif
#endif
#ifdef MPMCM
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    measure_status = MEASURE_de_init();
    MEASURE_stack_error(ERROR_BASE_MEASURE);
#endif
#ifdef MPMCM_LINKY_TIC_ENABLE
    tic_status = TIC_de_init();
    TIC_stack_error(ERROR_BASE_TIC);
#endif
#endif
#ifdef DSM_RGB_LED
    LED_status_t led_status = LED_SUCCESS;
    led_status = LED_de_init();
    LED_stack_error(ERROR_BASE_NODE + NODE_ERROR_BASE_LED);
#endif
    return status;
}

/*******************************************************************/
NODE_status_t NODE_process(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#if (((defined LVRM) && (defined LVRM_MODE_BMS)) || (defined BPSM) || (defined BCM) || (defined DSM_IOUT_INDICATOR))
    NODE_status_t node_status = NODE_SUCCESS;
#endif
#if ((defined MPMCM) && (defined MPMCM_LINKY_TIC_ENABLE))
    TIC_status_t tic_status = TIC_SUCCESS;
#endif
    // Read RTRG bit.
    if (SWREG_read_field(NODE_RAM_REGISTER[COMMON_REGISTER_ADDRESS_CONTROL_0], COMMON_REGISTER_CONTROL_0_MASK_RTRG) != 0) {
        // Reset MCU.
        PWR_software_reset();
    }
#if ((defined LVRM) && (defined LVRM_MODE_BMS))
    status = LVRM_bms_process();
    NODE_stack_error(ERROR_BASE_NODE);
#endif
#ifdef BPSM
    node_status = BPSM_low_voltage_detector_process();
    NODE_stack_error(ERROR_BASE_NODE);
#ifndef BPSM_CHEN_FORCED_HARDWARE
    node_status = BPSM_charge_process();
    NODE_stack_error(ERROR_BASE_NODE);
#endif
#endif
#if ((defined MPMCM) && (defined MPMCM_LINKY_TIC_ENABLE))
    // Process TIC interface.
    tic_status = TIC_process();
    TIC_stack_error(ERROR_BASE_TIC);
#endif
#ifdef BCM
    node_status = BCM_low_voltage_detector_process();
    NODE_stack_error(ERROR_BASE_NODE);
#ifndef BCM_CHEN_FORCED_HARDWARE
    node_status = BCM_charge_process();
    NODE_stack_error(ERROR_BASE_NODE);
#endif
#endif
#ifdef DSM_IOUT_INDICATOR
    // Check measurements period.
    if (RTC_get_uptime_seconds() >= node_ctx.iout_measurements_next_time_seconds) {
        // Update next time.
        node_ctx.iout_measurements_next_time_seconds = RTC_get_uptime_seconds() + NODE_IOUT_MEASUREMENTS_PERIOD_SECONDS;
        // Perform measurements.
        node_status = _NODE_iout_measurement();
        NODE_stack_error(ERROR_BASE_NODE);
    }
    // Check LED period.
    if (RTC_get_uptime_seconds() >= node_ctx.iout_indicator_next_time_seconds) {
        // Update next time.
        node_ctx.iout_indicator_next_time_seconds = RTC_get_uptime_seconds() + NODE_IOUT_INDICATOR_PERIOD_SECONDS;
        // Perform LED task.
        node_status = _NODE_iout_indicator();
        NODE_stack_error(ERROR_BASE_NODE);
    }
#endif
    return status;
}

#ifdef MPMCM
/*******************************************************************/
NODE_status_t NODE_tick_second(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    MEASURE_status_t measure_status = MEASURE_SUCCESS;
#endif
#ifdef MPMCM_LINKY_TIC_ENABLE
    // Call TIC interface tick.
    TIC_tick_second();
#endif
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
    // Call measure tick.
    measure_status = MEASURE_tick_second();
    MEASURE_exit_error(ERROR_BASE_MEASURE);
#endif
#ifdef MPMCM_ANALOG_MEASURE_ENABLE
errors:
#endif
    return status;
}
#endif

/*******************************************************************/
NODE_state_t NODE_get_state(void) {
    // Local variables.
    NODE_state_t state = NODE_STATE_IDLE;
#ifdef MPMCM
    state = ((TIC_get_state() == TIC_STATE_OFF) && (MEASURE_get_state() == MEASURE_STATE_OFF) && (LED_get_state() == LED_STATE_OFF)) ? NODE_STATE_IDLE : NODE_STATE_RUNNING;
#else
#ifdef DSM_IOUT_INDICATOR
    state = (LED_get_state() == LED_STATE_OFF) ? NODE_STATE_IDLE : NODE_STATE_RUNNING;
#endif
#endif
    return state;
}

/*******************************************************************/
NODE_status_t NODE_write_register(uint8_t reg_addr, uint32_t reg_value, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    NODE_status_t node_status = NODE_SUCCESS;
    uint32_t safe_reg_mask = reg_mask;
    uint32_t safe_reg_value = reg_value;
    // Check address.
    if (reg_addr >= NODE_REGISTER_ADDRESS_LAST) {
        status = NODE_ERROR_REGISTER_ADDRESS;
        goto errors;
    }
    // Check access.
    if ((node_ctx.internal_access == 0) && (NODE_REGISTER[reg_addr].access == UNA_REGISTER_ACCESS_READ_ONLY)) {
        status = NODE_ERROR_REGISTER_READ_ONLY;
        goto errors;
    }
    // Check input value.
    node_status = _NODE_secure_register(reg_addr, reg_value, &safe_reg_mask, &safe_reg_value);
    NODE_stack_error(ERROR_BASE_NODE);
    // Write RAM register.
    SWREG_modify_register((uint32_t*) &(NODE_RAM_REGISTER[reg_addr]), reg_value, safe_reg_mask);
    // Secure register.
    node_status = _NODE_secure_register(reg_addr, NODE_RAM_REGISTER[reg_addr], &safe_reg_mask, &(NODE_RAM_REGISTER[reg_addr]));
    NODE_stack_error(ERROR_BASE_NODE);
    // Store value in NVM if needed.
    if (NODE_REGISTER[reg_addr].reset_value == UNA_REGISTER_RESET_VALUE_NVM) {
        node_status = _NODE_store_register(reg_addr);
        NODE_stack_error(ERROR_BASE_NODE);
    }
    // Check actions.
    if (node_ctx.internal_access == 0) {
        node_status = _NODE_process_register(reg_addr, reg_mask);
        NODE_stack_error(ERROR_BASE_NODE);
    }
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_read_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Check parameters.
    if (reg_addr >= NODE_REGISTER_ADDRESS_LAST) {
        status = NODE_ERROR_REGISTER_ADDRESS;
        goto errors;
    }
    if (reg_value == NULL) {
        status = NODE_ERROR_NULL_PARAMETER;
        goto errors;
    }
    _NODE_refresh_register(reg_addr);
    // Read register.
    (*reg_value) = NODE_RAM_REGISTER[reg_addr];
errors:
    return status;
}
