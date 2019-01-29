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

#include "gpio.h"
#include "nvic_config.h"

// system drivers
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"

/** SPI Slave GPIOs **/

#define SPI_SLAVE_GPIO               GPIOB
#define SPI_SLAVE_GPIO_CLK           RCC_AHB1Periph_GPIOB
#define SPI_SLAVE_CS_PIN             GPIO_Pin_12

/** Sensor IRQ GPIOs **/

#define SENSOR_IRQ_D7_GPIO             GPIOA
#define SENSOR_IRQ_D7_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define SENSOR_IRQ_D7_PIN              GPIO_Pin_8

#define SENSOR_IRQ_D6_GPIO             GPIOB
#define SENSOR_IRQ_D6_GPIO_CLK         RCC_AHB1Periph_GPIOB
#define SENSOR_IRQ_D6_PIN              GPIO_Pin_10

#define SENSOR_IRQ_D5_GPIO             GPIOB
#define SENSOR_IRQ_D5_GPIO_CLK         RCC_AHB1Periph_GPIOB
#define SENSOR_IRQ_D5_PIN              GPIO_Pin_4

#define SENSOR_IRQ_D4_GPIO             GPIOB
#define SENSOR_IRQ_D4_GPIO_CLK         RCC_AHB1Periph_GPIOB
#define SENSOR_IRQ_D4_PIN              GPIO_Pin_5

/*
 * Callback for End of frame transfer interruption
 */
static void (*sEnd_of_frame_cb)(void);

/*
 * Callback for GPIO interruption
 */
static void (*sInterrupt_cb)(void * context, int int_num);

/*
 * Context for GPIO interruption callback
 */
static void *sContext;

void gpio_init_pin_out(unsigned pin_num)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;

	if (pin_num == GPIO_PA9) {
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

		/* Init PA.9 as alternate function */
		GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		/* reset GPIO PA.9 */
		GPIOA->BSRRH = GPIO_Pin_9;

	} else if (pin_num == GPIO_PB3) {
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

		/* Init PB.3 as output */
		GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		/* reset GPIO PB.3 */
		GPIOB->BSRRH = GPIO_Pin_3;
	}
}


int gpio_get_state(unsigned enable_mask)
{
	if(enable_mask & TO_MASK(GPIO_SENSOR_IRQ_D7)) {
		return GPIO_ReadInputDataBit(SENSOR_IRQ_D7_GPIO, SENSOR_IRQ_D7_PIN);
	}
	if(enable_mask & TO_MASK(GPIO_SENSOR_IRQ_D6)) {
		return GPIO_ReadInputDataBit(SENSOR_IRQ_D6_GPIO, SENSOR_IRQ_D6_PIN);
	}
	if(enable_mask & TO_MASK(GPIO_SENSOR_IRQ_D5)) {
		return GPIO_ReadInputDataBit(SENSOR_IRQ_D5_GPIO, SENSOR_IRQ_D5_PIN);
	}
	if(enable_mask & TO_MASK(GPIO_SENSOR_IRQ_D4)) {
		return GPIO_ReadInputDataBit(SENSOR_IRQ_D4_GPIO, SENSOR_IRQ_D4_PIN);
	}
	return -1;
}
void gpio_toggle_pin(unsigned pin_num)
{
	/* <pin_num> comes from enum so it needs translation to GPIO address */
	if(pin_num == GPIO_PB3)
		GPIOB->ODR ^= GPIO_Pin_3;
	else if(pin_num == GPIO_PA9)
		GPIOA->ODR ^= GPIO_Pin_9;
}

void gpio_init_ad0_high(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	/* Configure I2C address with PA6 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* PA6 is high to be compatible with InvenSense AKM0991x daughter board */
	GPIO_SetBits(GPIOA, GPIO_Pin_6);
}

void gpio_init_ncs_high(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_6);
}

