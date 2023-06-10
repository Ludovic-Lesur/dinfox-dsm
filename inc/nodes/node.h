/*
 * node.h
 *
 *  Created on: 18 feb. 2023
 *      Author: Ludo
 */

#ifndef __NODE_H__
#define __NODE_H__

#include "adc.h"
#include "digital.h"
#include "i2c.h"
#include "load.h"
#include "neom8n.h"
#include "nvm.h"
#include "s2lp.h"
#include "sht3x.h"
#include "types.h"
#include "usart.h"

/*** NODE structures ***/

typedef enum {
	NODE_SUCCESS = 0,
	NODE_ERROR_NULL_PARAMETER,
	NODE_ERROR_DATA_SIZE,
	NODE_ERROR_REGISTER_ADDRESS,
	NODE_ERROR_REGISTER_READ_ONLY,
	NODE_ERROR_RADIO_STATE,
	NODE_ERROR_RADIO_POWER,
	NODE_ERROR_MESSAGE_TYPE,
	NODE_ERROR_BASE_ADC = 0x0100,
	NODE_ERROR_BASE_I2C = (NODE_ERROR_BASE_ADC + ADC_ERROR_BASE_LAST),
	NODE_ERROR_BASE_NVM = (NODE_ERROR_BASE_I2C + I2C_ERROR_BASE_LAST),
	NODE_ERROR_BASE_USART2 = (NODE_ERROR_BASE_NVM + NVM_ERROR_BASE_LAST),
	NODE_ERROR_BASE_DIGITAL = (NODE_ERROR_BASE_USART2 + USART_ERROR_BASE_LAST),
	NODE_ERROR_BASE_LOAD = (NODE_ERROR_BASE_DIGITAL + DIGITAL_ERROR_BASE_LAST),
	NODE_ERROR_BASE_S2LP = (NODE_ERROR_BASE_LOAD + LOAD_ERROR_BASE_LAST),
	NODE_ERROR_BASE_NEOM8N = (NODE_ERROR_BASE_S2LP + S2LP_ERROR_BASE_LAST),
	NODE_ERROR_BASE_SHT3X = (NODE_ERROR_BASE_NEOM8N + NEOM8N_ERROR_BASE_LAST),
	NODE_ERROR_BASE_SIGFOX = (NODE_ERROR_BASE_SHT3X + SHT3X_ERROR_BASE_LAST),
	NODE_ERROR_BASE_LAST = (NODE_ERROR_BASE_SIGFOX + 0x4000)
} NODE_status_t;

typedef enum {
	NODE_REQUEST_SOURCE_INTERNAL = 0,
	NODE_REQUEST_SOURCE_EXTERNAL,
	NODE_REQUEST_SOURCE_LAST
} NODE_request_source_t;

typedef uint8_t	NODE_address_t;

/*** NODE functions ***/

NODE_status_t NODE_init(void);

NODE_status_t NODE_read_register(NODE_request_source_t request_source, uint8_t reg_addr, uint32_t* reg_value);
NODE_status_t NODE_read_field(NODE_request_source_t request_source, uint8_t reg_addr, uint32_t field_mask, uint32_t* field_value);
NODE_status_t NODE_read_byte_array(NODE_request_source_t request_source, uint8_t reg_addr_base, uint8_t* data, uint8_t data_size_byte);

NODE_status_t NODE_write_register(NODE_request_source_t request_source, uint8_t reg_addr, uint32_t reg_mask, uint32_t reg_value);
NODE_status_t NODE_write_field(NODE_request_source_t request_source, uint8_t reg_addr, uint32_t field_mask, uint32_t field_value);
NODE_status_t NODE_write_byte_array(NODE_request_source_t request_source, uint8_t reg_addr_base, uint8_t* data, uint8_t data_size_byte);

#define NODE_status_check(error_base) { if (node_status != NODE_SUCCESS) { status = error_base + node_status; goto errors; }}
#define NODE_error_check() { ERROR_status_check(node_status, NODE_SUCCESS, ERROR_BASE_NODE); }
#define NODE_error_check_print() { ERROR_status_check_print(node_status, NODE_SUCCESS, ERROR_BASE_NODE); }

#endif /* __NODE_H__ */
