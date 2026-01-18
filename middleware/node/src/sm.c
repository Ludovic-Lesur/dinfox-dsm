/*
 * sm.c
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#include "sm.h"

#ifdef SM

#include "analog.h"
#include "digital.h"
#include "dsm_flags.h"
#include "error.h"
#include "i2c_address.h"
#include "load.h"
#include "node_register.h"
#include "node_status.h"
#include "sht3x.h"
#include "sm_registers.h"
#include "swreg.h"
#include "types.h"
#include "una.h"

/*** SM local macros ***/

#ifdef SM_AIN_ENABLE
#define SM_FLAG_AINF    0b1
#else
#define SM_FLAG_AINF    0b0
#endif
#ifdef SM_DIO_ENABLE
#define SM_FLAG_DIOF    0b1
#else
#define SM_FLAG_DIOF    0b0
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
#define SM_FLAG_DIGF    0b1
#else
#define SM_FLAG_DIGF    0b0
#endif

/*** SM functions ***/

/*******************************************************************/
NODE_status_t SM_init(void) {
    return NODE_SUCCESS;
}

/*******************************************************************/
void SM_init_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case SM_REGISTER_ADDRESS_FLAGS_1:
        SWREG_write_field(reg_value, &unused_mask, SM_FLAG_AINF, SM_REGISTER_FLAGS_1_MASK_AINF);
        SWREG_write_field(reg_value, &unused_mask, SM_FLAG_DIOF, SM_REGISTER_FLAGS_1_MASK_DIOF);
        SWREG_write_field(reg_value, &unused_mask, SM_FLAG_DIGF, SM_REGISTER_FLAGS_1_MASK_DIGF);
        break;
    case SM_REGISTER_ADDRESS_FLAGS_2:
        SWREG_write_field(reg_value, &unused_mask, ((SM_AIN0_GAIN_TYPE == ANALOG_GAIN_TYPE_AMPLIFICATION) ? 0b1 : 0b0), SM_REGISTER_FLAGS_2_MASK_AI0T);
        SWREG_write_field(reg_value, &unused_mask, SM_AIN0_GAIN, SM_REGISTER_FLAGS_2_MASK_AI0G);
        SWREG_write_field(reg_value, &unused_mask, ((SM_AIN1_GAIN_TYPE == ANALOG_GAIN_TYPE_AMPLIFICATION) ? 0b1 : 0b0), SM_REGISTER_FLAGS_2_MASK_AI1T);
        SWREG_write_field(reg_value, &unused_mask, SM_AIN1_GAIN, SM_REGISTER_FLAGS_2_MASK_AI1G);
        break;
    case SM_REGISTER_ADDRESS_FLAGS_3:
        SWREG_write_field(reg_value, &unused_mask, ((SM_AIN2_GAIN_TYPE == ANALOG_GAIN_TYPE_AMPLIFICATION) ? 0b1 : 0b0), SM_REGISTER_FLAGS_3_MASK_AI2T);
        SWREG_write_field(reg_value, &unused_mask, SM_AIN2_GAIN, SM_REGISTER_FLAGS_3_MASK_AI2G);
        SWREG_write_field(reg_value, &unused_mask, ((SM_AIN2_GAIN_TYPE == ANALOG_GAIN_TYPE_AMPLIFICATION) ? 0b1 : 0b0), SM_REGISTER_FLAGS_3_MASK_AI3T);
        SWREG_write_field(reg_value, &unused_mask, SM_AIN2_GAIN, SM_REGISTER_FLAGS_3_MASK_AI3G);
        break;
    default:
        break;
    }
}

/*******************************************************************/
void SM_refresh_register(uint8_t reg_addr) {
    UNUSED(reg_addr);
}

/*******************************************************************/
NODE_status_t SM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value) {
    UNUSED(reg_addr);
    UNUSED(new_reg_value);
    UNUSED(reg_mask);
    UNUSED(reg_value);
    return NODE_SUCCESS;
}

/*******************************************************************/
NODE_status_t SM_process_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // None control bit in SM registers.
    UNUSED(reg_addr);
    UNUSED(reg_mask);
    return status;
}

/*******************************************************************/
NODE_status_t SM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef SM_AIN_ENABLE
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    uint32_t* reg_analog_data_1_ptr = &(NODE_RAM_REGISTER[SM_REGISTER_ADDRESS_ANALOG_DATA_1]);
    uint32_t* reg_analog_data_2_ptr = &(NODE_RAM_REGISTER[SM_REGISTER_ADDRESS_ANALOG_DATA_2]);
    int32_t adc_data = 0;
