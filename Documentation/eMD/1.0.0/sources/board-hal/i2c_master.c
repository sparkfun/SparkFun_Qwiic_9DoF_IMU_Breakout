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

#include "i2c_master.h"

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_rcc.h"

/********************************* Globals ************************************/

/********************************* Prototypes *********************************/

static uint8_t IsInited = 0;

void i2c_master_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	if(IsInited != 0)
		return;

	/* Enable I2Cx clock */
	RCC_APB1PeriphClockCmd(SENSORS_I2C_RCC_CLK, ENABLE);

	/* Enable I2C GPIO clock */
	RCC_AHB1PeriphClockCmd(SENSORS_I2C_SCL_GPIO_CLK | SENSORS_I2C_SDA_GPIO_CLK, ENABLE);

	/* Configure I2Cx pin: SCL ----------------------------------------*/
	GPIO_InitStructure.GPIO_Pin =  SENSORS_I2C_SCL_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	/* Connect pins to Periph */
	GPIO_PinAFConfig(SENSORS_I2C_SCL_GPIO_PORT, SENSORS_I2C_SCL_GPIO_PINSOURCE, SENSORS_I2C_AF_SCL);
	GPIO_Init(SENSORS_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

	/* Configure I2Cx pin: SDA ----------------------------------------*/
	GPIO_InitStructure.GPIO_Pin = SENSORS_I2C_SDA_GPIO_PIN;

	/* Connect pins to Periph */
	GPIO_PinAFConfig(SENSORS_I2C_SDA_GPIO_PORT, SENSORS_I2C_SDA_GPIO_PINSOURCE, SENSORS_I2C_AF_SDA);
	GPIO_Init(SENSORS_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);

	I2C_DeInit(SENSORS_I2C);
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = I2C_OWN_ADDRESS;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;

	/* Enable the I2C peripheral */
	I2C_Cmd(SENSORS_I2C, ENABLE);

	/* Initialize the I2C peripheral */
	I2C_Init(SENSORS_I2C, &I2C_InitStructure);

	IsInited = 1;
}

/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
static uint32_t I2Cx_TIMEOUT_UserCallback(char value)
{
	(void)value;

	/* The following code allows I2C error recovery and return to normal communication
	 if the error source doesn’t still exist (ie. hardware issue..) */
	I2C_InitTypeDef I2C_InitStructure;

	I2C_GenerateSTOP(SENSORS_I2C, ENABLE);

	I2C_SoftwareResetCmd(SENSORS_I2C, ENABLE);
	I2C_SoftwareResetCmd(SENSORS_I2C, DISABLE);

	I2C_DeInit(SENSORS_I2C);
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = I2C_OWN_ADDRESS;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;

	/* Enable the I2C peripheral */
	I2C_Cmd(SENSORS_I2C, ENABLE);

	/* Initialize the I2C peripheral */
	I2C_Init(SENSORS_I2C, &I2C_InitStructure);

	return 1;
}

