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
#define DEF_ST_ACCEL_FS                 2
#define DEF_ST_GYRO_FS_DPS              250
#define DEF_ST_SCALE                    32768
#define DEF_SELFTEST_GYRO_SENS			(DEF_ST_SCALE / DEF_ST_GYRO_FS_DPS)
#define DEF_ST_ACCEL_FS_MG				2000

#ifndef INV20948_ABS
#define INV20948_ABS(x) (((x) < 0) ? -(x) : (x))
#endif

/* Forward declaration */
int icm20948_sensor_setup(void);
void iddwrapper_protocol_event_cb(enum DynProtocolEtype etype, enum DynProtocolEid eid, const DynProtocolEdata_t * edata, void * cookie);
void iddwrapper_transport_event_cb(enum DynProTransportEvent e, union DynProTransportEventData data, void * cookie);
void sensor_event(const inv_sensor_event_t * event, void * arg);
int handle_command(enum DynProtocolEid eid, const DynProtocolEdata_t * edata, DynProtocolEdata_t * respdata);
int icm20948_run_selftest(void);
void inv_icm20948_get_st_bias(struct inv_icm20948 * s, int *gyro_bias, int *accel_bias, int * st_bias);
void InvEMDFrontEnd_busyWaitUsHook(uint32_t us);
int InvEMDFrontEnd_isHwFlowCtrlSupportedHook(void);
int InvEMDFrontEnd_putcharHook(int c);
void build_sensor_event_data(void * context, uint8_t sensortype, uint64_t timestamp, const void * data, const void *arg);
void inv_icm20948_sleep(int ms);
int load_dmp3(void);
void check_rc(int rc, const char * msg_context);

extern inv_icm20948_t icm_device;
extern DynProtocol_t protocol;
extern DynProTransportUart_t transport;