#endif
#ifdef SM_DIO_ENABLE
    DIGITAL_status_t digital_status = DIGITAL_SUCCESS;
    uint32_t* reg_digital_data_1_ptr = &(NODE_RAM_REGISTER[SM_REGISTER_ADDRESS_DIGITAL_DATA]);
    uint8_t state = 0;
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
    SHT3X_status_t sht3x_status = SHT3X_SUCCESS;
    uint32_t* reg_analog_data_3_ptr = &(NODE_RAM_REGISTER[SM_REGISTER_ADDRESS_ANALOG_DATA_3]);
    int32_t tamb_tenth_degrees = 0;
    int32_t hamb_percent = 0;
#endif
#if ((defined SM_AIN_ENABLE) || (defined SM_DIO_ENABLE) ||  (defined SM_DIGITAL_SENSORS_ENABLE))
    uint32_t unused_mask = 0;
#endif
    // Reset data.
    (*reg_analog_data_1_ptr) = NODE_REGISTER[SM_REGISTER_ADDRESS_ANALOG_DATA_1].error_value;
    (*reg_analog_data_2_ptr) = NODE_REGISTER[SM_REGISTER_ADDRESS_ANALOG_DATA_2].error_value;
    (*reg_analog_data_2_ptr) = NODE_REGISTER[SM_REGISTER_ADDRESS_ANALOG_DATA_3].error_value;
    (*reg_digital_data_1_ptr) = NODE_REGISTER[SM_REGISTER_ADDRESS_DIGITAL_DATA].error_value;
#ifdef SM_AIN_ENABLE
    // AIN0.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_AIN0_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(adc_data), SM_REGISTER_ANALOG_DATA_1_MASK_VAIN0);
    // AIN0.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_AIN1_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(adc_data), SM_REGISTER_ANALOG_DATA_1_MASK_VAIN1);
    // AIN2.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_AIN2_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_2_ptr, &unused_mask, UNA_convert_mv(adc_data), SM_REGISTER_ANALOG_DATA_2_MASK_VAIN2);
    // AIN3.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_AIN3_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_2_ptr, &unused_mask, UNA_convert_mv(adc_data), SM_REGISTER_ANALOG_DATA_2_MASK_VAIN3);
#endif
#ifdef SM_DIO_ENABLE
    // Turn digital front-end on.
    POWER_enable(POWER_REQUESTER_ID_SM, POWER_DOMAIN_DIGITAL, LPTIM_DELAY_MODE_SLEEP);
    // DIO0.
    digital_status = DIGITAL_read_channel(DIGITAL_CHANNEL_DIO0, &state);
    DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
    SWREG_write_field(reg_digital_data_1_ptr, &unused_mask, (uint32_t) state, SM_REGISTER_DIGITAL_DATA_MASK_DIO0);
    // DIO1.
    digital_status = DIGITAL_read_channel(DIGITAL_CHANNEL_DIO1, &state);
    DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
    SWREG_write_field(reg_digital_data_1_ptr, &unused_mask, (uint32_t) state, SM_REGISTER_DIGITAL_DATA_MASK_DIO1);
    // DIO2.
    digital_status = DIGITAL_read_channel(DIGITAL_CHANNEL_DIO2, &state);
    DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
    SWREG_write_field(reg_digital_data_1_ptr, &unused_mask, (uint32_t) state, SM_REGISTER_DIGITAL_DATA_MASK_DIO2);
    // DIO3.
    digital_status = DIGITAL_read_channel(DIGITAL_CHANNEL_DIO3, &state);
    DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
    SWREG_write_field(reg_digital_data_1_ptr, &unused_mask, (uint32_t) state, SM_REGISTER_DIGITAL_DATA_MASK_DIO3);
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
    // Turn sensors on.
    POWER_enable(POWER_REQUESTER_ID_SM, POWER_DOMAIN_SENSORS, LPTIM_DELAY_MODE_STOP);
    // TAMB.
    sht3x_status = SHT3X_get_temperature_humidity(I2C_ADDRESS_SHT30, &tamb_tenth_degrees, &hamb_percent);
    SHT3X_exit_error(NODE_ERROR_BASE_SHT3X);
    SWREG_write_field(reg_analog_data_3_ptr, &unused_mask, UNA_convert_tenth_degrees(tamb_tenth_degrees), SM_REGISTER_ANALOG_DATA_3_MASK_TAMB);
    SWREG_write_field(reg_analog_data_3_ptr, &unused_mask, (uint32_t) hamb_percent, SM_REGISTER_ANALOG_DATA_3_MASK_HAMB);
#endif
errors:
#ifdef SM_DIO_ENABLE
    POWER_disable(POWER_REQUESTER_ID_SM, POWER_DOMAIN_DIGITAL);
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
    POWER_disable(POWER_REQUESTER_ID_SM, POWER_DOMAIN_SENSORS);
#endif
    return status;
}

#endif /* SM */
