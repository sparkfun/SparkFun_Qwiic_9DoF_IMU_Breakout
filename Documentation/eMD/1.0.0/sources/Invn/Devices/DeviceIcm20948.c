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

#include "DeviceIcm20948.h"
#include "Invn/InvExport.h"
#include "Invn/EmbUtils/Message.h"

static const inv_device_vt_t device_icm20948_vt = {
	inv_device_icm20948_whoami,
	inv_device_icm20948_reset,
	inv_device_icm20948_setup,
	inv_device_icm20948_cleanup,
	inv_device_icm20948_load,
	inv_device_icm20948_poll,
	inv_device_icm20948_self_test,
	0,//inv_device_icm20948_get_fw_info,
	inv_device_icm20948_ping_sensor,
	0,//inv_device_icm20948_set_running_state,
	inv_device_icm20948_enable_sensor,
	inv_device_icm20948_set_sensor_period_us,
	inv_device_icm20948_set_sensor_timeout,
	0,//inv_device_icm20948_flush_sensor,
	0,//inv_device_icm20948_set_sensor_bias,
	0,//inv_device_icm20948_get_sensor_bias,
	inv_device_icm20948_set_sensor_mounting_matrix,
	0,//inv_device_icm20948_get_sensor_data,
	inv_device_icm20948_set_sensor_config,
	inv_device_icm20948_get_sensor_config,
	inv_device_icm20948_write_mems_register,
	inv_device_icm20948_read_mems_register,
};

static enum inv_icm20948_sensor idd_sensortype_2_driver(int sensor)
{
	switch(sensor) {
	case INV_SENSOR_TYPE_RAW_ACCELEROMETER:       return INV_ICM20948_SENSOR_RAW_ACCELEROMETER;
	case INV_SENSOR_TYPE_RAW_GYROSCOPE:           return INV_ICM20948_SENSOR_RAW_GYROSCOPE;
	case INV_SENSOR_TYPE_ACCELEROMETER:           return INV_ICM20948_SENSOR_ACCELEROMETER;
	case INV_SENSOR_TYPE_GYROSCOPE:               return INV_ICM20948_SENSOR_GYROSCOPE;
	case INV_SENSOR_TYPE_UNCAL_MAGNETOMETER:      return INV_ICM20948_SENSOR_MAGNETIC_FIELD_UNCALIBRATED;
	case INV_SENSOR_TYPE_UNCAL_GYROSCOPE:         return INV_ICM20948_SENSOR_GYROSCOPE_UNCALIBRATED;
	case INV_SENSOR_TYPE_BAC:                     return INV_ICM20948_SENSOR_ACTIVITY_CLASSIFICATON;
	case INV_SENSOR_TYPE_STEP_DETECTOR:           return INV_ICM20948_SENSOR_STEP_DETECTOR;
	case INV_SENSOR_TYPE_STEP_COUNTER:            return INV_ICM20948_SENSOR_STEP_COUNTER;
	case INV_SENSOR_TYPE_GAME_ROTATION_VECTOR:    return INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR;
	case INV_SENSOR_TYPE_ROTATION_VECTOR:         return INV_ICM20948_SENSOR_ROTATION_VECTOR;
	case INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR:  return INV_ICM20948_SENSOR_GEOMAGNETIC_ROTATION_VECTOR;
	case INV_SENSOR_TYPE_MAGNETOMETER:            return INV_ICM20948_SENSOR_GEOMAGNETIC_FIELD;
	case INV_SENSOR_TYPE_SMD:                     return INV_ICM20948_SENSOR_WAKEUP_SIGNIFICANT_MOTION;
	case INV_SENSOR_TYPE_PICK_UP_GESTURE:         return INV_ICM20948_SENSOR_FLIP_PICKUP;
	case INV_SENSOR_TYPE_TILT_DETECTOR:           return INV_ICM20948_SENSOR_WAKEUP_TILT_DETECTOR;
	case INV_SENSOR_TYPE_GRAVITY:                 return INV_ICM20948_SENSOR_GRAVITY;
	case INV_SENSOR_TYPE_LINEAR_ACCELERATION:     return INV_ICM20948_SENSOR_LINEAR_ACCELERATION;
	case INV_SENSOR_TYPE_ORIENTATION:             return INV_ICM20948_SENSOR_ORIENTATION;
	case INV_SENSOR_TYPE_B2S:                     return INV_ICM20948_SENSOR_B2S;
	default:                                      return INV_ICM20948_SENSOR_MAX;
	}
}

