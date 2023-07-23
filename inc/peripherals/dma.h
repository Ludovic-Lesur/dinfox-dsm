/*
 * dma.h
 *
 *  Created on: 16 aug. 2020
 *      Author: Ludo
 */

#ifndef __DMA_H__
#define __DMA_H__

#include "types.h"

/*** DMA structures ***/

/*!******************************************************************
 * \fn DMA_transfer_complete_irq_cb
 * \brief DMA transfer complete interrupt callback.
 *******************************************************************/
typedef void (*DMA_transfer_complete_irq_cb)(uint8_t line_end_flag);

/*** DMA functions ***/

#ifdef UHFM
/*!******************************************************************
 * \fn void DMA1_CH3_init(void)
 * \brief Init channel 3 of DMA1 channel peripheral for S2LP FIFO data transfer.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_CH3_init(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn void DMA1_CH3_de_init(void)
 * \brief Release channel 3 of DMA1.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_CH3_de_init(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn void DMA1_CH3_start(void)
 * \brief Start DMA1 channel 3.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_CH3_start(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn void DMA1_CH3_stop(void)
 * \brief Stop DMA1 channel 3.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_CH3_stop(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn void DMA1_CH3_set_source_address(uint32_t source_buffer_addr, uint16_t source_buffer_size)
 * \brief Set DMA1 channel 3 source buffer.
 * \param[in]  	source_buffer_addr: Source buffer address.
 * \param[in] 	source_buffer_size: Source buffer size (number of bytes to transfer).
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_CH3_set_source_address(uint32_t source_buffer_addr, uint16_t source_buffer_size);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn uint8_t DMA1_CH3_get_transfer_status(void)
 * \brief Read DMA1 channel 3 transfer status.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		0 if the transfer is running, 1 if the transfer is complete.
 *******************************************************************/
uint8_t DMA1_CH3_get_transfer_status(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn void DMA1_CH6_init(void)
 * \brief Init channel 6 of DMA1 channel peripheral for NEOM8N NMEA frames transfer.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_CH6_init(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn void DMA1_CH6_set_transfer_complete_callback(void)
 * \brief Set DMA1 channel 6 transfer complete callback.
 * \param[in]  	irq_callback: Function to call on interrupt.
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_CH6_set_transfer_complete_callback(DMA_transfer_complete_irq_cb irq_callback);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn void DMA1_CH6_start(void)
 * \brief Start DMA1 channel 3.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_CH6_start(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn void DMA1_CH6_stop(void)
 * \brief Start DMA1 channel 3.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_CH6_stop(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn void DMA1_CH6_set_destination_address(uint32_t destination_buffer_addr, uint16_t destination_buffer_size
 * \brief Set DMA1 channel 6 destination buffer.
 * \param[in]  	destination_buffer_addr: Destination buffer address.
 * \param[in] 	destination_buffer_size: Destination buffer size (number of bytes to transfer).
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DMA1_CH6_set_destination_address(uint32_t destination_buffer_addr, uint16_t destination_buffer_size);
#endif

#endif /* __DMA_H__ */
