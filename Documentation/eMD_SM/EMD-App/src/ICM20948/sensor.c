/*
* ________________________________________________________________________________________________________
* Copyright (c) 2017 InvenSense Inc. All rights reserved.
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
#include <asf.h>

/* InvenSense drivers and utils */
#include "Invn/Devices/Drivers/Icm20948/Icm20948.h"
#include "Invn/Devices/Drivers/Icm20948/Icm20948MPUFifoControl.h"
#include "Invn/Devices/Drivers/Ak0991x/Ak0991x.h"
#include "Invn/Devices/SensorTypes.h"
#include "Invn/Devices/SensorConfig.h"
#include "Invn/EmbUtils/InvScheduler.h"
#include "Invn/EmbUtils/RingByteBuffer.h"
#include "Invn/EmbUtils/Message.h"
#include "Invn/EmbUtils/ErrorHelper.h"
#include "Invn/EmbUtils/DataConverter.h"
#include "Invn/EmbUtils/RingBuffer.h"
#include "Invn/DynamicProtocol/DynProtocol.h"
#include "Invn/DynamicProtocol/DynProtocolTransportUart.h"

/* Atmel system */
#include "system.h"
/* TDK Sensor */
#include "sensor.h"

#include "ASF/sam/drivers/pio/pio.h"
#include "ASF/sam/drivers/pio/pio_handler.h"
#include "ASF/sam/drivers/twi/twi.h"
#include "ASF/sam/drivers/tc/tc.h"

#include "main.h"

static const uint8_t dmp3_image[] = {
#include "icm20948_img.dmp3a.h"
};

/*
* Just a handy variable to handle the icm20948 object
*/
inv_icm20948_t icm_device;

static const uint8_t EXPECTED_WHOAMI[] = { 0xEA }; /* WHOAMI value for ICM20948 or derivative */

/* FSR configurations */
int32_t cfg_acc_fsr = 4; // Default = +/- 4g. Valid ranges: 2, 4, 8, 16
int32_t cfg_gyr_fsr = 2000; // Default = +/- 2000dps. Valid ranges: 250, 500, 1000, 2000

/*
* Mounting matrix configuration applied for both Accel and Gyro
*/
static const float cfg_mounting_matrix[9]= {
	1.f, 0, 0,
	0, 1.f, 0,
	0, 0, 1.f
};


static uint8_t convert_to_generic_ids[INV_ICM20948_SENSOR_MAX] = {
	INV_SENSOR_TYPE_ACCELEROMETER,
	INV_SENSOR_TYPE_GYROSCOPE,
	INV_SENSOR_TYPE_RAW_ACCELEROMETER,
	INV_SENSOR_TYPE_RAW_GYROSCOPE,
	INV_SENSOR_TYPE_UNCAL_MAGNETOMETER,
	INV_SENSOR_TYPE_UNCAL_GYROSCOPE,
	INV_SENSOR_TYPE_BAC,
	INV_SENSOR_TYPE_STEP_DETECTOR,
	INV_SENSOR_TYPE_STEP_COUNTER,
	INV_SENSOR_TYPE_GAME_ROTATION_VECTOR,
	INV_SENSOR_TYPE_ROTATION_VECTOR,
	INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR,
	INV_SENSOR_TYPE_MAGNETOMETER,
	INV_SENSOR_TYPE_SMD,
	INV_SENSOR_TYPE_PICK_UP_GESTURE,
	INV_SENSOR_TYPE_TILT_DETECTOR,
	INV_SENSOR_TYPE_GRAVITY,
	INV_SENSOR_TYPE_LINEAR_ACCELERATION,
	INV_SENSOR_TYPE_ORIENTATION,
	INV_SENSOR_TYPE_B2S
};

/*
* Dynamic protocol and transport handles
*/
DynProtocol_t protocol;
DynProTransportUart_t transport;

/*
* Mask to keep track of enabled sensors
*/
static uint32_t enabled_sensor_mask = 0;

static void convert_sensor_event_to_dyn_prot_data(const inv_sensor_event_t * event, VSensorDataAny * vsensor_data);
static enum inv_icm20948_sensor idd_sensortype_conversion(int sensor);
static void icm20948_apply_mounting_matrix(void);
static void icm20948_set_fsr(void);
static uint8_t icm20948_get_grv_accuracy(void);

