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

#include "rtc_timer.h"

#include "nvic_config.h"

#include "stm32f4xx.h"
#include <stddef.h>

#define LSI_A_PREDIV 		0x1 /* to have room to correct LSI_S_PREDIV depending on measured LSI, res = 31.25 micro seconde */
#define LSI_S_PREDIV 		0x3E7F /* to ensure by default  */

#define LSE_A_PREDIV 		0x0
#define LSE_S_PREDIV 		0x7FFF

static int32_t rtc_timer_set_RTC_wakeup(uint32_t wake_period_us, uint32_t RTC_freq);
static uint32_t rtc_timer_measure_LSI_frequency(void);
static uint64_t rtc_timer_get_timestamp_us(void);

/*
 * Callback for RTC IRQ handler
 */
static void (*sRtc_irq_handler_cb)(void);

static struct
{
	uint32_t wu_period;// Wake Up period, 0 if not set
	uint32_t freq;     // Measured RTC frequency
	uint32_t sprediv;  // Value of S Prediv register
	float reso_subsecond;  // Resolution of subsecond counter in us
} rtc_config_state = {0};

/* Public functions  -------------------------------------------------*/

void rtc_timer_init(void (*rtc_irq_handler)(void), uint32_t wake_period, enum rtc_timer_clock use_clock)
{
	EXTI_InitTypeDef  EXTI_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	RTC_InitTypeDef   RTC_InitStructure;
	RTC_TimeTypeDef   RTC_TimeStructure;
	uint32_t RTC_APrediv = LSI_A_PREDIV;

	sRtc_irq_handler_cb = rtc_irq_handler;

	/* Initiate and switch on RTC */
	/* Enable the PWR clock */
	/* Allow access to RTC */   
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
 
	if (use_clock == RTC_TIMER_CLOCK_LSE) {
		/* Enable the LSE OSC */
		RCC_LSEConfig(RCC_LSE_ON);

		/* Wait till LSE is ready */  
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
		}

		/* Select the RTC Clock Source */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

		RTC_APrediv = LSE_A_PREDIV;
		rtc_config_state.sprediv = LSE_S_PREDIV;
	}
	else {
		/* The RTC Clock may varies due to LSI frequency dispersion. */   
		/* Enable the LSI OSC */ 
		RCC_LSICmd(ENABLE);

		/* Wait till LSI is ready */  
		while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {
		}

		/* Select the RTC Clock Source */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

		RTC_APrediv = LSI_A_PREDIV;
		rtc_config_state.sprediv = LSI_S_PREDIV;
	}
	/* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

	/* RTC Wakeup Interrupt Generation: Clock Source: RTCCLK_Div2, Wakeup Time Base: ~1ms
	 Wakeup Time Base (s) = (Divider / (LSE or LSI = 32k)) * WakeUpCounter
	 WakeUpCounter = SCHEDULER_PERIOD (µs) * 1e-6 * (LSE = 32768 or LSI = 32000) / 4
	*/
	RTC_InitStructure.RTC_AsynchPrediv = RTC_APrediv;
	RTC_InitStructure.RTC_SynchPrediv  = rtc_config_state.sprediv;
	RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);
    
	/* Wait for RTC APB registers synchronisation */
	RTC_WaitForSynchro();
    
	RTC_WakeUpCmd(DISABLE);
	RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div2);

	if (use_clock == RTC_TIMER_CLOCK_LSE) {
		rtc_config_state.freq = 32768;
	}
	else {
		/* Measure the LSI Freq to adapt the wake up counter */
		rtc_config_state.freq = rtc_timer_measure_LSI_frequency();
		/* Correct calendar clock for timestamping */
		rtc_config_state.sprediv = rtc_config_state.freq/(RTC_APrediv + 1) - 1;
		RTC_InitStructure.RTC_AsynchPrediv = RTC_APrediv;
		RTC_InitStructure.RTC_SynchPrediv  = rtc_config_state.sprediv;
		RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;
		RTC_Init(&RTC_InitStructure);
    }
	rtc_config_state.reso_subsecond = (1000000000ULL * (RTC_APrediv + 1)) / (uint64_t)rtc_config_state.freq; // ns

	/* Set the time to 00h 00mn 00s AM */
	RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
	RTC_TimeStructure.RTC_Hours   = 0;
	RTC_TimeStructure.RTC_Minutes = 0;
	RTC_TimeStructure.RTC_Seconds = 0;  
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);
    
	rtc_timer_set_RTC_wakeup(wake_period, rtc_config_state.freq);
	rtc_config_state.wu_period = wake_period;
   
	RTC_ClearFlag(RTC_FLAG_WUTF);
    
	/* Prepare the Wakeup Interrupt */
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	/* Connect EXTI_Line22 to the RTC Wakeup event */
	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable the RTC Wakeup Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	/* I2cMaster IRQ task could take more than 1ms so the RTC interrupt priority must be highest to keep the clock fonctional */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_RTC;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RTC_WakeUpCmd(ENABLE);
}

