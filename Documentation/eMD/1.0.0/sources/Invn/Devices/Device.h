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

/** @defgroup Device Device
 *	@brief    Abstract device interface definition
 *
 *            All functions declared in this file are virtual.
 *            They aim to provide a unified way of accessing InvenSense devices.
 *            All functions shall return a int for which 0 indicates success and
 *            a negative value indicates an error as described by enum inv_error
 *
 *            If a particular device implementation does not support any of the
 *            method declared here, it shall return INV_ERROR_NIMPL.
 *
 *            Implementation is not expected to be thread-safe.
 *
 *            Refer to concrete device implementation for additionnal and specific
 *            information about API usage related to a particular device.
 *
 *	@{
 */

#ifndef _INV_DEVICE_H_
#define _INV_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Invn/InvBool.h"
#include "Invn/InvError.h"
#include "Invn/Devices/SensorTypes.h"
#include "Invn/Devices/SensorConfig.h"
#include "Invn/Devices/HostSerif.h"

#include <assert.h>

/** @brief FW version structure definition
 */
typedef struct inv_fw_version
{
	uint8_t  major, minor, patch; /**< major, minor, patch version number */
	char     suffix[16];          /**< version suffix string (always terminated by '\0') */
	uint32_t crc;                 /**< FW checksum */
} inv_fw_version_t;

/** @brief Device virtual table definition
 */
typedef struct inv_device_vt {
	int (*whoami)(void * self, uint8_t * whoami);
	int (*reset)(void * self);
	int (*setup)(void * self);
	int (*cleanup)(void * self);
	int (*load)(void * self, int type, const uint8_t * image, uint32_t size,
			inv_bool_t verify, inv_bool_t force);
	int (*poll)(void * self);
	int (*self_test)(void * self, int sensor);
	int (*get_fw_info)(void * self, struct inv_fw_version * version);
	int (*ping_sensor)(void * self, int sensor);
	int (*set_running_state)(void * self, inv_bool_t state);
	int (*enable_sensor)(void * self, int sensor, inv_bool_t start);
	int (*set_sensor_period_us)(void * self, int sensor, uint32_t period);
	int (*set_sensor_timeout)(void * self, int sensor, uint32_t timeout);
	int (*flush_sensor)(void * self, int sensor);
	int (*set_sensor_bias)(void * self, int sensor, const float bias[3]);
	int (*get_sensor_bias)(void * self, int sensor, float bias[3]);
	int (*set_sensor_mounting_matrix)(void * self, int sensor, const float matrix[9]);
	int (*get_sensor_data)(void * self, int sensor, inv_sensor_event_t * event);
	int (*set_sensor_config)(void * self, int sensor, int setting, const void * arg, unsigned size);
	int (*get_sensor_config)(void * self, int sensor, int setting, void * arg, unsigned size);
	int (*write_mems_register)(void * self, int sensor, uint16_t reg_addr, const void * value, unsigned size);
	int (*read_mems_register)(void * self, int sensor, uint16_t reg_addr, void * value, unsigned size);
} inv_device_vt_t;

/** @brief Abtract device object definition
 */
typedef struct inv_device
{
	void *                        instance; /**< pointer to object instance */
	const struct inv_device_vt *  vt;       /**< pointer to object virtual table */
	const inv_sensor_listener_t * listener; /**< pointer to listener instance */
} inv_device_t;

/** @brief Gets WHO AM I value
 *
 *  Can be called before performing device setup
 *
 *  @param[in]  dev     pointer to device object instance
 *  @param[out] whoami  WHO AM I value
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 */
static inline int inv_device_whoami(const inv_device_t * dev, uint8_t * whoami)
{
	assert(dev && dev->vt);

	if(dev->vt->whoami)
		return dev->vt->whoami(dev->instance, whoami);

	return INV_ERROR_NIMPL;
}