/*
* Sleep implementation for ICM20948
*/
void inv_icm20948_sleep(int ms) {
	delay_ms(ms);
}

void inv_icm20948_sleep_us(int us){
	delay_us(us);
}

int load_dmp3(void){
	int rc = 0;
	INV_MSG(INV_MSG_LEVEL_INFO, "Load DMP3 image");
	rc = inv_icm20948_load(&icm_device, dmp3_image, sizeof(dmp3_image));
	return rc;
}

static void icm20948_apply_mounting_matrix(void){
	int ii;

	for (ii = 0; ii < INV_ICM20948_SENSOR_MAX; ii++) {
		inv_icm20948_set_matrix(&icm_device, cfg_mounting_matrix, ii);
	}
}

static void icm20948_set_fsr(void){
	inv_icm20948_set_fsr(&icm_device, INV_ICM20948_SENSOR_RAW_ACCELEROMETER, (const void *)&cfg_acc_fsr);
	inv_icm20948_set_fsr(&icm_device, INV_ICM20948_SENSOR_ACCELEROMETER, (const void *)&cfg_acc_fsr);
	inv_icm20948_set_fsr(&icm_device, INV_ICM20948_SENSOR_RAW_GYROSCOPE, (const void *)&cfg_gyr_fsr);
	inv_icm20948_set_fsr(&icm_device, INV_ICM20948_SENSOR_GYROSCOPE, (const void *)&cfg_gyr_fsr);
	inv_icm20948_set_fsr(&icm_device, INV_ICM20948_SENSOR_GYROSCOPE_UNCALIBRATED, (const void *)&cfg_gyr_fsr);
}

int icm20948_sensor_setup(void){
	int rc;
	uint8_t i, whoami = 0xff;

	/*
	* Just get the whoami
	*/
	rc = inv_icm20948_get_whoami(&icm_device, &whoami);
	if (interface_is_SPI() == 0)	{		// If we're using I2C
		if (whoami == 0xff) {				// if whoami fails try the other I2C Address
			switch_I2C_to_revA();
			rc = inv_icm20948_get_whoami(&icm_device, &whoami);
		}
	}
	INV_MSG(INV_MSG_LEVEL_INFO, "ICM20948 WHOAMI value=0x%02x", whoami);

	/*
	* Check if WHOAMI value corresponds to any value from EXPECTED_WHOAMI array
	*/
	for(i = 0; i < sizeof(EXPECTED_WHOAMI)/sizeof(EXPECTED_WHOAMI[0]); ++i) {
		if(whoami == EXPECTED_WHOAMI[i]) {
			break;
		}
	}

	if(i == sizeof(EXPECTED_WHOAMI)/sizeof(EXPECTED_WHOAMI[0])) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "Bad WHOAMI value. Got 0x%02x.", whoami);
		return rc;
	}

	/* Setup accel and gyro mounting matrix and associated angle for current board */
	inv_icm20948_init_matrix(&icm_device);
	icm20948_apply_mounting_matrix();

	/* set default power mode */
	INV_MSG(INV_MSG_LEVEL_VERBOSE, "Putting Icm20948 in sleep mode...");
	rc = inv_icm20948_initialize(&icm_device, dmp3_image, sizeof(dmp3_image));
	if (rc != 0) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "Initialization failed. Error loading DMP3...");
		return rc;
	}

	/*
	* Configure and initialize the ICM20948 for normal use
	*/
	INV_MSG(INV_MSG_LEVEL_INFO, "Booting up icm20948...");

	/* Initialize auxiliary sensors */
	inv_icm20948_register_aux_compass( &icm_device, INV_ICM20948_COMPASS_ID_AK09916, AK0991x_DEFAULT_I2C_ADDR);
	rc = inv_icm20948_initialize_auxiliary(&icm_device);
	if (rc == -1) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "Compass not detected...");
	}

	icm20948_set_fsr();

	/* re-initialize base state structure */
	inv_icm20948_init_structure(&icm_device);

	/* we should be good to go ! */
	INV_MSG(INV_MSG_LEVEL_VERBOSE, "We're good to go !");

	return 0;
}

