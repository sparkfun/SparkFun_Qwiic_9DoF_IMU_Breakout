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

#include "spi_master.h"

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h" 
#include "stm32f4xx_spi.h" 
#include "stm32f4xx_rcc.h"

/********************************* Defines ************************************/

// SPI1 defines
#define SENSORS_SPI_GPIO_PORT              GPIOA
#define SENSORS_SPI_GPIO_CLK               RCC_AHB1Periph_GPIOA

#define SENSORS_SPI_SCK_GPIO_PIN           GPIO_Pin_5
#define SENSORS_SPI_SCK_GPIO_PINSOURCE     GPIO_PinSource5
#define SENSORS_SPI_MISO_GPIO_PIN          GPIO_Pin_6
#define SENSORS_SPI_MISO_GPIO_PINSOURCE    GPIO_PinSource6
#define SENSORS_SPI_MOSI_GPIO_PIN          GPIO_Pin_7
#define SENSORS_SPI_MOSI_GPIO_PINSOURCE    GPIO_PinSource7

#define SENSORS_CS_GPIO_PORT               GPIOB
#define SENSORS_CS_GPIO_CLK                RCC_AHB1Periph_GPIOB
#define SENSORS_CS_GPIO_PIN                GPIO_Pin_6

#define SENSORS_SPI                        SPI1
#define SENSORS_GPIO_AF_SPI                GPIO_AF_SPI1
#define SENSORS_SPI_RCC_CLK                RCC_APB2Periph_SPI1

//SPI2 defines
#define SENSORS_SPI2_GPIO_PORT             GPIOB
#define SENSORS_SPI2_GPIO_CLK              RCC_AHB1Periph_GPIOB

#define SENSORS_SPI2_SCK_GPIO_PIN          GPIO_Pin_13
#define SENSORS_SPI2_SCK_GPIO_PINSOURCE    GPIO_PinSource13
#define SENSORS_SPI2_MISO_GPIO_PIN         GPIO_Pin_14
#define SENSORS_SPI2_MISO_GPIO_PINSOURCE   GPIO_PinSource14
#define SENSORS_SPI2_MOSI_GPIO_PIN         GPIO_Pin_15
#define SENSORS_SPI2_MOSI_GPIO_PINSOURCE   GPIO_PinSource15

#define SENSORS_CS2_GPIO_PORT              GPIOB
#define SENSORS_CS2_GPIO_CLK               RCC_AHB1Periph_GPIOB
#define SENSORS_CS2_GPIO_PIN               GPIO_Pin_12

#define SENSORS_SPI2                       SPI2
#define SENSORS_GPIO_AF_SPI2               GPIO_AF_SPI2
#define SENSORS_SPI2_RCC_CLK               RCC_APB1Periph_SPI2

// bit mask to notify a SPI read register
#define READ_BIT_MASK                      0x80

#define SPIx_FLAG_TIMEOUT                 ((uint32_t) 900)
#define SPIx_LONG_TIMEOUT                 ((uint32_t) (300 * SPIx_FLAG_TIMEOUT))

#define WAIT_FOR_FLAG(SPI, flag, timeout, errorcode) SPITimeout = timeout;\
            while(!SPI_I2S_GetFlagStatus(SPI, flag))  {\
              if((SPITimeout--) == 0) return SPIx_TIMEOUT_UserCallback(errorcode); \
            }\

/********************************* Globals ************************************/
static uint8_t IsInitedSpi1 = 0;
static uint8_t IsInitedSpi2 = 0;

/********************************* Private Prototypes **************************/

static void spi_master_init_gpio_spi1(GPIO_InitTypeDef* GPIO_InitStructure)
{
	//Check if we already inited it
	if(IsInitedSpi1 != 0)
		return;
	/* Enable SPIx clock */
	RCC_APB2PeriphClockCmd(SENSORS_SPI_RCC_CLK, ENABLE);

	/* Enable SPI GPIO clock */
	RCC_AHB1PeriphClockCmd(SENSORS_SPI_GPIO_CLK | SENSORS_CS_GPIO_CLK, ENABLE);

	/* Configure SPIx pin: SCK, MOSI and MISO -------------------------*/
	GPIO_InitStructure->GPIO_Pin =  SENSORS_SPI_SCK_GPIO_PIN | SENSORS_SPI_MISO_GPIO_PIN | SENSORS_SPI_MOSI_GPIO_PIN; 
	GPIO_InitStructure->GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure->GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure->GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure->GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(SENSORS_SPI_GPIO_PORT, GPIO_InitStructure);

	/* Connect SPIx pins to AF4 */
	GPIO_PinAFConfig(SENSORS_SPI_GPIO_PORT, SENSORS_SPI_SCK_GPIO_PINSOURCE, SENSORS_GPIO_AF_SPI);  
	GPIO_PinAFConfig(SENSORS_SPI_GPIO_PORT, SENSORS_SPI_MISO_GPIO_PINSOURCE, SENSORS_GPIO_AF_SPI);  
	GPIO_PinAFConfig(SENSORS_SPI_GPIO_PORT, SENSORS_SPI_MOSI_GPIO_PINSOURCE, SENSORS_GPIO_AF_SPI);  

	IsInitedSpi1 = 1;
}

