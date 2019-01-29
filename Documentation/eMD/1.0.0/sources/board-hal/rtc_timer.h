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

/** @defgroup RTC_Timer RTC_Timer
	@ingroup Low_Level_Driver
	@{
*/

#ifndef _RTC_TIMER_H_
#define _RTC_TIMER_H_

#include <stdint.h>

enum rtc_timer_clock {
	RTC_TIMER_CLOCK_LSI = 0,
	RTC_TIMER_CLOCK_LSE
};

/**
  * @brief  Init system timer based on RTC clock  
  * @Note   Initialize clocks, PLL, RTC, adapt RTC if on LSI
  * @param  rtc_irq_handler RTC clock IRQ handler
  * @param  wake_period scheduler period (RTC period)
  * @param  useLSE use external clock LSE or internal clock LSI
  */
void rtc_timer_init(void (*rtc_irq_handler)(void), uint32_t wake_period, enum rtc_timer_clock use_clock);

/**
  * @brief  Reconfigure RTC clock IRQ handler
  * @param  rtc_irq_handler RTC clock IRQ handler
  */
void rtc_timer_update_irq_callback(void (*rtc_irq_handler)(void));

/**
  * @brief  Update RTC wake up period
  * @Note   Enable again the RTC if RTC was enabled
  * @param  wake_period scheduler period (RTC period)
  * @retval error (if negative)
  */
int32_t rtc_timer_update_wake_period(uint32_t wake_period);

/**
  * @brief  Get timestamps from RTC calendar counters in microsecond and prevent rollover
  * @retval timestamps in microsecond
  */
uint64_t rtc_timer_get_time_us(void);

#endif /* _RTC_TIMER_H_ */

/** @} */