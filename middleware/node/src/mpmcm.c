/*
 * ddrm.c
 *
 *  Created on: 03 sep. 2023
 *      Author: Ludo
 */

#include "mpmcm.h"

#ifdef MPMCM

#include "adc.h"
#include "data.h"
#include "dsm_flags.h"
#include "error.h"
#include "error_base.h"
#include "measure.h"
#include "mpmcm_registers.h"
#include "node_register.h"
#include "node_status.h"
#include "swreg.h"
#include "types.h"
#include "una.h"

/*** MPMCM local macros ***/

#ifdef MPMCM_ANALOG_MEASURE_ENABLE
#define MPMCM_FLAG_AME      0b1
#else
#define MPMCM_FLAG_AME      0b0
#endif
#ifdef MPMCM_LINKY_TIC_ENABLE
#define MPMCM_FLAG_LTE      0b1
#else
#define MPMCM_FLAG_LTE      0b0
#endif
#ifdef MPMCM_LINKY_TIC_MODE_HISTORIC
#define MPMCM_FLAG_LTM      0b0
#else
#define MPMCM_FLAG_LTM      0b1
#endif

/*** MPMCM local functions ***/

/*******************************************************************/
static void _MPMCM_set_analog_gains(void) {
    // Local variables.
    MEASURE_status_t measure_status = MEASURE_SUCCESS;
    uint32_t reg_config_0 = NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CONFIGURATION_0];
    uint32_t reg_config_1 = NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CONFIGURATION_1];
    uint32_t reg_config_2 = NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CONFIGURATION_2];
    uint16_t transformer_gain = 0;
    uint16_t current_sensors_gain[MEASURE_NUMBER_OF_ACI_CHANNELS];
    // Compute gains.
    transformer_gain = SWREG_read_field(reg_config_0, MPMCM_REGISTER_CONFIGURATION_0_MASK_TRANSFORMER_GAIN);
    current_sensors_gain[0] = SWREG_read_field(reg_config_1, MPMCM_REGISTER_CONFIGURATION_1_MASK_CH1_CURRENT_SENSOR_GAIN);
    current_sensors_gain[1] = SWREG_read_field(reg_config_1, MPMCM_REGISTER_CONFIGURATION_1_MASK_CH2_CURRENT_SENSOR_GAIN);
    current_sensors_gain[2] = SWREG_read_field(reg_config_2, MPMCM_REGISTER_CONFIGURATION_2_MASK_CH3_CURRENT_SENSOR_GAIN);
    current_sensors_gain[3] = SWREG_read_field(reg_config_2, MPMCM_REGISTER_CONFIGURATION_2_MASK_CH4_CURRENT_SENSOR_GAIN);
    // Set gains.
    measure_status = MEASURE_set_gains(transformer_gain, current_sensors_gain);
    MEASURE_stack_error(ERROR_BASE_MEASURE);
}

/*******************************************************************/
static void _MPMCM_set_tic_sampling_period(void) {
    // Local variables.
    TIC_status_t tic_status = TIC_SUCCESS;
    uint32_t reg_config_3 = NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CONFIGURATION_3];
    uint32_t period_seconds = 0;
    // Compute period.
    period_seconds = UNA_get_seconds(SWREG_read_field(reg_config_3, MPMCM_REGISTER_CONFIGURATION_3_MASK_TIC_SAMPLING_PERIOD));
    // Set period.
    tic_status = TIC_set_sampling_period(period_seconds);
    TIC_stack_error(ERROR_BASE_TIC);
}

/*** MPMCM functions ***/

/*******************************************************************/
NODE_status_t MPMCM_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Init mains measure driver.
    _MPMCM_set_analog_gains();
    _MPMCM_set_tic_sampling_period();
    return status;
}

/*******************************************************************/
void MPMCM_init_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    uint32_t unused_mask = 0;
#ifdef DSM_NVM_FACTORY_RESET
    uint16_t mpmcm_sct013_gain[MEASURE_NUMBER_OF_ACI_CHANNELS] = MPMCM_SCT013_GAIN;
