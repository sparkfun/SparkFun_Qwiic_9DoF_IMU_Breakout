/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2015-2015 InvenSense Inc. All rights reserved.
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

/** @defgroup SerifHal Serial Interface abstraction layer
 *	@brief    Abstraction layer for I2C/SPI serial interface between host and device
 *  @ingroup  Drivers
 *	@{
 */

#ifndef _INV_SERIF_HAL_H_
#define _INV_SERIF_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <assert.h>

#include "Invn/InvError.h"

/** @brief Serial Interface type definition
 */
enum inv_serif_hal_type {
	INV_SERIF_HAL_TYPE_NONE = 0,
	INV_SERIF_HAL_TYPE_I2C  = 1,
	INV_SERIF_HAL_TYPE_SPI  = 2,
};

/** @brief Serial Interface interface definition
 */
typedef struct inv_serif_hal
{
	/** @brief Perform a read register transaction over the serial interface
	 *	@param[in]	context pointer to some context for underlying driver
	 *	@param[in]	reg     register
	 *	@param[out]	data    pointer to output buffer
	 *	@param[in]  len     number of byte to read (should not exceed MAX_TRANSACTION_SIZE)
	 *	@return 0 on sucess, negative value on error
	 */
	int (*read_reg)(void * context, uint8_t reg, uint8_t * data, uint32_t len);

	/** @brief Perform a write register transaction over the serial interface
	*	@param[in]	context pointer to some context for underlying driver
	 *	@param[in]	reg    register
	 *	@param[out]	data   pointer to output buffer
	 *	@param[in]  len    number of byte to read (should not exceed MAX_TRANSACTION_SIZE)
	 *	@return 0 on sucess, negative value on error
	 */
	int (*write_reg)(void * context, uint8_t reg, const uint8_t * data, uint32_t len);

	/** @brief Maximum number of bytes allowed per serial read transaction
	 */
	uint32_t	max_read_transaction_size;

	/** @brief Maximum number of bytes allowed per serial write transaction
	 */
	uint32_t	max_write_transaction_size;

	/** @brief Type of underlying serial interface
	 */
	int	        serif_type;

	/** @brief Pointer to some context for underlying driver
	 */
	void *      context;
} inv_serif_hal_t;

/** @brief Helper method to initialize a inv_serif_hal_t object
 */
static inline void inv_serif_hal_init(inv_serif_hal_t * instance,
		void * context, int serif_type,
		uint32_t max_read_transaction_size,
		uint32_t max_write_transaction_size,
		int (*read_reg)(void * context, uint8_t reg, uint8_t * data, uint32_t len),
		int (*write_reg)(void * context, uint8_t reg, const uint8_t * data, uint32_t len)
)
{
	assert(instance);

	instance->context                    = context;
	instance->serif_type                 = serif_type;
	instance->max_read_transaction_size  = max_read_transaction_size;
	instance->max_write_transaction_size = max_write_transaction_size;
	instance->read_reg                   = read_reg;
	instance->write_reg                  = write_reg;
}

/******************************************************************************/
/* Helper methods to call members of a Serial Interface object                */
/******************************************************************************/

/** @brief Helper method to call read_reg() method of a Serial Interface object
 */
static inline int inv_serif_hal_read_reg(inv_serif_hal_t * instance,
	uint8_t reg, uint8_t * data, uint32_t len)
{
	assert(instance);

	return instance->read_reg(instance->context, reg, data, len);
}

/** @brief Helper method to call write_reg() method of a Serial Interface object
 */
static inline int inv_serif_hal_write_reg(const inv_serif_hal_t * instance,
	uint8_t reg, const uint8_t * data, uint32_t len)
{
	assert(instance);

	return instance->write_reg(instance->context, reg, data, len);
}

/** @brief Helper method to get serial interface type of a Serial Interface object
 */
static inline int inv_serif_hal_get_type(const inv_serif_hal_t * instance)
{
	assert(instance);

	return instance->serif_type;
}

/** @brief Helper method to get max read transaction size value of a Serial Interface object
 */
static inline uint32_t inv_serif_hal_get_max_read_transaction_size(const inv_serif_hal_t * instance)
{
	assert(instance);

	return instance->max_read_transaction_size;
}

/** @brief Helper method to get max write transaction size value of a Serial Interface object
 */
static inline uint32_t inv_serif_hal_get_max_write_transaction_size(const inv_serif_hal_t * instance)
{
	assert(instance);

	return instance->max_write_transaction_size;	
}

/** @brief Helper method to check if serial interface type is I2C for a Serial Interface object
 */
static inline int inv_serif_hal_is_i2c(const inv_serif_hal_t * instance)
{
	return (inv_serif_hal_get_type(instance) == INV_SERIF_HAL_TYPE_I2C);
}

/** @brief Helper method to check if serial interface type is SPI for a Serial Interface object
 */
static inline int inv_serif_hal_is_spi(const inv_serif_hal_t * instance)
{
	return (inv_serif_hal_get_type(instance) == INV_SERIF_HAL_TYPE_SPI);
}

#ifdef __cplusplus
}
#endif

#endif /* _INV_SERIF_HAL_H_ */

/** @} */
