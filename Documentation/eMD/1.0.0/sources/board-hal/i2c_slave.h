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


/** @defgroup I2C slave peripheral
    @ingroup  Low_Level_Driver
    @{
*/
#ifndef __I2C_SLAVE_H__
#define __I2C_SLAVE_H__

#define I2C_SLAVE_ADDRESS     0x3B

#define MAX_I2C_SLAVE_BUF     64
#define NUM_I2C_BUFFERS 32

typedef struct i2c_slave_struct
{
	uint8_t       data[MAX_I2C_SLAVE_BUF];
	uint8_t       data_index;
	uint8_t       packet_available;
} i2c_slave_data;

/** @brief Configures I2C slave peripheral
 */
void i2c_slave_init(void);

/** @brief Desactivates the I2C master peripheral
 */
void i2c_slave_deinit(void);

/** @brief Read a register through the slave interface I2C 
 *  @param[in] register_addr, register address (location) to access
 *  @param[in] register_len, length value to read
 *  @param[in] register_value, pointer on byte value to read
 *  @retval 0 if correct communication, else wrong communication
 */
unsigned long i2c_slave_read_register(unsigned char register_addr, unsigned short register_len, unsigned char *register_value);

void i2c_slave_irq_init(void (*interrupt_cb)(void));

#endif /* __I2C_SLAVE_H__ */


