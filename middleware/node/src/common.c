/*
 * common.c
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#include "common.h"

#include "adc.h"
#include "bcm.h"
#include "bpsm.h"
#include "common_registers.h"
#include "ddrm.h"
#include "dsm_flags.h"
#include "error.h"
#include "gpsm.h"
#include "lvrm.h"
#include "mpmcm.h"
#include "node_register.h"
#include "node_status.h"
#include "nvm.h"
#include "nvm_address.h"
#include "pwr.h"
#include "rrm.h"
#include "sm.h"
#include "swreg.h"
#include "types.h"
#include "uhfm.h"
#include "version.h"
#include "una.h"

/*** COMMON local macros ***/

#ifdef HW1_0
#define COMMON_HW_VERSION_MAJOR     1
#define COMMON_HW_VERSION_MINOR     0
#endif
#ifdef HW2_0
#define COMMON_HW_VERSION_MAJOR     2
#define COMMON_HW_VERSION_MINOR     0
#endif
#ifdef DSM_DEBUG
#define COMMON_FLAG_DF              0b1
#else
#define COMMON_FLAG_DF              0b0
#endif
#ifdef DSM_NVM_FACTORY_RESET
#define COMMON_FLAG_NFRF            0b1
#else
#define COMMON_FLAG_NFRF            0b0
#endif

/*** COMMON local functions ***/

/*******************************************************************/
static NODE_status_t _COMMON_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    int32_t vmcu_mv = 0;
    int32_t tmcu_degrees = 0;
    uint32_t* reg_analog_data_0_ptr = &(NODE_RAM_REGISTER[COMMON_REGISTER_ADDRESS_ANALOG_DATA_0]);
    uint32_t unused_mask = 0;
    // Reset data.
    (*reg_analog_data_0_ptr) = NODE_REGISTER[COMMON_REGISTER_ADDRESS_ANALOG_DATA_0].error_value;
    // Turn analog front-end on.
    POWER_enable(POWER_REQUESTER_ID_COMMON, POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
    // MCU voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VMCU_MV, &vmcu_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_0_ptr, &unused_mask, (uint32_t) UNA_convert_mv(vmcu_mv), COMMON_REGISTER_ANALOG_DATA_0_MASK_VMCU);
    // MCU temperature.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_TMCU_DEGREES, &tmcu_degrees);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_0_ptr, &unused_mask, (uint32_t) UNA_convert_tenth_degrees(tmcu_degrees * 10), COMMON_REGISTER_ANALOG_DATA_0_MASK_TMCU);
    // Specific analog data.
    status = NODE_MTRG_CALLBACK();
    if (status != NODE_SUCCESS) goto errors;
errors:
    POWER_disable(POWER_REQUESTER_ID_COMMON, POWER_DOMAIN_ANALOG);
    return status;
}

/*** COMMON functions ***/

/*******************************************************************/
void COMMON_init_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    uint8_t self_address = 0;
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case COMMON_REGISTER_ADDRESS_NODE_ID:
#ifdef DSM_NVM_FACTORY_RESET
#ifdef MPMCM
        NVM_write_word(NVM_ADDRESS_SELF_ADDRESS, (uint32_t) DSM_NODE_ADDRESS);
#else
        NVM_write_byte(NVM_ADDRESS_SELF_ADDRESS, DSM_NODE_ADDRESS);
#endif
#endif
#ifdef MPMCM
        NVM_read_word(NVM_ADDRESS_SELF_ADDRESS, &unused_mask);
        self_address = (UNA_node_address_t) unused_mask;
#else
        NVM_read_byte(NVM_ADDRESS_SELF_ADDRESS, &self_address);
