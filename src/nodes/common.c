/*
 * common.c
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#include "common.h"

#include "adc.h"
#include "bpsm.h"
#include "ddrm.h"
#include "common_reg.h"
#include "dinfox.h"
#include "error.h"
#include "gpsm.h"
#include "lvrm.h"
#include "node.h"
#include "nvm.h"
#include "pwr.h"
#include "rcc_reg.h"
#include "rrm.h"
#include "sm.h"
#include "types.h"
#include "uhfm.h"
#include "version.h"

/*** COMMON local functions ***/

/*******************************************************************/
static void _COMMON_reset_analog_data(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Reset fields to error value.
	reg_value |= DINFOX_VOLTAGE_ERROR_VALUE << DINFOX_get_shift(COMMON_REG_ANALOG_DATA_0_MASK_VMCU);
	reg_mask |= COMMON_REG_ANALOG_DATA_0_MASK_VMCU;
	reg_value |= DINFOX_TEMPERATURE_ERROR_VALUE << DINFOX_get_shift(COMMON_REG_ANALOG_DATA_0_MASK_TMCU);
	reg_mask |= COMMON_REG_ANALOG_DATA_0_MASK_TMCU;
	// Write register.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_ANALOG_DATA_0, reg_mask, reg_value);
	NODE_stack_error();
}

/*******************************************************************/
NODE_status_t _COMMON_mtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t vmcu_mv = 0;
	int8_t tmcu_degrees = 0;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Reset results.
	_COMMON_reset_analog_data();
#ifdef LVRM
	status = LVRM_mtrg_callback(&adc1_status);
#endif
#ifdef BPSM
	status = BPSM_mtrg_callback(&adc1_status);
#endif
#ifdef DDRM
	status = DDRM_mtrg_callback(&adc1_status);
#endif
#ifdef UHFM
	status = UHFM_mtrg_callback(&adc1_status);
#endif
#ifdef GPSM
	status = GPSM_mtrg_callback(&adc1_status);
#endif
#ifdef SM
	status = SM_mtrg_callback(&adc1_status);
#endif
#ifdef RRM
	status = RRM_mtrg_callback(&adc1_status);
#endif
	// Note: status is not checked here in order to try reading VMCU and TMCU, but will be returned anyway at the end.
	if (adc1_status == ADC_SUCCESS) {
		// MCU voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VMCU_MV, &vmcu_mv);
		ADC1_stack_error();
		reg_value |= (DINFOX_convert_mv(vmcu_mv) << DINFOX_get_shift(COMMON_REG_ANALOG_DATA_0_MASK_VMCU));
		reg_mask |= COMMON_REG_ANALOG_DATA_0_MASK_VMCU;
		// MCU temperature.
		adc1_status = ADC1_get_tmcu(&tmcu_degrees);
		ADC1_stack_error();
		reg_value |= (DINFOX_convert_degrees(tmcu_degrees) << DINFOX_get_shift(COMMON_REG_ANALOG_DATA_0_MASK_TMCU));
		reg_mask |= COMMON_REG_ANALOG_DATA_0_MASK_TMCU;
		// Write register.
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_ANALOG_DATA_0, reg_mask, reg_value);
		NODE_stack_error();
	}
	// Clear flag.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_STATUS_CONTROL_0, COMMON_REG_STATUS_CONTROL_0_MASK_MTRG, 0);
	NODE_stack_error();
	return status;
}

/*** COMMON functions ***/

/*******************************************************************/
void COMMON_init_registers(NODE_address_t self_address) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Node ID register.
	reg_value = (self_address << DINFOX_get_shift(COMMON_REG_NODE_ID_MASK_NODE_ADDR));
	reg_mask = COMMON_REG_NODE_ID_MASK_NODE_ADDR;
	reg_value |= (NODE_BOARD_ID << DINFOX_get_shift(COMMON_REG_NODE_ID_MASK_BOARD_ID));
	reg_mask |= COMMON_REG_NODE_ID_MASK_BOARD_ID;
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_NODE_ID, reg_mask, reg_value);
	NODE_stack_error();
	// HW version register.