unsigned long i2c_master_write_register(unsigned char Address, unsigned char RegisterAddr, unsigned short RegisterLen, const unsigned char *RegisterValue)
{
	uint32_t  result = 0;
	uint32_t  i = 0;
	__IO uint32_t  I2CTimeout = I2Cx_LONG_TIMEOUT;

	/* Shift the 7-bit address */
	Address = Address << 1;

	/* Wait for the busy flag to be cleared */
	WAIT_FOR_FLAG (I2C_FLAG_BUSY, RESET, I2Cx_LONG_TIMEOUT, 1);

	/* Start the config sequence */
	I2C_GenerateSTART(SENSORS_I2C, ENABLE);

	/* Wait for the start bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_SB, SET, I2Cx_FLAG_TIMEOUT, 2);

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(SENSORS_I2C, Address, I2C_Direction_Transmitter);

	/* Wait for address bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_ADDR, SET, I2Cx_FLAG_TIMEOUT, 3);

	/* clear the ADDR interrupt bit  - this is done by reading SR1 and SR2*/
	CLEAR_ADDR_BIT

	/* Wait for address bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_TXE, SET, I2Cx_FLAG_TIMEOUT, 4);

	/* Transmit the first address for write operation */
	I2C_SendData(SENSORS_I2C, RegisterAddr);

	for(i=0; i<(RegisterLen); i++)
	{
		/* Wait for address bit to be set */
		WAIT_FOR_FLAG (I2C_FLAG_TXE, SET, I2Cx_FLAG_TIMEOUT, 5);

		/* Prepare the register value to be sent */
		I2C_SendData(SENSORS_I2C, RegisterValue[i]);
	}

	/* Wait for address bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_BTF, SET, I2Cx_FLAG_TIMEOUT, 6);

	/* End the configuration sequence */
	I2C_GenerateSTOP(SENSORS_I2C, ENABLE);

	/* Return the verifying value: 0 (Passed) or 1 (Failed) */
	return result;
}

unsigned long i2c_master_write_raw(unsigned char Address, unsigned short RegisterLen, const unsigned char *RegisterValue)
{
	uint32_t  result = 0;
	uint32_t  i = 0;
	__IO uint32_t  I2CTimeout = I2Cx_LONG_TIMEOUT;

	/* Shift the 7-bit address */
	Address = Address << 1;

	/* Wait for the busy flag to be cleared */
	WAIT_FOR_FLAG (I2C_FLAG_BUSY, RESET, I2Cx_LONG_TIMEOUT, 1);

	/* Start the config sequence */
	I2C_GenerateSTART(SENSORS_I2C, ENABLE);

	/* Wait for the start bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_SB, SET, I2Cx_FLAG_TIMEOUT, 2);

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(SENSORS_I2C, Address, I2C_Direction_Transmitter);

	/* Wait for address bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_ADDR, SET, I2Cx_FLAG_TIMEOUT, 3);

	/* clear the ADDR interrupt bit  - this is done by reading SR1 and SR2*/
	CLEAR_ADDR_BIT

	for(i=0; i<(RegisterLen); i++)
	{
		/* Wait for address bit to be set */
		WAIT_FOR_FLAG (I2C_FLAG_TXE, SET, I2Cx_FLAG_TIMEOUT, 5);

		/* Prepare the register value to be sent */
		I2C_SendData(SENSORS_I2C, RegisterValue[i]);
	}

	/* Wait for address bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_BTF, SET, I2Cx_FLAG_TIMEOUT, 6);

	/* End the configuration sequence */
	I2C_GenerateSTOP(SENSORS_I2C, ENABLE);

	/* Return the verifying value: 0 (Passed) or 1 (Failed) */
	return result;
}