/*
* Helper function to check RC value and block program execution
*/
void check_rc(int rc, const char * msg_context){
	if(rc < 0) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "%s: error %d (%s)", msg_context, rc, inv_error_str(rc));
		while(1);
	}
}

/*
* IddWrapper protocol handler function
*
* Will dispatch command and send response back
*/
void iddwrapper_protocol_event_cb(
	enum DynProtocolEtype etype,
	enum DynProtocolEid eid,
	const DynProtocolEdata_t * edata,
	void * cookie
	){
		(void)cookie;

		static DynProtocolEdata_t resp_edata; /* static to take on .bss */
		static uint8_t respBuffer[256]; /* static to take on .bss */
		uint16_t respLen;

		switch(etype) {
		case DYN_PROTOCOL_ETYPE_CMD:
			resp_edata.d.response.rc = handle_command(eid, edata, &resp_edata);

			/* send back response */
			if(DynProtocol_encodeResponse(&protocol, eid, &resp_edata,
				respBuffer, sizeof(respBuffer), &respLen) != 0) {
					goto error_dma_buffer;
			}

			DynProTransportUart_tx(&transport, respBuffer, respLen);
			break;

		default:
			INV_MSG(INV_MSG_LEVEL_WARNING, "DeviceEmdWrapper: unexpected packet received. Ignored.");
			break; /* no suppose to happen */
		}
		return;

error_dma_buffer:
		INV_MSG(INV_MSG_LEVEL_WARNING, "iddwrapper_protocol_event_cb: encode error, response dropped");

		return;
}

/*
* IddWrapper transport handler function
*
* This function will:
*  - feed the Dynamic protocol layer to analyze for incoming CMD packet
*  - forward byte coming from transport layer to be send over uart to the host
*/
void iddwrapper_transport_event_cb(enum DynProTransportEvent e,
union DynProTransportEventData data, void * cookie){
	(void)cookie;

	int rc;
	int timeout = 5000; /* us */

	switch(e) {
	case DYN_PRO_TRANSPORT_EVENT_ERROR:
		INV_MSG(INV_MSG_LEVEL_ERROR, "ERROR event with value %d received from IddWrapper transport", data.error);
		break;

	case DYN_PRO_TRANSPORT_EVENT_PKT_SIZE:
		break;

	case DYN_PRO_TRANSPORT_EVENT_PKT_BYTE:
		/* Feed IddWrapperProtocol to look for packet */
		rc = DynProtocol_processPktByte(&protocol, data.pkt_byte);
		if(rc < 0) {
			INV_MSG(INV_MSG_LEVEL_DEBUG, "DynProtocol_processPktByte(%02x) returned %d", data.pkt_byte, rc);
		}
		break;

	case DYN_PRO_TRANSPORT_EVENT_PKT_END:
		break;

		/* forward buffer from EMD Transport, to the SERIAL */
	case DYN_PRO_TRANSPORT_EVENT_TX_START:
		break;

	case DYN_PRO_TRANSPORT_EVENT_TX_BYTE:
		while ((InvEMDFrontEnd_putcharHook(data.tx_byte) == EOF) && (timeout > 0)) {
			InvEMDFrontEnd_busyWaitUsHook(10);
			timeout -= 10;
		}
		break;

	case DYN_PRO_TRANSPORT_EVENT_TX_END:
		break;

	case DYN_PRO_TRANSPORT_EVENT_TX_START_DMA:
		break;
	}
}

static uint8_t icm20948_get_grv_accuracy(void){
	uint8_t accel_accuracy;
	uint8_t gyro_accuracy;

	accel_accuracy = (uint8_t)inv_icm20948_get_accel_accuracy();
	gyro_accuracy = (uint8_t)inv_icm20948_get_gyro_accuracy();
	return (min(accel_accuracy, gyro_accuracy));
}

