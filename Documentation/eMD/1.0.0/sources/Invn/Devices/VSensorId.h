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

#ifndef _INV_VSensorId_H_
#define _INV_VSensorId_H_

#include "Invn/InvExport.h"
#include "SensorTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file VSensorType.h
 * Temporary file that will be replaced by this one from VSensor-framework when VSensorID will be well aligned.
 */

/**
 * Temporary.
 * Allow to use only VSensorID any where, without to manage differently SensorType(from IDD) & VSensorID(from VSensorFwk).
 *   - Symbol names follow VSensorID names (from VSensorFwk)
 *     as it doesn't matter when we are using IDD directly,
 *     only making sense when generating the custom firmware.
 *   - Symbol values follow SensorType values (from IDD)
 *     as we don't use values when generating custom firmware.
 */
enum VSensorId
{
	VSENSOR_ID_NONE                   = INV_SENSOR_TYPE_RESERVED,
	VSENSOR_ID_ACCELEROMETER          = INV_SENSOR_TYPE_ACCELEROMETER,
	VSENSOR_ID_MAGNETOMETER           = INV_SENSOR_TYPE_MAGNETOMETER,
	VSENSOR_ID_ORIENTATION            = INV_SENSOR_TYPE_ORIENTATION,
	VSENSOR_ID_GYROSCOPE              = INV_SENSOR_TYPE_GYROSCOPE,
	VSENSOR_ID_LIGHT                  = INV_SENSOR_TYPE_LIGHT,
	VSENSOR_ID_PRESSURE               = INV_SENSOR_TYPE_PRESSURE,
	VSENSOR_ID_TEMPERATURE            = INV_SENSOR_TYPE_TEMPERATURE,
	VSENSOR_ID_PROXIMITY              = INV_SENSOR_TYPE_PROXIMITY,
	VSENSOR_ID_GRAVITY                = INV_SENSOR_TYPE_GRAVITY,
	VSENSOR_ID_LINEAR_ACCELERATION    = INV_SENSOR_TYPE_LINEAR_ACCELERATION,
	VSENSOR_ID_ROTATION_VECTOR        = INV_SENSOR_TYPE_ROTATION_VECTOR,
	VSENSOR_ID_HUMIDITY               = INV_SENSOR_TYPE_HUMIDITY,
	VSENSOR_ID_UNCAL_MAGNETOMETER     = INV_SENSOR_TYPE_UNCAL_MAGNETOMETER,
	VSENSOR_ID_AMBIENT_TEMPERATURE    = INV_SENSOR_TYPE_AMBIENT_TEMPERATURE,
	VSENSOR_ID_GAME_ROTATION_VECTOR   = INV_SENSOR_TYPE_GAME_ROTATION_VECTOR,
	VSENSOR_ID_UNCAL_GYROSCOPE        = INV_SENSOR_TYPE_UNCAL_GYROSCOPE,
	VSENSOR_ID_SMD                    = INV_SENSOR_TYPE_SMD,
	VSENSOR_ID_STEP_DETECTOR          = INV_SENSOR_TYPE_STEP_DETECTOR,
	VSENSOR_ID_STEP_COUNTER           = INV_SENSOR_TYPE_STEP_COUNTER,
	VSENSOR_ID_GEOMAG_ROTATION_VECTOR = INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR,
	VSENSOR_ID_HEART_RATE             = INV_SENSOR_TYPE_HEART_RATE,
	VSENSOR_ID_TILT_DETECTOR          = INV_SENSOR_TYPE_TILT_DETECTOR,
	VSENSOR_ID_WAKE_GESTURE           = INV_SENSOR_TYPE_WAKE_GESTURE,
	VSENSOR_ID_GLANCE_GESTURE         = INV_SENSOR_TYPE_GLANCE_GESTURE,
	VSENSOR_ID_PICK_UP_GESTURE        = INV_SENSOR_TYPE_PICK_UP_GESTURE,
	VSENSOR_ID_RAW_ACCELEROMETER      = INV_SENSOR_TYPE_RAW_ACCELEROMETER,
	VSENSOR_ID_RAW_MAGNETOMETER       = INV_SENSOR_TYPE_RAW_MAGNETOMETER,
	VSENSOR_ID_RAW_GYROSCOPE          = INV_SENSOR_TYPE_RAW_GYROSCOPE,
	VSENSOR_ID_RAW_TEMPERATURE        = INV_SENSOR_TYPE_RAW_TEMPERATURE,
	VSENSOR_ID_BAC                    = INV_SENSOR_TYPE_BAC,
	VSENSOR_ID_B2S                    = INV_SENSOR_TYPE_B2S,
	VSENSOR_ID_PDR                    = INV_SENSOR_TYPE_PDR,
	VSENSOR_ID_ACC_ROTATION_VECTOR    = INV_SENSOR_TYPE_3AXIS,
	VSENSOR_ID_RAW_PPG                = INV_SENSOR_TYPE_RAW_PPG,
	VSENSOR_ID_HRV                    = INV_SENSOR_TYPE_HRV,
	VSENSOR_ID_SLEEP_ANALYSIS         = INV_SENSOR_TYPE_SLEEP_ANALYSIS,
	VSENSOR_ID_AAR_STATISTICS         = INV_SENSOR_TYPE_BAC_STATISTICS,
	VSENSOR_ID_ENERGY_EXPENDITURE     = INV_SENSOR_TYPE_ENERGY_EXPENDITURE,
	VSENSOR_ID_FLOOR_CLIMB_COUNTER    = INV_SENSOR_TYPE_FLOOR_CLIMB_COUNTER,
	VSENSOR_ID_AAR_WEARABLE           = INV_SENSOR_TYPE_BAC_EXTENDED,
	VSENSOR_ID_DOUBLE_TAP             = INV_SENSOR_TYPE_DOUBLE_TAP,
	VSENSOR_ID_DISTANCE               = INV_SENSOR_TYPE_DISTANCE,
	VSENSOR_ID_TS_IMU                 = INV_SENSOR_TYPE_TSIMU,
	VSENSOR_ID_SEDENTARY_REMIND       = INV_SENSOR_TYPE_SEDENTARY_REMIND,
	VSENSOR_ID_DATA_ENCRYPTION        = INV_SENSOR_TYPE_DATA_ENCRYPTION,
	VSENSOR_ID_WOM                    = INV_SENSOR_TYPE_WOM,
	VSENSOR_ID_CUSTOM_0               = INV_SENSOR_TYPE_CUSTOM0,
	VSENSOR_ID_CUSTOM_1               = INV_SENSOR_TYPE_CUSTOM1,
	VSENSOR_ID_CUSTOM_2               = INV_SENSOR_TYPE_CUSTOM2,
	VSENSOR_ID_CUSTOM_3               = INV_SENSOR_TYPE_CUSTOM3,
	VSENSOR_ID_CUSTOM_4               = INV_SENSOR_TYPE_CUSTOM4,
	VSENSOR_ID_CUSTOM_5               = INV_SENSOR_TYPE_CUSTOM5,
	VSENSOR_ID_CUSTOM_6               = INV_SENSOR_TYPE_CUSTOM6,
	VSENSOR_ID_CUSTOM_7               = INV_SENSOR_TYPE_CUSTOM7,
	/* not yet present from VSensorID:*/
	VSENSOR_ID_EIS                    = INV_SENSOR_TYPE_EIS,
	VSENSOR_ID_OIS                    = INV_SENSOR_TYPE_OIS,
	VSENSOR_ID_CUSTOM_PRESSURE        = INV_SENSOR_TYPE_CUSTOM_PRESSURE,
	VSENSOR_ID_MIC                    = INV_SENSOR_TYPE_MIC,
	VSENSOR_ID_SHAKE                  = INV_SENSOR_TYPE_SHAKE,
	VSENSOR_ID_FSYNC_EVENT            = INV_SENSOR_TYPE_FSYNC_EVENT,
	VSENSOR_ID_HIGH_RATE_GYRO         = INV_SENSOR_TYPE_HIGH_RATE_GYRO,
	VSENSOR_ID_CUSTOM_BSCD            = INV_SENSOR_TYPE_CUSTOM_BSCD,
	VSENSOR_ID_HRM_LOGGER             = INV_SENSOR_TYPE_HRM_LOGGER,
	/* Starting from there, the SensorID is coded with more than 6bits so check that communication protocol is adequate */
	VSENSOR_ID_PREDICTIVE_QUATERNION  = INV_SENSOR_TYPE_PREDICTIVE_QUATERNION,
	VSENSOR_ID_MAX                    = INV_SENSOR_TYPE_MAX
};

/**
 * Retrieve the name of a sensor according to its type.
 * The wake-up flag is ignored.
 * @param sensor Sensor type as VSensorId enumeration.
 * @return Name of the sensor (without prefix, underscored upper cases)
 *         if sensor type is valid, otherwise an empty string.
 */
const char INV_EXPORT * inv_sensor_id_name(unsigned int sensor);

/**
 * Retrieve the type of a sensor according to its name.
 * @param sensor_name Name of the sensor (following VSensorType enumeration items
 *                    without the prefix 'VSENSOR_ID_').
 * @return Sensor type value if the name is valid, otherwise 0 is returned.
 */
unsigned int INV_EXPORT inv_sensor_id_value(const char* sensor_name);

#ifdef __cplusplus
}
#endif

#endif // _INV_VSensorId_H_