#ifdef HW1_0
	reg_value = (1 << DINFOX_get_shift(COMMON_REG_HW_VERSION_MASK_MAJOR));
	reg_mask = COMMON_REG_HW_VERSION_MASK_MAJOR;
	reg_value |= (0 << DINFOX_get_shift(COMMON_REG_HW_VERSION_MASK_MINOR));
	reg_mask |= COMMON_REG_HW_VERSION_MASK_MINOR;
#endif
#ifdef HW2_0
	reg_value = (2 << DINFOX_get_shift(COMMON_REG_HW_VERSION_MASK_MAJOR));
	reg_mask = COMMON_REG_HW_VERSION_MASK_MAJOR;
	reg_value |= (0 << DINFOX_get_shift(COMMON_REG_HW_VERSION_MASK_MINOR));
	reg_mask |= COMMON_REG_HW_VERSION_MASK_MINOR;
#endif
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_HW_VERSION, reg_mask, reg_value);
	NODE_stack_error();
	// SW version register 0.
	reg_value = (GIT_MAJOR_VERSION << DINFOX_get_shift(COMMON_REG_SW_VERSION_0_MASK_MAJOR));
	reg_mask = COMMON_REG_SW_VERSION_0_MASK_MAJOR;
	reg_value |= (GIT_MINOR_VERSION << DINFOX_get_shift(COMMON_REG_SW_VERSION_0_MASK_MINOR));
	reg_mask |= COMMON_REG_SW_VERSION_0_MASK_MINOR;
	reg_value |= (GIT_COMMIT_INDEX << DINFOX_get_shift(COMMON_REG_SW_VERSION_0_MASK_COMMIT_INDEX));
	reg_mask |= COMMON_REG_SW_VERSION_0_MASK_COMMIT_INDEX;
	reg_value |= (GIT_DIRTY_FLAG << DINFOX_get_shift(COMMON_REG_SW_VERSION_0_MASK_DTYF));
	reg_mask |= COMMON_REG_SW_VERSION_0_MASK_DTYF;
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_SW_VERSION_0, reg_mask, reg_value);
	NODE_stack_error();
	// SW version register 1.
	reg_value = (GIT_COMMIT_ID << DINFOX_get_shift(COMMON_REG_SW_VERSION_1_MASK_COMMIT_ID));
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_SW_VERSION_1, COMMON_REG_SW_VERSION_1_MASK_COMMIT_ID, reg_value);
	NODE_stack_error();
	// Reset flags registers.
	reg_value = ((((RCC -> CSR) >> 24) & 0xFF) << DINFOX_get_shift(COMMON_REG_RESET_FLAGS_MASK_ALL));
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_RESET_FLAGS, COMMON_REG_RESET_FLAGS_MASK_ALL, reg_value);
	NODE_stack_error();
	// Load default values.
	_COMMON_reset_analog_data();
}

/*******************************************************************/
NODE_status_t COMMON_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t reg_value = (ERROR_stack_read() << DINFOX_get_shift(COMMON_REG_ADDR_ERROR_STACK));
	// Check address.
	switch (reg_addr) {
	case COMMON_REG_ADDR_ERROR_STACK:
		// Unstack error.
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_ERROR_STACK, COMMON_REG_ERROR_STACK_MASK_ERROR, reg_value);
		NODE_stack_error();
		break;
	default:
		// Nothing to do.
		break;
	}
	return status;
}

/*******************************************************************/
NODE_status_t COMMON_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t reg_value = 0;
	// Read register.
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_STATUS_CONTROL_0, &reg_value);
	NODE_stack_error();
	// Check address.
	switch (reg_addr) {
	case COMMON_REG_ADDR_STATUS_CONTROL_0:
		// Reset trigger bit.
		if ((DINFOX_read_field(reg_value, COMMON_REG_STATUS_CONTROL_0_MASK_RTRG)) != 0) {
			// Reset MCU.
			PWR_software_reset();
		}
		// Measure trigger bit.
		if ((DINFOX_read_field(reg_value, COMMON_REG_STATUS_CONTROL_0_MASK_MTRG)) != 0) {
			// Perform measurements.
			_COMMON_mtrg_callback();
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	return status;
}