unsigned long i2c_master_read_register(unsigned char Address, unsigned char RegisterAddr, unsigned short RegisterLen, unsigned char *RegisterValue)
{
	uint32_t i=0, result = 0;
	__IO uint32_t  I2CTimeout = I2Cx_LONG_TIMEOUT;

	/* Shift the 7-bit address */
	Address = Address << 1;

	/* Wait for the busy flag to be cleared */
	WAIT_FOR_FLAG (I2C_FLAG_BUSY, RESET, I2Cx_LONG_TIMEOUT, 7);

	/* Start the config sequence */
	I2C_GenerateSTART(SENSORS_I2C, ENABLE);

	/* Wait for the start bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_SB, SET, I2Cx_FLAG_TIMEOUT, 8);

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(SENSORS_I2C, Address, I2C_Direction_Transmitter);

	/* Wait for the start bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_ADDR, SET, I2Cx_FLAG_TIMEOUT, 9);

	/* clear the ADDR interrupt bit  - this is done by reading SR1 and SR2*/
	CLEAR_ADDR_BIT;

	/* Wait for address bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_TXE, SET, I2Cx_FLAG_TIMEOUT, 10);

	/* Transmit the register address to be read */
	I2C_SendData(SENSORS_I2C, RegisterAddr);

	/* Wait for address bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_TXE, SET, I2Cx_FLAG_TIMEOUT, 11);

	/*!< Send START condition a second time */
	I2C_GenerateSTART(SENSORS_I2C, ENABLE);

	/* Wait for the start bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_SB, SET, I2Cx_FLAG_TIMEOUT, 12);

	/*!< Send address for read */
	I2C_Send7bitAddress(SENSORS_I2C, Address, I2C_Direction_Receiver);

	/* Wait for the start bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_ADDR, SET, I2Cx_FLAG_TIMEOUT, 13);

	if (RegisterLen == 1)
	{
		/*!< Disable Acknowledgment */
		I2C_AcknowledgeConfig(SENSORS_I2C, DISABLE);

		/* clear the ADDR interrupt bit  - this is done by reading SR1 and SR2*/
		CLEAR_ADDR_BIT;

		/*!< Send STOP Condition */
		I2C_GenerateSTOP(SENSORS_I2C, ENABLE);

		/* Wait for the RXNE bit to be set */
		WAIT_FOR_FLAG (I2C_FLAG_RXNE, SET, I2Cx_FLAG_TIMEOUT, 14);

		RegisterValue[0] = I2C_ReceiveData(SENSORS_I2C);
	}
	else if( RegisterLen == 2)
	{
		 /*!< Disable Acknowledgment */
		I2C_AcknowledgeConfig(SENSORS_I2C, DISABLE);

		/* Set POS bit */
		SENSORS_I2C->CR1 |= I2C_CR1_POS;

		/* clear the ADDR interrupt bit  - this is done by reading SR1 and SR2*/
		CLEAR_ADDR_BIT;

		/* Wait for the buffer full bit to be set */
		WAIT_FOR_FLAG (I2C_FLAG_BTF, SET, I2Cx_FLAG_TIMEOUT, 15);

		/*!< Send STOP Condition */
		I2C_GenerateSTOP(SENSORS_I2C, ENABLE);

		/* Read 2 bytes */
		RegisterValue[0] = I2C_ReceiveData(SENSORS_I2C);
		RegisterValue[1] = I2C_ReceiveData(SENSORS_I2C);
	}
	else if( RegisterLen == 3)
	{
		CLEAR_ADDR_BIT;

		/* Wait for the buffer full bit to be set */
		WAIT_FOR_FLAG (I2C_FLAG_BTF, SET, I2Cx_FLAG_TIMEOUT, 16);

		/*!< Disable Acknowledgment */
		I2C_AcknowledgeConfig(SENSORS_I2C, DISABLE);

		/* Read 1 bytes */
		RegisterValue[0] = I2C_ReceiveData(SENSORS_I2C);

		/*!< Send STOP Condition */
		I2C_GenerateSTOP(SENSORS_I2C, ENABLE);

		/* Read 1 bytes */
		RegisterValue[1] = I2C_ReceiveData(SENSORS_I2C);

		/* Wait for the buffer full bit to be set */
		WAIT_FOR_FLAG (I2C_FLAG_RXNE, SET, I2Cx_FLAG_TIMEOUT, 17);

		/* Read 1 bytes */
		RegisterValue[2] = I2C_ReceiveData(SENSORS_I2C);
	}
	else /* more than 2 bytes */
	{
		/* clear the ADDR interrupt bit  - this is done by reading SR1 and SR2*/
		CLEAR_ADDR_BIT;

		for(i=0; i<(RegisterLen); i++)
		{
			if(i==((unsigned short)(RegisterLen-3)))
			{
				/* Wait for the buffer full bit to be set */
				WAIT_FOR_FLAG (I2C_FLAG_BTF, SET, I2Cx_FLAG_TIMEOUT, 16);

				/*!< Disable Acknowledgment */
				I2C_AcknowledgeConfig(SENSORS_I2C, DISABLE);

				/* Read 1 bytes */
				RegisterValue[i++] = I2C_ReceiveData(SENSORS_I2C);

				/*!< Send STOP Condition */
				I2C_GenerateSTOP(SENSORS_I2C, ENABLE);

				/* Read 1 bytes */
				RegisterValue[i++] = I2C_ReceiveData(SENSORS_I2C);

				/* Wait for the buffer full bit to be set */
				WAIT_FOR_FLAG (I2C_FLAG_RXNE, SET, I2Cx_FLAG_TIMEOUT, 17);

				/* Read 1 bytes */
				RegisterValue[i++] = I2C_ReceiveData(SENSORS_I2C);
				goto endReadLoop;
			}
			/* Wait for the RXNE bit to be set */
			WAIT_FOR_FLAG (I2C_FLAG_RXNE, SET, I2Cx_FLAG_TIMEOUT, 18);
			RegisterValue[i] = I2C_ReceiveData(SENSORS_I2C);
		}
	}

