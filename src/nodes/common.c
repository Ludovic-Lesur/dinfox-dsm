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
#include "dinfox_common.h"
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

/* RESET COMMON ANALOG DATA.
 * @param:			None.
 * @return status:	Function execution status.
 */
static NODE_status_t _COMMON_reset_analog_data(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	// Reset fields to error value.
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_ANALOG_DATA_0, COMMON_REG_ANALOG_DATA_0_MASK_VMCU, DINFOX_VOLTAGE_ERROR_VALUE);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_ANALOG_DATA_0, COMMON_REG_ANALOG_DATA_0_MASK_TMCU, DINFOX_TEMPERATURE_ERROR_VALUE);
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

/* MEASURE TRIGGER CALLBACK.
 * @param:			None.
 * @return status:	Function execution status.
 */
NODE_status_t _COMMON_mtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t vmcu_mv = 0;
	int8_t tmcu_degrees = 0;
	// Reset results.
	status = _COMMON_reset_analog_data();
	if (status != NODE_SUCCESS) goto errors;
#ifdef LVRM
	status = LVRM_mtrg_callback();
	if (status != NODE_SUCCESS) goto errors;
#endif
#ifdef BPSM
	status = BPSM_mtrg_callback();
	if (status != NODE_SUCCESS) goto errors;
#endif
#ifdef DDRM
	status = DDRM_mtrg_callback();
	if (status != NODE_SUCCESS) goto errors;
#endif
#ifdef UHFM
	status = UHFM_mtrg_callback();
	if (status != NODE_SUCCESS) goto errors;
#endif
#ifdef GPSM
	status = GPSM_mtrg_callback();
	if (status != NODE_SUCCESS) goto errors;
#endif
#ifdef SM
	status = SM_mtrg_callback();
	if (status != NODE_SUCCESS) goto errors;
#endif
	// MCU voltage.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VMCU_MV, &vmcu_mv);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_ANALOG_DATA_0, COMMON_REG_ANALOG_DATA_0_MASK_VMCU, (uint32_t) DINFOX_convert_mv(vmcu_mv));
	if (status != NODE_SUCCESS) goto errors;
	// MCU temperature.
	adc1_status = ADC1_get_tmcu(&tmcu_degrees);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_ANALOG_DATA_0, COMMON_REG_ANALOG_DATA_0_MASK_TMCU, (uint32_t) DINFOX_convert_degrees(tmcu_degrees));
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

/*** COMMON functions ***/

/* INIT COMMON REGISTERS.
 * @param:			None.
 * @return status:	Function execution status.
 */
NODE_status_t COMMON_init_registers(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	uint8_t generic_u8 = 0;
	// Read self address.
	nvm_status = NVM_read_byte(NVM_ADDRESS_SELF_ADDRESS, &generic_u8);
	NVM_status_check(NODE_ERROR_BASE_NVM);
	// Node ID register.
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_NODE_ID, COMMON_REG_NODE_ID_MASK_NODE_ADDR, generic_u8);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_NODE_ID, COMMON_REG_NODE_ID_MASK_BOARD_ID, NODE_BOARD_ID);
	if (status != NODE_SUCCESS) goto errors;
	// HW version register.
#ifdef HW1_0
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_HW_VERSION, COMMON_REG_HW_VERSION_MASK_MAJOR, 1);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_HW_VERSION, COMMON_REG_HW_VERSION_MASK_MINOR, 0);
	if (status != NODE_SUCCESS) goto errors;
#endif
#ifdef HW2_0
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_HW_VERSION, COMMON_REG_HW_VERSION_MASK_MAJOR, 2);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_HW_VERSION, COMMON_REG_HW_VERSION_MASK_MINOR, 0);
	if (status != NODE_SUCCESS) goto errors;
#endif
	// SW version register 0.
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_SW_VERSION_0, COMMON_REG_SW_VERSION_0_MASK_MAJOR, GIT_MAJOR_VERSION);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_SW_VERSION_0, COMMON_REG_SW_VERSION_0_MASK_MINOR, GIT_MINOR_VERSION);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_SW_VERSION_0, COMMON_REG_SW_VERSION_0_MASK_COMMIT_INDEX, GIT_COMMIT_INDEX);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_SW_VERSION_0, COMMON_REG_SW_VERSION_0_MASK_DTYF, GIT_DIRTY_FLAG);
	if (status != NODE_SUCCESS) goto errors;
	// SW version register 1.
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_SW_VERSION_1, COMMON_REG_SW_VERSION_1_MASK_COMMIT_ID, GIT_COMMIT_ID);
	if (status != NODE_SUCCESS) goto errors;
	// Reset flags registers.
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_RESET_FLAGS, COMMON_REG_RESET_FLAGS_MASK_ALL, ((uint8_t) (((RCC -> CSR) >> 24) & 0xFF)));
	if (status != NODE_SUCCESS) goto errors;
	// Load default values.
	status = _COMMON_reset_analog_data();
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

/* UPDATE COMMON REGISTER.
 * @param reg_addr:	Address of the register to update.
 * @return status:	Function execution status.
 */
NODE_status_t COMMON_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	// Check address.
	switch (reg_addr) {
	case COMMON_REG_ADDR_ERROR_STACK:
		// Unstack error.
		status = NODE_write_field(COMMON_REG_ADDR_ERROR_STACK, ERROR_stack_read(), COMMON_REG_ERROR_STACK_MASK_ERROR, 0);
		if (status != NODE_SUCCESS) goto errors;
		break;
	default:
		// Nothing to do.
		break;
	}
errors:
	return status;
}

/* CHECK COMMON NODE ACTIONS.
 * @param reg_addr:	Address of the register to check.
 * @return status:	Function execution status.
 */
NODE_status_t COMMON_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t generic_u32 = 0;
	// Check address.
	switch (reg_addr) {
	case COMMON_REG_ADDR_STATUS_CONTROL_0:
		// Reset trigger bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_STATUS_CONTROL_0, COMMON_REG_STATUS_CONTROL_0_MASK_RTRG, &generic_u32);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit.
		if (generic_u32 != 0) {
			// Clear flag.
			NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_STATUS_CONTROL_0, COMMON_REG_STATUS_CONTROL_0_MASK_RTRG, 0);
			// Reset MCU.
			PWR_software_reset();
		}
		// Measure trigger bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_STATUS_CONTROL_0, COMMON_REG_STATUS_CONTROL_0_MASK_MTRG, &generic_u32);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit.
		if (generic_u32 != 0) {
			// Perform measurements.
			status = _COMMON_mtrg_callback();
			// Clear flag.
			NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_STATUS_CONTROL_0, COMMON_REG_STATUS_CONTROL_0_MASK_MTRG, 0);
			// Check status.
			if (status != NODE_SUCCESS) goto errors;
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}