void rtc_timer_update_irq_callback(void (*rtc_irq_handler)(void))
{
	sRtc_irq_handler_cb = rtc_irq_handler;
}

int32_t rtc_timer_update_wake_period(uint32_t wake_period) 
{
	uint32_t wake_state;
	int32_t error = 0;

	if (wake_period == rtc_config_state.wu_period)
		return 0;
	/* Check if RTC is enabled */
	wake_state = (RTC->CR & RTC_CR_WUTE);
	/* Disable in any case */
	if (RTC_WakeUpCmd(DISABLE) != SUCCESS)
		return -1;
	RTC_ClearFlag(RTC_FLAG_WUTF);

	RTC_ITConfig(RTC_IT_WUT, DISABLE);
	// b) Clear the RTC Wakeup (WUTF) flag
	RTC_ClearITPendingBit(RTC_IT_WUT);
	// c) Clear the PWR Wakeup (WUF) flag
	PWR_ClearFlag(PWR_FLAG_WU);
	// d) Enable the RTC Wakeup interrupt
	RTC_ITConfig(RTC_IT_WUT, ENABLE);
	// e) Re-enter the low power mode
	  // Will be done after all processing needed for the product

	error = rtc_timer_set_RTC_wakeup(wake_period, rtc_config_state.freq);
	rtc_config_state.wu_period = wake_period;
	/* Enable again if it was enabled, if error old value is still programmed */
	if (wake_state == RTC_CR_WUTE)
	{
		RTC_WaitForSynchro();
		if (RTC_WakeUpCmd(ENABLE) != SUCCESS)
			return -1;
	}

	return error;
}

uint64_t rtc_timer_get_time_us(void)
{
	static uint64_t timestamp_monotonic_us = 0;

	uint64_t ts = rtc_timer_get_timestamp_us();

	while (ts < timestamp_monotonic_us) {
		ts += 24ULL * 3600ULL * 1000000ULL;
	}

	timestamp_monotonic_us = ts;

	return timestamp_monotonic_us;
}

/* Interrupt management ------------------------------------------------------*/

/**
  * @brief  RTC IT wake up handler
  * @param  None
  * @retval None
  */
void RTC_WKUP_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
	{
		// Feature #8135 : Integrate Safe RTC alternate function wakeup flag clearing sequence
		// Extracted from stm32f401 errata sheet
		/*
		  ST recommendation :

		  When using RTC wakeup to wake up the device from the low-power modes:
		  a) Disable the RTC Wakeup interrupt (WUTIE bit in the RTC_CR register)
		  b) Clear the RTC Wakeup (WUTF) flag
		  c) Clear the PWR Wakeup (WUF) flag
		  d) Enable the RTC Wakeup interrupt
		  e) Re-enter the low power mode
		*/

		// a) Disable the RTC Wakeup interrupt (WUTIE bit in the RTC_CR register)
		RTC_ITConfig(RTC_IT_WUT, DISABLE);
		// b) Clear the RTC Wakeup (WUTF) flag
		RTC_ClearITPendingBit(RTC_IT_WUT);
		// c) Clear the PWR Wakeup (WUF) flag
		PWR_ClearFlag(PWR_FLAG_WU);
		// d) Enable the RTC Wakeup interrupt
		RTC_ITConfig(RTC_IT_WUT, ENABLE);
		// e) Re-enter the low power mode
		  // Will be done after all processing needed for the product

		if(sRtc_irq_handler_cb != NULL)
			sRtc_irq_handler_cb();
	}

	// CLear the exti 22
	EXTI_ClearITPendingBit(EXTI_Line22);
}

/* Private function  -----------------------------------------------*/

static volatile uint32_t CaptureNumber = 0;
static volatile uint32_t PeriodValue = 0;
static volatile uint16_t tmpCC4[2] = {0, 0};

/**
  * @brief  Get timestamps from RTC calendar counters in microsecond
  * @retval timestamps in microsecond on 24 hours
  */
static uint64_t rtc_timer_get_timestamp_us(void)
{
	uint32_t counter_subsec;
	uint64_t subsecond_us;
	uint64_t base_us;

	RTC_TimeTypeDef RTC_TimeStruct;
	//Wait until shadow registers are synchronized
	RTC_WaitForSynchro();
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
	counter_subsec = RTC_GetSubSecond(); /* downcounter from S_PREDIV to 0 */
    
	base_us = ((uint64_t)(rtc_config_state.sprediv - counter_subsec) * (uint64_t)rtc_config_state.reso_subsecond) / 1000ULL;
	subsecond_us = (uint64_t)RTC_TimeStruct.RTC_Hours * 3600ULL * 1000000ULL
				  + (uint64_t)RTC_TimeStruct.RTC_Minutes * 60ULL * 1000000ULL
				  + (uint64_t)RTC_TimeStruct.RTC_Seconds * 1000000ULL
				  + base_us;

	return subsecond_us;
}

/**
 * @brief Set RTC calibrated wake up counter
 * @details
 * @param wake_period_us required wake up period in uS
 * @param RTC_freq in Hz
 * @retval error code (<0 if error)
 */
