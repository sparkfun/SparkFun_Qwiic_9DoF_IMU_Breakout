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

#include "common.h"
#include "timer.h"
#include "gpio.h"

// system drivers
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"

#include "Invn/EmbUtils/RingBuffer.h"

static uint8_t timer2_initied = 0;
static RINGBUFFER(int_timestamp_buffer, 256, uint32_t);

#define MAX_TIMER_CHANNELS (4)
/* Channel count goes from 1 to @see MAX_TIMER_CHANNELS, index starts from 0. By default CH3 is used for input capture */
#define TIMER_CHANNEL_FOR_INPUT_CAPTURE (2)
#if (TIMER_CHANNEL_FOR_INPUT_CAPTURE != 2)
	#error "Input capture configuration is expected to be on CH3, further updates are required"
#endif

struct timer_state {
	struct channel_state {
		void (*callback) (void *context);
		void *context;
		uint32_t ccr_value;
	} channels[MAX_TIMER_CHANNELS];
	uint8_t occupied_channels;				/* Flag to identify which channels are already occupied */
};

static struct timer_state timer2_state;
static struct timer_state timer3_state;
static struct timer_state timer4_state;

/*
 * Callback and context for input capture timer interruption
 */
static void (*sInterrupt_cb)(void * context, int int_num);
static void *sContext;

void timer_enable(unsigned timer_num)
{
	switch(timer_num) {
		case TIMER2:
			TIM_Cmd(TIM2, ENABLE);
			break;
		case TIMER3:
			TIM_Cmd(TIM3, ENABLE);
			break;
		case TIMER4:
			TIM_Cmd(TIM4, ENABLE);
			break;
		default:
			break;
	}
}

int timer_configure_timebase(unsigned timer_num, uint32_t frequency)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	uint32_t prescaler;

	/* Compute the prescaler value for the requested frequency */
	prescaler = (SystemCoreClock / frequency) - 1; 
	if(prescaler > UINT16_MAX)
		return -1;

	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)prescaler;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	if(timer_num == TIMER2) {
		/* TIM2 clock enable */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

		TIM_TimeBaseStructure.TIM_Period = UINT32_MAX;
		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

		/* Enable the TIM2 global Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
		NVIC_Init(&NVIC_InitStructure);

		timer2_initied = 1;
		timer2_state.occupied_channels |= (1 << TIMER_CHANNEL_FOR_INPUT_CAPTURE);

	} else if(timer_num == TIMER3) {
		/* TIM3 clock enable */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

		TIM_TimeBaseStructure.TIM_Period = UINT16_MAX;
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

		/* Enable the TIM3 global Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_Init(&NVIC_InitStructure);

	} else if(timer_num == TIMER4) {
		/* TIM4 clock enable */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

		TIM_TimeBaseStructure.TIM_Period = UINT16_MAX;
		TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

		/* Enable the TIM4 global Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
		NVIC_Init(&NVIC_InitStructure);
	} else
		return -1;

	return 0;
}

int timer_configure_irq_capture(unsigned enable_mask, 
		void (*interrupt_cb)(void * context, int int_num), void * context)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;

	sInterrupt_cb = interrupt_cb;
	sContext = context;

	if(enable_mask & TO_MASK(GPIO_SENSOR_IRQ_D6)) {

		if(!timer2_initied) {
			timer2_state.occupied_channels &= ~(1 << TIMER_CHANNEL_FOR_INPUT_CAPTURE);
			return -1;
		}

		/* Configure as alternate function */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_TIM2);  

		/* TIM2 configuration: Input Capture mode ---------------------
		 The external signal is connected to TIM2 CH3 pin (PB.10)  
		 The Rising edge is used as active edge,
		 The TIM2 CCR3 is used to compute the frequency value 
		------------------------------------------------------------ */

		TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
		TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
		TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
		TIM_ICInitStructure.TIM_ICFilter = 0x0;

		TIM_ICInit(TIM2, &TIM_ICInitStructure);

		/* Clear the buffer used to store the timestamp catched on interrupt */
		RINGBUFFER_CLEAR(&int_timestamp_buffer);

		/* Enable the TIM2 CC3 Interrupt Request */
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);
		TIM_ITConfig(TIM2, TIM_IT_CC3, ENABLE);

		/* TIM enable counter */
		TIM_Cmd(TIM2, ENABLE);

		return 0;
	}
	return -1;
}

