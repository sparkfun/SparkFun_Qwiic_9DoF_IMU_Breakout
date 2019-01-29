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

/********************************* Defines ************************************/

#define I2C_SPEED                     400000
#define I2C_OWN_ADDRESS               0x00

#define I2Cx_FLAG_TIMEOUT             ((uint32_t) 900) //0x1100
#define I2Cx_LONG_TIMEOUT             ((uint32_t) (300 * I2Cx_FLAG_TIMEOUT))

#define SENSORS_I2C_SCL_GPIO_PORT         GPIOB
#define SENSORS_I2C_SCL_GPIO_CLK          RCC_AHB1Periph_GPIOB
#define SENSORS_I2C_SCL_GPIO_PIN          GPIO_Pin_8
#define SENSORS_I2C_SCL_GPIO_PINSOURCE    GPIO_PinSource8

#define SENSORS_I2C_SDA_GPIO_PORT         GPIOB
#define SENSORS_I2C_SDA_GPIO_CLK          RCC_AHB1Periph_GPIOB
#define SENSORS_I2C_SDA_GPIO_PIN          GPIO_Pin_9
#define SENSORS_I2C_SDA_GPIO_PINSOURCE    GPIO_PinSource9

#define SENSORS_I2C                       I2C1
#define SENSORS_I2C_AF_SCL                GPIO_AF_I2C1
#define SENSORS_I2C_AF_SDA                GPIO_AF_I2C1
#define SENSORS_I2C_RCC_CLK               RCC_APB1Periph_I2C1

#define WAIT_FOR_FLAG(flag, value, timeout, errorcode)  I2CTimeout = timeout;\
          while(I2C_GetFlagStatus(SENSORS_I2C, flag) != value) {\
            if((I2CTimeout--) == 0) return I2Cx_TIMEOUT_UserCallback(errorcode); \
          }\

#define CLEAR_ADDR_BIT      I2C_ReadRegister(SENSORS_I2C, I2C_Register_SR1);\
                            I2C_ReadRegister(SENSORS_I2C, I2C_Register_SR2);\


/** @defgroup I2C_Master I2C_Master
	@ingroup Low_Level_Driver
	@{
*/
#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

/** @brief Configures I2C master peripheral
*/
void i2c_master_init(void);

/** @brief Desactivates the I2C master peripheral
*/
void i2c_master_deinit(void);

/** @brief Read a register through the control interface I2C
* @param[in] address, I2c 7bit-address
* @param[in] register_addr, register address (location) to access
* @param[in] register_len, length value to read
* @param[in] register_value, pointer on byte value to read
* @retval 0 if correct communication, else wrong communication
*/
unsigned long i2c_master_read_register(unsigned char address, unsigned char register_addr,
                                          unsigned short register_len, unsigned char *register_value);

/** @brief Write a register through the control interface I2C
* @param[in] address, I2c 7bit-address
* @param[in] register_addr, register address (location) to access
* @param[in] register_len, length value to write
* @param[in] register_value, pointer on byte value to write
* @retval 0 if correct communication, else wrong communication
*/
unsigned long i2c_master_write_register(unsigned char address, unsigned char register_addr,
                                           unsigned short register_len, const unsigned char *register_value);

/** @brief Read data through the control interface I2C
* @param[in] Address, I2c 7bit-address
* @param[in] RegisterLen, length value to read
* @param[in] RegisterValue, pointer on byte value to read
* @retval 0 if correct communication, else wrong communication
*/
unsigned long i2c_master_read_raw(unsigned char Address, unsigned short RegisterLen, unsigned char *RegisterValue);

/** @brief Write a command through the control interface I2C
* @param[in] Address, I2c 7bit-address
* @param[in] RegisterLen, length value to write
* @param[in] RegisterValue, pointer on byte value to write
* @retval 0 if correct communication, else wrong communication
*/
unsigned long i2c_master_write_raw(unsigned char Address, unsigned short RegisterLen, const unsigned char *RegisterValue);

#endif /* __I2C_MASTER_H__ */

/** @} */
