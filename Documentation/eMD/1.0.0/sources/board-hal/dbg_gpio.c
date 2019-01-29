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

#include "dbg_gpio.h"

#include "stm32f4xx_gpio.h"

static const uint16_t mapping[DBG_GPIO_MAX] = { GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_8 };

static void do_init_gpio_out(int pin)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin     = pin;

	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

static void do_set_gpio(int pin)
{
	GPIOC->BSRRL = pin;
}

static void do_reset_gpio(int pin)
{
	GPIOC->BSRRH = pin;
}

static void do_toggle_gpio(int pin)
{
	GPIOC->ODR ^= pin;
}

void dbg_gpio_init_out(int gpio_num)
{
	if(gpio_num < DBG_GPIO_MAX) {
		do_init_gpio_out(mapping[gpio_num]);
	}
}

void dbg_gpio_set(int gpio_num)
{
	if(gpio_num < DBG_GPIO_MAX) {
		do_set_gpio(mapping[gpio_num]);
	}
}

void dbg_gpio_clear(int gpio_num)
{
	if(gpio_num < DBG_GPIO_MAX) {
		do_reset_gpio(mapping[gpio_num]);
	}
}

void dbg_gpio_toggle(int gpio_num)
{
	if(gpio_num < DBG_GPIO_MAX) {
		do_toggle_gpio(mapping[gpio_num]);
	}
}