uint64_t timer_get_irq_timestamp(unsigned mask)
{
	if(mask & TO_MASK(GPIO_SENSOR_IRQ_D6)) {
		uint32_t timestamp = 0;

		disable_irq();
		if(!RINGBUFFER_EMPTY(&int_timestamp_buffer))
			RINGBUFFER_POP(&int_timestamp_buffer, &timestamp);
		enable_irq();

		return (uint64_t)timestamp;
	}
	return 0;
}

int timer_clear_irq_timestamp(unsigned mask)
{
	if(mask & TO_MASK(GPIO_SENSOR_IRQ_D6)) {
		disable_irq();
		RINGBUFFER_CLEAR(&int_timestamp_buffer);
		enable_irq();
		return 0;
	}
	return -1;
}

uint64_t timer_get_counter(unsigned timer_num)
{
	if(timer_num == TIMER2) {
		return (uint64_t)(TIM_GetCounter(TIM2));
	}
	return 0;
}

static int timer_verify_freq(unsigned timer_num, uint32_t freq)
{
	uint32_t timer_counting_freq, timer_period, min_freq, max_freq;
	TIM_TypeDef* TIMx;

	/*
		Prescaler is configured by <timer_configure_timebase> as: (SystemCoreClock / frequency) - 1.
		Timer frequency can be obtained using the following:
			PSC = (SystemCoreClock / frequency) - 1
			PSC + 1 = (SystemCoreClock / frequency)
			frequency = SystemCoreClock / (PSC + 1)
		For example, a timer with 1Mhz resolution on 16bit can do a minimum of:
		1,000,000 / 2^16 ~= 15.26Hz
	*/

	if(timer_num == TIMER2) {
		TIMx = TIM2;
		timer_period = UINT32_MAX;
	} else if(timer_num == TIMER3) {
		TIMx = TIM3;
		timer_period = UINT16_MAX;
	} else if(timer_num == TIMER4) {
		TIMx = TIM4;
		timer_period = UINT16_MAX;
	} else {
		return -1;
	}

	timer_counting_freq = SystemCoreClock / (TIM_GetPrescaler(TIMx) + 1);

	min_freq = timer_counting_freq / timer_period;
	if (min_freq == 0)
		/* above line will return 0 if timer_period>timer_counting_freq,
		setting 1 here also protects from <freq> being 0 */
		min_freq = 1;
	max_freq = timer_counting_freq; /*maximum one interrupt every tick*/

	if (freq >= min_freq && freq <= max_freq) {
		return 0;
	}

	return -1;
}