static int idd_driver_2_sensortype(enum inv_icm20948_sensor sensor)
{
	switch(sensor) {
	case INV_ICM20948_SENSOR_ACCELEROMETER:                    return INV_SENSOR_TYPE_ACCELEROMETER;
	case INV_ICM20948_SENSOR_GYROSCOPE:                        return INV_SENSOR_TYPE_GYROSCOPE;
	case INV_ICM20948_SENSOR_RAW_ACCELEROMETER:                return INV_SENSOR_TYPE_RAW_ACCELEROMETER;
	case INV_ICM20948_SENSOR_RAW_GYROSCOPE:                    return INV_SENSOR_TYPE_RAW_GYROSCOPE;
	case INV_ICM20948_SENSOR_MAGNETIC_FIELD_UNCALIBRATED:      return INV_SENSOR_TYPE_UNCAL_MAGNETOMETER;
	case INV_ICM20948_SENSOR_GYROSCOPE_UNCALIBRATED:           return INV_SENSOR_TYPE_UNCAL_GYROSCOPE;
	case INV_ICM20948_SENSOR_ACTIVITY_CLASSIFICATON:           return INV_SENSOR_TYPE_BAC;
	case INV_ICM20948_SENSOR_STEP_DETECTOR:                    return INV_SENSOR_TYPE_STEP_DETECTOR;
	case INV_ICM20948_SENSOR_STEP_COUNTER:                     return INV_SENSOR_TYPE_STEP_COUNTER;
	case INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR:             return INV_SENSOR_TYPE_GAME_ROTATION_VECTOR;
	case INV_ICM20948_SENSOR_ROTATION_VECTOR:                  return INV_SENSOR_TYPE_ROTATION_VECTOR;
	case INV_ICM20948_SENSOR_GEOMAGNETIC_ROTATION_VECTOR:      return INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR;
	case INV_ICM20948_SENSOR_GEOMAGNETIC_FIELD:                return INV_SENSOR_TYPE_MAGNETOMETER;
	case INV_ICM20948_SENSOR_WAKEUP_SIGNIFICANT_MOTION:        return INV_SENSOR_TYPE_SMD;
	case INV_ICM20948_SENSOR_FLIP_PICKUP:                      return INV_SENSOR_TYPE_PICK_UP_GESTURE;
	case INV_ICM20948_SENSOR_WAKEUP_TILT_DETECTOR:             return INV_SENSOR_TYPE_TILT_DETECTOR;
	case INV_ICM20948_SENSOR_GRAVITY:                          return INV_SENSOR_TYPE_GRAVITY;
	case INV_ICM20948_SENSOR_LINEAR_ACCELERATION:              return INV_SENSOR_TYPE_LINEAR_ACCELERATION;
	case INV_ICM20948_SENSOR_ORIENTATION:                      return INV_SENSOR_TYPE_ORIENTATION;
	case INV_ICM20948_SENSOR_B2S:                              return INV_SENSOR_TYPE_B2S;
	default:                                                   return INV_SENSOR_TYPE_MAX;
	}
}

static void data_handler(void * context, enum inv_icm20948_sensor sensor, uint64_t timestamp,
		const void * data, const void *arg);

static inv_bool_t build_sensor_event(inv_device_icm20948_t * self,
		int sensorid, uint64_t timestamp, const void * data, const void *arg,
		inv_sensor_event_t * event);

static int host_serif_read_reg_legacy(void * context, uint8_t reg, uint8_t * data, uint32_t len)
{
	inv_host_serif_t * serif = (inv_host_serif_t *)context;

	return serif->read_reg(reg, data, len);
}

/* For BW compatibility
 * wrapper function to adapt prototype of write_reg() as defined in inv_host_serif
 * to expected prototype for inv_serif_hal
 */
static int host_serif_write_reg_legacy(void * context, uint8_t reg, const uint8_t * data, uint32_t len)
{
	inv_host_serif_t * serif = (inv_host_serif_t *)context;

	return serif->write_reg(reg, data, len);
}

void inv_device_icm20948_init(inv_device_icm20948_t * self,
		const inv_host_serif_t * serif, const inv_sensor_listener_t * listener,
		const uint8_t  * dmp3_image, uint32_t dmp3_image_size)
{
	/* create an a inv_serif_hal_t object from a inv_host_serif_t */
	const inv_serif_hal_t serif_hal = {
		host_serif_read_reg_legacy, host_serif_write_reg_legacy, /* use small wrappers to adapt prototype */
		serif->max_read_size, serif->max_write_size,
		serif->serif_type, &self->legacy_serif
	};

	/* call the 'constructor' */
	inv_device_icm20948_init2(self, &serif_hal, listener, dmp3_image, dmp3_image_size);
	/* keep a copy of the user inv_host_serif_t (used in the _legacy callbacks) */
	self->legacy_serif = *serif;
}

