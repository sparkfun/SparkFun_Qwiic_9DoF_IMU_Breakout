/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2015-2016 InvenSense Inc. All rights reserved.
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

/** @defgroup VSensorType VSensorType
 * 	@brief    Definitions for standard VSensor type
 *
 *  Any VSensor is associated to a type, allowing to identify and properly
 *  interpret data it generates.
 *
 *  Expected data type is also given in brackets.
 *
 *	@ingroup VSensorFwk
 *  @{
 */

#ifndef _V_SENSOR_TYPE_H_
#define _V_SENSOR_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Reserved value - should not be used
 */
#define VSENSOR_TYPE_UNKNOWN                           0

/** @brief 3-axis accelerometer (VSensorDataAccelerometer)
 */
#define VSENSOR_TYPE_ACCELEROMETER                     1

/** @brief 3-axis magnetometer (VSensorDataMagnetometer)
 */
#define VSENSOR_TYPE_MAGNETOMETER                      2

/** @brief Orientation as defined by Android (VSensorData3dAngles)
 */
#define VSENSOR_TYPE_ORIENTATION                       3

/** @brief 3-axis Gyroscope sensor (VSensorDataGyroscope)
 */
#define VSENSOR_TYPE_GYROSCOPE                         4

/** @brief Ambient light sensor (VSensorDataLight)
 */
#define VSENSOR_TYPE_LIGHT                             5

/** @brief Barometer (VSensorDataPressure)
 */
#define VSENSOR_TYPE_PRESSURE                          6

/** @brief Temperature (VSensorDataTemperature)
 */
#define VSENSOR_TYPE_TEMPERATURE                       7

/** @brief Proximity (VSensorDataProximity)
 */
#define VSENSOR_TYPE_PROXIMITY                         8

/** @brief Gravity (VSensorDataGravity)
 */
#define VSENSOR_TYPE_GRAVITY                           9

/** @brief Linear acceleration (VSensorDataLinearAcc)
 */
#define VSENSOR_TYPE_LINEAR_ACCELERATION               10

/** @brief 9-axis quaternion (A,G,M) (VSensorDataQuaternion)
 */
#define VSENSOR_TYPE_ROTATION_VECTOR                   11

/** @brief Relative humidity (VSensorDataHumidity)
 */
#define VSENSOR_TYPE_HUMIDITY                          12

/** @brief Raw magnetic field with bias (VSensorDataMagnetometerUncal)
 */
#define VSENSOR_TYPE_UNCAL_MAGNETOMETER                13

/** @brief Ambient temperature (VSensorDataTemperature)
 */
#define VSENSOR_TYPE_AMBIENT_TEMPERATURE               14

/** @brief 6-axis quaternion based on Acc and Gyr (VSensorDataQuaternion)
 */
#define VSENSOR_TYPE_GAME_ROTATION_VECTOR              15

/** @brief Uncalibrated gyroscope (VSensorDataGyroscopeUncal)
 */
#define VSENSOR_TYPE_UNCAL_GYROSCOPE                   16

/** @brief Significant motion detection (VSensorData)
 */
#define VSENSOR_TYPE_SMD                               17

/** @brief Step detector (VSensorData)
 */
#define VSENSOR_TYPE_STEP_DETECTOR                     18

/** @brief Step counter (VSensorDataStepCount)
 */
#define VSENSOR_TYPE_STEP_COUNTER                      19

/** @brief 6-axis quaternion based on Acc and Mag (VSensorDataQuaternion)
 */
#define VSENSOR_TYPE_GEOMAG_ROTATION_VECTOR            20

/** @brief Heart rate (VSensorDataHeartRate)
 */
#define VSENSOR_TYPE_HEART_RATE                        21

/** @brief Tilt detector (VSensorData)
 */
#define VSENSOR_TYPE_TILT_DETECTOR                     22

/** @brief Wake-up gesture (VSensorData)
 */
#define VSENSOR_TYPE_WAKE_GESTURE                      23

/** @brief Glance gesture (VSensorData)
 */
#define VSENSOR_TYPE_GLANCE_GESTURE                    24

/** @brief Pick-up gesture (VSensorData)
 */
#define VSENSOR_TYPE_PICK_UP_GESTURE                   25

/** @brief 3-axis raw accelerometer without bias (VSensorDataRaw3d)
 */
#define VSENSOR_TYPE_RAW_ACCELEROMETER                 26

/** @brief 3-axis raw magnetometer without bias (VSensorDataRaw3d)
 */
#define VSENSOR_TYPE_RAW_MAGNETOMETER                  27

/** @brief 3-axis raw gyroscope without bias (VSensorDataRaw3d)
 */
#define VSENSOR_TYPE_RAW_GYROSCOPE                     28

/** @brief Basic activity classifier (VSensorDataAAR)
 */
#define VSENSOR_TYPE_AAR                               29
#define VSENSOR_TYPE_BAC                               VSENSOR_TYPE_AAR

/** @brief Bring to see gesture (VSensorData)
 */
#define VSENSOR_TYPE_B2S                               30

/** @brief Invn PDR sensor (reserved)
 */
#define VSENSOR_TYPE_PDR                               31

/** @brief 3-axis quaternion based on Acc (VSensorDataQuaternion)
 */
#define VSENSOR_TYPE_ACC_ROTATION_VECTOR               32

/** @brief 1-axis raw temperature value (VSensorDataRaw1d)
 */
#define VSENSOR_TYPE_RAW_TEMPERATURE                   33

/*
 * Custom sensor range
 */

#define VSENSOR_TYPE_CUSTOM_MIN  48                        

/** @brief Maximum allowed value to designate a custom sensor
 */
#define VSENSOR_TYPE_CUSTOM_MAX  VSENSOR_TYPE_CUSTOM(8)    

/** @brief Helper macro to designate a custom sensor
 */
#define VSENSOR_TYPE_CUSTOM(num)	(VSENSOR_TYPE_CUSTOM_MIN+(num))

/** @brief Maximum value for VSensor type
 */
#define VSENSOR_TYPE_MAX 64                     

/*
 * For backward compatibility only - do not use
 */

#define VSENSOR_TYPE_GYROMETER         VSENSOR_TYPE_GYROSCOPE
#define VSENSOR_TYPE_RAW_GYROMETER     VSENSOR_TYPE_RAW_GYROSCOPE
#define VSENSOR_TYPE_UNCAL_GYROMETER   VSENSOR_TYPE_UNCAL_GYROSCOPE

#ifdef __cplusplus
}
#endif

#endif /* _V_SENSOR_TYPE_H_ */

/** @} **/
