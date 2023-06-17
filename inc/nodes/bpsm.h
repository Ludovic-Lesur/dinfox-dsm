/*
 * bpsm.h
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#ifndef __BPSM_H__
#define __BPSM_H__

#include "bpsm_reg.h"
#include "common.h"
#include "common_reg.h"
#include "dinfox.h"
#include "node.h"

#ifdef BPSM

/*** BPSM macros ***/

#define NODE_BOARD_ID		DINFOX_BOARD_ID_BPSM
#define NODE_REG_ADDR_LAST	BPSM_REG_ADDR_LAST

/*** BPSM global variables ***/

static const DINFOX_register_access_t NODE_REG_ACCESS[BPSM_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};

/*** BPSM functions ***/

void BPSM_init_registers(void);

NODE_status_t BPSM_update_register(uint8_t reg_addr);
NODE_status_t BPSM_check_register(uint8_t reg_addr);

NODE_status_t BPSM_mtrg_callback(ADC_status_t* adc_status);

#endif /* BPSM */

#endif /* __BPSM_H__ */
