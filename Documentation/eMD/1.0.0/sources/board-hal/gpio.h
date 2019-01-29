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

/** @defgroup GPIO GPIO
	@ingroup  Low_Level_Driver
	@{
*/
#ifndef _GPIO_H_
#define _GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * GPIO INTERRUPT line Dx (refer to NUCLEO-F411RE pinouts)
 */
enum gpio_sensor_irq_num {
	GPIO_SENSOR_IRQ_D7,
	GPIO_SENSOR_IRQ_D6,
	GPIO_SENSOR_IRQ_D5,
	GPIO_SENSOR_IRQ_D4
};

/**
 * GPIO FSYNC line (refer to NUCLEO-F411RE pinouts)
 * The different FSYNC pins are supported according the Nucleo Carrier board revision
 */
enum gpio_fsync_num {
	GPIO_PA9,  /** on Carrier board rev-A */
	GPIO_PB3   /** on Carrier board rev-B */
};

/**
 * Helper to create a mask
 */
#ifndef TO_MASK
#define TO_MASK(a) (1U << (unsigned)(a))
#endif

/** @brief Init the GPIO to output
*   @param[in] pin_num to define which pin to configure
*/
void gpio_init_pin_out(unsigned pin_num);

/** @brief Get the GPIO state
*   @param[in] enable_mask mask of line to read
*   @return the GPIO state (SET=1 or RESET=0), -1 otherwise
*/
int gpio_get_state(unsigned enable_mask);

/** @brief Toggle GPIO state
*   @param[in] pin_num which GPIO pin shall be toggled
*/
void gpio_toggle_pin(unsigned pin_num);

/** @brief Init the AD0 line (PA6) as output and set pin level high
*/
void gpio_init_ad0_high(void);

/** @brief Init the nCS line (PB6) as output and set pin level high
*/
void gpio_init_ncs_high(void);

/** @brief Init the SPI slave chip select line as interrupt to detect the end of frame
 *  @param[in]	end_of_frame_cb    callback to call on interrupt
*/
void gpio_init_spi_slave_cs_as_int(void (*end_of_frame_cb)(void));

/** @brief Init the sensor line interrupt to wake-up the MCU
 *  @param[in]	enable_mask     mask of irq line to enable
 *  @param[in]	interrupt_cb    callback to call on interrupt
 *  @param[in]	context         context passed to callback
*/
void gpio_sensor_irq_init(unsigned enable_mask,
		void (*interrupt_cb)(void * context, int int_num), void * context);

#ifdef __cplusplus
}
#endif

#endif /* _GPIO_H_ */

/** @} */
