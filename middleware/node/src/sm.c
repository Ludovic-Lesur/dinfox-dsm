/*
 * sm.c
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#include "sm.h"

#include "analog.h"
#include "digital.h"
#include "error.h"
#include "i2c_address.h"
#include "load.h"
#include "node.h"
#include "sht3x.h"
#include "sm_registers.h"
#include "swreg.h"
#include "una.h"
#include "xm_flags.h"

#ifdef SM

/*** SM local functions ***/

/*******************************************************************/
static void _SM_reset_analog_data(void) {
    // Local variables.
    uint32_t reg_analog_data_1 = 0;
    uint32_t reg_analog_data_1_mask = 0;
    uint32_t reg_analog_data_2 = 0;
    uint32_t reg_analog_data_2_mask = 0;
    uint32_t reg_analog_data_3 = 0;
    uint32_t reg_analog_data_3_mask = 0;
    // Reset fields to error value.
    // AIN0 / AIN1.
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, SM_REGISTER_ANALOG_DATA_1_MASK_VAIN0);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, SM_REGISTER_ANALOG_DATA_1_MASK_VAIN1);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
    // AIN2 / AIN3.
    SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_VOLTAGE_ERROR_VALUE, SM_REGISTER_ANALOG_DATA_2_MASK_VAIN2);
    SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_VOLTAGE_ERROR_VALUE, SM_REGISTER_ANALOG_DATA_2_MASK_VAIN3);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REGISTER_ADDRESS_ANALOG_DATA_2, reg_analog_data_2, reg_analog_data_2_mask);
    // TAMB / HAMB.
    SWREG_write_field(&reg_analog_data_3, &reg_analog_data_3_mask, UNA_TEMPERATURE_ERROR_VALUE, SM_REGISTER_ANALOG_DATA_3_MASK_TAMB);
    SWREG_write_field(&reg_analog_data_3, &reg_analog_data_3_mask, UNA_HUMIDITY_ERROR_VALUE, SM_REGISTER_ANALOG_DATA_3_MASK_HAMB);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REGISTER_ADDRESS_ANALOG_DATA_3, reg_analog_data_3, reg_analog_data_3_mask);
}

/*******************************************************************/
static void _SM_reset_digital_data(void) {
    // Local variables.
    uint32_t reg_digital_data = 0;
    uint32_t reg_digital_data_mask = 0;
    // Reset fields to error value.
    SWREG_write_field(&reg_digital_data, &reg_digital_data_mask, UNA_BIT_ERROR, SM_REGISTER_DIGITAL_DATA_MASK_DIO0);
    SWREG_write_field(&reg_digital_data, &reg_digital_data_mask, UNA_BIT_ERROR, SM_REGISTER_DIGITAL_DATA_MASK_DIO1);
    SWREG_write_field(&reg_digital_data, &reg_digital_data_mask, UNA_BIT_ERROR, SM_REGISTER_DIGITAL_DATA_MASK_DIO2);
    SWREG_write_field(&reg_digital_data, &reg_digital_data_mask, UNA_BIT_ERROR, SM_REGISTER_DIGITAL_DATA_MASK_DIO3);
    // Write register.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REGISTER_ADDRESS_DIGITAL_DATA, reg_digital_data, reg_digital_data_mask);
}

/*** SM functions ***/

