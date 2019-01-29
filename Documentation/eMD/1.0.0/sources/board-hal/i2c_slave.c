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
#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h" 
#include "stm32f4xx_i2c.h" 
#include "stm32f4xx_rcc.h"

#include "i2c_master.h"
#include "i2c_slave.h"

#include "nvic_config.h"

/********************************* Prototypes *********************************/
                               
/********************************* Globals ************************************/

static i2c_slave_data   i2c_data[NUM_I2C_BUFFERS];
static uint8_t  i2c_cur_data = 0;
static uint8_t  i2c_read_next = 0;
static uint8_t IsInited = 0;

static void (*I2C_interrupt_cb)(void) = (void *)0;              

unsigned long i2c_slave_read_register(unsigned char RegisterAddr, unsigned short RegisterLen, unsigned char *RegisterValue)
{
	uint8_t packet_len = 0;

	if (i2c_data[i2c_read_next].packet_available)
	{
		packet_len = (RegisterLen < i2c_data[i2c_read_next].data_index) ? RegisterLen : i2c_data[i2c_read_next].data_index;
		memcpy(&RegisterValue[1], i2c_data[i2c_read_next].data, packet_len);        
		i2c_data[i2c_read_next].data_index = 0;
		i2c_data[i2c_read_next].packet_available = 0;
		i2c_read_next = (++i2c_read_next == NUM_I2C_BUFFERS) ? 0 : i2c_read_next;
	}
	RegisterValue[0] = packet_len;             // Because we return the number of bytes written in the first byte of the buffer.            
	return 0;
}

void i2c_slave_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef i2c_init;
	NVIC_InitTypeDef NVIC_InitStructure, NVIC_InitStructure2;
	uint8_t i;

	if(IsInited != 0)
		return;

	for (i = 0; i < NUM_I2C_BUFFERS; i++)
	{
		i2c_data[i].data_index = 0;
		i2c_data[i].packet_available = 0;
	}
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
	i2c_init.I2C_Mode = I2C_Mode_I2C;
	i2c_init.I2C_DutyCycle = I2C_DutyCycle_2;
	i2c_init.I2C_OwnAddress1 = (I2C_SLAVE_ADDRESS << 1);
	i2c_init.I2C_Ack = I2C_Ack_Enable;
	i2c_init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	i2c_init.I2C_ClockSpeed = I2C_SPEED;
	I2C_Init(SENSORS_I2C, &i2c_init);
	I2C_StretchClockCmd(SENSORS_I2C, DISABLE);

	NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_I2C_SLAVE;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure2.NVIC_IRQChannel = I2C1_ER_IRQn;
	NVIC_InitStructure2.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_I2C_SLAVE;
	NVIC_InitStructure2.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure2.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure2);   

	I2C_ITConfig(SENSORS_I2C, I2C_IT_EVT | I2C_IT_ERR | I2C_IT_BUF, ENABLE);
	I2C_Cmd(SENSORS_I2C, ENABLE);

	IsInited = 1;
}


void i2c_slave_deinit(void)
{
	/* Disable the I2C peripheral */
	I2C_Cmd(SENSORS_I2C, DISABLE);  
	IsInited = 0;
}

void I2C1_ER_IRQHandler(void)
{
	volatile uint32_t SR1Register, SR2Register;
	/* Read the I2C1 status register */
	SR1Register = SENSORS_I2C->SR1;

	/* If AF or BERR, send STOP*/
	if(SR1Register & 0x0500)
		I2C_GenerateSTOP(SENSORS_I2C,ENABLE);//program the Stop
	/* If AF, BERR or ARLO, abandon the current job and send a start to commence new */
	if(SR1Register & 0x0700) 
		SR2Register = SENSORS_I2C->SR2;//read second status register to clear ADDR if it is set (note that BTF will not be set after a NACK)
	SENSORS_I2C->SR1 &=~0x0F00;		//reset all the error bits to clear the interrupt
}

void i2c_slave_irq_init(void (*interrupt_cb)(void))
{
	I2C_interrupt_cb = interrupt_cb;
}

void I2C1_EV_IRQHandler(void)
{
	static int receiving_i2c_data = 0;

	switch(I2C_GetLastEvent(I2C1))
	{
	case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED :
		I2C_ClearFlag(SENSORS_I2C,I2C_FLAG_ADDR);
		receiving_i2c_data = 1;                                 // We're receiving data.
		i2c_data[i2c_cur_data].data_index = 0;                  // Make sure we start with the index equal to 0. This cleans up some spurious data from Geneva
		break;
	case I2C_EVENT_SLAVE_BYTE_RECEIVED:
		if (i2c_data[i2c_cur_data].data_index < MAX_I2C_SLAVE_BUF)
		  i2c_data[i2c_cur_data].data[i2c_data[i2c_cur_data].data_index++] = I2C_ReceiveData(SENSORS_I2C);
		break;
	case I2C_EVENT_SLAVE_STOP_DETECTED:
		if(I2C_GetFlagStatus(SENSORS_I2C,I2C_FLAG_ADDR) == SET)
			I2C_ClearFlag(SENSORS_I2C,I2C_FLAG_ADDR);
		if(I2C_GetFlagStatus(SENSORS_I2C,I2C_FLAG_STOPF) == SET)
			I2C_ClearFlag(SENSORS_I2C,I2C_FLAG_STOPF);
		if (receiving_i2c_data)                                 // If the stop was generated after receiving data
		{
			i2c_data[i2c_cur_data].packet_available = 1;
			i2c_cur_data = (++i2c_cur_data == NUM_I2C_BUFFERS) ? 0 : i2c_cur_data;
			if (I2C_interrupt_cb)
				I2C_interrupt_cb();
		}
		break;
	case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:
		I2C_ClearFlag(SENSORS_I2C,I2C_FLAG_ADDR);
		receiving_i2c_data = 0;                                 // We're transmitting data, i.e. not receiving.
		break;
	case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:              // This is not normally a transmitter, but because of the I2C HW bug
		I2C_SendData(SENSORS_I2C,0x00);                        // in Geneva, the master read after it writes, and so this slave
		I2C_GenerateSTOP(SENSORS_I2C, ENABLE);
		break;                                          // has to handle read requests by sending dummy data.
	case I2C_EVENT_SLAVE_ACK_FAILURE:
		SENSORS_I2C->SR1 &= 0x00FF;
		break;
	default:
		break;
	}

	while ((SENSORS_I2C->SR1 & I2C_SR1_ADDR) == I2C_SR1_ADDR)
	{
		volatile uint32_t temp;

		temp=SENSORS_I2C->SR1;
		temp=SENSORS_I2C->SR2;
	}
	while ((SENSORS_I2C->SR1&I2C_SR1_STOPF) == I2C_SR1_STOPF)
	{
		volatile uint32_t temp;

		temp=SENSORS_I2C->SR1;
		SENSORS_I2C->CR1 |= 0x1;
	}
	I2C_ClearITPendingBit(SENSORS_I2C,I2C_IT_SMBALERT | I2C_IT_TIMEOUT | I2C_IT_PECERR | I2C_IT_OVR |
			I2C_IT_AF | I2C_IT_ARLO | I2C_IT_BERR);
}