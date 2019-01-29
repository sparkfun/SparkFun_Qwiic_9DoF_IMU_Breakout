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

/** @defgroup DeviceIcm20948 DeviceIcm20948
 *	@brief    Concrete implementation of the 'Device' interface for Icm20948 devices
 *
 *            See @ref ExampleDeviceIcm20948.c example.
 *
 *  @ingroup  Device
 *	@{
 */

#ifndef _INV_DEVICE_ICM20948_H_
#define _INV_DEVICE_ICM20948_H_

#include "Invn/InvExport.h"
#include "Invn/Devices/Device.h"
#include "Invn/Devices/SerifHal.h"
#include "Invn/Devices/Drivers/Icm20948/Icm20948.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief States for Icm20948 device
 */
typedef struct inv_device_icm20948
{
	inv_device_t         base;
	inv_host_serif_t          legacy_serif;     /* used for backward API compatibility */
	struct inv_icm20948  icm20948_states;
	//dmp3 Image
	const uint8_t * dmp3_image;
	uint32_t dmp3_image_size;
} inv_device_icm20948_t;

/** @brief Allowed config setting value for 20649 device.
 */
enum  inv_device_icm20948_config {
	INV_DEVICE_ICM20948_CONFIG_FSR             = INV_SENSOR_CONFIG_FSR,
	INV_DEVICE_ICM20948_CONFIG_POWER_MODE      = INV_SENSOR_CONFIG_POWER_MODE,
	INV_DEVICE_ICM20948_CONFIG_OFFSET          = INV_SENSOR_CONFIG_OFFSET,
	//INV_DEVICE_ICM20948_CONFIG_WOM_THRESHOLD,
};

/** @brief Return handle to underlying driver states
 *
 *  @param[in] self         handle to device
 *  @return pointer to underlying driver states
 */
static inline struct inv_icm20948 * inv_device_icm20948_get_driver_handle(inv_device_icm20948_t * self)
{
	return &self->icm20948_states;
}

/** @brief constructor-like function for basesensor device
 *
 *  Will initialize inv_device_icm20948_t object states to default value for basesensor.
 *
 *  @param[in] self         handle to device
 *  @param[in] serif        reference to Serial Interface object
 *  @param[in] listener     reference to Sensor Event Listener object
 */
void INV_EXPORT inv_device_icm20948_init(inv_device_icm20948_t * self,
		const inv_host_serif_t * serif, const inv_sensor_listener_t * listener,
		const uint8_t  * dmp3_image, uint32_t dmp3_image_size);
/** @brief constructor-like function for Icm20948 device
 *
 *  Will initialize inv_device_icm20948_t object states to default value for ICM20948.
 *
 *  @param[in] self         handle to device
 *  @param[in] serif        reference to Host Serial Interface object
 *  @param[in] listener     reference to Sensor Event Listener object
 */
void INV_EXPORT inv_device_icm20948_init2(inv_device_icm20948_t * self,
		const inv_serif_hal_t * serif, const inv_sensor_listener_t * listener,
		const uint8_t  * dmp3_image, uint32_t dmp3_image_size);

/** @brief Helper function to get handle to base object
 */
static inline inv_device_t * inv_device_icm20948_get_base(inv_device_icm20948_t * self)
{
	if(self)
		return &self->base;

	return 0;
}

/*
 * Functions below are described in Device.h
 */

int INV_EXPORT inv_device_icm20948_whoami(void * context, uint8_t * whoami);

int INV_EXPORT inv_device_icm20948_reset(void * context);

int INV_EXPORT inv_device_icm20948_setup(void * context);

int INV_EXPORT inv_device_icm20948_cleanup(void * context);

int INV_EXPORT inv_device_icm20948_load(void * context, int what,
		const uint8_t * image, uint32_t size, inv_bool_t verify, inv_bool_t force);
		
int INV_EXPORT inv_device_icm20948_ping_sensor(void * context, int sensor);

int INV_EXPORT inv_device_icm20948_enable_sensor(void * context, int sensor, inv_bool_t en);

int INV_EXPORT inv_device_icm20948_set_sensor_period_us(void * context,
		int sensor, uint32_t period);

int INV_EXPORT inv_device_icm20948_set_sensor_timeout(void * context,
		int sensor, uint32_t timeout);

// int inv_device_icm20948_flush_sensor(void * context, int sensor);

// int inv_device_icm20948_set_sensor_bias(void * context, int sensor,
// 		const float bias[3]);

// int inv_device_icm20948_get_sensor_bias(void * context, int sensor,
// 		float bias[3]);

int INV_EXPORT inv_device_icm20948_set_sensor_mounting_matrix(void * context,
		int sensor, const float matrix[9]);

// int inv_device_icm20948_get_sensor_data(void * context, int sensor,
// 		inv_sensor_event_t * event);

int INV_EXPORT inv_device_icm20948_poll(void * context);

int INV_EXPORT inv_device_icm20948_self_test(void * context, int sensor);

int INV_EXPORT inv_device_icm20948_write_mems_register(void * context, int sensor, uint16_t reg_addr,
		const void * data, unsigned size);

int INV_EXPORT inv_device_icm20948_read_mems_register(void * context, int sensor, uint16_t reg_addr,
		void * data, unsigned size);

void INV_EXPORT inv_device_icm20948_init_aux_compass(inv_device_icm20948_t * self,
	int aux_compass_id, uint8_t aux_compass_addr);

int INV_EXPORT inv_device_icm20948_set_sensor_config(void * context, int sensor, int setting,
	const void * value, unsigned size);

int INV_EXPORT inv_device_icm20948_get_sensor_config(void * context, int sensor, int setting,
		void *value_out, unsigned size);

#ifdef __cplusplus
}
#endif

#endif /* _INV_DEVICE_ICM20948_H_ */


/** @} */
