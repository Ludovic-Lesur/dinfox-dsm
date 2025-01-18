/*
 * nvm_address.h
 *
 *  Created on: 19 jun. 2018
 *      Author: Ludo
 */

#ifndef __NVM_ADDRESS_H__
#define __NVM_ADDRESS_H__

#include "sigfox_types.h"

/*!******************************************************************
 * \enum NVM_address_mapping_t
 * \brief NVM address mapping.
 *******************************************************************/
typedef enum {
    NVM_ADDRESS_SELF_ADDRESS = 0,
    NVM_ADDRESS_SIGFOX_EP_ID = 1,
    NVM_ADDRESS_SIGFOX_EP_KEY = (NVM_ADDRESS_SIGFOX_EP_ID + SIGFOX_EP_ID_SIZE_BYTES),
    NVM_ADDRESS_SIGFOX_EP_LIB_DATA = (NVM_ADDRESS_SIGFOX_EP_KEY + SIGFOX_EP_KEY_SIZE_BYTES),
    NVM_ADDRESS_REGISTERS = 0x40,
} NVM_address_mapping_t;

#endif /* __NVM_ADDRESS_H__ */