void inv_device_icm20948_init2(inv_device_icm20948_t * self,
		const inv_serif_hal_t * serif, const inv_sensor_listener_t * listener,
		const uint8_t  * dmp3_image, uint32_t dmp3_image_size)
{
	struct inv_icm20948_serif icm20948_serif;

	assert(self);

	memset(self, 0, sizeof(*self));
	
	/* initialize icm20948 serif structure */
	icm20948_serif.context   = serif->context;
	icm20948_serif.read_reg  = serif->read_reg;
	icm20948_serif.write_reg = serif->write_reg;
	icm20948_serif.max_read  = serif->max_read_transaction_size;
	icm20948_serif.max_write = serif->max_write_transaction_size;
	icm20948_serif.is_spi    = !!(serif->serif_type == INV_SERIF_HAL_TYPE_SPI);
	
	/* build base */
	self->base.instance = self;
	self->base.vt       = &device_icm20948_vt;
	self->base.listener = listener;
	self->dmp3_image = dmp3_image;
	self->dmp3_image_size = dmp3_image_size;
	/* reset icm20948 driver states */
	inv_icm20948_reset_states(&self->icm20948_states, &icm20948_serif);
	
	/* initialise mounting matrix to identity */
	self->icm20948_states.mounting_matrix[0] = 1;
	self->icm20948_states.mounting_matrix[4] = 1;
	self->icm20948_states.mounting_matrix[8] = 1;

	/*initialise base state structure*/
	inv_icm20948_init_structure(&self->icm20948_states);
	
#if (MEMS_CHIP == HW_ICM20948)
	/*force the usage of akm9916 for 20948*/
	inv_device_icm20948_init_aux_compass( self, INV_ICM20948_COMPASS_ID_AK09916, 0x0C);
#endif
}

int inv_device_icm20948_poll(void * context)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;

	return inv_icm20948_poll_sensor(&self->icm20948_states, self, data_handler);
}

int inv_device_icm20948_whoami(void * context, uint8_t * whoami)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;

	assert(whoami);

	return inv_icm20948_get_whoami(&self->icm20948_states, whoami);
}

int inv_device_icm20948_reset(void * context)
{
	int rc = 0;

	INV_MSG(INV_MSG_LEVEL_VERBOSE, "Reseting device...");
	rc |= inv_device_icm20948_cleanup(context);
	rc |= inv_device_icm20948_setup(context);
	
		if(rc != 0) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "Icm20648 reset returned %d", rc);
		return rc;
	}

	return 0;
}

int inv_device_icm20948_setup(void * context)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;
	int rc;
	uint8_t whoami;

	INV_MSG(INV_MSG_LEVEL_INFO, "Booting up Icm20948...");

	/* Check WHOAMI */
	INV_MSG(INV_MSG_LEVEL_VERBOSE, "Reading WHOAMI...");
	if((rc = inv_device_icm20948_whoami(self, &whoami)) != 0) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "Error %d when reading WHOAMI value", rc);
		return rc;
	}

	if(whoami == 0 || whoami == 0xff) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "Unexpected WHOAMI value 0x%x. Aborting setup.", whoami);
		return INV_ERROR;
	} else {
		INV_MSG(INV_MSG_LEVEL_INFO, "WHOAMI value: 0x%x", whoami);
	}
	/* Setup accel and gyro mounting matrix and associated angle for current board */
	inv_icm20948_init_matrix(&self->icm20948_states);

	/* set default power mode */
	INV_MSG(INV_MSG_LEVEL_VERBOSE, "Putting Icm20948 in sleep mode...");
	if((rc = inv_icm20948_initialize(&self->icm20948_states, self->dmp3_image,
			self->dmp3_image_size)) != 0)
		goto error;
 
	/* Initialize auxiliary sensors */
	inv_icm20948_initialize_auxiliary(&self->icm20948_states);

	inv_icm20948_init_scale(&self->icm20948_states);

	/* re-initialise base state structure */
	inv_icm20948_init_structure(&self->icm20948_states);
	
	/* we should be good to go ! */
	INV_MSG(INV_MSG_LEVEL_VERBOSE, "We're good to go !");

	return 0;