#endif
    // Check address.
    switch (reg_addr) {
    case MPMCM_REGISTER_ADDRESS_FLAGS_1:
        SWREG_write_field(reg_value, &unused_mask, MPMCM_FLAG_AME, MPMCM_REGISTER_FLAGS_1_MASK_AME);
        SWREG_write_field(reg_value, &unused_mask, MPMCM_FLAG_LTE, MPMCM_REGISTER_FLAGS_1_MASK_LTE);
        SWREG_write_field(reg_value, &unused_mask, MPMCM_FLAG_LTM, MPMCM_REGISTER_FLAGS_1_MASK_LTM);
        SWREG_write_field(reg_value, &unused_mask, MPMCM_TRANSFORMER_ATTEN, MPMCM_REGISTER_FLAGS_1_MASK_TRANSFORMER_ATTEN);
        break;
    case MPMCM_REGISTER_ADDRESS_FLAGS_2:
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) MEASURE_SCT013_ATTEN[0], MPMCM_REGISTER_FLAGS_2_MASK_CH1_CURRENT_SENSOR_ATTEN);
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) MEASURE_SCT013_ATTEN[1], MPMCM_REGISTER_FLAGS_2_MASK_CH2_CURRENT_SENSOR_ATTEN);
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) MEASURE_SCT013_ATTEN[2], MPMCM_REGISTER_FLAGS_2_MASK_CH3_CURRENT_SENSOR_ATTEN);
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) MEASURE_SCT013_ATTEN[3], MPMCM_REGISTER_FLAGS_2_MASK_CH4_CURRENT_SENSOR_ATTEN);
        break;
#ifdef DSM_NVM_FACTORY_RESET
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_write_field(reg_value, &unused_mask, MPMCM_TRANSFORMER_GAIN, MPMCM_REGISTER_CONFIGURATION_0_MASK_TRANSFORMER_GAIN);
        break;
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_1:
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) mpmcm_sct013_gain[0], MPMCM_REGISTER_CONFIGURATION_1_MASK_CH1_CURRENT_SENSOR_GAIN);
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) mpmcm_sct013_gain[1], MPMCM_REGISTER_CONFIGURATION_1_MASK_CH2_CURRENT_SENSOR_GAIN);
        break;
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_2:
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) mpmcm_sct013_gain[2], MPMCM_REGISTER_CONFIGURATION_2_MASK_CH3_CURRENT_SENSOR_GAIN);
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) mpmcm_sct013_gain[3], MPMCM_REGISTER_CONFIGURATION_2_MASK_CH4_CURRENT_SENSOR_GAIN);
        break;
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_3:
        SWREG_write_field(reg_value, &unused_mask, (uint32_t) UNA_convert_seconds(TIC_SAMPLING_PERIOD_SECONDS_DEFAULT), MPMCM_REGISTER_CONFIGURATION_3_MASK_TIC_SAMPLING_PERIOD);
        break;
#endif
    default:
        break;
    }
}

/*******************************************************************/
void MPMCM_refresh_register(uint8_t reg_addr) {
    // Local variables.
    MEASURE_status_t measure_status = MEASURE_SUCCESS;
    TIC_status_t tic_status = TIC_SUCCESS;
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
    uint8_t channel_idx = 0;
    uint32_t unused_mask = 0;
    uint8_t generic_u8 = 0;
    // Check address.
    switch (reg_addr) {
    case MPMCM_REGISTER_ADDRESS_STATUS_1:
        // Update probe detect flags.
        for (channel_idx = 0; channel_idx < MEASURE_NUMBER_OF_ACI_CHANNELS; channel_idx++) {
            // Read flag.
            measure_status = MEASURE_get_probe_detect_flag(channel_idx, &generic_u8);
            MEASURE_stack_error(ERROR_BASE_MEASURE);
            // Update field.
            SWREG_write_field(reg_ptr, &unused_mask, (uint32_t) generic_u8, (0b1 << channel_idx));
        }
        // Update mains detect flag.
        measure_status = MEASURE_get_mains_detect_flag(&generic_u8);
        MEASURE_stack_error(ERROR_BASE_MEASURE);
        // Update field.
        SWREG_write_field(reg_ptr, &unused_mask, (uint32_t) generic_u8, MPMCM_REGISTER_STATUS_1_MASK_MVD);
        // Update Linky TIC detect flag.
        tic_status = TIC_get_detect_flag(&generic_u8);
        TIC_stack_error(ERROR_BASE_TIC);
        // Update field.
        SWREG_write_field(reg_ptr, &unused_mask, (uint32_t) generic_u8, MPMCM_REGISTER_STATUS_1_MASK_TICD);
        break;
    default:
        break;
    }
}