/** @brief Resets the device to a known state
 *
 *  Will perform an HW and SW reset of device, and reset internal driver states
 *  To know value.
 *  Should be called before setup or when device state is unknown.
 *
 *  @param[in] dev      pointer to device object instance
 *  @return             0 on success
 *                      INV_ERROR_TIMEOUT if reset does not complete in time
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 */
static inline int inv_device_reset(const inv_device_t * dev)
{
	assert(dev && dev->vt);

	if(dev->vt->reset)
		return dev->vt->reset(dev->instance);

	return INV_ERROR_NIMPL;
}

/** @brief Performs basic device initialization
 *
 *  Except if device's flash memory is outdated, devuie should be able to handle
 *  request after setup() is complete.
 *  If devices's flash memory need to be updated, load_begin()/load_continue()/
 *  load_end() methods must be called first with suitable argument.
 *  
 *  @param[in] dev      pointer to device object instance
 *  @return             0 on success
 *                      INV_ERROR_TIMEOUT if setup does not complete in time
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 */
static inline int inv_device_setup(const inv_device_t * dev)
{
	assert(dev && dev->vt);

	if(dev->vt->setup)
		return dev->vt->setup(dev->instance);

	return INV_ERROR_NIMPL;
}

/** @brief Shutdown the device and clean-up internal states
 *
 *  @param[in] dev      pointer to device object instance
 *  @return             0 on success
 *                      INV_ERROR_TIMEOUT if clean-up does not complete in time
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 */
static inline int inv_device_cleanup(const inv_device_t * dev)
{
	assert(dev && dev->vt);

	if(dev->vt->cleanup)
		return dev->vt->cleanup(dev->instance);

	return INV_ERROR_NIMPL;
}

/** @brief Polls the device for data
 *
 *  Will read device interrupt registers and data registers or FIFOs.
 *  Will parse data and called sensor events handler provided at init time. 
 *  Handler will be called in the same context of this function.
 * 
 *  @warning Care should be taken regarding concurency. If this function is
 *           called in a dedicated thread, suitable protection must be used to
 *           avoid concurent calls to poll() or any other device methods.
 *
 *  @param[in] dev      pointer to device object instance
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_UNEXPECTED in case of bad formated or 
 *                                           un-handled data frame
 */
static inline int inv_device_poll(const inv_device_t * dev)
{
	assert(dev && dev->vt);

	if(dev->vt->poll)
		return dev->vt->poll(dev->instance);

	return INV_ERROR_NIMPL;
}

/** @brief Begins loading procedure for device's image(s)
 *
 *  Will start the process of loading an image to device's memory.
 *  Type of images to load will depend on the device type and/or FW version.
 *
 *  @param[in] dev      pointer to device object instance
 *  @param[in] type     type of image to load. Can vary from one implementation
 *                      to another. Refer to specific implementation for details.
 *  @param[in] image    pointer to image (or image chunk) data
 *  @param[in] size     size of image (or size of image chunk)
 *  @param[in] verify   true to perform image integrity verification, false to skip it
 *  @param[in] force    true to load image even if identical to current image,
 *                      false to compare image first
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_TIMEOUT if device does not respond in time
 *                      INV_ERROR_SIZE if image size does not fit in device memory
 */
static inline int inv_device_load(const inv_device_t * dev, int type,
		const uint8_t * image, uint32_t size, inv_bool_t verify, inv_bool_t force)
{
	assert(dev && dev->vt);

	if(dev->vt->load)
		return dev->vt->load(dev->instance, type, image, size, verify, force);

	return INV_ERROR_NIMPL;
}

/** @brief Gets device FW version
 *
 *  @param[in]  dev     pointer to device object instance
 *  @param[out] version version structure placeholder
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_TIMEOUT if device does not respond in time
 */
static inline int inv_device_get_fw_info(const inv_device_t * dev,
		struct inv_fw_version * version)
{
	assert(dev && dev->vt);

	if(dev->vt->get_fw_info)
		return dev->vt->get_fw_info(dev->instance, version);

	return INV_ERROR_NIMPL;
}

