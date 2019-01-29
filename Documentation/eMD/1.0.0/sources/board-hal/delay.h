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

/** @defgroup delay delay
 *  @brief    API implemeting some active wait funcionnalities.
 *
 *	@ingroup  Low_Level_Driver
 *  @{
*/

#ifndef _DELAY_H_
#define _DELAY_H_

#include <stdint.h>

/**
  * @brief  Initialize the timer delay
  * @param timer_num Timer peripheral number 
  * @return 0 if success, -1 on error
  */
int delay_init(unsigned timer_num);

/**
  * @brief  Busy wait based on a 100MHz clock timer
  * The timer is start and stop for each call to this function to avoid power consumption
  * @warning Maximum timing value supported is 40s
  * @param  Timing in us 
  */
void delay_us(uint32_t us);

/**
  * @brief  Busy wait based on a 100MHz clock timer
  * The timer is start and stop for each call to this function to avoid power consumption
  * @warning Maximum timing value supported is 40s
  * @param  Timing in ms 
  */
void delay_ms(uint32_t ms);

#endif

/** @} */
