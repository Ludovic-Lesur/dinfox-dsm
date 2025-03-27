/*
 * node.c
 *
 *  Created on: 28 may. 2023
 *      Author: Ludo
 */

#include "node.h"

#include "bpsm.h"
#include "bpsm_registers.h"
#include "common.h"
#include "ddrm.h"
#include "ddrm_registers.h"
#include "error.h"
#include "error_base.h"
#include "gpsm.h"
#include "gpsm_registers.h"
#include "led.h"
#include "lvrm.h"
#include "lvrm_registers.h"
#include "nvm.h"
#include "nvm_address.h"
#include "power.h"
#include "rtc.h"
#include "rrm.h"
#include "rrm_registers.h"
#include "sm.h"
#include "sm_registers.h"
#include "swreg.h"
#include "uhfm.h"
#include "uhfm_registers.h"
#include "una.h"
#include "xm_flags.h"

/*** NODE local macros ***/

#ifdef XM_IOUT_INDICATOR
#define NODE_IOUT_MEASUREMENTS_PERIOD_SECONDS   60
#define NODE_IOUT_INDICATOR_PERIOD_SECONDS      10
#define NODE_IOUT_INDICATOR_RANGE               7
#define NODE_IOUT_INDICATOR_POWER_THRESHOLD_MV  6000
#define NODE_IOUT_INDICATOR_BLINK_DURATION_MS   2000
#endif

/*** NODE local structures ***/

#ifdef XM_IOUT_INDICATOR
/*******************************************************************/
typedef struct {
    int32_t threshold_ua;
    LED_color_t led_color;
} NODE_iout_indicator_t;
#endif

/*******************************************************************/
typedef struct {
    volatile uint32_t registers[NODE_REGISTER_ADDRESS_LAST];
    NODE_state_t state;
#ifdef XM_IOUT_INDICATOR
    uint32_t iout_measurements_next_time_seconds;
    uint32_t iout_indicator_next_time_seconds;
    int32_t input_voltage_mv;
    int32_t iout_ua;
#endif
} NODE_context_t;

/*** NODE local global variables ***/

#ifdef XM_IOUT_INDICATOR
static const NODE_iout_indicator_t LVRM_IOUT_INDICATOR[NODE_IOUT_INDICATOR_RANGE] = {
    {0, LED_COLOR_GREEN},
    {50000, LED_COLOR_YELLOW},
    {500000, LED_COLOR_RED},
    {1000000, LED_COLOR_MAGENTA},
    {2000000, LED_COLOR_BLUE},
    {3000000, LED_COLOR_CYAN},
    {4000000, LED_COLOR_WHITE}
};
#endif

static NODE_context_t node_ctx = {
    .registers = { [0 ... (NODE_REGISTER_ADDRESS_LAST - 1)] = 0x00000000 },
    .state = NODE_STATE_IDLE,
#ifdef XM_IOUT_INDICATOR
    .iout_measurements_next_time_seconds = 0,
    .iout_indicator_next_time_seconds = 0,
    .input_voltage_mv = 0,
    .iout_ua = 0,
#endif
};

/*** NODE local functions ***/

#ifdef XM_IOUT_INDICATOR
/*******************************************************************/
static NODE_status_t _NODE_iout_measurement(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    // Turn analog front-end on.
    POWER_enable(POWER_REQUESTER_ID_NODE, POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
    // Check input voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VIN_MV, &(node_ctx.input_voltage_mv));
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    // Enable RGB LED only if power input supplies the board.
    if (node_ctx.input_voltage_mv > NODE_IOUT_INDICATOR_POWER_THRESHOLD_MV) {
        // Read output current.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_IOUT_UA, &(node_ctx.iout_ua));
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    }
errors:
    POWER_disable(POWER_REQUESTER_ID_NODE, POWER_DOMAIN_ANALOG);
    return status;
}
#endif

#ifdef XM_IOUT_INDICATOR
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
            led_color = LVRM_IOUT_INDICATOR[idx].led_color;
            if (node_ctx.iout_ua >= LVRM_IOUT_INDICATOR[idx].threshold_ua) break;
        }
        while (idx > 0);
        // Blink LED.
        led_status = LED_start_single_blink(NODE_IOUT_INDICATOR_BLINK_DURATION_MS, led_color);
        LED_exit_error(NODE_ERROR_BASE_LED);
    }
errors:
    return status;
}
#endif

/*******************************************************************/
static NODE_status_t _NODE_update_register(uint8_t reg_addr) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Update common registers.
    status = COMMON_update_register(reg_addr);
    if (status != NODE_SUCCESS) goto errors;
    // Update specific registers.
#ifdef LVRM
    status = LVRM_update_register(reg_addr);
#endif
#ifdef BPSM
    status = BPSM_update_register(reg_addr);
#endif
#ifdef DDRM
    status = DDRM_update_register(reg_addr);
#endif
#ifdef UHFM
    status = UHFM_update_register(reg_addr);