/*******************************************************************/
NODE_status_t SM_init_registers(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Load default values.
    _SM_reset_analog_data();
    _SM_reset_digital_data();
    // Read init state.
    status = SM_update_register(SM_REGISTER_ADDRESS_CONFIGURATION_0);
    if (status != NODE_SUCCESS) goto errors;
    status = SM_update_register(SM_REGISTER_ADDRESS_CONFIGURATION_1);
    if (status != NODE_SUCCESS) goto errors;
    status = SM_update_register(SM_REGISTER_ADDRESS_CONFIGURATION_2);
    if (status != NODE_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
NODE_status_t SM_update_register(uint8_t reg_addr) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Check address.
    switch (reg_addr) {
    case SM_REGISTER_ADDRESS_CONFIGURATION_0:
#ifdef SM_AIN_ENABLE
        // Analog inputs enable flag.
        SWREG_write_field(&reg_value, &reg_mask, 0b1, SM_REGISTER_CONFIGURATION_0_MASK_AINF);
#else
        SWREG_write_field(&reg_value, &reg_mask, 0b0, SM_REGISTER_CONFIGURATION_0_MASK_AINF);
#endif
        // Digital inputs enable flag.
#ifdef SM_DIO_ENABLE
        SWREG_write_field(&reg_value, &reg_mask, 0b1, SM_REGISTER_CONFIGURATION_0_MASK_DIOF);
#else
        SWREG_write_field(&reg_value, &reg_mask, 0b0, SM_REGISTER_CONFIGURATION_0_MASK_DIOF);
#endif
        // Digital sensors enable flag.
#ifdef SM_DIGITAL_SENSORS_ENABLE
        SWREG_write_field(&reg_value, &reg_mask, 0b1, SM_REGISTER_CONFIGURATION_0_MASK_DIGF);
#else
        SWREG_write_field(&reg_value, &reg_mask, 0b0, SM_REGISTER_CONFIGURATION_0_MASK_DIGF);
#endif
        break;
#ifdef SM_AIN_ENABLE
    case SM_REGISTER_ADDRESS_CONFIGURATION_1:
        // Analog inputs type and gain.
        SWREG_write_field(&reg_value, &reg_mask, ((SM_AIN0_GAIN_TYPE == ANALOG_GAIN_TYPE_AMPLIFICATION) ? 0b1 : 0b0), SM_REGISTER_CONFIGURATION_1_MASK_AI0T);
        SWREG_write_field(&reg_value, &reg_mask, SM_AIN0_GAIN, SM_REGISTER_CONFIGURATION_1_MASK_AI0G);
        SWREG_write_field(&reg_value, &reg_mask, ((SM_AIN1_GAIN_TYPE == ANALOG_GAIN_TYPE_AMPLIFICATION) ? 0b1 : 0b0), SM_REGISTER_CONFIGURATION_1_MASK_AI1T);
        SWREG_write_field(&reg_value, &reg_mask, SM_AIN1_GAIN, SM_REGISTER_CONFIGURATION_1_MASK_AI1G);
        break;
    case SM_REGISTER_ADDRESS_CONFIGURATION_2:
        // Analog inputs type and gain.
        SWREG_write_field(&reg_value, &reg_mask, ((SM_AIN2_GAIN_TYPE == ANALOG_GAIN_TYPE_AMPLIFICATION) ? 0b1 : 0b0), SM_REGISTER_CONFIGURATION_2_MASK_AI2T);
        SWREG_write_field(&reg_value, &reg_mask, SM_AIN2_GAIN, SM_REGISTER_CONFIGURATION_2_MASK_AI2G);
        SWREG_write_field(&reg_value, &reg_mask, ((SM_AIN2_GAIN_TYPE == ANALOG_GAIN_TYPE_AMPLIFICATION) ? 0b1 : 0b0), SM_REGISTER_CONFIGURATION_2_MASK_AI3T);
        SWREG_write_field(&reg_value, &reg_mask, SM_AIN2_GAIN, SM_REGISTER_CONFIGURATION_2_MASK_AI3G);
        break;
#endif
    default:
        // Nothing to do for other registers.
        break;
    }
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, reg_mask);
    return status;
}

/*******************************************************************/
NODE_status_t SM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
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
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
#ifdef SM_AIN_ENABLE
    int32_t adc_data = 0;
    uint32_t reg_analog_data_1 = 0;
    uint32_t reg_analog_data_1_mask = 0;
    uint32_t reg_analog_data_2 = 0;
    uint32_t reg_analog_data_2_mask = 0;
