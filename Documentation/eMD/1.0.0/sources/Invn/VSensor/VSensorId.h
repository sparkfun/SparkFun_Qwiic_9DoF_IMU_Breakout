/*
    Copyright (c) 2014-2015 InvenSense Inc. Portions Copyright (c) 2014-2015 Movea. All rights reserved.

    This software, related documentation and any modifications thereto (collectively "Software") is subject
    to InvenSense and its licensors' intellectual property rights under U.S. and international copyright and
    other intellectual property rights laws.

    InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
    and any use, reproduction, disclosure or distribution of the Software without an express license
    agreement from InvenSense is strictly prohibited.
*/

/** @defgroup 	VSensorId	VSensorId 
 * 	@brief 		Definitions for standard VSensor ID
 *  @deprecated Use VSensorType.h
 *
 *  Only intended to be used with SensorStudio. Its sole purpose is to maintain compatibility 
 *  with former projects.
 *
 *  VSensorId were removed from the VSensor framework and replaced by VSensor type.
 *
 *  @ingroup 	VSensorFwk
 *  @{
 */

#ifndef _V_SENSOR_ID_H_
#define _V_SENSOR_ID_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Invn/VSensor/VSensorType.h"

enum VSensorId {
	VSENSOR_ID_NONE                   = VSENSOR_TYPE_UNKNOWN,
	VSENSOR_ID_ACCELEROMETER          = VSENSOR_TYPE_ACCELEROMETER,
	VSENSOR_ID_MAGNETOMETER           = VSENSOR_TYPE_MAGNETOMETER,
	VSENSOR_ID_ORIENTATION            = VSENSOR_TYPE_ORIENTATION,
	VSENSOR_ID_GYROSCOPE              = VSENSOR_TYPE_GYROSCOPE,
	VSENSOR_ID_LIGHT                  = VSENSOR_TYPE_LIGHT,
	VSENSOR_ID_PRESSURE               = VSENSOR_TYPE_PRESSURE,
	VSENSOR_ID_TEMPERATURE            = VSENSOR_TYPE_TEMPERATURE,
	VSENSOR_ID_PROXIMITY              = VSENSOR_TYPE_PROXIMITY,
	VSENSOR_ID_GRAVITY                = VSENSOR_TYPE_GRAVITY,
	VSENSOR_ID_LINEAR_ACCELERATION    = VSENSOR_TYPE_LINEAR_ACCELERATION,
	VSENSOR_ID_ROTATION_VECTOR        = VSENSOR_TYPE_ROTATION_VECTOR,
	VSENSOR_ID_HUMIDITY               = VSENSOR_TYPE_HUMIDITY,
	VSENSOR_ID_UNCAL_MAGNETOMETER     = VSENSOR_TYPE_UNCAL_MAGNETOMETER,
	VSENSOR_ID_AMBIENT_TEMPERATURE    = VSENSOR_TYPE_AMBIENT_TEMPERATURE,
	VSENSOR_ID_GAME_ROTATION_VECTOR   = VSENSOR_TYPE_GAME_ROTATION_VECTOR,
	VSENSOR_ID_UNCAL_GYROSCOPE        = VSENSOR_TYPE_UNCAL_GYROSCOPE,
	VSENSOR_ID_SMD                    = VSENSOR_TYPE_SMD,
	VSENSOR_ID_STEP_DETECTOR          = VSENSOR_TYPE_STEP_DETECTOR,
	VSENSOR_ID_STEP_COUNTER           = VSENSOR_TYPE_STEP_COUNTER,
	VSENSOR_ID_GEOMAG_ROTATION_VECTOR = VSENSOR_TYPE_GEOMAG_ROTATION_VECTOR,
	VSENSOR_ID_HEART_RATE             = VSENSOR_TYPE_HEART_RATE,
	VSENSOR_ID_TILT_DETECTOR          = VSENSOR_TYPE_TILT_DETECTOR,
	VSENSOR_ID_WAKE_GESTURE           = VSENSOR_TYPE_WAKE_GESTURE,
	VSENSOR_ID_GLANCE_GESTURE         = VSENSOR_TYPE_GLANCE_GESTURE,
	VSENSOR_ID_PICK_UP_GESTURE        = VSENSOR_TYPE_PICK_UP_GESTURE,
	VSENSOR_ID_RAW_ACCELEROMETER      = VSENSOR_TYPE_RAW_ACCELEROMETER,
	VSENSOR_ID_RAW_MAGNETOMETER       = VSENSOR_TYPE_RAW_MAGNETOMETER,
	VSENSOR_ID_RAW_GYROSCOPE          = VSENSOR_TYPE_RAW_GYROSCOPE,
	VSENSOR_ID_RAW_TEMPERATURE        = VSENSOR_TYPE_TEMPERATURE,
	VSENSOR_ID_BAC                    = VSENSOR_TYPE_BAC,
	VSENSOR_ID_B2S                    = VSENSOR_TYPE_B2S,
	VSENSOR_ID_PDR                    = VSENSOR_TYPE_PDR,
	VSENSOR_ID_ACC_ROTATION_VECTOR    = VSENSOR_TYPE_ACC_ROTATION_VECTOR,

	/* custom sensor range */
	VSENSOR_ID_CUSTOM_0               = VSENSOR_TYPE_CUSTOM(0),
	VSENSOR_ID_CUSTOM_1               = VSENSOR_TYPE_CUSTOM(1),
	VSENSOR_ID_CUSTOM_2               = VSENSOR_TYPE_CUSTOM(2),
	VSENSOR_ID_CUSTOM_3               = VSENSOR_TYPE_CUSTOM(3),
	VSENSOR_ID_CUSTOM_4               = VSENSOR_TYPE_CUSTOM(4),
	VSENSOR_ID_CUSTOM_5               = VSENSOR_TYPE_CUSTOM(5),
	VSENSOR_ID_CUSTOM_6               = VSENSOR_TYPE_CUSTOM(6),
	VSENSOR_ID_CUSTOM_7               = VSENSOR_TYPE_CUSTOM(7),

	VSENSOR_ID_MAX                      
};

/*  Helper defines to access custom sensor range
 */
#define VSENSOR_ID_CUSTOM_BASE     VSENSOR_ID_CUSTOM_0
#define VSENSOR_ID_CUSTOM_MAX      (VSENSOR_ID_CUSTOM_7+1)

/*
 * For backward compatibility only - do not use
 */
#define VSENSOR_ID_GYROMETER       VSENSOR_ID_GYROSCOPE
#define VSENSOR_ID_RAW_GYROMETER   VSENSOR_ID_RAW_GYROSCOPE
#define VSENSOR_ID_UNCAL_GYROMETER VSENSOR_ID_UNCAL_GYROSCOPE

#ifdef __cplusplus
}
#endif

#endif /* _V_SENSOR_ID_H_ */

/** @} **/