/*******************************************************************/
NODE_status_t MPMCM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t generic_u32 = 0;
    // Check address.
    switch (reg_addr) {
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_secure_field(MPMCM_REGISTER_CONFIGURATION_0_MASK_TRANSFORMER_GAIN,,, == 0, == 0, 10, status = NODE_ERROR_REGISTER_FIELD_VALUE);
        break;
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_1:
        SWREG_secure_field(MPMCM_REGISTER_CONFIGURATION_1_MASK_CH1_CURRENT_SENSOR_GAIN,,, == 0, == 0, 10, status = NODE_ERROR_REGISTER_FIELD_VALUE);
        SWREG_secure_field(MPMCM_REGISTER_CONFIGURATION_1_MASK_CH2_CURRENT_SENSOR_GAIN,,, == 0, == 0, 10, status = NODE_ERROR_REGISTER_FIELD_VALUE);
        break;
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_2:
        SWREG_secure_field(MPMCM_REGISTER_CONFIGURATION_2_MASK_CH3_CURRENT_SENSOR_GAIN,,, == 0, == 0, 10, status = NODE_ERROR_REGISTER_FIELD_VALUE);
        SWREG_secure_field(MPMCM_REGISTER_CONFIGURATION_2_MASK_CH4_CURRENT_SENSOR_GAIN,,, == 0, == 0, 10, status = NODE_ERROR_REGISTER_FIELD_VALUE);
        break;
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_3:
        SWREG_secure_field(
            MPMCM_REGISTER_CONFIGURATION_3_MASK_TIC_SAMPLING_PERIOD,
            UNA_get_seconds,
            UNA_convert_seconds,
            < TIC_SAMPLING_PERIOD_SECONDS_MIN,
            > TIC_SAMPLING_PERIOD_SECONDS_MAX,
            TIC_SAMPLING_PERIOD_SECONDS_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    default:
        break;
    }
    return status;
}