error:
	INV_MSG(INV_MSG_LEVEL_ERROR, "Error %d while setting-up device.", rc);

	return rc;
}

int inv_device_icm20948_cleanup(void * context)
{
	int i = 0;
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;

	/* Disable all supported sensors */
	for (i=0; i<INV_SENSOR_TYPE_MAX; ++i) {
		if (inv_device_icm20948_ping_sensor(context, i) == 0)
			inv_device_icm20948_enable_sensor(context, i, 0);
	}	
	
	return inv_icm20948_soft_reset(&self->icm20948_states);
}

int inv_device_icm20948_load(void * context, int what,
		const uint8_t * image, uint32_t size, inv_bool_t verify, inv_bool_t force)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;
	(void)what;
	(void)verify;
	(void)force;

	return inv_icm20948_load(&self->icm20948_states, image, size);
}

int inv_device_icm20948_self_test(void * context, int sensor)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;
	int rc;

	if(inv_device_icm20948_ping_sensor(context, sensor) != 0)
		return INV_ERROR_BAD_ARG;

	if((sensor != INV_SENSOR_TYPE_RAW_ACCELEROMETER) &&
			(sensor != INV_SENSOR_TYPE_ACCELEROMETER) &&
			(sensor != INV_SENSOR_TYPE_RAW_GYROSCOPE) &&
			(sensor != INV_SENSOR_TYPE_GYROSCOPE) &&
			(sensor != INV_SENSOR_TYPE_MAGNETOMETER))
			//(sensor != INV_SENSOR_TYPE_RAW_MAGNETOMETER) AxL add rawmag
		return INV_ERROR_BAD_ARG;

	if(self->icm20948_states.selftest_done) {
		INV_MSG(INV_MSG_LEVEL_WARNING, "Self-test already ran once!");
		return INV_ERROR_SUCCESS;
	}

	/* Reset the device in case the self-test doesn't run at start */
	inv_device_icm20948_cleanup(context);
	if((inv_icm20948_run_selftest(&self->icm20948_states) & INV_ICM20948_SELF_TEST_OK) 
			== INV_ICM20948_SELF_TEST_OK) {
		self->icm20948_states.selftest_done = 1;
		rc = 0;
	}
	else
		rc = INV_ERROR;

	/* It's advised to re-init the device after self-test for normal use */
	inv_device_icm20948_reset(context);
	return rc;
}

int inv_device_icm20948_ping_sensor(void * context, int sensor)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;
	
	/* HW sensors */
	if( (sensor == INV_SENSOR_TYPE_RAW_ACCELEROMETER) ||
	    (sensor == INV_SENSOR_TYPE_RAW_GYROSCOPE) ||
	    (sensor == INV_SENSOR_TYPE_GAME_ROTATION_VECTOR) ||
	    (sensor == INV_SENSOR_TYPE_ACCELEROMETER) ||
	    (sensor == INV_SENSOR_TYPE_GYROSCOPE) ||
	    (sensor == INV_SENSOR_TYPE_UNCAL_GYROSCOPE) ||
	    (sensor == INV_SENSOR_TYPE_SMD) ||
	    (sensor == INV_SENSOR_TYPE_STEP_DETECTOR) ||
	    (sensor == INV_SENSOR_TYPE_STEP_COUNTER) ||
	    (sensor == INV_SENSOR_TYPE_B2S) ||
	    (sensor == INV_SENSOR_TYPE_PICK_UP_GESTURE) ||
	    (sensor == INV_SENSOR_TYPE_BAC) ||
	    (sensor == INV_SENSOR_TYPE_GRAVITY) ||
	    (sensor == INV_SENSOR_TYPE_LINEAR_ACCELERATION) ||
	    (sensor == INV_SENSOR_TYPE_TILT_DETECTOR) ) {
		return 0;
	} else if( (sensor == INV_SENSOR_TYPE_MAGNETOMETER) ||
	           (sensor == INV_SENSOR_TYPE_UNCAL_MAGNETOMETER) ||
	           (sensor == INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR) ||
	           (sensor == INV_SENSOR_TYPE_ORIENTATION) ||
	           (sensor == INV_SENSOR_TYPE_ROTATION_VECTOR) ) {
		if(inv_icm20948_compass_isconnected(&self->icm20948_states))
			return 0;
	}

	return INV_ERROR_BAD_ARG;
}

int inv_device_icm20948_enable_sensor(void * context, int sensor, inv_bool_t en)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;

	return inv_icm20948_enable_sensor(&self->icm20948_states, idd_sensortype_2_driver(sensor), en);
}

