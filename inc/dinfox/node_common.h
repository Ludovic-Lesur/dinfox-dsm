/*
 * node_common.h
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#ifndef __NODE_COMMON_H__
#define __NODE_COMMON_H__

#include "types.h"

/*** NODES common structures ***/

typedef uint8_t	NODE_address_t;

typedef enum {
	NODE_PROTOCOL_NONE = 0,
	NODE_PROTOCOL_AT_BUS,
	NODE_PROTOCOL_R4S8CR,
	NODE_PROTOCOL_LAST
} NODE_protocol_t;

typedef enum {
	NODE_REPLY_TYPE_NONE = 0,
	NODE_REPLY_TYPE_OK,
	NODE_REPLY_TYPE_VALUE,
	NODE_REPLY_TYPE_LAST
} NODE_reply_type_t;

typedef struct {
	NODE_reply_type_t type;
	uint32_t timeout_ms;
} NODE_reply_parameters_t;

typedef struct {
	NODE_address_t node_addr;
	char_t* command;
} NODE_command_parameters_t;

typedef struct {
	NODE_address_t node_addr;
	uint8_t reg_addr;
	NODE_reply_parameters_t reply_params;
} NODE_access_parameters_t;

typedef union {
	struct {
		unsigned error_received : 1;
		unsigned parser_error : 1;
		unsigned reply_timeout : 1;
		unsigned sequence_timeout : 1;
	};
	uint8_t all;
} NODE_access_status_t;

#endif /* __NODE_COMMON_H__ */