/*******************************************************************/
NODE_status_t MPMCM_process_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    MEASURE_status_t measure_status = MEASURE_SUCCESS;
    TIC_status_t tic_status = TIC_SUCCESS;
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
    DATA_accumulated_t single_data;
    DATA_accumulated_channel_t channel_data;
    uint32_t field_value = 0;
    uint32_t unused_mask = 0;
    uint8_t channel_idx = 0;
    uint8_t reg_offset = 0;
    // Check address.
    switch (reg_addr) {
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_0:
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_1:
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_2:
        // Update measurements gains.
        _MPMCM_set_analog_gains();
        break;
    case MPMCM_REGISTER_ADDRESS_CONFIGURATION_3:
        // Update TIC sampling period.
        _MPMCM_set_tic_sampling_period();
        break;
    case MPMCM_REGISTER_ADDRESS_CONTROL_1:
        // FRQS.
        if ((reg_mask & MPMCM_REGISTER_CONTROL_1_MASK_FRQS) != 0) {
            // Check bit.
            if (SWREG_read_field((*reg_ptr), MPMCM_REGISTER_CONTROL_1_MASK_FRQS) != 0) {
                // Clear request.
                SWREG_write_field(reg_ptr, &unused_mask, 0b0, MPMCM_REGISTER_CONTROL_1_MASK_FRQS);
                // Read and reset measurements.
                measure_status = MEASURE_get_accumulated_data(MEASURE_DATA_INDEX_MAINS_FREQUENCY_MHZ, &single_data);
                MEASURE_exit_error(NODE_ERROR_BASE_MEASURE);
                // Write registers.
                field_value = (single_data.number_of_samples > 0) ? (uint32_t) (single_data.rolling_mean / 10.0) : UNA_MAINS_FREQUENCY_ERROR_VALUE;
                SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_MAINS_FREQUENCY_0]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MEAN);
                field_value = (single_data.number_of_samples > 0) ? (uint32_t) (single_data.min / 10.0) : UNA_MAINS_FREQUENCY_ERROR_VALUE;
                SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_MAINS_FREQUENCY_1]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MIN);
                field_value = (single_data.number_of_samples > 0) ? (uint32_t) (single_data.max / 10.0) : UNA_MAINS_FREQUENCY_ERROR_VALUE;
                SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_MAINS_FREQUENCY_1]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MAX);
            }
        }
        // CHxS and TICS.
        for (channel_idx = 0; channel_idx < (MEASURE_NUMBER_OF_ACI_CHANNELS + 1); channel_idx++) {
            //  Check mask.
            if ((reg_mask & (0b1 << channel_idx)) != 0) {
                // Check bit.
                if (SWREG_read_field((*reg_ptr), (0b1 << channel_idx)) != 0) {
                    // Clear request.
                    SWREG_write_field(reg_ptr, &unused_mask, 0b0, (0b1 << channel_idx));
                    // Read and reset measurements.
                    if (channel_idx < MEASURE_NUMBER_OF_ACI_CHANNELS) {
                        // Read from analog measure for channels 0 to 3.
                        measure_status = MEASURE_get_channel_accumulated_data(channel_idx, &channel_data);
                        MEASURE_exit_error(NODE_ERROR_BASE_MEASURE);
                    }
                    else {
                        // Read from TIC.
                        tic_status = TIC_get_channel_accumulated_data(&channel_data);
                        TIC_exit_error(NODE_ERROR_BASE_TIC);
                    }
                    // Compute registers offset.
                    reg_offset = (MPMCM_NUMBER_OF_REGISTERS_PER_DATA * channel_idx);
                    // Active power.
                    field_value = (channel_data.active_power_mw.number_of_samples > 0) ? UNA_convert_mw_mva((int32_t) channel_data.active_power_mw.rolling_mean) : UNA_ELECTRICAL_POWER_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_ACTIVE_POWER_0 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MEAN);
                    field_value = (channel_data.active_power_mw.number_of_samples > 0) ? UNA_convert_mw_mva((int32_t) channel_data.active_power_mw.min) : UNA_ELECTRICAL_POWER_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_ACTIVE_POWER_1 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MIN);
                    field_value = (channel_data.active_power_mw.number_of_samples > 0) ? UNA_convert_mw_mva((int32_t) channel_data.active_power_mw.max) : UNA_ELECTRICAL_POWER_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_ACTIVE_POWER_1 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MAX);
                    // RMS voltage.
                    field_value = (channel_data.rms_voltage_mv.number_of_samples > 0) ? UNA_convert_mv((int32_t) channel_data.rms_voltage_mv.rolling_mean) : UNA_VOLTAGE_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_RMS_VOLTAGE_0 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MEAN);
                    field_value = (channel_data.rms_voltage_mv.number_of_samples > 0) ? UNA_convert_mv((int32_t) channel_data.rms_voltage_mv.min) : UNA_VOLTAGE_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_RMS_VOLTAGE_1 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MIN);
                    field_value = (channel_data.rms_voltage_mv.number_of_samples > 0) ? UNA_convert_mv((int32_t) channel_data.rms_voltage_mv.max) : UNA_VOLTAGE_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_RMS_VOLTAGE_1 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MAX);
                    // RMS current.
                    field_value = (channel_data.rms_current_ma.number_of_samples > 0) ? UNA_convert_ua((int32_t) (channel_data.rms_current_ma.rolling_mean * 1000.0)) : UNA_CURRENT_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_RMS_CURRENT_0 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MEAN);
                    field_value = (channel_data.rms_current_ma.number_of_samples > 0) ? UNA_convert_ua((int32_t) (channel_data.rms_current_ma.min * 1000.0)) : UNA_CURRENT_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_RMS_CURRENT_1 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MIN);
                    field_value = (channel_data.rms_current_ma.number_of_samples > 0) ? UNA_convert_ua((int32_t) (channel_data.rms_current_ma.max * 1000.0)) : UNA_CURRENT_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_RMS_CURRENT_1 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MAX);
                    // Apparent power.
                    field_value = (channel_data.apparent_power_mva.number_of_samples > 0) ? UNA_convert_mw_mva((int32_t) channel_data.apparent_power_mva.rolling_mean) : UNA_ELECTRICAL_POWER_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_APPARENT_POWER_0 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MEAN);
                    field_value = (channel_data.apparent_power_mva.number_of_samples > 0) ? UNA_convert_mw_mva((int32_t) channel_data.apparent_power_mva.min) : UNA_ELECTRICAL_POWER_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_APPARENT_POWER_1 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MIN);
                    field_value = (channel_data.apparent_power_mva.number_of_samples > 0) ? UNA_convert_mw_mva((int32_t) channel_data.apparent_power_mva.max) : UNA_ELECTRICAL_POWER_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_APPARENT_POWER_1 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MAX);
                    // Power factor.
                    field_value = (channel_data.power_factor.number_of_samples > 0) ? UNA_convert_power_factor((int32_t) channel_data.power_factor.rolling_mean) : UNA_POWER_FACTOR_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_POWER_FACTOR_0 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MEAN);
                    field_value = (channel_data.power_factor.number_of_samples > 0) ? UNA_convert_power_factor((int32_t) channel_data.power_factor.min) : UNA_POWER_FACTOR_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_POWER_FACTOR_1 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MIN);
                    field_value = (channel_data.power_factor.number_of_samples > 0) ? UNA_convert_power_factor((int32_t) channel_data.power_factor.max) : UNA_POWER_FACTOR_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_POWER_FACTOR_1 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_MAX);
                    // Active and apparent energy.
                    field_value = (channel_data.active_energy_mwh.number_of_samples > 0) ? UNA_convert_mwh_mvah((int32_t) channel_data.active_energy_mwh.value) : UNA_ELECTRICAL_ENERGY_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_ENERGY + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_ACTIVE_ENERGY);
                    field_value = (channel_data.apparent_energy_mvah.number_of_samples > 0) ? UNA_convert_mwh_mvah((int32_t) channel_data.apparent_energy_mvah.value) : UNA_ELECTRICAL_ENERGY_ERROR_VALUE;
                    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_ENERGY + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_APPARENT_ENERGY);
                }
            }
        }
        break;
    default:
        break;
    }
