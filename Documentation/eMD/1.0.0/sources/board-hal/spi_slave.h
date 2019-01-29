/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2016-2016 InvenSense Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively “Software”) is subject
 * to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */

/** @defgroup SPI_Slave SPI_Slave
	@ingroup  Low_Level_Driver
	@{
*/
#ifndef __SPI_SLAVE_H__
#define __SPI_SLAVE_H__

#include <stdint.h>

/** @brief Configures SPI slave peripheral
 *  @param[in]	end_of_tx_cb    callback to call on end of TX transfer interrupt
*/
void spi_slave_init(void(*end_of_tx_cb)(void));

/** @brief Desactivates the SPI slave peripheral
*/
void spi_slave_deinit(void);

/** @brief Register a callback called at SPI slave end of frame event
 *  @param[in]	end_of_frame_cb    callback to call on end of frame transfer
*/
void spi_slave_set_end_of_frame_callback(void (*end_of_frame_cb)(void));

/** @brief Receive a byte through SPI
*  @param[out]	value    read value pointer
*/
void spi_slave_rx(uint8_t *value);

/** @brief Send a byte through SPI
*  @param[in]	value    pointer on value to transfer
*/
void spi_slave_tx(uint8_t *value);

#endif /* __SPI_SLAVE_H__ */

/** @} */