#endif
#ifdef GPSM
    status = GPSM_update_register(reg_addr);
#endif
#ifdef SM
    status = SM_update_register(reg_addr);
#endif
#ifdef RRM
    status = RRM_update_register(reg_addr);
#endif
errors:
    return status;
}

/*******************************************************************/
static NODE_status_t _NODE_check_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Check common registers.
    status = COMMON_check_register(reg_addr, reg_mask);
    if (status != NODE_SUCCESS) goto errors;
    // Check specific registers.
#ifdef LVRM
    status = LVRM_check_register(reg_addr, reg_mask);
#endif
#ifdef BPSM
    status = BPSM_check_register(reg_addr, reg_mask);
#endif
#ifdef DDRM
    status = DDRM_check_register(reg_addr, reg_mask);
#endif
#ifdef UHFM
    status = UHFM_check_register(reg_addr, reg_mask);
#endif
#ifdef GPSM
    status = GPSM_check_register(reg_addr, reg_mask);
#endif
#ifdef SM
    status = SM_check_register(reg_addr, reg_mask);
#endif
#ifdef RRM
    status = RRM_check_register(reg_addr, reg_mask);
#endif
errors:
    return status;
}

/*** NODE functions ***/

/*******************************************************************/
NODE_status_t NODE_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    NVM_status_t nvm_status = NVM_SUCCESS;
#ifdef XM_RGB_LED
    LED_status_t led_status = LED_SUCCESS;
#endif
    UNA_node_address_t self_address = 0;
    uint8_t idx = 0;
    // Init context.
    for (idx = 0; idx < NODE_REGISTER_ADDRESS_LAST; idx++) {
        node_ctx.registers[idx] = NODE_REGISTER_ERROR_VALUE[idx];
    }
    node_ctx.state = NODE_STATE_IDLE;
#ifdef XM_IOUT_INDICATOR
    node_ctx.iout_measurements_next_time_seconds = 0;
    node_ctx.iout_indicator_next_time_seconds = 0;
    node_ctx.input_voltage_mv = 0;
    node_ctx.iout_ua = 0;
#endif
#ifdef XM_NVM_FACTORY_RESET
    nvm_status = NVM_write_byte(NVM_ADDRESS_SELF_ADDRESS, XM_NODE_ADDRESS);
    NVM_exit_error(NODE_ERROR_BASE_NVM);
#endif
    // Read self address in NVM.
    nvm_status = NVM_read_byte(NVM_ADDRESS_SELF_ADDRESS, &self_address);
    NVM_exit_error(NODE_ERROR_BASE_NVM);
    // Init common registers.
    status = COMMON_init_registers(self_address);
    if (status != NODE_SUCCESS) goto errors;
    // Init specific registers.
#ifdef LVRM
    status = LVRM_init_registers();
#endif
#ifdef BPSM
    status = BPSM_init_registers();
#endif
#ifdef DDRM
    status = DDRM_init_registers();
#endif
#ifdef UHFM
    status = UHFM_init_registers();
#endif
#ifdef GPSM
    status = GPSM_init_registers();
#endif
#ifdef SM
    status = SM_init_registers();
#endif
#ifdef RRM
    status = RRM_init_registers();
#endif
    if (status != NODE_SUCCESS) goto errors;
#ifdef XM_LOAD_CONTROL
    LOAD_init();
#endif
#ifdef XM_RGB_LED
    led_status = LED_init();
    LED_exit_error(NODE_ERROR_BASE_LED);
#endif
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_de_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef XM_RGB_LED
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
#if (((defined LVRM) && (defined LVRM_MODE_BMS)) || ((defined BPSM) && !(defined BPSM_CHEN_FORCED_HARDWARE)))
    NODE_status_t node_status = NODE_SUCCESS;
#endif
    // Reset state to default.
    node_ctx.state = NODE_STATE_IDLE;
#if ((defined LVRM) && (defined LVRM_MODE_BMS))
    status = LVRM_bms_process();
    NODE_stack_error(ERROR_BASE_NODE);
#endif
#if ((defined BPSM) && !(defined BPSM_CHEN_FORCED_HARDWARE))
    node_status = BPSM_charge_process();
    NODE_stack_error(ERROR_BASE_NODE);
#endif
#ifdef XM_IOUT_INDICATOR
    // Check measurements period.
    if (RTC_get_uptime_seconds() >= node_ctx.iout_measurements_next_time_seconds) {
        // Update next time.
        node_ctx.iout_measurements_next_time_seconds = RTC_get_uptime_seconds() + NODE_IOUT_MEASUREMENTS_PERIOD_SECONDS;
        // Perform measurements.
        status = _NODE_iout_measurement();
        if (status != NODE_SUCCESS) goto errors;
    }
    // Check LED period.
    if (RTC_get_uptime_seconds() >= node_ctx.iout_indicator_next_time_seconds) {
        // Update next time.
        node_ctx.iout_indicator_next_time_seconds = RTC_get_uptime_seconds() + NODE_IOUT_INDICATOR_PERIOD_SECONDS;
        // Perform LED task.
        status = _NODE_iout_indicator();
        if (status != NODE_SUCCESS) goto errors;
    }