int inv_device_icm20948_set_sensor_period_us(void * context,
		int sensor, uint32_t period)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;

	/* convert period back to ms as this is what expects the driver for now */
	period /= 1000;

	return inv_icm20948_set_sensor_period(&self->icm20948_states, idd_sensortype_2_driver(sensor), period);
}

int inv_device_icm20948_set_sensor_timeout(void * context,
		int sensor, uint32_t period)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;
	(void)sensor;

	return inv_icm20948_enable_batch_timeout(&self->icm20948_states, period);
}

int inv_device_icm20948_set_sensor_mounting_matrix(void * context,
		int sensor, const float matrix[9])
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;
	
	return inv_icm20948_set_matrix(&self->icm20948_states, matrix, idd_sensortype_2_driver(sensor));
}

int inv_device_icm20948_write_mems_register(void * context, int sensor, uint16_t reg_addr,
		const void * data, unsigned size)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;

	(void)sensor;

	return inv_icm20948_write_reg(&self->icm20948_states, (uint8_t)reg_addr, (uint8_t*)data, size);
}

int inv_device_icm20948_read_mems_register(void * context, int sensor, uint16_t reg_addr,
		void * data, unsigned size)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;

	(void)sensor;

	return inv_icm20948_read_reg(&self->icm20948_states, (uint8_t)reg_addr, (uint8_t*)data, size);
}

void inv_device_icm20948_init_aux_compass(inv_device_icm20948_t * self,
	int aux_compass_id, uint8_t aux_compass_addr)
{
	/* register auxiliary compass (assuming AK09911) */
	inv_icm20948_register_aux_compass(&self->icm20948_states,
			(enum inv_icm20948_compass_id)aux_compass_id, aux_compass_addr);
}

int inv_device_icm20948_set_sensor_config(void * context, int sensor, int setting,
		const void * value, unsigned size)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;
	int rc = 0;
	(void)size;
	
	switch (setting) {
		case INV_DEVICE_ICM20948_CONFIG_FSR : {
			int temp_bias[3];
			//In case FSR change, we save and apply new bias
			inv_icm20948_get_bias(&self->icm20948_states, idd_sensortype_2_driver(sensor), temp_bias);
			rc = inv_icm20948_set_fsr(&self->icm20948_states, idd_sensortype_2_driver(sensor), value);
			inv_icm20948_set_bias(&self->icm20948_states, idd_sensortype_2_driver(sensor), temp_bias);
			break;
		}
		case INV_DEVICE_ICM20948_CONFIG_POWER_MODE :
			return inv_icm20948_set_lowpower_or_highperformance(&self->icm20948_states, *((uint8_t *)value));
		case INV_DEVICE_ICM20948_CONFIG_OFFSET :
			return inv_icm20948_set_bias(&self->icm20948_states, idd_sensortype_2_driver(sensor), value);
		/*case INV_DEVICE_ICM20948_CONFIG_WOM_THRESHOLD: //AxL
			switch(sensor) {
			case INV_SENSOR_TYPE_WOM:
				return inv_icm20948_set_wom_threshold(&self->icm20948_states, (uint8_t)(*(const inv_device_icm20948_config_wom_threshold_t *)value));
			default:
				return -1;
			}*/
		default :
			return -1;
	}
	return rc;
}

int inv_device_icm20948_get_sensor_config(void * context, int sensor, int setting,
		void *value_out, unsigned size)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;
	(void)size;
	switch (setting) {
		case INV_DEVICE_ICM20948_CONFIG_FSR :
			return inv_icm20948_get_fsr(&self->icm20948_states, idd_sensortype_2_driver(sensor), value_out);
		case INV_DEVICE_ICM20948_CONFIG_POWER_MODE :
			return inv_icm20948_get_lowpower_or_highperformance(&self->icm20948_states, value_out);
		case INV_DEVICE_ICM20948_CONFIG_OFFSET :
			return inv_icm20948_get_bias(&self->icm20948_states, idd_sensortype_2_driver(sensor), value_out);
		default :
			return -1;
	}
}
/******************************************************************************/

static void data_handler(void * context, enum inv_icm20948_sensor sensor, uint64_t timestamp,
		const void * data, const void *arg)
{
	inv_device_icm20948_t * self = (inv_device_icm20948_t *)context;
	inv_sensor_event_t event;
	const int sensortype = idd_driver_2_sensortype(sensor);
	
	if(build_sensor_event(self, sensortype, timestamp, data, arg, &event)) {
		inv_sensor_listener_notify(self->base.listener, &event);
	}
}

