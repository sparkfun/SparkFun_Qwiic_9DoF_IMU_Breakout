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

/** @defgroup dbg_gpio dbg_gpio
 *  @brief    This API allows to control the level of several GPIO for
 * debug purpose.
 *
 *	@ingroup  Low_Level_Driver
 *  @{
*/

#ifndef _PLAT_DBG_GPIO_H_
#define _PLAT_DBG_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @brief simple enum to abstract physical mapping of the GPIOs
  *
  * To find the actual physical mapping of the GPIO controlled by the API
  * check in source code board_hal/dbg_gpio.c
  */
enum dbg_gpio_num {
	DBG_GPIO_0,
	DBG_GPIO_1,
	DBG_GPIO_2,
	DBG_GPIO_MAX
};

/** @brief Initialise the GPIO controller driving the GPIO 
 * passed in parameter
 * @param gpio_num GPIO number defined in enum dbg_gpio_num
 */
void dbg_gpio_init_out(int gpio_num);

/** @brief Set level of the GPIO passed in parameter
 * @param gpio_num GPIO number defined in enum dbg_gpio_num
 */
void dbg_gpio_set(int gpio_num);

/** @brief Reset level of the GPIO passed in parameter
 * @param gpio_num GPIO number defined in enum dbg_gpio_num
 */
void dbg_gpio_clear(int gpio_num);

static inline void dbg_gpio_ctrl(int gpio_num, int en)
{
	if(en) {
		dbg_gpio_set(gpio_num);
	}
	else {
		dbg_gpio_clear(gpio_num);
	}
}

/** @brief Toggle level of the GPIO passed in parameter
 * @param gpio_num GPIO number defined in enum dbg_gpio_num
 */
void dbg_gpio_toggle(int gpio_num);

#ifdef __cplusplus
}
#endif

#endif /* _PLAT_DBG_GPIO_H_*/

/** @} */
