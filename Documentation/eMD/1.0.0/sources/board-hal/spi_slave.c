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
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITHé& THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */

#include "spi_slave.h"
#include "gpio.h"

#include "nvic_config.h"

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h" 
#include "stm32f4xx_spi.h" 
#include "stm32f4xx_rcc.h"

#include <stddef.h>

/********************************* Defines ************************************/

#define SPI_SLAVE_GPIO_PORT              GPIOB
#define SPI_SLAVE_GPIO_CLK               RCC_AHB1Periph_GPIOB

#define SPI_SLAVE_SCK_GPIO_PIN           GPIO_Pin_13
#define SPI_SLAVE_SCK_GPIO_PINSOURCE     GPIO_PinSource13
#define SPI_SLAVE_MISO_GPIO_PIN          GPIO_Pin_14
#define SPI_SLAVE_MISO_GPIO_PINSOURCE    GPIO_PinSource14
#define SPI_SLAVE_MOSI_GPIO_PIN          GPIO_Pin_15
#define SPI_SLAVE_MOSI_GPIO_PINSOURCE    GPIO_PinSource15

#define SPI_SLAVE                        SPI2
#define GPIO_AF_SPI                      GPIO_AF_SPI2
#define SPI_SLAVE_RCC_CLK                RCC_APB1Periph_SPI2
        

/********************************* Globals ************************************/
#include "Invn/EmbUtils/RingBuffer.h"
static RINGBUFFER(sSPIBufferReceived, 2048, uint8_t);
static RINGBUFFER(sSPIBufferTransmit, 2048, uint8_t);
static uint8_t IsInited = 0;

/*
 * Callback for End of TX transfer interruption
 */
static void (*sEnd_of_TX_cb)(void);

/********************************* Prototypes *********************************/

static void spi_slave_init_peripheral(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	if(IsInited != 0)
		return;

	/* Enable SPIx clock */
	RCC_APB1PeriphClockCmd(SPI_SLAVE_RCC_CLK, ENABLE);

	/* Enable SPI GPIO clock */
	RCC_AHB1PeriphClockCmd(SPI_SLAVE_GPIO_CLK, ENABLE);

	/* Configure SPIx pin: SCK, MOSI and MISO -------------------------*/
	GPIO_InitStructure.GPIO_Pin =  SPI_SLAVE_SCK_GPIO_PIN | SPI_SLAVE_MISO_GPIO_PIN | SPI_SLAVE_MOSI_GPIO_PIN; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(SPI_SLAVE_GPIO_PORT, &GPIO_InitStructure);

	/* Connect SPIx pins to AF5 */
	GPIO_PinAFConfig(SPI_SLAVE_GPIO_PORT, SPI_SLAVE_SCK_GPIO_PINSOURCE, GPIO_AF_SPI);  
	GPIO_PinAFConfig(SPI_SLAVE_GPIO_PORT, SPI_SLAVE_MISO_GPIO_PINSOURCE, GPIO_AF_SPI);  
	GPIO_PinAFConfig(SPI_SLAVE_GPIO_PORT, SPI_SLAVE_MOSI_GPIO_PINSOURCE, GPIO_AF_SPI);  

	/* Initialize the SPI peripheral */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_Init(SPI_SLAVE, &SPI_InitStructure);

	IsInited = 1;
}

void spi_slave_set_end_of_frame_callback(void (*end_of_frame_cb)(void))
{
	/* Configure the CS line to detect the end of the frame transferred */
	gpio_init_spi_slave_cs_as_int(end_of_frame_cb);
}

void spi_slave_init(void(*end_of_tx_cb)(void))
{
	NVIC_InitTypeDef  NVIC_InitStructure;

	/* Set callback to be called after each end of TX transfer */
	sEnd_of_TX_cb = end_of_tx_cb;

	spi_slave_init_peripheral();

	/* Configure NVIC interrupt to SPI slave */
	NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_SPI_SLAVE;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the Rx buffer not empty interrupt */
	SPI_I2S_ITConfig(SPI_SLAVE, SPI_I2S_IT_RXNE, ENABLE);

	/* Enable the SPI peripheral */
	SPI_Cmd(SPI_SLAVE, ENABLE);  
}

void spi_slave_deinit(void)
{
	SPI_Cmd(SPI_SLAVE, DISABLE);
	IsInited = 0;
}

void spi_slave_rx(uint8_t *value)
{
	if (!RINGBUFFER_EMPTY(&sSPIBufferReceived))
	{
		RINGBUFFER_POP(&sSPIBufferReceived, value);
	}
}

void spi_slave_tx(uint8_t *value)
{
	if (!RINGBUFFER_FULL(&sSPIBufferTransmit))
	{
		RINGBUFFER_PUSH(&sSPIBufferTransmit, value);
	}
}

/* Interrupt management ------------------------------------------------------*/

void SPI2_IRQHandler(void)
{
	if (SPI_I2S_GetITStatus(SPI_SLAVE, SPI_I2S_IT_RXNE) == SET)
	{
		if (!RINGBUFFER_FULL(&sSPIBufferReceived))
		{
			uint8_t lRxByte = SPI_I2S_ReceiveData(SPI_SLAVE);
			RINGBUFFER_PUSH(&sSPIBufferReceived, &lRxByte);

			if(sEnd_of_TX_cb != NULL)
				sEnd_of_TX_cb();
		}

		if ((SPI_I2S_GetFlagStatus(SPI_SLAVE, SPI_I2S_FLAG_BSY) == RESET)
            && (!RINGBUFFER_EMPTY(&sSPIBufferTransmit)))
		{
			uint8_t lTxByte;
			RINGBUFFER_POP(&sSPIBufferTransmit, &lTxByte);
			SPI_I2S_SendData(SPI_SLAVE, lTxByte);
		}
	}
}

