/*
 * dinfox.h
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#ifndef __DINFOX_H__
#define __DINFOX_H__

#include "dinfox_reg.h"
#include "node.h"
#include "types.h"

/*** DINFOX structures ***/

typedef enum {
	DINFOX_REG_ACCESS_READ_ONLY = 0,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_LAST
} DINFOX_register_access_t;

/*** DINFOX macros ***/

#define DINFOX_REG_ACCESS \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_ONLY,  \
	DINFOX_REG_ACCESS_READ_WRITE, \
	DINFOX_REG_ACCESS_READ_ONLY,  \

/*** DINFOX functions ***/

NODE_status_t DINFOX_init_registers(void);
NODE_status_t DINFOX_update_register(uint8_t reg_addr);
NODE_status_t DINFOX_check_register(uint8_t reg_addr);

#endif /* __DINFOX_H__ */
