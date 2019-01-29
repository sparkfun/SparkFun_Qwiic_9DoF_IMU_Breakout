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

/** @defgroup HostSerif Host Serial Interface
 *	@brief    Virtual abstraction of host adapter for serial interface
 *  @deprecated Use SerifHal.h instead
 *  @ingroup  Drivers
 *	@{
 */

#ifndef _INV_HOST_SERIF_H_
#define _INV_HOST_SERIF_H_

#include "Invn/InvExport.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <assert.h>

#include "Invn/InvError.h"

/** @brief Serial Interface type definition
 */
enum inv_host_serif_type {
	INV_HOST_SERIF_TYPE_NONE = 0,
	INV_HOST_SERIF_TYPE_I2C  = 1,
	INV_HOST_SERIF_TYPE_SPI  = 2,
	INV_HOST_SERIF_TYPE_SPI3 = 3,
};


/** @brief Serial Interface common error code definition
 */
enum inv_host_serif_error {
	INV_HOST_SERIF_ERROR_SUCCESS    = 0,
	INV_HOST_SERIF_ERROR            = -1,
	INV_HOST_SERIF_ERROR_NOT_FOUND  = -2,
	INV_HOST_SERIF_ERROR_OPEN       = -3,
};

/** @brief Serial Interface interface definition
 */
typedef struct inv_host_serif
{
	/** @brief Open connection to and initialize Serial Interface adapter
	 *	@return 0 on sucess, negative value on error	
	 */
	int (*open)(void);
	
	/** @brief Close connection to Serial Interface adapter
	 *	@return 0 on sucess, negative value on error	
	 */
	int (*close)(void);
	
	/** @brief Perform a read register transaction over the serial interface
	 *	@param[in]	reg    register
	 *	@param[out]	data   pointer to output buffer
	 *	@param[in]  len    number of byte to read (should not exceed MAX_TRANSACTION_SIZE)
	 *	@return 0 on sucess, negative value on error	
	 */
	int (*read_reg)(uint8_t reg, uint8_t * data, uint32_t len);

	/** @brief Perform a write register transaction over the serial interface
	 *	@param[in]	reg    register
	 *	@param[out]	data   pointer to output buffer
	 *	@param[in]  len    number of byte to read (should not exceed MAX_TRANSACTION_SIZE)
	 *	@return 0 on sucess, negative value on error	
	 */	
	int (*write_reg)(uint8_t reg, const uint8_t * data, uint32_t len);

	/** @brief Register a callback to the adapter
	 *	@param[in]	interrupt_cb    callback to call on interrupt
	 *	@param[in]	context         context passed to callback
	 *	@return 0 on sucess, negative value on error
	 */
	int (*register_interrupt_callback)(
			void (*interrupt_cb)(void * context, int int_num), void * context);

	/** @brief Maximum number of bytes allowed per serial read
	 */
	uint32_t	max_read_size;

	/** @brief Maximum number of bytes allowed per serial write
	 */
	uint32_t	max_write_size;

	/** @brief Type of underlying serial interface
	 */
	int	        serif_type;

} inv_host_serif_t;

/** @brief Set global instance for Serial Interface
 */
void INV_EXPORT inv_host_serif_set_instance(const inv_host_serif_t * instance);

/** @brief Return global instance for Serial Interface
 */
const inv_host_serif_t * inv_host_serif_get_instance(void);

/******************************************************************************/
/* Helper methods to call members of a Serial Interface object                */
/******************************************************************************/

/** @brief Helper method to call open() method of a Serial Interface object
 */
static inline int inv_host_serif_open(const inv_host_serif_t * instance)
{
	assert(instance);

	return instance->open();
}

/** @brief Helper method to call close() method of a Serial Interface object
 */
static inline int inv_host_serif_close(const inv_host_serif_t * instance)
{
	assert(instance);

	return instance->close();
}

/** @brief Helper method to call read_reg() method of a Serial Interface object
 */
static inline int inv_host_serif_read_reg(const inv_host_serif_t * instance,
	uint8_t reg, uint8_t * data, uint32_t len)
{
	assert(instance);

	return instance->read_reg(reg, data, len);
}

