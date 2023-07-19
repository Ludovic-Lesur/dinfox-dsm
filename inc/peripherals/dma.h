/*
 * dma.h
 *
 *  Created on: 16 aug. 2020
 *      Author: Ludo
 */

#ifndef __DMA_H__
#define __DMA_H__

#include "types.h"

/*** DMA functions ***/

#ifdef UHFM
/*!******************************************************************
 * \fn void DMA1_init_channel3(void)
 * \brief Init channel 3 of DMA1 channel peripheral for S2LP FIFO data transfer.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_init_channel3(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn void DMA1_start_channel3(void)
 * \brief Start DMA1 channel 3.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_start_channel3(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn void DMA1_stop_channel3(void)
 * \brief Stop DMA1 channel 3.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_stop_channel3(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn void DMA1_set_channel3_source_addr(uint32_t source_buffer_addr, uint16_t source_buffer_size)
 * \brief Set DMA1 channel 3 source buffer.
 * \param[in]  	source_buffer_addr: Source buffer address.
 * \param[in] 	source_buffer_size: Source buffer size (number of bytes to transfer).
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_set_channel3_source_addr(uint32_t source_buffer_addr, uint16_t source_buffer_size);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn uint8_t DMA1_get_channel3_status(void)
 * \brief Read DMA1 channel 3 transfer status.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		0 if the transfer is running, 1 if the transfer is complete.
 *******************************************************************/
uint8_t DMA1_get_channel3_status(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn void DMA1_init_channel6(void)
 * \brief Init channel 6 of DMA1 channel peripheral for NEOM8N NMEA frames transfer.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_init_channel6(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn void DMA1_start_channel6(void)
 * \brief Start DMA1 channel 3.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_start_channel6(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn void DMA1_stop_channel6(void)
 * \brief Start DMA1 channel 3.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_stop_channel6(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn void DMA1_set_channel6_dest_addr(uint32_t destination_buffer_addr, uint16_t destination_buffer_size
 * \brief Set DMA1 channel 6 destination buffer.
 * \param[in]  	destination_buffer_addr: Destination buffer address.
 * \param[in] 	destination_buffer_size: Destination buffer size (number of bytes to transfer).
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_set_channel6_dest_addr(uint32_t destination_buffer_addr, uint16_t destination_buffer_size);
#endif

#endif /* __DMA_H__ */