#endif
#ifdef SM_DIO_ENABLE
    DIGITAL_status_t digital_status = DIGITAL_SUCCESS;
    uint8_t state = 0;
    uint32_t reg_digital_data = 0;
    uint32_t reg_digital_data_mask = 0;
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
    SHT3X_status_t sht3x_status = SHT3X_SUCCESS;
    int32_t tamb_degrees = 0;
    int32_t hamb_percent = 0;
    uint32_t reg_analog_data_3 = 0;
    uint32_t reg_analog_data_3_mask = 0;
#endif
    // Reset results.
    _SM_reset_analog_data();
    _SM_reset_digital_data();
#ifdef SM_AIN_ENABLE
    // AIN0.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_AIN0_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(adc_data), SM_REGISTER_ANALOG_DATA_1_MASK_VAIN0);
    // AIN0.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_AIN1_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(adc_data), SM_REGISTER_ANALOG_DATA_1_MASK_VAIN1);
    // Write register.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
    // AIN2.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_AIN2_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_convert_mv(adc_data), SM_REGISTER_ANALOG_DATA_2_MASK_VAIN2);
    // AIN3.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_AIN3_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_convert_mv(adc_data), SM_REGISTER_ANALOG_DATA_2_MASK_VAIN3);
    // Write register.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REGISTER_ADDRESS_ANALOG_DATA_2, reg_analog_data_2, reg_analog_data_2_mask);
#endif
#ifdef SM_DIO_ENABLE
    // Turn digital front-end on.
    POWER_enable(POWER_REQUESTER_ID_SM, POWER_DOMAIN_DIGITAL, LPTIM_DELAY_MODE_SLEEP);
    // DIO0.
    digital_status = DIGITAL_read_channel(DIGITAL_CHANNEL_DIO0, &state);
    DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
    SWREG_write_field(&reg_digital_data, &reg_digital_data_mask, (uint32_t) state, SM_REGISTER_DIGITAL_DATA_MASK_DIO0);
    // DIO1.
    digital_status = DIGITAL_read_channel(DIGITAL_CHANNEL_DIO1, &state);
    DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
    SWREG_write_field(&reg_digital_data, &reg_digital_data_mask, (uint32_t) state, SM_REGISTER_DIGITAL_DATA_MASK_DIO1);
    // DIO2.
    digital_status = DIGITAL_read_channel(DIGITAL_CHANNEL_DIO2, &state);
    DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
    SWREG_write_field(&reg_digital_data, &reg_digital_data_mask, (uint32_t) state, SM_REGISTER_DIGITAL_DATA_MASK_DIO2);
    // DIO3.
    digital_status = DIGITAL_read_channel(DIGITAL_CHANNEL_DIO3, &state);
    DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
    SWREG_write_field(&reg_digital_data, &reg_digital_data_mask, (uint32_t) state, SM_REGISTER_DIGITAL_DATA_MASK_DIO3);
    // Write register.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REGISTER_ADDRESS_DIGITAL_DATA, reg_digital_data, reg_digital_data_mask);
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
    // Turn sensors on.
    POWER_enable(POWER_REQUESTER_ID_SM, POWER_DOMAIN_SENSORS, LPTIM_DELAY_MODE_STOP);
    // TAMB.
    sht3x_status = SHT3X_get_temperature_humidity(I2C_ADDRESS_SHT30, &tamb_degrees, &hamb_percent);
    SHT3X_exit_error(NODE_ERROR_BASE_SHT3X);
    SWREG_write_field(&reg_analog_data_3, &reg_analog_data_3_mask, UNA_convert_degrees(tamb_degrees), SM_REGISTER_ANALOG_DATA_3_MASK_TAMB);
    SWREG_write_field(&reg_analog_data_3, &reg_analog_data_3_mask, (uint32_t) hamb_percent, SM_REGISTER_ANALOG_DATA_3_MASK_HAMB);
    // Write register.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REGISTER_ADDRESS_ANALOG_DATA_3, reg_analog_data_3, reg_analog_data_3_mask);
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