void gpio_init_spi_slave_cs_as_int(void (*end_of_frame_cb)(void))
{
	GPIO_InitTypeDef   GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable SPI GPIO clock */
	RCC_AHB1PeriphClockCmd(SPI_SLAVE_GPIO_CLK, ENABLE);

	/* Set callback to be called after each end of TX transfer */
	sEnd_of_frame_cb = end_of_frame_cb;

	/* Configure the CS line as EXTI to detect the end of the frame transferred */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = SPI_SLAVE_CS_PIN;
	GPIO_Init(SPI_SLAVE_GPIO, &GPIO_InitStructure);

	/* Enable the clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure EXTI line for CS */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, GPIO_PinSource12);

	EXTI_InitStructure.EXTI_Line = EXTI_Line12;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStructure);

	/* Configure NVIC interrupt to EXTI line */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_GPIO_SPI_SLAVE_CS;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void gpio_sensor_irq_init(unsigned enable_mask,
		void (*interrupt_cb)(void * context, int int_num), void * context)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	sInterrupt_cb = interrupt_cb;
	sContext = context;

	/* Configure as input */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	/* Enable the clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	if(enable_mask & TO_MASK(GPIO_SENSOR_IRQ_D7)) {

		RCC_AHB1PeriphClockCmd(SENSOR_IRQ_D7_GPIO_CLK, ENABLE);

		/* Configure IRQ_D7 (PA8) */
		GPIO_InitStructure.GPIO_Pin = SENSOR_IRQ_D7_PIN;
		GPIO_Init(SENSOR_IRQ_D7_GPIO, &GPIO_InitStructure);

		/* Configure EXTI line for PA8 pin */
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);

		EXTI_InitStructure.EXTI_Line = EXTI_Line8;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_Init(&EXTI_InitStructure);

		/* Configure NVIC interrupt to EXTI line */
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_GPIO_IRQ;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}

	if(enable_mask & TO_MASK(GPIO_SENSOR_IRQ_D6)) {

		RCC_AHB1PeriphClockCmd(SENSOR_IRQ_D6_GPIO_CLK, ENABLE);

		/* Configure IRQ_D6 (PB10) */
		GPIO_InitStructure.GPIO_Pin = SENSOR_IRQ_D6_PIN;
		GPIO_Init(SENSOR_IRQ_D6_GPIO, &GPIO_InitStructure);

		/* Configure EXTI line for PB10 pin */
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource10);

		EXTI_InitStructure.EXTI_Line = EXTI_Line10;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_Init(&EXTI_InitStructure);

		/* Configure NVIC interrupt to EXTI line */
		NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_GPIO_IRQ;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}

	if(enable_mask & TO_MASK(GPIO_SENSOR_IRQ_D5)) {

		RCC_AHB1PeriphClockCmd(SENSOR_IRQ_D5_GPIO_CLK, ENABLE);

		/* Configure IRQ_D5 (PB4) */
		GPIO_InitStructure.GPIO_Pin = SENSOR_IRQ_D5_PIN;
		GPIO_Init(SENSOR_IRQ_D5_GPIO, &GPIO_InitStructure);

		/* Configure EXTI line for PB4 pin */
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);

		EXTI_InitStructure.EXTI_Line = EXTI_Line4;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_Init(&EXTI_InitStructure);

		/* Configure NVIC interrupt to EXTI line */
		NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_GPIO_IRQ;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}

	if(enable_mask & TO_MASK(GPIO_SENSOR_IRQ_D4)) {

		RCC_AHB1PeriphClockCmd(SENSOR_IRQ_D4_GPIO_CLK, ENABLE);

		/* Configure IRQ_D4 (PB5) */
		GPIO_InitStructure.GPIO_Pin = SENSOR_IRQ_D4_PIN;
		GPIO_Init(SENSOR_IRQ_D4_GPIO, &GPIO_InitStructure);

		/* Configure EXTI line for PB5 pin */
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource5);

		EXTI_InitStructure.EXTI_Line = EXTI_Line5;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_Init(&EXTI_InitStructure);

		/* Configure NVIC interrupt to EXTI line */
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_GPIO_IRQ;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
}

void EXTI4_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line4) != RESET) {

		sInterrupt_cb(sContext, GPIO_SENSOR_IRQ_D5);
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line10) != RESET) {

		sInterrupt_cb(sContext, GPIO_SENSOR_IRQ_D6);
		EXTI_ClearITPendingBit(EXTI_Line10);
	}

	if(EXTI_GetITStatus(EXTI_Line12) != RESET) {
		sEnd_of_frame_cb();
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
}

void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line8) != RESET) {

		sInterrupt_cb(sContext, GPIO_SENSOR_IRQ_D7);
		EXTI_ClearITPendingBit(EXTI_Line8);
	}

	if(EXTI_GetITStatus(EXTI_Line5) != RESET) {

		sInterrupt_cb(sContext, GPIO_SENSOR_IRQ_D4);
		EXTI_ClearITPendingBit(EXTI_Line5);
	}
}