static int32_t rtc_timer_set_RTC_wakeup(uint32_t wake_period_us, uint32_t RTC_freq)
{
	uint32_t value;
	uint32_t freqHz;
	uint32_t RTC_div_wake;
	uint32_t divider;

	if (wake_period_us == 0)
		return -1;
		
	freqHz = 1000000/wake_period_us;
	if (freqHz == 0)
		return -1;

	RTC_div_wake = (RTC->CR & 0x07);

	switch (RTC_div_wake) 
	{
		case RTC_WakeUpClock_RTCCLK_Div16:
			divider = 16;
			break;
		case RTC_WakeUpClock_RTCCLK_Div8:
			divider = 8;
			break;
		case RTC_WakeUpClock_RTCCLK_Div4:
			divider = 4;
			break;
		case RTC_WakeUpClock_RTCCLK_Div2:
			divider = 2;
			break;
		default:
			return -1;
	}

	value = RTC_freq/(freqHz * divider)-1; // Wakeup Time Base
	if (value < 1)
		return -1;

	RTC_SetWakeUpCounter(value);

	return 0;
}

/**
  * @brief  Configures TIM5 to measure the LSI oscillator frequency. 
  * @param  None
  * @retval LSI Frequency in Hz
  */
static uint32_t rtc_timer_measure_LSI_frequency(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	RCC_ClocksTypeDef  RCC_ClockFreq;
	uint32_t LSI_Freq;

	/* TIM5 configuration *******************************************************/ 
	/* Enable TIM5 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	/* Connect internally the TIM5_CH4 Input Capture to the LSI clock output */
	TIM_RemapConfig(TIM5, TIM5_LSI);

	/* Configure TIM5 presclaer */
	TIM_PrescalerConfig(TIM5, 0, TIM_PSCReloadMode_Immediate);

	/* TIM5 configuration: Input Capture mode ---------------------
	   The LSI oscillator is connected to TIM5 CH4
	   The Rising edge is used as active edge,
	   The TIM5 CCR4 is used to compute the frequency value 
	------------------------------------------------------------ */
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV8;
	TIM_ICInitStructure.TIM_ICFilter = 0;
	TIM_ICInit(TIM5, &TIM_ICInitStructure);

	/* Enable TIM5 Interrupt channel */
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_RTC;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable TIM5 counter */
	TIM_Cmd(TIM5, ENABLE);

	/* Reset the flags */
	TIM5->SR = 0;
    
	CaptureNumber = 0;
    
	/* Enable the CC4 Interrupt Request */  
	TIM_ITConfig(TIM5, TIM_IT_CC4, ENABLE);

	/* Wait until the TIM5 get 2 LSI edges (refer to TIM5_IRQHandler() in 
	  stm32f4xx_it.c file) ******************************************************/

	while(CaptureNumber < 2)
		;
		
	/* Disable TIM5 clock */
	/* Deinitialize the TIM5 peripheral registers to their default reset values */
	/* After reset, the peripheral clock (used for registers read/write access) */
	/* is disabled and the application software has to enable this clock before */
	/* using it. */
	/* Enable the CC4 Interrupt Request */  
	TIM_ITConfig(TIM5, TIM_IT_CC4, DISABLE);
	TIM_DeInit(TIM5);

	/* Deinitialize the NVIC interrupt for TIM5 channel */
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_RTC;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Disable TIM5 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, DISABLE);

	/* Compute the LSI frequency, depending on TIM5 input clock frequency (PCLK1)*/
	/* Get SYSCLK, HCLK and PCLKx frequency */
	RCC_GetClocksFreq(&RCC_ClockFreq);

	/* Get PCLK1 prescaler */
	if ((RCC->CFGR & RCC_CFGR_PPRE1) == 0)
	{ 
		/* PCLK1 prescaler equal to 1 => TIMCLK = PCLK1 */
		LSI_Freq = ((RCC_ClockFreq.PCLK1_Frequency / PeriodValue) * 8);
	}
	else
	{ /* PCLK1 prescaler different from 1 => TIMCLK = 2 * PCLK1 */
		LSI_Freq = (((2 * RCC_ClockFreq.PCLK1_Frequency) / PeriodValue) * 8) ;
	}
	return LSI_Freq;
}

/**
  * @brief  This function handles TIM5 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM5_IRQHandler(void)
{
	if ((TIM_GetITStatus(TIM5, TIM_IT_CC4) != RESET) && (CaptureNumber < 2))
	{
		/* Get the Input Capture value */
		tmpCC4[CaptureNumber++] = TIM_GetCapture4(TIM5);

		/* Clear CC4 Interrupt pending bit */
		TIM_ClearITPendingBit(TIM5, TIM_IT_CC4);

		if (CaptureNumber >= 2)
		{
			// Compute the period length 
			uint16_t tmp_volatile_access = tmpCC4[0];
			PeriodValue = (uint16_t)(0xFFFF - tmp_volatile_access + tmpCC4[1] + 1);
		}
	}
}
