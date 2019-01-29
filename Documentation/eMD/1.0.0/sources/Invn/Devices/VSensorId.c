//-----------------------------------------------------------------------------
/*
	Copyright © 2016 InvenSense Inc. All rights reserved.
	This software, related documentation and any modifications thereto collectively “Software” is subject
	to InvenSense and its licensors' intellectual property rights under U.S. and international copyright and
	other intellectual property rights laws.
	InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
	and any use, reproduction, disclosure or distribution of the Software without an express license
	agreement from InvenSense is strictly prohibited.
*/
//-----------------------------------------------------------------------------

#include <string.h>
#include "VSensorId.h"

//-----------------------------------------------------------------------------

#define _STR(x) #x

#define _INV_IMPL_SENSOR_TYPE_CONV(_F) \
	_F(ACCELEROMETER) \
	_F(MAGNETOMETER) \
	_F(ORIENTATION) \
	_F(GYROSCOPE) \
	_F(LIGHT) \
	_F(PRESSURE) \
	_F(TEMPERATURE) \
	_F(PROXIMITY) \
	_F(GRAVITY) \
	_F(LINEAR_ACCELERATION) \
	_F(ROTATION_VECTOR) \
	_F(HUMIDITY) \
	_F(AMBIENT_TEMPERATURE) \
	_F(UNCAL_MAGNETOMETER) \
	_F(GAME_ROTATION_VECTOR) \
	_F(UNCAL_GYROSCOPE) \
	_F(SMD) \
	_F(STEP_DETECTOR) \
	_F(STEP_COUNTER) \
	_F(GEOMAG_ROTATION_VECTOR) \
	_F(HEART_RATE) \
	_F(TILT_DETECTOR) \
	_F(WAKE_GESTURE) \
	_F(GLANCE_GESTURE) \
	_F(PICK_UP_GESTURE) \
	_F(RAW_ACCELEROMETER) \
	_F(RAW_MAGNETOMETER) \
	_F(RAW_GYROSCOPE) \
	_F(RAW_TEMPERATURE) \
	_F(BAC) \
	_F(B2S) \
	_F(PDR) \
	_F(ACC_ROTATION_VECTOR) \
	_F(RAW_PPG) \
	_F(HRV) \
	_F(SLEEP_ANALYSIS) \
	_F(AAR_STATISTICS) \
	_F(ENERGY_EXPENDITURE) \
	_F(FLOOR_CLIMB_COUNTER) \
	_F(AAR_WEARABLE) \
	_F(DOUBLE_TAP) \
	_F(DISTANCE) \
	_F(TS_IMU) \
	_F(SEDENTARY_REMIND) \
	_F(DATA_ENCRYPTION) \
	_F(WOM) \
	_F(CUSTOM_0) \
	_F(CUSTOM_1) \
	_F(CUSTOM_2) \
	_F(CUSTOM_3) \
	_F(CUSTOM_4) \
	_F(CUSTOM_5) \
	_F(CUSTOM_6) \
	_F(CUSTOM_7) \
	/* missing from VSensorID: */ \
	_F(EIS) \
	_F(OIS) \
	_F(CUSTOM_PRESSURE) \
	_F(MIC) \
	_F(SHAKE) \
	_F(FSYNC_EVENT) \
	_F(HIGH_RATE_GYRO) \
	_F(CUSTOM_BSCD) \
	_F(HRM_LOGGER) \
	_F(PREDICTIVE_QUATERNION) \

//-----------------------------------------------------------------------------

#define _INV_CASE_SENSOR_TYPE_TO_NAME(_SensorType) \
	case VSENSOR_ID_ ## _SensorType: return _STR(_SensorType);

const char* inv_sensor_id_name(unsigned int sensor)
{
	switch(INV_SENSOR_ID_TO_TYPE(sensor)) {
	_INV_IMPL_SENSOR_TYPE_CONV(_INV_CASE_SENSOR_TYPE_TO_NAME)
	}
	return "";
}

//-----------------------------------------------------------------------------

#define _INV_CASE_SENSOR_TYPE_FROM_NAME(_SensorType) \
	if (!strcmp(sensor_name,_STR(_SensorType))) return VSENSOR_ID_ ## _SensorType;

unsigned int inv_sensor_id_value(const char* sensor_name)
{
	_INV_IMPL_SENSOR_TYPE_CONV(_INV_CASE_SENSOR_TYPE_FROM_NAME)
	return 0;
}

//-----------------------------------------------------------------------------