/** @brief Indicates to device current RUN/SUSPEND state of the host
 *
 *  If SUSPEND state (false) is set, device should not notify any sensor events
 *  (besides event comming from a wake-up source). If RUNNING state (true) is
 *  set, all sensor events will be notify to host.
 *  Device will consider host to be in RUNNING state after a reset/setup.
 * 
 *  @param[in] dev      pointer to device object instance
 *  @param[in] state    RUNNING (true) or SUSPEND (false) state
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 */
static inline int inv_device_set_running_state(const inv_device_t * dev, inv_bool_t state)
{
	if(dev->vt->set_running_state)
		return dev->vt->set_running_state(dev->instance, state);

	return INV_ERROR_NIMPL;
}

/** @brief Checks if a sensor is supported by the device
 *
 *  @param[in] dev      pointer to device object instance
 *  @param[in] sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_TIMEOUT if device does not respond in time
 *                      INV_ERROR if sensor is not supported by the device
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 */
static inline int inv_device_ping_sensor(const inv_device_t * dev, int sensor)
{
	assert(dev && dev->vt);
	
	if(dev->vt->ping_sensor)
		return dev->vt->ping_sensor(dev->instance, sensor);

	return INV_ERROR_NIMPL;
}

/** @brief Enable/Disable a sensor
 *
 *  Send a command to start or stop a sensor.
 *  See inv_device_start_sensor() and inv_device_stop_sensor()
 *
 *  @param[in] dev      pointer to device object instance
 *  @param[in] sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[in] start    true to start the sensor, false to stop the sensor
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 */
static inline int inv_device_enable_sensor(const inv_device_t * dev, int sensor, inv_bool_t start)
{
	assert(dev && dev->vt);

	if(dev->vt->enable_sensor)
		return dev->vt->enable_sensor(dev->instance, sensor, start);

	return INV_ERROR_NIMPL;
}

/** @brief Alias of inv_device_enable_sensor() for backward compatibility
 *  @deprecated use inv_device_enable_sensor
 */
#define inv_device_enable inv_device_enable_sensor

/** @brief Starts a sensor
 * 
 *  Send a command to start a sensor. Device will start sending events if sensor
 *  is supported (ie: ping() returns 0 for this sensor type).
 *
 *  @param[in] dev      pointer to device object instance
 *  @param[in] sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 */
static inline int inv_device_start_sensor(const inv_device_t * dev, int sensor)
{
	assert(dev && dev->vt);

	if(dev->vt->enable_sensor)
		return dev->vt->enable_sensor(dev->instance, sensor, 1);

	return INV_ERROR_NIMPL;
}

/** @brief Stops a sensor
 * 
 *  Send a command to stop a sensor. Device will stop sending events if sensor
 *  was previously started.
 *
 *  @param[in] dev      pointer to device object instance
 *  @param[in] sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 */
static inline int inv_device_stop_sensor(const inv_device_t * dev, int sensor)
{
	assert(dev && dev->vt);

	if(dev->vt->enable_sensor)
		return dev->vt->enable_sensor(dev->instance, sensor, 0);

	return INV_ERROR_NIMPL;
}

/** @brief Configure sensor output data period.
 * 
 *  Send a command to set sensor output data period. Period is a hint only.
 *  Depending on sensor type or device capability, the effective output data
 *  might be different. User shall refer to sensor events timestamp to determine
 *  effective output data period.
 *
 *  @param[in] dev      pointer to device object instance
 *  @param[in] sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[in] period   requested data period in us
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 */
static inline int inv_device_set_sensor_period_us(const inv_device_t * dev,
		int sensor, uint32_t period)
{
	assert(dev && dev->vt);

	if(dev->vt->set_sensor_period_us)
		return dev->vt->set_sensor_period_us(dev->instance, sensor, period);

	return INV_ERROR_NIMPL;
}

/** @brief Configure sensor output data period.
 *
 *  Similar to inv_device_set_sensor_period_us() except period is specified in ms.
 *  Will simply call inv_device_set_sensor_period_us() after converting input period.
 *
 *  @param[in] dev      pointer to device object instance
 *  @param[in] sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[in] period   requested data period in ms
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 */
static inline int inv_device_set_sensor_period(const inv_device_t * dev,
		int sensor, uint32_t period)
{
	return inv_device_set_sensor_period_us(dev, sensor, period*1000U);
}