static void spi_master_init_gpio_spi2(GPIO_InitTypeDef* GPIO_InitStructure)
{
	//Check if we already inited it
	if(IsInitedSpi2 != 0)
		return;

	/* Enable SPIx clock */
	RCC_APB1PeriphClockCmd(SENSORS_SPI2_RCC_CLK, ENABLE);

	/* Enable SPI GPIO clock */
	RCC_AHB1PeriphClockCmd(SENSORS_SPI2_GPIO_CLK | SENSORS_CS2_GPIO_CLK, ENABLE);

	/* Configure SPIx pin: SCK, MOSI and MISO -------------------------*/
	GPIO_InitStructure->GPIO_Pin =  SENSORS_SPI2_SCK_GPIO_PIN | SENSORS_SPI2_MISO_GPIO_PIN | SENSORS_SPI2_MOSI_GPIO_PIN; 
	GPIO_InitStructure->GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure->GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure->GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure->GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(SENSORS_SPI2_GPIO_PORT, GPIO_InitStructure);

	/* Connect SPIx pins to AF4 */
	GPIO_PinAFConfig(SENSORS_SPI2_GPIO_PORT, SENSORS_SPI2_SCK_GPIO_PINSOURCE, SENSORS_GPIO_AF_SPI2);  
	GPIO_PinAFConfig(SENSORS_SPI2_GPIO_PORT, SENSORS_SPI2_MISO_GPIO_PINSOURCE, SENSORS_GPIO_AF_SPI2);  
	GPIO_PinAFConfig(SENSORS_SPI2_GPIO_PORT, SENSORS_SPI2_MOSI_GPIO_PINSOURCE, SENSORS_GPIO_AF_SPI2);  

	IsInitedSpi2 = 1;
}

static SPI_TypeDef* spi_master_get_instance(spi_num_t spinum)
{
	//set SPI type according to required line
	if(spinum == SPI_NUM1)
		return SENSORS_SPI;
	else if(spinum == SPI_NUM2)
		return SENSORS_SPI2;
	else
		return 0;
}

static GPIO_TypeDef* spi_master_get_cs_port(SPI_TypeDef* spi)
{
	if(spi == SENSORS_SPI)
		return SENSORS_CS_GPIO_PORT;
	else if(spi == SENSORS_SPI2)
		return SENSORS_CS2_GPIO_PORT;
	else
		return 0;
}

static uint16_t spi_master_get_cs_pin(SPI_TypeDef* spi)
{
	if(spi == SENSORS_SPI)
		return SENSORS_CS_GPIO_PIN;
	else if(spi == SENSORS_SPI2)
		return SENSORS_CS2_GPIO_PIN;
	else
		return 0;
}

static uint16_t spi_master_get_prescaler_spi1(spi_speed_t speed)
{
	switch(speed)
	{
	case SPI_25MHZ:
		return SPI_BaudRatePrescaler_4;
	case SPI_12MHZ:
		return SPI_BaudRatePrescaler_8;
	case SPI_6MHZ:
		return SPI_BaudRatePrescaler_16;
	case SPI_3MHZ:
		return SPI_BaudRatePrescaler_32;
	case SPI_1562KHZ:
		return SPI_BaudRatePrescaler_64;
	case SPI_781KHZ:
		return SPI_BaudRatePrescaler_128;
	case SPI_390KHZ:
		return SPI_BaudRatePrescaler_256;
	}
	return 0;
}

static uint16_t spi_master_get_prescaler_spi2(spi_speed_t speed)
{
	switch(speed)
	{
	case SPI_25MHZ:
		return SPI_BaudRatePrescaler_2;
	case SPI_12MHZ:
		return SPI_BaudRatePrescaler_4;
	case SPI_6MHZ:
		return SPI_BaudRatePrescaler_8;
	case SPI_3MHZ:
		return SPI_BaudRatePrescaler_16;
	case SPI_1562KHZ:
		return SPI_BaudRatePrescaler_32;
	case SPI_781KHZ:
		return SPI_BaudRatePrescaler_64;
	case SPI_390KHZ:
		return SPI_BaudRatePrescaler_128;
	}
	return 0;
}

static uint16_t spi_master_get_prescaler(SPI_TypeDef* spi, spi_speed_t speed)
{
	// According to speed and spi num, we select which prescaler to apply
	// NB: SPI1 on APB2(100MHz max), SPI2 on APB1(50MHz max)
	if(spi == SENSORS_SPI)
		return spi_master_get_prescaler_spi1(speed);
	else if(spi == SENSORS_SPI2)
		return spi_master_get_prescaler_spi2(speed);
	return 0;
}

