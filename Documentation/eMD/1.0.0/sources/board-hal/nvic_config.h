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


/** @defgroup NVIC priority configuration for all IPs
    @ingroup  Low_Level_Driver
    @{
*/
#ifndef __NVIC_CONFIG_H__
#define __NVIC_CONFIG_H__

/** @brief In case FreeRTOS is used, this must absolutely be at least equal to configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY value since 
	any interrupt service routine that uses an RTOS API function must have its priority manually set to a value that is numerically equal
	to or greater than the configMAX_SYSCALL_INTERRUPT_PRIORITY setting.
	This ensures the interrupt's logical priority is equal to or less than the configMAX_SYSCALL_INTERRUPT_PRIORITY setting.
	Default settings requires that 4 bits are assigned to priority value and 0 to subpriority value so
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	must be called at the very beginning of the program execution */
#ifndef NVIC_PRIORITY_MAX
	#define NVIC_PRIORITY_MAX				5
#endif

#ifndef NVIC_PRIORITY_I2C_SLAVE
	#define NVIC_PRIORITY_I2C_SLAVE				NVIC_PRIORITY_MAX
#endif
#ifndef NVIC_PRIORITY_RTC
	#define NVIC_PRIORITY_RTC					NVIC_PRIORITY_MAX
#endif
#ifndef NVIC_PRIORITY_GPIO_IRQ
	#define NVIC_PRIORITY_GPIO_IRQ				NVIC_PRIORITY_MAX
#endif
#ifndef NVIC_PRIORITY_TIMER
	#define NVIC_PRIORITY_TIMER					NVIC_PRIORITY_MAX+1
#endif
#ifndef NVIC_PRIORITY_SPI_SLAVE
	#define NVIC_PRIORITY_SPI_SLAVE				NVIC_PRIORITY_MAX+2
#endif
#ifndef NVIC_PRIORITY_GPIO_SPI_SLAVE_CS
	#define NVIC_PRIORITY_GPIO_SPI_SLAVE_CS		NVIC_PRIORITY_MAX+3
#endif
#ifndef NVIC_PRIORITY_UART
	#define NVIC_PRIORITY_UART					NVIC_PRIORITY_MAX+4
#endif

#endif /* __NVIC_CONFIG_H__ */