/** @brief Configure sensor output timeout.
 * 
 *  Send a command to set sensor maximum report latency (or batch timeout).
 *  This allows to enable batch mode. Provided timeout is a hint only and sensor
 *  events may be notified at a faster rate depending on sensor type or device
 *  capability or other active sensors.
 *
 *  @param[in] dev      pointer to device object instance
 *  @param[in] sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[in] timeout  allowed timeout in ms
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 */
static inline int inv_device_set_sensor_timeout(const inv_device_t * dev,
		int sensor, uint32_t timeout)
{
	assert(dev && dev->vt);

	if(dev->vt->set_sensor_timeout)
		return dev->vt->set_sensor_timeout(dev->instance, sensor, timeout);

	return INV_ERROR_NIMPL;
}

/** @brief Configure sensor output timeout.
 *
 *  Similar to inv_device_set_sensor_timeout() except period is specified in ms.
 *  Will simply call inv_device_set_sensor_timeout() after converting input period.
 *
 *  @warning If input timeout is < 1000, value will be rounded to 0.
 *
 *  @param[in] dev      pointer to device object instance
 *  @param[in] sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[in] timeout  allowed timeout in us
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 */
static inline int inv_device_set_sensor_timeout_us(const inv_device_t * dev,
		int sensor, uint32_t timeout)
{
	return inv_device_set_sensor_timeout(dev, sensor, timeout / 1000U);
}

/** @brief Forces flush of devices's internal buffers
 * 
 *  Send a command a flush command to device. 
 *  Device will imediatly send all sensor events that may be store in its
 *  internal buffers.
 *
 *  @param[in] dev      pointer to device object instance
 *  @param[in] sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 */
static inline int inv_device_flush_sensor(const inv_device_t * dev,
		int sensor)
{
	assert(dev && dev->vt);

	if(dev->vt->flush_sensor)
		return dev->vt->flush_sensor(dev->instance, sensor);

	return INV_ERROR_NIMPL;
}

/** @brief Configure bias value for a sensor
 * 
 *  Bias configuration makes sense only for few sensor types: 
 *   - INV_SENSOR_TYPE_ACCELEROMETER
 *   - INV_SENSOR_TYPE_MAGNETOMETER
 *   - INV_SENSOR_TYPE_GYROSCOPE
 *  Bias unit is the same as the corresponding sensor unit.
 *  @sa inv_sensor_event_t for details.
 *
 *  If this feature is supported by the implementation but not by the device,
 *  behavior is undefined (but will most probably have no effect).
 *
 *  @param[in] dev      pointer to device object instance
 *  @param[in] sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[in] bias     bias to set
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 */
static inline int inv_device_set_sensor_bias(const inv_device_t * dev,
		int sensor, const float bias[3])
{
	assert(dev && dev->vt);

	if(dev->vt->set_sensor_bias)
		return dev->vt->set_sensor_bias(dev->instance, sensor, bias);

	return INV_ERROR_NIMPL;
}

/** @brief Gets bias value for a sensor
 * 
 *  Bias configuration makes sense only for few sensor types: 
 *   - INV_SENSOR_TYPE_ACCELEROMETER
 *   - INV_SENSOR_TYPE_MAGNETOMETER
 *   - INV_SENSOR_TYPE_GYROSCOPE
 *  Bias unit is the same as the corresponding sensor unit.
 *  @sa inv_sensor_event_t for details.
 *
 *  @param[in]  dev     pointer to device object instance
 *  @param[in]  sensor  sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[out] bias    returned bias
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_TIMEOUT if device does not respond in time
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 */
static inline int inv_device_get_sensor_bias(const inv_device_t * dev,
		int sensor, float bias[3])
{
	assert(dev && dev->vt);

	if(dev->vt->get_sensor_bias)
		return dev->vt->get_sensor_bias(dev->instance, sensor, bias);

	return INV_ERROR_NIMPL;
}

