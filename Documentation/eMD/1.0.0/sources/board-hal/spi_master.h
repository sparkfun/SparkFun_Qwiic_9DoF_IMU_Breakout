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

/** @defgroup SPI_master SPI_master
	@ingroup  Low_Level_Driver
	@{
*/
#ifndef __SPI_MASTER_H__
#define __SPI_MASTER_H__

typedef enum spi_num {
	SPI_NUM1,
	SPI_NUM2,
}spi_num_t;

typedef enum spi_speed {
	SPI_25MHZ,
	SPI_12MHZ,
	SPI_6MHZ,
	SPI_3MHZ,
	SPI_1562KHZ,
	SPI_781KHZ,
	SPI_390KHZ
}spi_speed_t;

/** @brief Configures SPI master peripheral
* @param[in] spinum, required spi line number
* @param[in] speed, required spi speed
*/
void spi_master_init(spi_num_t spinum, spi_speed_t speed);

/** @brief Desactivates the SPI master peripheral
* @param[in] spinum, required spi line number
*/
void spi_master_deinit(spi_num_t spinum);

/** @brief Read a register through the control interface SPI 
* @param[in] spinum, required spi line number
* @param[in] register_addr, register address (location) to access
* @param[in] register_len, length value to read
* @param[in] register_value, pointer on byte value to read
* @retval 0 if correct communication, else wrong communication
*/
unsigned long spi_master_read_register(spi_num_t spinum, unsigned char register_addr, 
                                          unsigned short register_len, unsigned char *register_value);

/** @brief Write a register through the control interface SPI  
* @param[in] spinum, required spi line number
* @param[in] register_addr, register address (location) to access
* @param[in] register_len, length value to write
* @param[in] register_value, pointer on byte value to write
* @retval 0 if correct communication, else wrong communication
*/
unsigned long spi_master_write_register(spi_num_t spinum, unsigned char register_addr, 
                                           unsigned short register_len, const unsigned char *register_value);

#endif /* __SPI_MASTER_H__ */

/** @} */