void build_sensor_event_data(void * context, uint8_t sensortype, uint64_t timestamp, const void * data, const void *arg){
	float raw_bias_data[6];
	inv_sensor_event_t event;
	(void)context;
	uint8_t sensor_id = convert_to_generic_ids[sensortype];

	memset((void *)&event, 0, sizeof(event));
	event.sensor = sensor_id;
	event.timestamp = timestamp;
	switch(sensor_id) {
	case INV_SENSOR_TYPE_UNCAL_GYROSCOPE:
		memcpy(raw_bias_data, data, sizeof(raw_bias_data));
		memcpy(event.data.gyr.vect, &raw_bias_data[0], sizeof(event.data.gyr.vect));
		memcpy(event.data.gyr.bias, &raw_bias_data[3], sizeof(event.data.gyr.bias));
		memcpy(&(event.data.gyr.accuracy_flag), arg, sizeof(event.data.gyr.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_UNCAL_MAGNETOMETER:
		memcpy(raw_bias_data, data, sizeof(raw_bias_data));
		memcpy(event.data.mag.vect, &raw_bias_data[0], sizeof(event.data.mag.vect));
		memcpy(event.data.mag.bias, &raw_bias_data[3], sizeof(event.data.mag.bias));
		memcpy(&(event.data.gyr.accuracy_flag), arg, sizeof(event.data.gyr.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_GYROSCOPE:
		memcpy(event.data.gyr.vect, data, sizeof(event.data.gyr.vect));
		memcpy(&(event.data.gyr.accuracy_flag), arg, sizeof(event.data.gyr.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_GRAVITY:
		memcpy(event.data.acc.vect, data, sizeof(event.data.acc.vect));
		event.data.acc.accuracy_flag = inv_icm20948_get_accel_accuracy();
		break;
	case INV_SENSOR_TYPE_LINEAR_ACCELERATION:
	case INV_SENSOR_TYPE_ACCELEROMETER:
		memcpy(event.data.acc.vect, data, sizeof(event.data.acc.vect));
		memcpy(&(event.data.acc.accuracy_flag), arg, sizeof(event.data.acc.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_MAGNETOMETER:
		memcpy(event.data.mag.vect, data, sizeof(event.data.mag.vect));
		memcpy(&(event.data.mag.accuracy_flag), arg, sizeof(event.data.mag.accuracy_flag));
		break;
	case INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR:
	case INV_SENSOR_TYPE_ROTATION_VECTOR:
		memcpy(&(event.data.quaternion.accuracy), arg, sizeof(event.data.quaternion.accuracy));
		memcpy(event.data.quaternion.quat, data, sizeof(event.data.quaternion.quat));
		break;
	case INV_SENSOR_TYPE_GAME_ROTATION_VECTOR:
		memcpy(event.data.quaternion.quat, data, sizeof(event.data.quaternion.quat));
		event.data.quaternion.accuracy_flag = icm20948_get_grv_accuracy();
		break;
	case INV_SENSOR_TYPE_BAC:
		memcpy(&(event.data.bac.event), data, sizeof(event.data.bac.event));
		break;
	case INV_SENSOR_TYPE_PICK_UP_GESTURE:
	case INV_SENSOR_TYPE_TILT_DETECTOR:
	case INV_SENSOR_TYPE_STEP_DETECTOR:
	case INV_SENSOR_TYPE_SMD:
		event.data.event = true;
		break;
	case INV_SENSOR_TYPE_B2S:
		event.data.event = true;
		memcpy(&(event.data.b2s.direction), data, sizeof(event.data.b2s.direction));
		break;
	case INV_SENSOR_TYPE_STEP_COUNTER:
		memcpy(&(event.data.step.count), data, sizeof(event.data.step.count));
		break;
	case INV_SENSOR_TYPE_ORIENTATION:
		//we just want to copy x,y,z from orientation data
		memcpy(&(event.data.orientation), data, 3*sizeof(float));
		break;
	case INV_SENSOR_TYPE_RAW_ACCELEROMETER:
	case INV_SENSOR_TYPE_RAW_GYROSCOPE:
		memcpy(event.data.raw3d.vect, data, sizeof(event.data.raw3d.vect));
		break;
	default:
		return;
	}
	sensor_event(&event, NULL);
}

void sensor_event(const inv_sensor_event_t * event, void * arg){
	/* arg will contained the value provided at init time */
	(void)arg;

	/*
	* Encode sensor event and sent to host over UART through IddWrapper protocol
	*/
	static DynProtocolEdata_t async_edata; /* static to take on .bss */
	static uint8_t async_buffer[256]; /* static to take on .bss */
	uint16_t async_bufferLen;

	async_edata.sensor_id = event->sensor;
	async_edata.d.async.sensorEvent.status = DYN_PRO_SENSOR_STATUS_DATA_UPDATED;
	convert_sensor_event_to_dyn_prot_data(event, &async_edata.d.async.sensorEvent.vdata);

	if(DynProtocol_encodeAsync(&protocol,
		DYN_PROTOCOL_EID_NEW_SENSOR_DATA, &async_edata,
		async_buffer, sizeof(async_buffer), &async_bufferLen) != 0) {
			goto error_dma_buf;
	}

	DynProTransportUart_tx(&transport, async_buffer, async_bufferLen);
	return;

error_dma_buf:
	INV_MSG(INV_MSG_LEVEL_WARNING, "sensor_event_cb: encode error, frame dropped");

	return;
}

/*
* Convert sensor_event to VSensorData because dynamic protocol transports VSensorData
*/
static void convert_sensor_event_to_dyn_prot_data(const inv_sensor_event_t * event, VSensorDataAny * vsensor_data){
	vsensor_data->base.timestamp = event->timestamp;

	switch(event->sensor) {
		case DYN_PRO_SENSOR_TYPE_RESERVED:
		break;
		case DYN_PRO_SENSOR_TYPE_GRAVITY:
		case DYN_PRO_SENSOR_TYPE_LINEAR_ACCELERATION:
		case DYN_PRO_SENSOR_TYPE_ACCELEROMETER:
		inv_dc_float_to_sfix32(&event->data.acc.vect[0], 3, 16, (int32_t *)&vsensor_data->data.u32[0]);
		vsensor_data->base.meta_data = event->data.acc.accuracy_flag;
		break;
		case DYN_PRO_SENSOR_TYPE_GYROSCOPE:
		inv_dc_float_to_sfix32(&event->data.gyr.vect[0], 3, 16, (int32_t *)&vsensor_data->data.u32[0]);
		vsensor_data->base.meta_data = event->data.gyr.accuracy_flag;
		break;
		case DYN_PRO_SENSOR_TYPE_UNCAL_GYROSCOPE:
		inv_dc_float_to_sfix32(&event->data.gyr.vect[0], 3, 16, (int32_t *)&vsensor_data->data.u32[0]);
		inv_dc_float_to_sfix32(&event->data.gyr.bias[0], 3, 16, (int32_t *)&vsensor_data->data.u32[3]);
		vsensor_data->base.meta_data = event->data.gyr.accuracy_flag;
		break;
		case DYN_PRO_SENSOR_TYPE_PRED_QUAT_0:
		case DYN_PRO_SENSOR_TYPE_PRED_QUAT_1:
		case DYN_PRO_SENSOR_TYPE_GAME_ROTATION_VECTOR:
		inv_dc_float_to_sfix32(&event->data.quaternion.quat[0], 4, 30, (int32_t *)&vsensor_data->data.u32[0]);
		vsensor_data->base.meta_data = event->data.quaternion.accuracy_flag;
		break;
		case DYN_PRO_SENSOR_TYPE_MAGNETOMETER:
		inv_dc_float_to_sfix32(&event->data.mag.vect[0], 3, 16, (int32_t *)&vsensor_data->data.u32[0]);
		vsensor_data->base.meta_data = event->data.mag.accuracy_flag;
		break;
		case DYN_PRO_SENSOR_TYPE_UNCAL_MAGNETOMETER:
		inv_dc_float_to_sfix32(&event->data.mag.vect[0], 3, 16, (int32_t *)&vsensor_data->data.u32[0]);
		inv_dc_float_to_sfix32(&event->data.mag.bias[0], 3, 16, (int32_t *)&vsensor_data->data.u32[3]);
		vsensor_data->base.meta_data = event->data.mag.accuracy_flag;
		break;
		case DYN_PRO_SENSOR_TYPE_ROTATION_VECTOR:
		case DYN_PRO_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR:
		inv_dc_float_to_sfix32(&event->data.quaternion.quat[0], 4, 30, (int32_t *)&vsensor_data->data.u32[0]);
		inv_dc_float_to_sfix32(&event->data.quaternion.accuracy, 1, 16, (int32_t *)&vsensor_data->data.u32[4]);
		vsensor_data->base.meta_data = event->data.quaternion.accuracy_flag;
		break;
		case DYN_PRO_SENSOR_TYPE_ORIENTATION:
		inv_dc_float_to_sfix32(&event->data.orientation.x, 1, 16, (int32_t *)&vsensor_data->data.u32[0]);
		inv_dc_float_to_sfix32(&event->data.orientation.y, 1, 16, (int32_t *)&vsensor_data->data.u32[1]);
		inv_dc_float_to_sfix32(&event->data.orientation.z, 1, 16, (int32_t *)&vsensor_data->data.u32[2]);
		vsensor_data->base.meta_data = event->data.orientation.accuracy_flag;
		break;
		case DYN_PRO_SENSOR_TYPE_RAW_ACCELEROMETER:
		case DYN_PRO_SENSOR_TYPE_RAW_GYROSCOPE:
		vsensor_data->data.u32[0] = event->data.raw3d.vect[0];
		vsensor_data->data.u32[1] = event->data.raw3d.vect[1];
		vsensor_data->data.u32[2] = event->data.raw3d.vect[2];
		break;
		case DYN_PRO_SENSOR_TYPE_STEP_COUNTER:
		vsensor_data->data.u32[0] = (uint32_t)event->data.step.count;
		break;
		case DYN_PRO_SENSOR_TYPE_BAC:
		vsensor_data->data.u8[0] = (uint8_t)(int8_t)event->data.bac.event;
		break;
		case DYN_PRO_SENSOR_TYPE_WOM:
		vsensor_data->data.u8[0] = event->data.wom.flags;
		break;
		case DYN_PRO_SENSOR_TYPE_B2S:
		case DYN_PRO_SENSOR_TYPE_SMD:
		case DYN_PRO_SENSOR_TYPE_STEP_DETECTOR:
		case DYN_PRO_SENSOR_TYPE_TILT_DETECTOR:
		case DYN_PRO_SENSOR_TYPE_PICK_UP_GESTURE:
		vsensor_data->data.u8[0] = event->data.event;
		break;
		default:
		break;
	}
}

static enum inv_icm20948_sensor idd_sensortype_conversion(int sensor){
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

int handle_command(enum DynProtocolEid eid, const DynProtocolEdata_t * edata, DynProtocolEdata_t * respdata){
	int rc = 0;
	uint8_t whoami;
	const int sensor = edata->sensor_id;

	switch(eid) {

	case DYN_PROTOCOL_EID_GET_SW_REG:
		if(edata->d.command.regAddr == DYN_PROTOCOL_EREG_HANDSHAKE_SUPPORT)
			return InvEMDFrontEnd_isHwFlowCtrlSupportedHook();
		return 0;

	case DYN_PROTOCOL_EID_SETUP:
	{
		int i_sensor = INV_SENSOR_TYPE_MAX;
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command setup");

		/* Disable all sensors */
		while(i_sensor-- > 0) {
			rc = inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(i_sensor), 0);
		}

		/* Clear pio interrupt */
		pio_clear(PIN_EXT_INTERRUPT_PIO, PIN_EXT_INTERRUPT_MASK);

		/* Re-init the device */
		rc = icm20948_sensor_setup();
		rc += load_dmp3();

		/* .. no sensors are reporting on setup */
		enabled_sensor_mask = 0;
		return rc;
	}

	case DYN_PROTOCOL_EID_WHO_AM_I:
		rc = inv_icm20948_get_whoami(&icm_device, &whoami);
		return (rc == 0) ? whoami : rc;

	case DYN_PROTOCOL_EID_RESET:
	{
		int i_sensor = INV_SENSOR_TYPE_MAX;
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command reset");

		/* Disable all sensors */
		while(i_sensor-- > 0) {
			rc = inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(i_sensor), 0);
		}

		/* Clear pio interrupt */
		pio_clear(PIN_EXT_INTERRUPT_PIO, PIN_EXT_INTERRUPT_MASK);
		/* Soft reset */
		rc += inv_icm20948_soft_reset(&icm_device);

		/* --- Setup --- */
		/* Re-init the device */
		rc += icm20948_sensor_setup();
		rc += load_dmp3();

		/* All sensors stop reporting on reset */
		enabled_sensor_mask = 0;
		return rc;
	}

	case DYN_PROTOCOL_EID_PING_SENSOR:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command ping(%s)", inv_sensor_2str(sensor));
		if((sensor == INV_SENSOR_TYPE_RAW_ACCELEROMETER)
			|| (sensor == INV_SENSOR_TYPE_RAW_GYROSCOPE)
			|| (sensor == INV_SENSOR_TYPE_ACCELEROMETER)
			|| (sensor == INV_SENSOR_TYPE_GYROSCOPE)
			|| (sensor == INV_SENSOR_TYPE_UNCAL_GYROSCOPE)
			|| (sensor == INV_SENSOR_TYPE_GAME_ROTATION_VECTOR)
			|| (sensor == INV_SENSOR_TYPE_GRAVITY)
			|| (sensor == INV_SENSOR_TYPE_LINEAR_ACCELERATION)
			|| (sensor == INV_SENSOR_TYPE_STEP_COUNTER)
			|| (sensor == INV_SENSOR_TYPE_BAC)
			|| (sensor == INV_SENSOR_TYPE_B2S)
			|| (sensor == INV_SENSOR_TYPE_SMD)
			|| (sensor == INV_SENSOR_TYPE_STEP_DETECTOR)
			|| (sensor == INV_SENSOR_TYPE_TILT_DETECTOR)
			|| (sensor == INV_SENSOR_TYPE_PICK_UP_GESTURE)
			) {
				return 0;
		} else if((sensor == INV_SENSOR_TYPE_MAGNETOMETER)
			|| (sensor == INV_SENSOR_TYPE_UNCAL_MAGNETOMETER)
			|| (sensor == INV_SENSOR_TYPE_ROTATION_VECTOR)
			|| (sensor == INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR)){
				return 0;
		} else
			return INV_ERROR_BAD_ARG;

	case DYN_PROTOCOL_EID_SELF_TEST:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command self_test(%s)", inv_sensor_2str(sensor));
		if( (sensor == INV_SENSOR_TYPE_RAW_ACCELEROMETER || sensor == INV_SENSOR_TYPE_ACCELEROMETER) ||
			(sensor == INV_SENSOR_TYPE_RAW_GYROSCOPE || sensor == INV_SENSOR_TYPE_GYROSCOPE) ||
			(sensor == INV_SENSOR_TYPE_MAGNETOMETER)) {
				return icm20948_run_selftest();
		}
		else
			return INV_ERROR_BAD_ARG;
		break;
	case DYN_PROTOCOL_EID_START_SENSOR:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command start(%s)", inv_sensor_2str(sensor));
		if (sensor > 0 && idd_sensortype_conversion(sensor) < INV_ICM20948_SENSOR_MAX) {
			/* Sensor data will be notified */
			rc = inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(sensor), 1);
			enabled_sensor_mask |= (1 << idd_sensortype_conversion(sensor));
			return rc;
		} else
			return INV_ERROR_NIMPL; /*this sensor is not supported*/

	case DYN_PROTOCOL_EID_STOP_SENSOR:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command stop(%s)", inv_sensor_2str(sensor));
		if (sensor > 0 && idd_sensortype_conversion(sensor) < INV_ICM20948_SENSOR_MAX) {
			/* Sensor data will not be notified anymore */
			rc = inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(sensor), 0);
			enabled_sensor_mask &= ~(1 << idd_sensortype_conversion(sensor));
			return rc;
		} else
			return INV_ERROR_NIMPL; /*this sensor is not supported*/

	case DYN_PROTOCOL_EID_SET_SENSOR_PERIOD:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command set_period(%d us)",edata->d.command.period);
		rc = inv_icm20948_set_sensor_period(&icm_device, idd_sensortype_conversion(sensor), edata->d.command.period / 1000);
		return rc;

	case DYN_PROTOCOL_EID_SET_SENSOR_CFG:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command set_sensor_config(%s)", inv_sensor_2str(sensor));
		return INV_ERROR_NIMPL;

	case DYN_PROTOCOL_EID_CLEANUP:
	{
		int i_sensor = INV_SENSOR_TYPE_MAX;
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command cleanup");

		/* Disable all sensors on cleanup */
		while(i_sensor-- > 0) {
			rc = inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(i_sensor), 0);
		}

		/* Clear pio interrupt */
		pio_clear(PIN_EXT_INTERRUPT_PIO, PIN_EXT_INTERRUPT_MASK);

		/* Soft reset */
		rc += inv_icm20948_soft_reset(&icm_device);
		/* All sensors stop reporting on cleanup */
		enabled_sensor_mask = 0;
		return rc;
	}

	default:
		return INV_ERROR_NIMPL;
	}
}

void inv_icm20948_get_st_bias(struct inv_icm20948 * s, int *gyro_bias, int *accel_bias, int * st_bias){
	int axis, axis_sign;
	int gravity;
	int i, t;
	int check;
	int scale;

	/* check bias there ? */
	check = 0;
	for (i = 0; i < 3; i++) {
		if (gyro_bias[i] != 0)
			check = 1;
		if (accel_bias[i] != 0)
			check = 1;
	}

	/* if no bias, return all 0 */
	if (check == 0) {
		for (i = 0; i < 12; i++)
			st_bias[i] = 0;
		return;
	}

	/* dps scaled by 2^16 */
	scale = 65536 / DEF_SELFTEST_GYRO_SENS;

	/* Gyro normal mode */
	t = 0;
	for (i = 0; i < 3; i++)
		st_bias[i + t] = gyro_bias[i] * scale;

	axis = 0;
	axis_sign = 1;
	if (INV20948_ABS(accel_bias[1]) > INV20948_ABS(accel_bias[0]))
		axis = 1;
	if (INV20948_ABS(accel_bias[2]) > INV20948_ABS(accel_bias[axis]))
		axis = 2;
	if (accel_bias[axis] < 0)
		axis_sign = -1;

	/* gee scaled by 2^16 */
	scale = 65536 / (DEF_ST_SCALE / (DEF_ST_ACCEL_FS_MG / 1000));

	gravity = 32768 / (DEF_ST_ACCEL_FS_MG / 1000) * axis_sign;
	gravity *= scale;

	/* Accel normal mode */
	t += 3;
	for (i = 0; i < 3; i++) {
		st_bias[i + t] = accel_bias[i] * scale;
		if (axis == i)
			st_bias[i + t] -= gravity;
	}
}

int icm20948_run_selftest(void){
	static int rc = 0;		// Keep this value as we're only going to do this once.
	int gyro_bias_regular[THREE_AXES];
	int accel_bias_regular[THREE_AXES];
	static int raw_bias[THREE_AXES * 2];

	if (icm_device.selftest_done == 1) {
		INV_MSG(INV_MSG_LEVEL_INFO, "Self-test has already run. Skipping.");
	}
	else {
		/*
		* Perform self-test
		* For ICM20948 self-test is performed for both RAW_ACC/RAW_GYR
		*/
		INV_MSG(INV_MSG_LEVEL_INFO, "Running self-test...");

		/* Run the self-test */
		rc = inv_icm20948_run_selftest(&icm_device, gyro_bias_regular, accel_bias_regular);
		if ((rc & INV_ICM20948_SELF_TEST_OK) == INV_ICM20948_SELF_TEST_OK) {
			/* On A+G+M self-test success, offset will be kept until reset */
			icm_device.selftest_done = 1;
			rc = 0;
		} else {
			/* On A|G|M self-test failure, return Error */
			INV_MSG(INV_MSG_LEVEL_ERROR, "Self-test failure !");
			/* 0 would be considered OK, we want KO */
			rc = INV_ERROR;
		}

		/* It's advised to re-init the icm20948 device after self-test for normal use */
		icm20948_sensor_setup();
		inv_icm20948_get_st_bias(&icm_device, gyro_bias_regular, accel_bias_regular, raw_bias);
		INV_MSG(INV_MSG_LEVEL_INFO, "GYR bias (FS=250dps) (dps): x=%f, y=%f, z=%f", (float)(raw_bias[0] / (float)(1 << 16)), (float)(raw_bias[1] / (float)(1 << 16)), (float)(raw_bias[2] / (float)(1 << 16)));
		INV_MSG(INV_MSG_LEVEL_INFO, "ACC bias (FS=2g) (g): x=%f, y=%f, z=%f", (float)(raw_bias[0 + 3] / (float)(1 << 16)), (float)(raw_bias[1 + 3] / (float)(1 << 16)), (float)(raw_bias[2 + 3] / (float)(1 << 16)));
	}

	return rc;
}