static inv_bool_t build_sensor_event_data(inv_device_icm20948_t * self, 
		uint8_t sensortype, const void * data, const void *arg, 
		inv_sensor_event_t * event)
{
	float raw_bias_data[6];
	(void)self;

	switch(sensortype) {
	case INV_SENSOR_TYPE_UNCAL_GYROSCOPE:
		memcpy(raw_bias_data, data, sizeof(raw_bias_data));
		memcpy(event->data.gyr.vect, &raw_bias_data[0], sizeof(event->data.gyr.vect));
		memcpy(event->data.gyr.bias, &raw_bias_data[3], sizeof(event->data.gyr.bias));
		memcpy(&(event->data.gyr.accuracy_flag), arg, sizeof(event->data.gyr.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_UNCAL_MAGNETOMETER:
		memcpy(raw_bias_data, data, sizeof(raw_bias_data));
		memcpy(event->data.mag.vect, &raw_bias_data[0], sizeof(event->data.mag.vect));
		memcpy(event->data.mag.bias, &raw_bias_data[3], sizeof(event->data.mag.bias));
		memcpy(&(event->data.gyr.accuracy_flag), arg, sizeof(event->data.gyr.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_GYROSCOPE:
		memcpy(event->data.gyr.vect, data, sizeof(event->data.gyr.vect));
		memcpy(&(event->data.gyr.accuracy_flag), arg, sizeof(event->data.gyr.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_GRAVITY:
	case INV_SENSOR_TYPE_LINEAR_ACCELERATION:
	case INV_SENSOR_TYPE_ACCELEROMETER:
		memcpy(event->data.acc.vect, data, sizeof(event->data.acc.vect));
		memcpy(&(event->data.acc.accuracy_flag), arg, sizeof(event->data.acc.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_MAGNETOMETER:
		memcpy(event->data.mag.vect, data, sizeof(event->data.mag.vect));
		memcpy(&(event->data.mag.accuracy_flag), arg, sizeof(event->data.mag.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR:
	case INV_SENSOR_TYPE_ROTATION_VECTOR:
		memcpy(&(event->data.quaternion.accuracy), arg, sizeof(event->data.quaternion.accuracy));
		memcpy(event->data.quaternion.quat, data, sizeof(event->data.quaternion.quat));
		break;
	case INV_SENSOR_TYPE_GAME_ROTATION_VECTOR:
		memcpy(event->data.quaternion.quat, data, sizeof(event->data.quaternion.quat));
		break;
	case INV_SENSOR_TYPE_BAC:
		memcpy(&(event->data.bac.event), data, sizeof(event->data.bac.event));
		break;
	case INV_SENSOR_TYPE_PICK_UP_GESTURE:
	case INV_SENSOR_TYPE_TILT_DETECTOR:
	case INV_SENSOR_TYPE_STEP_DETECTOR:
	case INV_SENSOR_TYPE_SMD:
	case INV_SENSOR_TYPE_B2S:
		event->data.event = true;
		break;
	case INV_SENSOR_TYPE_STEP_COUNTER:
		memcpy(&(event->data.step.count), data, sizeof(event->data.step.count));
		break;
	case INV_SENSOR_TYPE_ORIENTATION:
		//we just want to copy x,y,z from orientation data
		memcpy(&(event->data.orientation), data, 3*sizeof(float));
		break;
	case INV_SENSOR_TYPE_RAW_ACCELEROMETER:
	case INV_SENSOR_TYPE_RAW_GYROSCOPE:
		memcpy(event->data.raw3d.vect, data, sizeof(event->data.raw3d.vect));
		break;
	default:
		return false;
	}

	return true;
}

static inv_bool_t build_sensor_event(inv_device_icm20948_t * self,
		int sensorid, uint64_t timestamp, const void * data, const void *arg,
		inv_sensor_event_t * event)
{
	assert(event);

	memset(event, 0, sizeof(*event));

	(void)self;

	if(!build_sensor_event_data(self, sensorid, data, arg, event)) {
		INV_MSG(INV_MSG_LEVEL_WARNING, "Unexpected sensor id %d. Data Ignored.", sensorid);
		return false;
	}

	/* finish up building event */
	event->sensor	= sensorid;
	event->timestamp = timestamp;
	event->status	= INV_SENSOR_STATUS_DATA_UPDATED;

	return true;
}
