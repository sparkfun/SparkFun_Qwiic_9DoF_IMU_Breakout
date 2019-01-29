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

#include "delay.h"
#include "timer.h"

#include "stm32f4xx.h"

#include <stdint.h>

static TIM_TypeDef* TIMx;

static void start_timer(void) 
{
	TIMx->CNT = 0;
	TIM_Cmd(TIMx, ENABLE);
}

static void stop_timer(void) 
{
	TIM_Cmd(TIMx, DISABLE);
}

static void internal_delay(uint16_t us)
{
	const uint16_t start = TIMx->CNT;

	uint32_t now, prev = 0;
	do{
		now = TIMx->CNT;

		/* handle rollover */
		if(now < prev)
			now = UINT16_MAX + now;
		prev = now;

	}while((now - start) <= us);
}

int delay_init(unsigned tim_num)
{
	if(tim_num == TIMER2)
		TIMx = TIM2;
	else if(tim_num == TIMER3)
		TIMx = TIM3;
	else if(tim_num == TIMER4)
		TIMx = TIM4;
	else 
		return -1;

	/* Timer configuration at 1MHz frequency */
	timer_configure_timebase(tim_num, 1000000);

	return 0;
}

void delay_us(uint32_t us)
{
	uint32_t i;

	start_timer();

	/* in case the delay is up to UINT16_MAX */
	if(us >= UINT16_MAX) {
		/* go to the loop as the internal_delay function only support uint16_t argument type */
		for(i = 0; i < (us / UINT16_MAX); i++)
			internal_delay(UINT16_MAX);
		internal_delay(us % UINT16_MAX);
	}
	else
		internal_delay(us);

	stop_timer();
}

void delay_ms(uint32_t ms)
{
	delay_us(ms * 1000);
}