endReadLoop:
	/* Clear BTF flag */
	I2C_ClearFlag(SENSORS_I2C, I2C_FLAG_BTF);

	/* Wait for the busy flag to be cleared */
	WAIT_FOR_FLAG (I2C_FLAG_BUSY, RESET, I2Cx_LONG_TIMEOUT, 19);

	/*!< Re-Enable Acknowledgment to be ready for another reception */
	I2C_AcknowledgeConfig(SENSORS_I2C, ENABLE);

	/* Disable POS */
	SENSORS_I2C->CR1 &= ~I2C_CR1_POS;

	/* Return the byte read from sensor */
	return result;
}

unsigned long i2c_master_read_raw(unsigned char Address, unsigned short RegisterLen, unsigned char *RegisterValue)
{
	uint32_t i=0, result = 0;
	__IO uint32_t  I2CTimeout = I2Cx_LONG_TIMEOUT;

	/* Shift the 7-bit address */
	Address = Address << 1;

	/* Wait for the busy flag to be cleared */
	WAIT_FOR_FLAG (I2C_FLAG_BUSY, RESET, I2Cx_LONG_TIMEOUT, 7);

	/*!< Send START condition a second time */
	I2C_GenerateSTART(SENSORS_I2C, ENABLE);

	/* Wait for the start bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_SB, SET, I2Cx_FLAG_TIMEOUT, 12);

	/*!< Send address for read */
	I2C_Send7bitAddress(SENSORS_I2C, Address, I2C_Direction_Receiver);

	/* Wait for the start bit to be set */
	WAIT_FOR_FLAG (I2C_FLAG_ADDR, SET, I2Cx_FLAG_TIMEOUT, 13);

	if (RegisterLen == 1)
	{
		/*!< Disable Acknowledgment */
		I2C_AcknowledgeConfig(SENSORS_I2C, DISABLE);

		/* clear the ADDR interrupt bit  - this is done by reading SR1 and SR2*/
		CLEAR_ADDR_BIT;

		/*!< Send STOP Condition */
		I2C_GenerateSTOP(SENSORS_I2C, ENABLE);

		/* Wait for the RXNE bit to be set */
		WAIT_FOR_FLAG (I2C_FLAG_RXNE, SET, I2Cx_FLAG_TIMEOUT, 14);

		RegisterValue[0] = I2C_ReceiveData(SENSORS_I2C);
	}
	else if( RegisterLen == 2)
	{
		 /*!< Disable Acknowledgment */
		I2C_AcknowledgeConfig(SENSORS_I2C, DISABLE);

		/* Set POS bit */
		SENSORS_I2C->CR1 |= I2C_CR1_POS;

		/* clear the ADDR interrupt bit  - this is done by reading SR1 and SR2*/
		CLEAR_ADDR_BIT;

		/* Wait for the buffer full bit to be set */
		WAIT_FOR_FLAG (I2C_FLAG_BTF, SET, I2Cx_FLAG_TIMEOUT, 15);

		/*!< Send STOP Condition */
		I2C_GenerateSTOP(SENSORS_I2C, ENABLE);

		/* Read 2 bytes */
		RegisterValue[0] = I2C_ReceiveData(SENSORS_I2C);
		RegisterValue[1] = I2C_ReceiveData(SENSORS_I2C);
	}
	else if( RegisterLen == 3)
	{
		CLEAR_ADDR_BIT;

		/* Wait for the buffer full bit to be set */
		WAIT_FOR_FLAG (I2C_FLAG_BTF, SET, I2Cx_FLAG_TIMEOUT, 16);

		/*!< Disable Acknowledgment */
		I2C_AcknowledgeConfig(SENSORS_I2C, DISABLE);

		/* Read 1 bytes */
		RegisterValue[0] = I2C_ReceiveData(SENSORS_I2C);

		/*!< Send STOP Condition */
		I2C_GenerateSTOP(SENSORS_I2C, ENABLE);

		/* Read 1 bytes */
		RegisterValue[1] = I2C_ReceiveData(SENSORS_I2C);

		/* Wait for the buffer full bit to be set */
		WAIT_FOR_FLAG (I2C_FLAG_RXNE, SET, I2Cx_FLAG_TIMEOUT, 17);

		/* Read 1 bytes */
		RegisterValue[2] = I2C_ReceiveData(SENSORS_I2C);
	}
	else /* more than 2 bytes */
	{
		/* clear the ADDR interrupt bit  - this is done by reading SR1 and SR2*/
		CLEAR_ADDR_BIT;

		for(i=0; i<(RegisterLen); i++)
		{
			if(i==((unsigned short)(RegisterLen-3)))
			{
				/* Wait for the buffer full bit to be set */
				WAIT_FOR_FLAG (I2C_FLAG_BTF, SET, I2Cx_FLAG_TIMEOUT, 16);

				/*!< Disable Acknowledgment */
				I2C_AcknowledgeConfig(SENSORS_I2C, DISABLE);

				/* Read 1 bytes */
				RegisterValue[i++] = I2C_ReceiveData(SENSORS_I2C);

				/*!< Send STOP Condition */
				I2C_GenerateSTOP(SENSORS_I2C, ENABLE);

				/* Read 1 bytes */
				RegisterValue[i++] = I2C_ReceiveData(SENSORS_I2C);

				/* Wait for the buffer full bit to be set */
				WAIT_FOR_FLAG (I2C_FLAG_RXNE, SET, I2Cx_FLAG_TIMEOUT, 17);

				/* Read 1 bytes */
				RegisterValue[i++] = I2C_ReceiveData(SENSORS_I2C);
				goto endReadLoop;
			}
			/* Wait for the RXNE bit to be set */
			WAIT_FOR_FLAG (I2C_FLAG_RXNE, SET, I2Cx_FLAG_TIMEOUT, 18);
			RegisterValue[i] = I2C_ReceiveData(SENSORS_I2C);
		}
	}

endReadLoop:
	/* Clear BTF flag */
	I2C_ClearFlag(SENSORS_I2C, I2C_FLAG_BTF);

	/* Wait for the busy flag to be cleared */
	WAIT_FOR_FLAG (I2C_FLAG_BUSY, RESET, I2Cx_LONG_TIMEOUT, 19);

	/*!< Re-Enable Acknowledgment to be ready for another reception */
	I2C_AcknowledgeConfig(SENSORS_I2C, ENABLE);

	/* Disable POS */
	SENSORS_I2C->CR1 &= ~I2C_CR1_POS;

	/* Return the byte read from sensor */
	return result;
}

void i2c_master_deinit(void)
{
	/* Disable the I2C peripheral */
	I2C_Cmd(SENSORS_I2C, DISABLE);
	IsInited = 0;
}