errors:
    return status;
}

/*******************************************************************/
NODE_status_t MPMCM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    MEASURE_status_t measure_status = MEASURE_SUCCESS;
    TIC_status_t tic_status = TIC_SUCCESS;
    DATA_run_t single_data;
    DATA_run_channel_t channel_data;
    uint32_t field_value = 0;
    uint32_t unused_mask = 0;
    uint8_t reg_offset = 0;
    uint8_t channel_idx = 0;
    // Mains frequency.
    measure_status = MEASURE_get_run_data(MEASURE_DATA_INDEX_MAINS_FREQUENCY_MHZ, &single_data);
    MEASURE_exit_error(NODE_ERROR_BASE_MEASURE);
    // Write register.
    field_value = (single_data.number_of_samples > 0) ? (uint32_t) (single_data.value / 10.0) : UNA_MAINS_FREQUENCY_ERROR_VALUE;
    SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_MAINS_FREQUENCY_0]), &unused_mask, field_value, MPMCM_REGISTER_MASK_RUN);
    // Update run registers for all channels.
    for (channel_idx = 0; channel_idx < (MEASURE_NUMBER_OF_ACI_CHANNELS + 1); channel_idx++) {
        // Read run data.
        if (channel_idx < MEASURE_NUMBER_OF_ACI_CHANNELS) {
            // Read from analog measure for channels 0 to 3.
            measure_status = MEASURE_get_channel_run_data(channel_idx, &channel_data);
            MEASURE_exit_error(NODE_ERROR_BASE_MEASURE);
        }
        else {
            // Read from TIC.
            tic_status = TIC_get_channel_run_data(&channel_data);
            TIC_exit_error(NODE_ERROR_BASE_TIC);
        }
        // Compute registers offset.
        reg_offset = (MPMCM_NUMBER_OF_REGISTERS_PER_DATA * channel_idx);
        // Active power.
        field_value = (channel_data.active_power_mw.number_of_samples > 0) ? UNA_convert_mw_mva((int32_t) channel_data.active_power_mw.value) : UNA_ELECTRICAL_POWER_ERROR_VALUE;
        SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_ACTIVE_POWER_0 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_RUN);
        // RMS voltage.
        field_value = (channel_data.rms_voltage_mv.number_of_samples > 0) ? UNA_convert_mv((int32_t) channel_data.rms_voltage_mv.value) : UNA_VOLTAGE_ERROR_VALUE;
        SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_RMS_VOLTAGE_0 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_RUN);
        // RMS current.
        field_value = (channel_data.rms_current_ma.number_of_samples > 0) ? UNA_convert_ua((int32_t) (channel_data.rms_current_ma.value * 1000.0)) : UNA_CURRENT_ERROR_VALUE;
        SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_RMS_CURRENT_0 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_RUN);
        // Apparent power.
        field_value = (channel_data.apparent_power_mva.number_of_samples > 0) ? UNA_convert_mw_mva((int32_t) channel_data.apparent_power_mva.value) : UNA_ELECTRICAL_POWER_ERROR_VALUE;
        SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_APPARENT_POWER_0 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_RUN);
        // Power factor.
        field_value = (channel_data.power_factor.number_of_samples > 0) ? UNA_convert_power_factor((int32_t) channel_data.power_factor.value) : UNA_POWER_FACTOR_ERROR_VALUE;
        SWREG_write_field(&(NODE_RAM_REGISTER[MPMCM_REGISTER_ADDRESS_CH1_POWER_FACTOR_0 + reg_offset]), &unused_mask, field_value, MPMCM_REGISTER_MASK_RUN);
    }
errors:
    return status;
}

#endif /* MPMCM */