int timer_configure_callback(unsigned timer_num, uint32_t freq,
	void * context, void (*callback) (void *context))
{
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_TypeDef* TIMx;
	uint16_t channel_it;
	void (*init_channel_fct) (TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct);
	uint8_t current_channel, i;
	struct timer_state *current_timer;

	if (timer_verify_freq(timer_num, freq) != 0) {
		return -1;
	}

	if(timer_num == TIMER2) {
		TIMx = TIM2; /*not for all channels*/
		current_timer = &timer2_state;
	} else if(timer_num == TIMER3) {
		TIMx = TIM3;
		current_timer = &timer3_state;
	} else if(timer_num == TIMER4) {
		TIMx = TIM4;
		current_timer = &timer4_state;
	} else
		return -1;

	current_channel = MAX_TIMER_CHANNELS; /*not found by default*/
	for (i = 0; i < MAX_TIMER_CHANNELS; i++) {
		if (0 == ((1 << i) & current_timer->occupied_channels)) {
			/* found an unused channel, break now */
			current_timer->occupied_channels |= (1 << i);
			current_channel = i;
			break;
		}
	}

	switch (current_channel) {
	case 0:
		channel_it = TIM_IT_CC1;
		init_channel_fct = TIM_OC1Init;
		break;
	case 1:
		channel_it = TIM_IT_CC2;
		init_channel_fct = TIM_OC2Init;
		break;
	case 2:
		if (timer_num == TIMER2) {
			/* used for input capture and nothing else, this case shall never be reached */
			return -1;
		}
		channel_it = TIM_IT_CC3;
		init_channel_fct = TIM_OC3Init;
		break;
	case 3:
		channel_it = TIM_IT_CC4;
		init_channel_fct = TIM_OC4Init;
		break;
	default:
		return -1;
	}

	/* 
		TIM2, TIM3, TIM4 input clock (TIM_CLK) is set to 2 * APB1 clock (PCLK1)
		TIM_CLK = 2 * PCLK1
		PCLK1 = HCLK / 2
		=> TIM_CLK = HCLK = SystemCoreClock

		For example :
		Compute the prescaler value to get TIM counter clock at 1MHz 
		Prescaler = (TIMCLK / TIM counter clock) - 1
		Prescaler = (SystemCoreClock) /1 MHz) - 1

		To get TIM output clock at 30 Hz, the period (ARR)) is computed as follows:
		ARR = (TIM counter clock / TIM output clock) - 1
			= 33332
		CCR_VALUE = AAR / 2 to get a signal with 50% duty cycle
			= 16666
	*/

	current_timer->channels[current_channel].ccr_value = 
		((SystemCoreClock / (TIM_GetPrescaler(TIMx) + 1)) / freq /*Hz*/);
	current_timer->channels[current_channel].callback = callback;
	current_timer->channels[current_channel].context = context;

	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	if(timer_num == TIMER2) { /* TIM2 is 32bit */
		TIM_OCInitStructure.TIM_Pulse = (TIM_GetCounter(TIMx) + current_timer->channels[current_channel].ccr_value);
	} else {
		TIM_OCInitStructure.TIM_Pulse = (uint16_t)(TIM_GetCounter(TIMx) + current_timer->channels[current_channel].ccr_value);
	}
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	/* Timing Mode configuration: Channel */
	init_channel_fct(TIMx, &TIM_OCInitStructure);

	/* TIM Interrupts enable */
	TIM_ClearITPendingBit(TIMx, channel_it);
	TIM_ITConfig(TIMx, channel_it, ENABLE);

	/* TIMx enable counter */
	TIM_Cmd(TIMx, ENABLE);

	return current_channel;
}

int timer_channel_stop(unsigned timer_num, uint8_t channel)
{
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	void (*init_channel_fct) (TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct);
	TIM_TypeDef* TIMx;
	uint16_t channel_it;
	struct timer_state *current_timer;

	if(timer_num == TIMER2) {
		TIMx = TIM2;
		current_timer = &timer2_state;
	} else if(timer_num == TIMER3) {
		TIMx = TIM3;
		current_timer = &timer3_state;
	} else if(timer_num == TIMER4) {
		TIMx = TIM4;
		current_timer = &timer4_state;
	} else
		return -1;

	if (0 == ((1 << channel) & current_timer->occupied_channels))
		/* unused channel shall not be stopped */
		return -1;

	switch (channel) {
	case 0:
		channel_it = TIM_IT_CC1;
		init_channel_fct = TIM_OC1Init;
		break;
	case 1:
		channel_it = TIM_IT_CC2;
		init_channel_fct = TIM_OC2Init;
		break;
	case 2:
		channel_it = TIM_IT_CC3;
		init_channel_fct = TIM_OC3Init;
		break;
	case 3:
		channel_it = TIM_IT_CC4;
		init_channel_fct = TIM_OC4Init;
		break;
	default:
		return -1;
	}
	current_timer->occupied_channels &= (uint8_t)(~(1 << channel));

	/* TIM Interrupts disable */
	TIM_ITConfig(TIMx, channel_it, DISABLE);
	TIM_ClearITPendingBit(TIMx, channel_it);
	
	current_timer->channels[channel].callback = 0;
	
	/* Reset channel to its default value */
	TIM_OCStructInit(&TIM_OCInitStructure);
	init_channel_fct(TIMx, &TIM_OCInitStructure);

	return 0;
}