/** @brief Helper method to call write_reg() method of a Serial Interface object
 */
static inline int inv_host_serif_write_reg(const inv_host_serif_t * instance,
	uint8_t reg, const uint8_t * data, uint32_t len)
{
	assert(instance);

	return instance->write_reg(reg, data, len);
}

/** @brief Helper method to call register_interrupt_callback() method of a Serial Interface object
 */
static inline int inv_host_serif_register_interrupt_callback(const inv_host_serif_t * instance,
	void (*interrupt_cb)(void * context, int int_num), void * context)
{
	assert(instance);

	if(instance->register_interrupt_callback)
		return instance->register_interrupt_callback(interrupt_cb, context);

	return INV_ERROR_NIMPL;
}

/** @brief Helper method to get serial interface type of a Serial Interface object
 */
static inline int inv_host_serif_get_type(const inv_host_serif_t * instance)
{
	assert(instance);

	return instance->serif_type;
}

/** @brief Helper method to get max read size value of a Serial Interface object
 */
static inline uint32_t inv_host_serif_get_max_read_transaction_size(const inv_host_serif_t * instance)
{
	assert(instance);

	return instance->max_read_size;
}

/** @brief Helper method to get max write size value of a Serial Interface object
 */
static inline uint32_t inv_host_serif_get_max_write_transaction_size(const inv_host_serif_t * instance)
{
	assert(instance);

	return instance->max_write_size;
}

/** @brief Helper method to check if serial interface type is I2C for a Serial Interface object
 */
static inline int inv_host_serif_is_i2c(const inv_host_serif_t * instance)
{
	return (inv_host_serif_get_type(instance) == INV_HOST_SERIF_TYPE_I2C);
}

/** @brief Helper method to check if serial interface type is SPI for a Serial Interface object
 */
static inline int inv_host_serif_is_spi(const inv_host_serif_t * instance)
{
	return (inv_host_serif_get_type(instance) == INV_HOST_SERIF_TYPE_SPI);
}

/******************************************************************************/
/* Helper methods to call members of a Serial Interface global instance       */
/******************************************************************************/

static inline int inv_host_serif_open_i(void)
{
	return inv_host_serif_open(inv_host_serif_get_instance());
}

static inline int inv_host_serif_close_i(void)
{
	return inv_host_serif_close(inv_host_serif_get_instance());
}

static inline int inv_host_serif_read_reg_i(uint8_t reg, uint8_t * data, uint32_t len)
{
	return inv_host_serif_read_reg(inv_host_serif_get_instance(), reg, data, len);
}

static inline int inv_host_serif_write_reg_i(uint8_t reg, const uint8_t * data, uint32_t len)
{
	return inv_host_serif_write_reg(inv_host_serif_get_instance(), reg, data, len);
}

static inline int inv_host_serif_register_interrupt_callback_i(
		void (*interrupt_cb)(void * context, int int_num), void * context)
{
	return inv_host_serif_register_interrupt_callback(inv_host_serif_get_instance(),
			interrupt_cb, context);
}

static inline int inv_host_serif_get_type_i(void)
{
	return inv_host_serif_get_type(inv_host_serif_get_instance());
}

static inline uint32_t inv_host_serif_get_max_read_transaction_size_i(void)
{
	return inv_host_serif_get_max_read_transaction_size(inv_host_serif_get_instance());
}

static inline uint32_t inv_host_serif_get_max_write_transaction_size_i(void)
{
	return inv_host_serif_get_max_write_transaction_size(inv_host_serif_get_instance());
}

static inline int inv_host_serif_is_i2c_i(void)
{
	return (inv_host_serif_get_type_i() == INV_HOST_SERIF_TYPE_I2C);
}

static inline int inv_host_serif_is_spi_i(void)
{
	return (inv_host_serif_get_type_i() == INV_HOST_SERIF_TYPE_SPI);
}

#ifdef __cplusplus
}
#endif

#endif /* _INV_HOST_SERIF_H_ */

/** @} */