errors:
    // Check LED state.
    if (LED_is_single_blink_done() == 0) {
        node_ctx.state = NODE_STATE_RUNNING;
    }
#endif
    return status;
}

/*******************************************************************/
NODE_state_t NODE_get_state(void) {
    return (node_ctx.state);
}

/*******************************************************************/
NODE_status_t NODE_write_register(NODE_request_source_t request_source, uint8_t reg_addr, uint32_t reg_value, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Check address.
    if (reg_addr >= NODE_REGISTER_ADDRESS_LAST) {
        status = NODE_ERROR_REGISTER_ADDRESS;
        goto errors;
    }
    // Check access.
    if ((request_source == NODE_REQUEST_SOURCE_EXTERNAL) && (NODE_REGISTER_ACCESS[reg_addr] == UNA_REGISTER_ACCESS_READ_ONLY)) {
        status = NODE_ERROR_REGISTER_READ_ONLY;
        goto errors;
    }
    // Write register.
    SWREG_modify_register((uint32_t*) &(node_ctx.registers[reg_addr]), reg_value, reg_mask);
    // Check actions.
    if (request_source == NODE_REQUEST_SOURCE_EXTERNAL) {
        // Check control bits.
        status = _NODE_check_register(reg_addr, reg_mask);
        if (status != NODE_SUCCESS) goto errors;
    }
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_write_byte_array(NODE_request_source_t request_source, uint8_t reg_addr_base, uint8_t* data, uint8_t data_size_byte) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint8_t idx = 0;
    uint8_t shift = 0;
    uint8_t reg_addr = 0;
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Check parameters.
    if (data == NULL) {
        status = NODE_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Byte loop.
    for (idx = 0; idx < data_size_byte; idx++) {
        // Compute address, mask and value.
        reg_addr = (reg_addr_base + (idx >> 2));
        shift = ((idx % 4) << 3);
        reg_mask = (0xFF << shift);
        reg_value = (data[idx] << shift);
        // Write register.
        status = NODE_write_register(request_source, reg_addr, reg_value, reg_mask);
        if (status != NODE_SUCCESS) goto errors;
    }
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_write_nvm(uint8_t reg_addr, uint32_t reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    NVM_status_t nvm_status = NVM_SUCCESS;
    uint8_t nvm_byte = 0;
    uint8_t idx = 0;
    // Byte loop.
    for (idx = 0; idx < 4; idx++) {
        // Compute byte.
        nvm_byte = (uint8_t) (((reg_value) >> (idx << 3)) & 0x000000FF);
        // Write NVM.
        nvm_status = NVM_write_byte((NVM_ADDRESS_REGISTERS + (reg_addr << 2) + idx), nvm_byte);
        NVM_exit_error(NODE_ERROR_BASE_NVM);
    }
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_read_register(NODE_request_source_t request_source, uint8_t reg_addr, uint32_t* reg_value) {
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
    // Check update type.
    if (request_source == NODE_REQUEST_SOURCE_EXTERNAL) {
        // Update register.
        status = _NODE_update_register(reg_addr);
        if (status != NODE_SUCCESS) goto errors;
    }
    // Read register.
    (*reg_value) = node_ctx.registers[reg_addr];
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_read_byte_array(NODE_request_source_t request_source, uint8_t reg_addr_base, uint8_t* data, uint8_t data_size_byte) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint8_t idx = 0;
    uint8_t reg_addr = 0;
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Check parameters.
    if (data == NULL) {
        status = NODE_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Byte loop.
    for (idx = 0; idx < data_size_byte; idx++) {
        // Compute address and mask.
        reg_addr = (reg_addr_base + (idx >> 2));
        reg_mask = (0xFF << ((idx % 4) << 3));
        // Read byte.
        status = NODE_read_register(request_source, reg_addr, &reg_value);
        // Fill data.
        data[idx] = (uint8_t) SWREG_read_field(reg_value, reg_mask);
    }
errors:
    return status;
}

/*******************************************************************/
NODE_status_t NODE_read_nvm(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    NVM_status_t nvm_status = NVM_SUCCESS;
    uint8_t nvm_byte = 0;
    uint8_t idx = 0;
    // Check parameter.
    if (reg_value == NULL) {
        status = NODE_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Reset output value.
    (*reg_value) = 0;
    // Byte loop.
    for (idx = 0; idx < 4; idx++) {
        // Read NVM.
        nvm_status = NVM_read_byte((NVM_ADDRESS_REGISTERS + (reg_addr << 2) + idx), &nvm_byte);
        NVM_exit_error(NODE_ERROR_BASE_NVM);
        // Update output value.
        (*reg_value) |= ((uint32_t) nvm_byte) << (idx << 3);
    }
errors:
    return status;
}