#endif
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) self_address, COMMON_REGISTER_NODE_ID_MASK_NODE_ADDR);
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) NODE_BOARD_ID, COMMON_REGISTER_NODE_ID_MASK_BOARD_ID);
        break;
    case COMMON_REGISTER_ADDRESS_HW_VERSION:
        SWREG_write_field(reg_value, &unused_mask, COMMON_HW_VERSION_MAJOR, COMMON_REGISTER_HW_VERSION_MASK_MAJOR);
        SWREG_write_field(reg_value, &unused_mask, COMMON_HW_VERSION_MINOR, COMMON_REGISTER_HW_VERSION_MASK_MINOR);
        break;
    case COMMON_REGISTER_ADDRESS_SW_VERSION_0:
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) GIT_MAJOR_VERSION, COMMON_REGISTER_SW_VERSION_0_MASK_MAJOR);
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) GIT_MINOR_VERSION, COMMON_REGISTER_SW_VERSION_0_MASK_MINOR);
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) GIT_COMMIT_INDEX, COMMON_REGISTER_SW_VERSION_0_MASK_COMMIT_INDEX);
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) GIT_DIRTY_FLAG, COMMON_REGISTER_SW_VERSION_0_MASK_DTYF);
        break;
    case COMMON_REGISTER_ADDRESS_SW_VERSION_1:
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) GIT_COMMIT_ID, COMMON_REGISTER_SW_VERSION_1_MASK_COMMIT_ID);
        break;
    case COMMON_REGISTER_ADDRESS_FLAGS_0:
        SWREG_write_field(reg_value, &unused_mask, COMMON_FLAG_DF, COMMON_REGISTER_FLAGS_0_MASK_DF);
        SWREG_write_field(reg_value, &unused_mask, COMMON_FLAG_NFRF, COMMON_REGISTER_FLAGS_0_MASK_NFRF);
        break;
    case COMMON_REGISTER_ADDRESS_STATUS_0:
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) PWR_get_reset_flags(), COMMON_REGISTER_STATUS_0_MASK_RESET_FLAGS);
        SWREG_write_field(reg_value, &unused_mask, 0b1, COMMON_REGISTER_STATUS_0_MASK_BF);
        break;
    default:
        break;
    }
}

/*******************************************************************/
void COMMON_refresh_register(uint8_t reg_addr) {
    // Local variables.
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case COMMON_REGISTER_ADDRESS_ERROR_STACK:
        SWREG_write_field(reg_ptr, &unused_mask, (uint32_t) ERROR_stack_read(), COMMON_REGISTER_ERROR_STACK_MASK_ERROR);
        break;
    case COMMON_REGISTER_ADDRESS_STATUS_0:
#ifdef UHFM
        ERROR_import_sigfox_stack();
#endif
        SWREG_write_field(reg_ptr, &unused_mask, ((ERROR_stack_is_empty() == 0) ? 0b1 : 0b0), COMMON_REGISTER_STATUS_0_MASK_ESF);
        break;
    default:
        break;
    }
}

/*******************************************************************/
NODE_status_t COMMON_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t generic_u32 = 0;
    // Check address.
    switch (reg_addr) {
    case COMMON_REGISTER_ADDRESS_NODE_ID:
        SWREG_secure_field(
            COMMON_REGISTER_NODE_ID_MASK_NODE_ADDR,,,
            <= UNA_NODE_ADDRESS_RS485_BRIDGE,
            >= UNA_NODE_ADDRESS_R4S8CR_START,
            UNA_NODE_ADDRESS_ERROR,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        SWREG_secure_field(
            COMMON_REGISTER_NODE_ID_MASK_BOARD_ID,,,
            >= UNA_BOARD_ID_LAST,
            >= UNA_BOARD_ID_LAST,
            UNA_BOARD_ID_ERROR,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    default:
        break;
    }
    return status;
}

/*******************************************************************/
NODE_status_t COMMON_process_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case COMMON_REGISTER_ADDRESS_CONTROL_0:
        // MTRG.
        if ((reg_mask & COMMON_REGISTER_CONTROL_0_MASK_MTRG) != 0) {
            // Read bit.
            if ((SWREG_read_field((*reg_ptr), COMMON_REGISTER_CONTROL_0_MASK_MTRG)) != 0) {
                // Clear request.
                SWREG_write_field(reg_ptr, &unused_mask, 0b0, COMMON_REGISTER_CONTROL_0_MASK_MTRG);
                // Perform measurements.
                status = _COMMON_mtrg_callback();
                if (status != NODE_SUCCESS) goto errors;
            }
        }
        // BFC.
        if ((reg_mask & COMMON_REGISTER_CONTROL_0_MASK_BFC) != 0) {
            // Read bit.
            if ((SWREG_read_field((*reg_ptr), COMMON_REGISTER_CONTROL_0_MASK_BFC)) != 0) {
                // Clear request and boot flag.
                SWREG_write_field(reg_ptr, &unused_mask, 0b0, COMMON_REGISTER_CONTROL_0_MASK_BFC);
                SWREG_write_field(&(NODE_RAM_REGISTER[COMMON_REGISTER_ADDRESS_STATUS_0]), &unused_mask, 0b0, COMMON_REGISTER_STATUS_0_MASK_BF);
                // Clear MCU reset flags.
                PWR_clear_reset_flags();
            }
        }
        // Note: RTRG bit is checked in the node process function in order to send the reply before reset.
        break;
    default:
        break;
    }
errors:
    return status;
}
