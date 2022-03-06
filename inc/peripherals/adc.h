/*
 * adc.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef ADC_H
#define ADC_H

/*** ADC structures ***/

typedef enum {
	ADC_DATA_IDX_VIN_MV = 0,
	ADC_DATA_IDX_VOUT_MV,
	ADC_DATA_IDX_IOUT_UA,
	ADC_DATA_IDX_VMCU_MV,
	ADC_DATA_IDX_MAX
} ADC_data_index_t;

/*** ADC functions ***/

void ADC1_init(void);
void ADC1_disable(void);
void ADC1_perform_measurements(void);
void ADC1_get_data(ADC_data_index_t data_idx, unsigned int* data);

#endif /* ADC_H */