/** @brief Sets the mounting matrix information for a multi-axis sensor
 * 
 *  Allow to specify the mounting matrix for multi-axis sensor in order to
 *  align axis of several sensors in the same reference frame.
 *  Sensor types allowed:
 *   - INV_SENSOR_TYPE_ACCELEROMETER
 *   - INV_SENSOR_TYPE_MAGNETOMETER
 *   - INV_SENSOR_TYPE_GYROSCOPE
 *  Depending on device capability, called to this function may have no effect.
 *
 *  @param[in]  dev     pointer to device object instance
 *  @param[in]  sensor  sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[in]  matrix  mounting matrix to apply
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_TIMEOUT if device does not respond in time
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 *						or if one of the mounting matrix value is not in the [-1;1] range
 */
static inline int inv_device_set_sensor_mounting_matrix(const inv_device_t * dev,
		int sensor, const float matrix[9])
{
	assert(dev && dev->vt);
	
	{
	uint8_t i; 
	for (i = 0; i < 9; i++) {
		if ((matrix[i] > 1) || (matrix[i] < -1))
			return INV_ERROR_BAD_ARG;
	}
	}
	
	if(dev->vt->set_sensor_mounting_matrix)
		return dev->vt->set_sensor_mounting_matrix(dev->instance, sensor, matrix);

	return INV_ERROR_NIMPL;
}

/** @brief Retrieve last known sensor event for a sensor
 * 
 *  Depending on device capability, a call to this function may have no effect or
 *  return an error.
 *
 *  @param[in]  dev     pointer to device object instance
 *  @param[in]  sensor  sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[out] event   last known event data
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_TIMEOUT if device does not respond in time
 *                      INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 *                      INV_ERROR if and event was received but unmanaged by the implementation
 */
static inline int inv_device_get_sensor_data(const inv_device_t * dev, int sensor,
		inv_sensor_event_t * event)
{
	assert(dev && dev->vt);

	if(dev->vt->get_sensor_data)
		return dev->vt->get_sensor_data(dev->instance, sensor, event);

	return INV_ERROR_NIMPL;
}

/** @brief Perform self-test procedure for MEMS component of the device
 * 
 *  Available MEMS vary depend on the device.
 *  Use following sensor type for the various MEMS:
 *   - INV_SENSOR_TYPE_ACCELEROMETER: for HW accelerometer sensor
 *   - INV_SENSOR_TYPE_MAGNETOMETER : for HW magnetometer sensor
 *   - INV_SENSOR_TYPE_GYROSCOPE    : for HW gyroscope sensor
 *   - INV_SENSOR_TYPE_PRESSURE     : for HW pressure sensor
 *
 *  @param[in]  dev     pointer to device object instance
 *  @param[in]  sensor  sensor type (as defined by @sa inv_sensor_type_t)
 *  @return             0 on success
 *                      INV_ERROR_TRANSPORT in case of low level serial error
 *                      INV_ERROR_TIMEOUT if device does not respond in time
 *                      INV_ERROR if self test has failed
 */
static inline int inv_device_self_test(const inv_device_t * dev, int sensor)
{
	assert(dev && dev->vt);

	if(dev->vt->self_test)
		return dev->vt->self_test(dev->instance, sensor);

	return INV_ERROR_NIMPL;
}

/** @brief Generic method to configure a sensor
 *
 *  Allow to configure a sensor (HW or virtual), such as FSR, BW, ...
 *
 *  For common settings, setting value is expected to be a value from @sa enum inv_sensor_config.
 *  Settings data is expected to point the proper type as describes in SensorConfig.h
 *
 *  For specific settings, refer to concrete device implementation for supported sensor and
 *  available configuration settings parameters.
 *
 *  @param[in]  dev      pointer to device object instance
 *  @param[in]  sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[in]  settings settings to configure
 *  @param[in]  arg      pointer to settings value
 *  @param[in]  size     settings value size
 *  @return              0 on sucess
 *                       INV_ERROR_TRANSPORT in case of low level serial error
 *                       INV_ERROR_TIMEOUT if device does not respond in time
 *                       INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 *                       INV_ERROR_SIZE size is above intenral buffer size / device capability
 *                       INV_ERROR if configuration has failed
 */
