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

/** @defgroup Timer Timer
	@ingroup  Low_Level_Driver
	@{
*/
#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* Timer peripheral available
*/
enum timer_num {
	TIMER2,
	TIMER3,
	TIMER4
};


/** @brief Init the time base
 *  @param timer_num Timer peripheral number
 *  @param frequency Timer frequency to configure
 *  @return -1 if the parameters are not configurable, 0 otherwise
*/
int timer_configure_timebase(unsigned timer_num, uint32_t frequency);

/** @brief Enable timer so that it starts counting without any IRQ fired
 *  @param timer_num Timer peripheral number
 *  @return None
*/
void timer_enable(unsigned timer_num);

/** @brief Init the timer Input Capture mode for the irq line
 *  @param[in]	enable_mask    mask of irq line to enable
 *  @param[in]	interrupt_cb   callback to call on interrupt
 *  @return -1 if the parameters are not configurable, 0 otherwise
*/
int timer_configure_irq_capture(unsigned enable_mask, 
		void (*interrupt_cb)(void * context, int int_num), void * context);

/** @brief Get the Input Capture timestamp value connected to the interrupt line
 *  @param	mask    mask of irq to get timestamp
 *  @return timestamp  timestamp value in us
*/
uint64_t timer_get_irq_timestamp(unsigned mask);

/** @brief Clear the Input Capture timestamp buffer
 *  @param	mask    mask of irq to get timestamp
 *  @return -1 if the parameter are not configurable, 0 otherwise
*/
int timer_clear_irq_timestamp(unsigned mask);

/** @brief Get the counter value by reading the timer register
 *  @param	timer_num  Timer peripheral number
 *  @return timestamp  timestamp value in us
*/
uint64_t timer_get_counter(unsigned timer_num);

/** @brief Configure a timer which will trigger a callback at the end of its period
 *  @param[in]	timer_num  Timer peripheral number
 *  @param[in]	freq       Timer frequency in Hz
 *  @param[in]	context    Callback context parameter
 *  @param[in]	callback   Called at the end of the timer interrupt handler
 *  @return -1 if parameters are not configurable, the timer's channel on which the callback is registered on success
*/
int timer_configure_callback(unsigned timer_num, uint32_t freq,
		void * context, void (*callback) (void *context));

/** @brief Stop the specified timer's channel
 *  @param[in]	timer_num  Timer peripheral number
 *  @param[in]	channel    Timer channel to turn off
 *  @return -1 if parameters are not configurable, 0 otherwise
*/
int timer_channel_stop(unsigned timer_num, uint8_t channel);

/** @brief Updated the timer's channel frequency
 *  @param[in]	timer_num  Timer peripheral number
 *  @param[in]	channel    Timer channel to update
 *  @param[in]	new_freq   Timer new frequency in Hz
 *  @return -1 if parameters are not configurable, 0 otherwise
*/
int timer_channel_reconfigure_freq(unsigned timer_num, uint8_t channel, uint32_t new_freq);

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H_ */

/** @} */
