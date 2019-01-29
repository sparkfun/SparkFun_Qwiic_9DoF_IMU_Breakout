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

/** @defgroup System_Timer System_Timer
	@ingroup  Low_Level_Driver
	@{
*/
#ifndef _SYS_TIMER_H_
#define _SYS_TIMER_H_

#include <stdint.h>

/**
  * @brief  Microsecond resolution of the monotonic timestamp
  */
#define SYS_TIMER_RES_US		1000

/**
  * @brief  Millisecond resolution of the monotonic timestamp
  */
#define SYS_TIMER_RES_MS		1

/**
  * @brief  Init system timer based on systick clock
  * @param systick_handler  systick IRQ handler 
  * @param  wake_period_ms scheduler period in ms (Systick period), minimum period = 1ms
  */
void sys_timer_init(void (*systick_handler)(void), uint32_t wake_period_ms);

/**
  * @brief  Get monotonic timestamp in maximum available resolution
  * @return timestamp in millisecond 
  */
uint32_t sys_timer_get_timestamp_ms(void);

/**
  * @brief  Get monotonic timestamp in maximum available resolution
  * Use TIMER_RES_US or TIMER_RES_MS to convert to second or so
  * @return timestamp in millisecond 
  */
uint64_t sys_timer_get_timestamp(void);

#endif /* _SYS_TIMER_H_ */

/** @} */