/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
static uint32_t SPIx_TIMEOUT_UserCallback(char value)
{
	return value;
}

/********************************* Public Prototypes **************************/

void spi_master_init(spi_num_t spinum, spi_speed_t speed)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	SPI_TypeDef* spi;

	spi = spi_master_get_instance(spinum);

	if(spi == SENSORS_SPI)
		spi_master_init_gpio_spi1(&GPIO_InitStructure);
	else if(spi == SENSORS_SPI2)
		spi_master_init_gpio_spi2(&GPIO_InitStructure);

	/* Configure ChipSelect pin only if it is not configured previously--------------*/
	GPIO_InitStructure.GPIO_Pin = spi_master_get_cs_pin(spi);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(spi_master_get_cs_port(spi), &GPIO_InitStructure);
	/* Initialize the SPI peripheral : WARNING CLK MAX is 6,4 MHz */
	SPI_StructInit(&SPI_InitStructure);
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = spi_master_get_prescaler(spi, speed);
	SPI_Init(spi, &SPI_InitStructure);
	/* Enable the SPI peripheral */
	GPIO_SetBits(spi_master_get_cs_port(spi), spi_master_get_cs_pin(spi));
	SPI_Cmd(spi, ENABLE);  
}

void spi_master_deinit(spi_num_t spinum)
{
	SPI_TypeDef* spi;

	spi = spi_master_get_instance(spinum);
	if(spi == SENSORS_SPI) {
		IsInitedSpi1 = 0;
	}
	else if(spi == SENSORS_SPI2) {
		IsInitedSpi2 = 0;
	}
	SPI_Cmd(spi, DISABLE);
}

unsigned long spi_master_write_register(spi_num_t spinum, unsigned char register_addr,
		unsigned short len, const unsigned char *value)
{
	int i;
	SPI_TypeDef* spi;

	spi = spi_master_get_instance(spinum);

	__IO uint32_t SPITimeout = SPIx_LONG_TIMEOUT;

	GPIO_ResetBits(spi_master_get_cs_port(spi), spi_master_get_cs_pin(spi));

	WAIT_FOR_FLAG (spi, SPI_I2S_FLAG_TXE, SPIx_FLAG_TIMEOUT, 0);
	SPI_I2S_SendData(spi, register_addr);

	WAIT_FOR_FLAG (spi, SPI_I2S_FLAG_RXNE, SPIx_FLAG_TIMEOUT, 1);
	SPI_I2S_ReceiveData(spi);

	for (i = 0; i < len; i++) {
		WAIT_FOR_FLAG (spi, SPI_I2S_FLAG_TXE, SPIx_FLAG_TIMEOUT, 2); 
		SPI_I2S_SendData(spi, value[i]);
		WAIT_FOR_FLAG (spi, SPI_I2S_FLAG_RXNE, SPIx_FLAG_TIMEOUT, 3);
		SPI_I2S_ReceiveData(spi);
	}

	GPIO_SetBits(spi_master_get_cs_port(spi), spi_master_get_cs_pin(spi));
	return 0;
}

unsigned long spi_master_read_register(spi_num_t spinum, unsigned char register_addr,
		unsigned short len, unsigned char *value)
{
	int i;
	SPI_TypeDef* spi;

	spi = spi_master_get_instance(spinum);

	__IO uint32_t  SPITimeout = SPIx_LONG_TIMEOUT;
 
	register_addr = READ_BIT_MASK | register_addr;

	GPIO_ResetBits(spi_master_get_cs_port(spi), spi_master_get_cs_pin(spi)); 

	WAIT_FOR_FLAG (spi, SPI_I2S_FLAG_TXE, SPIx_FLAG_TIMEOUT, 4); 
	SPI_I2S_SendData(spi, register_addr);
	WAIT_FOR_FLAG (spi, SPI_I2S_FLAG_RXNE, SPIx_FLAG_TIMEOUT, 5);
	SPI_I2S_ReceiveData(spi); //Clear RXNE bit

	for (i = 0; i < len; i++) {
		WAIT_FOR_FLAG (spi, SPI_I2S_FLAG_TXE, SPIx_FLAG_TIMEOUT, 6); 
		SPI_I2S_SendData(spi, 0x00); //Dummy byte to generate clock
		WAIT_FOR_FLAG (spi, SPI_I2S_FLAG_RXNE, SPIx_FLAG_TIMEOUT, 7);
		value[i] = SPI_I2S_ReceiveData(spi);
	}

	GPIO_SetBits(spi_master_get_cs_port(spi), spi_master_get_cs_pin(spi));
	return 0;
}