int timer_channel_reconfigure_freq(unsigned timer_num, uint8_t channel, uint32_t new_freq)
{
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	void (*init_channel_fct) (TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct);
	TIM_TypeDef* TIMx;
	uint16_t channel_it;
	struct timer_state *current_timer;

	if (timer_verify_freq(timer_num, new_freq) != 0) {
		return -1;
	}

	if(timer_num == TIMER2) {
		TIMx = TIM2; /*not for all channels*/
		current_timer = &timer2_state;
	} else if(timer_num == TIMER3) {
		TIMx = TIM3;
		current_timer = &timer3_state;
	} else if(timer_num == TIMER4) {
		TIMx = TIM4;
		current_timer = &timer4_state;
	} else
		return -1;

	if (0 == ((1 << channel) & current_timer->occupied_channels)) {
		/* unused channel shall not be reconfigured */
		return -1;
	}

	switch (channel) {
	case 0:
		channel_it = TIM_IT_CC1;
		init_channel_fct = TIM_OC1Init;
		break;
	case 1:
		channel_it = TIM_IT_CC2;
		init_channel_fct = TIM_OC2Init;
		break;
	case 2:
		if (timer_num == TIMER2) {
			/* used for input capture and nothing else, should never be reached */
			return -1;
		}
		channel_it = TIM_IT_CC3;
		init_channel_fct = TIM_OC3Init;
		break;
	case 3:
		channel_it = TIM_IT_CC4;
		init_channel_fct = TIM_OC4Init;
		break;
	default:
		return -1;
	}

	/* TIM Interrupts disable, do not wait a full period */
	TIM_ITConfig(TIMx, channel_it, DISABLE);
	
	/* Only CCR value will change */
	current_timer->channels[channel].ccr_value = 
		((SystemCoreClock / (TIM_GetPrescaler(TIMx) + 1)) / new_freq /*Hz*/);
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	if(timer_num == TIMER2) { /* TIM2 is 32bit */
		TIM_OCInitStructure.TIM_Pulse = (TIM_GetCounter(TIMx) + current_timer->channels[channel].ccr_value);
	} else {
		TIM_OCInitStructure.TIM_Pulse = (uint16_t)(TIM_GetCounter(TIMx) + current_timer->channels[channel].ccr_value);
	}
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	init_channel_fct(TIMx, &TIM_OCInitStructure);

	/* Compare has been reset by TIM_OCXInit */

	/* TIM Interrupts re-enable */
	TIM_ITConfig(TIMx, channel_it, ENABLE);

	return 0;
}