static inline int inv_device_set_sensor_config(const inv_device_t * dev,
		int sensor, int settings, const void * arg, uint16_t size)
{
	assert(dev && dev->vt);

	if(dev->vt->set_sensor_config)
		return dev->vt->set_sensor_config(dev->instance, sensor, settings, arg, size);

	return INV_ERROR_NIMPL;
}

/** @brief Generic method to retrieve configuration value for a sensor
 *
 *  For common settings, setting value is expected to be a value from @sa enum inv_sensor_config.
 *  Settings data is expected to point the proper type as describes in SensorConfig.h
 *
 *  For specific settings, refer to concrete device implementation for supported sensor and
 *  available configuration settings parameters.
 *
 *
 *  @param[in]  dev      pointer to device object instance
 *  @param[in]  sensor   sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[in]  settings settings to configure
 *  @param[out] value	 the value for the corresponding setting
 *  @param[in]  size	 maximum buffer size pointed by value
 *  @return              0 on success
 *                       >0 indicating success and the number of byte written to value
 *                       INV_ERROR_TRANSPORT in case of low level serial error
 *                       INV_ERROR_TIMEOUT if device does not respond in time
 *                       INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 *                       INV_ERROR_SIZE provided buffer is not big enough
 *                       INV_ERROR if configuration has failed
 */
static inline int inv_device_get_sensor_config(const inv_device_t * dev, int sensor,
		int settings, void *value, uint16_t size)
{
	assert(dev && dev->vt);

	if(dev->vt->get_sensor_config)
		return dev->vt->get_sensor_config(dev->instance, sensor, settings, value, size);

	return INV_ERROR_NIMPL;
}

/** @brief Set the MEMS register
 *
 *  @param[in]  dev         pointer to device object instance
 *  @param[in]  sensor      sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[in]  reg_addr    the register that should be written
 *  @param[in]  data        data to write at reg_addr
 *  @param[in]  length      length of data to write
 *  @return                 0 on success
 *                          INV_ERROR_TRANSPORT in case of low level serial error
 *                          INV_ERROR_TIMEOUT if device does not respond in time
 *                          INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 *                          INV_ERROR_SIZE request length is above device capability
 *                          INV_ERROR if configuration has failed
 */
static inline int inv_device_write_mems_register(const inv_device_t * dev, int sensor,
		uint16_t reg_addr, const void * data, uint16_t len)
{
	assert(dev && dev->vt);

	if(dev->vt->write_mems_register)
		return dev->vt->write_mems_register(dev->instance, sensor, reg_addr, data, len);

	return INV_ERROR_NIMPL;
}

/** @brief Read register of underlying MEMS or HW sensor
 *
 *  @param[in]  dev         pointer to device object instance
 *  @param[in]  sensor      sensor type (as defined by @sa inv_sensor_type_t)
 *  @param[in]  reg_addr    the register that should be read
 *  @param[in]  data        pointer to buffer to hold read data
 *  @param[in]  length      length of data to read
 *  @return                 0 on success
 *                          INV_ERROR_TRANSPORT in case of low level serial error
 *                          INV_ERROR_TIMEOUT if device does not respond in time
 *                          INV_ERROR_BAD_ARG if sensor is not supported by the implementation
 *                          INV_ERROR_SIZE request length is above device capability
 *                          INV_ERROR if configuration has failed
 */
static inline int inv_device_read_mems_register(const inv_device_t * dev, int sensor,
		uint16_t reg_addr, void * data, uint16_t len)
{
	assert(dev && dev->vt);

	if(dev->vt->read_mems_register)
		return dev->vt->read_mems_register(dev->instance, sensor, reg_addr, data, len);

	return INV_ERROR_NIMPL;
}

#ifdef __cplusplus
}
#endif

#endif /* _INV_DEVICE_H_ */

/** @} */