void TIM2_IRQHandler(void)
{ 
	/* channel 1 */
	if(TIM_GetITStatus(TIM2, TIM_IT_CC1) == SET) 
	{
		/* Clear TIM2 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

		uint32_t capture = TIM_GetCapture1(TIM2);
		TIM_SetCompare1(TIM2, capture + timer2_state.channels[0].ccr_value);

		if (timer2_state.channels[0].callback)
			timer2_state.channels[0].callback(timer2_state.channels[0].context);
	}
	/* channel 2 */
	if(TIM_GetITStatus(TIM2, TIM_IT_CC2) == SET) 
	{
		/* Clear TIM2 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);

		uint32_t capture = TIM_GetCapture2(TIM2);
		TIM_SetCompare2(TIM2, capture + timer2_state.channels[1].ccr_value);

		if (timer2_state.channels[1].callback)
			timer2_state.channels[1].callback(timer2_state.channels[1].context);
	}

	/* channel 3 */
	if(TIM_GetITStatus(TIM2, TIM_IT_CC3) == SET) 
	{
		/* Clear TIM2 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);

		/* Get the Input Capture value */
		uint32_t read_value = TIM_GetCapture3(TIM2);

		if(!RINGBUFFER_FULL(&int_timestamp_buffer))
			RINGBUFFER_PUSH(&int_timestamp_buffer, &read_value);

		/* callback on interrupt */
		if(sInterrupt_cb)
			sInterrupt_cb(sContext, GPIO_SENSOR_IRQ_D6);
	}
	
	/* channel 4 */
	if(TIM_GetITStatus(TIM2, TIM_IT_CC4) == SET) 
	{
		/* Clear TIM2 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC4);

		uint32_t capture = TIM_GetCapture4(TIM2);
		TIM_SetCompare4(TIM2, capture + timer2_state.channels[3].ccr_value);

		if (timer2_state.channels[3].callback)
			timer2_state.channels[3].callback(timer2_state.channels[3].context);
	}
}

void TIM3_IRQHandler(void)
{
	/* channel 1 */
	if(TIM_GetITStatus(TIM3, TIM_IT_CC1) == SET) 
	{
		/* Clear TIM3 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);

		uint32_t capture = TIM_GetCapture1(TIM3);
		TIM_SetCompare1(TIM3, (uint32_t)(uint16_t)(capture + timer3_state.channels[0].ccr_value));

		if (timer3_state.channels[0].callback)
			timer3_state.channels[0].callback(timer3_state.channels[0].context);
	}
	/* channel 2 */
	if(TIM_GetITStatus(TIM3, TIM_IT_CC2) == SET) 
	{
		/* Clear TIM3 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

		uint32_t capture = TIM_GetCapture2(TIM3);
		TIM_SetCompare2(TIM3, (uint32_t)(uint16_t)(capture + timer3_state.channels[1].ccr_value));

		if (timer3_state.channels[1].callback)
			timer3_state.channels[1].callback(timer3_state.channels[1].context);
	}
	/* channel 3 */
	if(TIM_GetITStatus(TIM3, TIM_IT_CC3) == SET) 
	{
		/* Clear TIM3 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);

		uint32_t capture = TIM_GetCapture3(TIM3);
		TIM_SetCompare3(TIM3, (uint32_t)(uint16_t)(capture + timer3_state.channels[2].ccr_value));

		if (timer3_state.channels[2].callback)
			timer3_state.channels[2].callback(timer3_state.channels[2].context);
	}
	/* channel 4 */
	if(TIM_GetITStatus(TIM3, TIM_IT_CC4) == SET) 
	{
		/* Clear TIM3 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);

		uint32_t capture = TIM_GetCapture4(TIM3);
		TIM_SetCompare4(TIM3, (uint32_t)(uint16_t)(capture + timer3_state.channels[3].ccr_value));

		if (timer3_state.channels[3].callback)
			timer3_state.channels[3].callback(timer3_state.channels[3].context);
	}
}

void TIM4_IRQHandler(void)
{
	/* channel 1 */
	if(TIM_GetITStatus(TIM4, TIM_IT_CC1) == SET) 
	{
		/* Clear TIM4 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);

		uint32_t capture = TIM_GetCapture1(TIM4);
		TIM_SetCompare1(TIM4, (uint32_t)(uint16_t)(capture + timer4_state.channels[0].ccr_value));

		if (timer4_state.channels[0].callback)
			timer4_state.channels[0].callback(timer4_state.channels[0].context);
	}
	/* channel 2 */
	if(TIM_GetITStatus(TIM4, TIM_IT_CC2) == SET) 
	{
		/* Clear TIM4 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);

		uint32_t capture = TIM_GetCapture2(TIM4);
		TIM_SetCompare2(TIM4, (uint32_t)(uint16_t)(capture + timer4_state.channels[1].ccr_value));

		if (timer4_state.channels[1].callback)
			timer4_state.channels[1].callback(timer4_state.channels[1].context);
	}
	/* channel 3 */
	if(TIM_GetITStatus(TIM4, TIM_IT_CC3) == SET) 
	{
		/* Clear TIM4 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC3);

		uint32_t capture = TIM_GetCapture3(TIM4);
		TIM_SetCompare3(TIM4, (uint32_t)(uint16_t)(capture + timer4_state.channels[2].ccr_value));

		if (timer4_state.channels[2].callback)
			timer4_state.channels[2].callback(timer4_state.channels[2].context);
	}
	/* channel 4 */
	if(TIM_GetITStatus(TIM4, TIM_IT_CC4) == SET) 
	{
		/* Clear TIM4 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC4);

		uint32_t capture = TIM_GetCapture4(TIM4);
		TIM_SetCompare4(TIM4, (uint32_t)(uint16_t)(capture + timer4_state.channels[3].ccr_value));

		if (timer4_state.channels[3].callback)
			timer4_state.channels[3].callback(timer4_state.channels[3].context);
	}
}
