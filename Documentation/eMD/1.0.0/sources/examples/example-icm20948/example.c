/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2016-2016 InvenSense Inc. All rights reserved.
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

/**
 * @file
 * @brief This application allows sensor-cli to control the 20948, Nucleo being only an intermediate 
 *        between the host running sensor-cli and the Invensense device. To be more precise, sensor-cli sends
 *        commands to the STM32F411 through an UART interface and the Dynamic protocol and the STM forwards
 *        these commands through an SPI interface to the 20948. This works the other 
 *        way around too. 
 *        There are two UART interface involved in the communication between Nucleo and the PC:
 *          - UART2 is used to output Nucleo's traces
 *          - UART1 is used by the PC to send and receive commands and sensor data from the 20948
 */

#include <stdint.h>
#include <stdio.h>

#include "Invn/EmbUtils/Message.h"
#include "Invn/EmbUtils/DataConverter.h"
#include "Invn/Devices/DeviceIcm20948.h"
#include "Invn/DynamicProtocol/DynProtocol.h"
#include "Invn/DynamicProtocol/DynProtocolTransportUart.h"

#include "idd_io_hal.h"

/* board driver */
#include "uart.h"
#include "sys_timer.h"
#include "delay.h"
#include "gpio.h"
#include "dbg_gpio.h"
#include "led.h"
#include "timer.h"
#include "flash_linker.h"
#include "flash_manager.h"

/*
 * Set to 1 to use IddWrapper. Set to 0 to disable.
 * This allows to control the sensors from sensor-cli host application and send sensor events to it
 */
#define USE_IDDWRAPPER   1

#define ODR_NONE       0 /* Asynchronous sensors don't need to have a configured ODR */

/*
 * Set O/1 to start the following sensors in this example
 * NB: In case you are using IddWrapper (USE_IDDWRAPPER = 1), the following compile switch will have no effect.
 */
#define USE_RAW_ACC 1
#define USE_RAW_GYR 1
#define USE_GRV     0
#define USE_CAL_ACC 0
#define USE_CAL_GYR 0
#define USE_CAL_MAG 0
#define USE_UCAL_GYR 0
#define USE_UCAL_MAG 0
#define USE_RV      0    /* requires COMPASS*/
#define USE_GEORV   0    /* requires COMPASS*/
#define USE_ORI     0    /* requires COMPASS*/
#define USE_STEPC   0
#define USE_STEPD   0
#define USE_SMD     0
#define USE_BAC     0
#define USE_TILT    0
#define USE_PICKUP  0
#define USE_GRAVITY 0
#define USE_LINACC  0
#define USE_B2S     0

/*
 * Sensor to start in this example
 */
#if !USE_IDDWRAPPER
static const struct {
	uint8_t  type;
	uint32_t period_us;
} sensor_list[] = {
#if USE_RAW_ACC
	{ INV_SENSOR_TYPE_RAW_ACCELEROMETER, 50000 /* 20 Hz */ },
#endif
#if USE_RAW_GYR
	{ INV_SENSOR_TYPE_RAW_GYROSCOPE,     50000 /* 20 Hz */ },
#endif
#if USE_CAL_ACC
	{ INV_SENSOR_TYPE_ACCELEROMETER, 50000 /* 20 Hz */ },
#endif
#if USE_CAL_GYR
	{ INV_SENSOR_TYPE_GYROSCOPE, 50000 /* 20 Hz */ },
#endif
#if USE_CAL_MAG
	{ INV_SENSOR_TYPE_MAGNETOMETER, 50000 /* 20 Hz */ },
#endif
#if USE_UCAL_GYR
	{ INV_SENSOR_TYPE_UNCAL_GYROSCOPE, 50000 /* 20 Hz */ },
#endif
#if USE_UCAL_MAG
	{ INV_SENSOR_TYPE_UNCAL_MAGNETOMETER, 50000 /* 20 Hz */ },
#endif
#if USE_GRV
	{ INV_SENSOR_TYPE_GAME_ROTATION_VECTOR, 50000 /* 20 Hz */ },
#endif
#if USE_RV
	{ INV_SENSOR_TYPE_ROTATION_VECTOR, 50000 /* 20 Hz */ },
#endif
#if USE_GEORV
	{ INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR, 50000 /* 20 Hz */ },
#endif
#if USE_ORI
	{ INV_SENSOR_TYPE_ORIENTATION, 50000 /* 20 Hz */ },
#endif
#if USE_STEPC
	{ INV_SENSOR_TYPE_STEP_COUNTER, ODR_NONE },
#endif
#if USE_STEPD
	{ INV_SENSOR_TYPE_STEP_DETECTOR, ODR_NONE},
#endif
#if USE_SMD
	{ INV_SENSOR_TYPE_SMD, ODR_NONE},
#endif
#if USE_BAC
	{ INV_SENSOR_TYPE_BAC, ODR_NONE},
#endif
#if USE_TILT
	{ INV_SENSOR_TYPE_TILT_DETECTOR, ODR_NONE},
#endif
#if USE_PICKUP
	{ INV_SENSOR_TYPE_PICK_UP_GESTURE, ODR_NONE},
#endif
#if USE_GRA
	{ INV_SENSOR_TYPE_GRAVITY, 50000 /* 20 Hz */},
#endif
#if USE_LINACC
	{ INV_SENSOR_TYPE_LINEAR_ACCELERATION, 50000 /* 20 Hz */},
#endif
#if USE_B2S
	{ INV_SENSOR_TYPE_B2S, ODR_NONE},
#endif
};
#endif

#if !USE_IDDWRAPPER
const char * activityName(int act);
#endif

#define UART_LOG_TX_FIFO_SIZE     4096
#define UART_MAIN_RX_FIFO_SIZE    256

/* Define msg level */
#define MSG_LEVEL INV_MSG_LEVEL_DEBUG

/* Forward declaration */
void ext_interrupt_cb(void * context, int int_num);
static void sensor_event_cb(const inv_sensor_event_t * event, void * arg);
void inv_icm20948_sleep_us(int us);
void inv_icm20948_sleep(int us);
uint64_t inv_icm20948_get_time_us(void);
uint64_t inv_icm20948_get_dataready_interrupt_time_us(void);
static void check_rc(int rc);
static void msg_printer(int level, const char * str, va_list ap);
#if USE_IDDWRAPPER
  int handle_command(enum DynProtocolEid eid, const DynProtocolEdata_t * edata, DynProtocolEdata_t * respdata);
  static void convert_sensor_event_to_dyn_prot_data(const inv_sensor_event_t * event, VSensorDataAny * vsensor_data);
  static int iddwrapper_set_encoding_format(int sensor, const void * buffer);
  static void iddwrapper_protocol_event_cb(enum DynProtocolEtype etype, enum DynProtocolEid eid, const DynProtocolEdata_t * edata, void * cookie);
  static void iddwrapper_transport_event_cb(enum DynProTransportEvent e, union DynProTransportEventData data, void * cookie);
  /* Offsets are written when cleanup function is received. It has no meaning without IDDWRAPPER */
  static void apply_stored_offsets(void);
  static void store_offsets(void);
#endif
/*
 * Flag set from device irq handler 
 */
static volatile int irq_from_device;

/*
 * Some memory to be used by the UART driver (4 kB)
 */
static uint8_t uart_logTx_buffer[UART_LOG_TX_FIFO_SIZE];
#if USE_IDDWRAPPER
  static uint8_t uart_mainRx_buffer[UART_MAIN_RX_FIFO_SIZE]; 
#endif

/* 
 * WHOAMI value for 20948
 */
static const uint8_t EXPECTED_WHOAMI[] = { 0xEA };

/*
 * Icm20948 device require a DMP image to be loaded on init
 * Provide such images by mean of a byte array
 */
static const uint8_t dmp3_image[] = {
	#include "icm20948_img.dmp3a.h"
};

/*
 * States for icm20948 device object
 */
static inv_device_icm20948_t device_icm20948;

/* 
 * Just a handy variable to keep the handle to device object
 */
static inv_device_t * device; 

/*
 * A listener object will handle sensor events
 */
static const inv_sensor_listener_t sensor_listener = {
	sensor_event_cb, /* callback that will receive sensor events */
	0                /* some pointer passed to the callback */
};

#if USE_IDDWRAPPER
  /*
   * Dynamic protocol and transport handles
   */
  static DynProtocol_t protocol;
  static DynProTransportUart_t transport;
#endif

/*
 * Last time at which 20948 IRQ was fired
 */
static volatile uint32_t last_irq_time = 0;

#define MAIN_UART_ID UART1 // Through FTDI cable
#define LOG_UART_ID  UART2 // Through ST-Link
#define DELAY_TIMER  TIMER3
#define TIMEBASE_TIMER TIMER2

int main(void)
{
	int rc = 0;
	unsigned i = 0;

	uint8_t whoami = 0xff;
	uart_init_struct_t uart_config;

	/*
	 * Set NVIC priority according to Nucleo LLD usage
	 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/*
	 * Initializes the flash manager by linking to the reserved section in NV memory
	 */
	flash_linker_init();
	
	/*
	 * Init timer peripheral for delay
	 */
	delay_init(DELAY_TIMER);
	
	/* init UART
	 * UART LOG:  USART2 - St-Link - 921600 baud ; 8bits ; no parity - IRQs enabled
	 * UART MAIN: USART1 - TX(PA9) / RX(PA10) / CTS(PA11) / RTS(PA12) - 2MHz baud; 8 bits ; no parity - IRQs enabled
	 */
	uart_config.uart_num = LOG_UART_ID;
	uart_config.irqs_on = 1;
	uart_config.use_for_printf = 1;
	uart_config.use_dma_for_tx = 0;
	uart_config.use_dma_for_rx = 0;
	uart_config.tx_buffer = uart_logTx_buffer;
	uart_config.rx_buffer = NULL;
	uart_config.tx_size = UART_LOG_TX_FIFO_SIZE;
	uart_config.rx_size = 0;
	uart_config.baudrate = 921600;
	uart_config.rx_interrupt_cb = NULL;
	uart_config.tx_interrupt_cb = NULL;
	uart_config.rx_context = NULL;    
	uart_config.tx_context = NULL;
	uart_init(&uart_config);
#if USE_IDDWRAPPER
	uart_config.uart_num = MAIN_UART_ID;
	uart_config.irqs_on = 1;
	uart_config.use_for_printf = 0;
	uart_config.use_dma_for_tx = 1;
	uart_config.use_dma_for_rx = 0;
	uart_config.tx_buffer = NULL;
	uart_config.rx_buffer = uart_mainRx_buffer;
	uart_config.tx_size = 0;
	uart_config.rx_size = UART_MAIN_RX_FIFO_SIZE;
	uart_config.baudrate = 2000000;
	uart_config.rx_interrupt_cb = NULL;
	uart_config.tx_interrupt_cb = NULL;
	uart_config.rx_context = NULL;    
	uart_config.tx_context = NULL;
	uart_init(&uart_config);
#endif

#if USE_IDDWRAPPER
	/*
	 * Initialize Dynamic protocol stuff
	 */
	DynProTransportUart_init(&transport, iddwrapper_transport_event_cb, 0);
	DynProtocol_init(&protocol, iddwrapper_protocol_event_cb, 0);
#endif

	timer_configure_timebase(TIMEBASE_TIMER, 1000000);
	
	/*
	 * Register a handler called upon external interrupt
	 */
	gpio_sensor_irq_init(TO_MASK(GPIO_SENSOR_IRQ_D6) | TO_MASK(GPIO_SENSOR_IRQ_D7), ext_interrupt_cb, 0);
	timer_enable(TIMEBASE_TIMER);
	
	/*
	 * Setup message facility to see internal traces from IDD
	 */
	INV_MSG_SETUP(MSG_LEVEL, msg_printer);

	/*
	 * Welcome message
	 */
	INV_MSG(INV_MSG_LEVEL_INFO, "###################################");
	INV_MSG(INV_MSG_LEVEL_INFO, "#          20948 example          #");
	INV_MSG(INV_MSG_LEVEL_INFO, "###################################");

	/*
	 * Open serial interface before using the device
	 * Init SPI communication: SPI1 - SCK(PA5) / MISO(PA6) / MOSI(PA7) / CS(PB6)
	 */
	INV_MSG(INV_MSG_LEVEL_INFO, "Open SPI serial interface");
	rc += inv_host_serif_open(idd_io_hal_get_serif_instance_spi());

	/*
	 * Create icm20948 Device 
	 * Pass to the driver:
	 * - reference to serial interface object,
	 * - reference to listener that will catch sensor events,
	 * - a static buffer for the driver to use as a temporary buffer
	 * - various driver option
	 */
	inv_device_icm20948_init(&device_icm20948, idd_io_hal_get_serif_instance_spi(),
			&sensor_listener, dmp3_image, sizeof(dmp3_image));

	/*
	 * Simply get generic device handle from icm20948 Device
	 */
	device = inv_device_icm20948_get_base(&device_icm20948);

	/*
	 * Just get the whoami
	 */
	rc = inv_device_whoami(device, &whoami);
	INV_MSG(INV_MSG_LEVEL_INFO, "ICM WHOAMI=%02x", whoami);
	check_rc(rc);

	/*
	 * Check if WHOAMI value corresponds to any value from EXPECTED_WHOAMI array
	 */
	for(i = 0; i < sizeof(EXPECTED_WHOAMI)/sizeof(EXPECTED_WHOAMI[0]); ++i) {
		if(whoami == EXPECTED_WHOAMI[i])
			break;
	}

	if(i == sizeof(EXPECTED_WHOAMI)/sizeof(EXPECTED_WHOAMI[0])) {
		INV_MSG(INV_MSG_LEVEL_ERROR, "Bad WHOAMI value. Got 0x%02x. Expected @EXPECTED_WHOAMI@.", whoami);
		check_rc(-1);
	}

	/*
	 * Configure and initialize the icm20948 device
	 */
	INV_MSG(INV_MSG_LEVEL_INFO, "Setting-up ICM device");
	rc = inv_device_setup(device);
	check_rc(rc);
	
	/*
	 * Now that Icm20948 device was inialized, we can proceed with DMP image loading
	 * This step is mandatory as DMP image are not store in non volatile memory
	 */
	INV_MSG(INV_MSG_LEVEL_INFO, "Load DMP3 image");
	rc = inv_device_load(device, NULL, dmp3_image, sizeof(dmp3_image), true /* verify */, NULL);
	check_rc(rc);
	
#if !USE_IDDWRAPPER
	{
		uint64_t available_sensor_mask; /* To keep track of available sensors*/
		unsigned i;
		/*
		 * Check sensor availibitlity
		 * if rc value is 0, it means sensor is available,
		 * if rc value is INV_ERROR or INV_ERROR_BAD_ARG, sensor is NA
		 */
		available_sensor_mask = 0;
		for(i = 0; i < sizeof(sensor_list)/sizeof(sensor_list[0]); ++i) {
			const int rc = inv_device_ping_sensor(device, sensor_list[i].type);
			INV_MSG(INV_MSG_LEVEL_INFO, "Ping %s %s", inv_sensor_2str(sensor_list[i].type), (rc == 0) ? "OK" : "KO");
			if(rc == 0) {
				available_sensor_mask |= (1ULL << sensor_list[i].type);
			}
		}

		/*
		 * Start all available sensors from the sensor list
		 */
		for(i = 0; i < sizeof(sensor_list)/sizeof(sensor_list[0]); ++i) {
			if(available_sensor_mask & (1ULL << sensor_list[i].type)) {
				INV_MSG(INV_MSG_LEVEL_INFO, "Starting %s @ %u us", inv_sensor_2str(sensor_list[i].type), sensor_list[i].period_us);
				rc  = inv_device_set_sensor_period_us(device, sensor_list[i].type, sensor_list[i].period_us);
				check_rc(rc);
				rc += inv_device_start_sensor(device, sensor_list[i].type);
				check_rc(rc);
			}
		}
	}
#endif

	do {
#if USE_IDDWRAPPER
		/*
		 * Look for RX bytes and feed IddWrapper transport (will ultimately call IddWrapper event handler)
		 */
		while(uart_available(MAIN_UART_ID)) {
			int c = uart_getc(MAIN_UART_ID);
			DynProTransportUart_rxProcessByte(&transport, (uint8_t)c);
		}
#endif
		/*
		 * Poll device for data
		 */
		if (irq_from_device & TO_MASK(GPIO_SENSOR_IRQ_D6)) {
			rc = inv_device_poll(device);
			check_rc(rc);

			if(rc >= 0) {
				__disable_irq();
				irq_from_device &= ~TO_MASK(GPIO_SENSOR_IRQ_D6);
				__enable_irq();
			}
		}
	} while(1);
}

/*
 * Callback called upon rising edge on external interrupt line
 */
void ext_interrupt_cb(void * context, int int_num)
{
	(void)context;
	last_irq_time = inv_icm20948_get_time_us();
	irq_from_device = TO_MASK(int_num);
}

/*
 * Callback called upon sensor event reception
 * This function is called in the same context as inv_device_poll()
 */
static void sensor_event_cb(const inv_sensor_event_t * event, void * arg)
{
	/* arg will contained the value provided at init time */
	(void)arg;

#if USE_IDDWRAPPER
	if(event->status == INV_SENSOR_STATUS_DATA_UPDATED) {
		/*
		 * Encode sensor event and send to host over UART through Dynamic protocol
		 */

		static DynProtocolEdata_t async_edata; /* static to take on .bss */
		uart_dma_tx_buffer_t * dma_buffer;
		DynProTransportUartFrame_t uart_frame;
		int timeout = 5000; /* us */;

		async_edata.sensor_id = event->sensor;
		async_edata.d.async.sensorEvent.status = DYN_PRO_SENSOR_STATUS_DATA_UPDATED;
		convert_sensor_event_to_dyn_prot_data(event, &async_edata.d.async.sensorEvent.vdata);

		while((uart_dma_tx_take_buffer(&dma_buffer) != 0) && (timeout > 0)) {
			inv_icm20948_sleep_us(10);
			timeout -= 10;
		}

		if (timeout <= 0) {
			/* if no available buffer, can't send the sensor data */
			INV_MSG(INV_MSG_LEVEL_WARNING, "sensor_event_cb: 'uart_dma_tx_take_buffer' timeouted, frame dropped");
			return;
		}

		/* get memory location for the iddwrapper protocol payload */
		if(DynProTransportUart_txAssignBuffer(&transport, &uart_frame,
				dma_buffer->data, sizeof(dma_buffer->data)) != 0) {
			goto error_dma_buf;
		}

		if(DynProtocol_encodeAsync(&protocol,
				DYN_PROTOCOL_EID_NEW_SENSOR_DATA, &async_edata,
				uart_frame.payload_data, uart_frame.max_payload_len, &uart_frame.payload_len) != 0) {
			goto error_dma_buf;
		}
		if(DynProTransportUart_txEncodeFrame(&transport, &uart_frame) != 0) {
			goto error_dma_buf;
		}

		/* send async event */
		dma_buffer->len = uart_frame.len;
		if(uart_dma_tx(MAIN_UART_ID, dma_buffer) != 0) {
			INV_MSG(INV_MSG_LEVEL_DEBUG, "sensor_event_cb: 'uart_dma_tx()' returned error");
		}
		return;

error_dma_buf:
		INV_MSG(INV_MSG_LEVEL_WARNING, "sensor_event_cb: encode error, frame dropped");
		uart_dma_tx_release_buffer(&dma_buffer);
		return;
	}
#else
/*
	 * In normal mode, display sensor event over UART messages
	 */

	if(event->status == INV_SENSOR_STATUS_DATA_UPDATED) {

		switch(INV_SENSOR_ID_TO_TYPE(event->sensor)) {
		case INV_SENSOR_TYPE_RAW_ACCELEROMETER:
		case INV_SENSOR_TYPE_RAW_GYROSCOPE:
			INV_MSG(INV_MSG_LEVEL_INFO, "data event %s (lsb): %llu %d %d %d", inv_sensor_str(event->sensor),
					event->timestamp,
					(int)event->data.raw3d.vect[0],
					(int)event->data.raw3d.vect[1],
					(int)event->data.raw3d.vect[2]);
			break;
		case INV_SENSOR_TYPE_ACCELEROMETER:
		case INV_SENSOR_TYPE_LINEAR_ACCELERATION:
		case INV_SENSOR_TYPE_GRAVITY:
			INV_MSG(INV_MSG_LEVEL_INFO, "data event %s (mg): %d %d %d", inv_sensor_str(event->sensor),
					(int)(event->data.acc.vect[0]*1000),
					(int)(event->data.acc.vect[1]*1000),
					(int)(event->data.acc.vect[2]*1000));
			break;
		case INV_SENSOR_TYPE_GYROSCOPE:
			INV_MSG(INV_MSG_LEVEL_INFO, "data event %s (mdps): %d %d %d", inv_sensor_str(event->sensor),
					(int)(event->data.gyr.vect[0]*1000),
					(int)(event->data.gyr.vect[1]*1000),
					(int)(event->data.gyr.vect[2]*1000));
			break;
		case INV_SENSOR_TYPE_MAGNETOMETER:
			INV_MSG(INV_MSG_LEVEL_INFO, "data event %s (nT): %d %d %d", inv_sensor_str(event->sensor),
					(int)(event->data.mag.vect[0]*1000),
					(int)(event->data.mag.vect[1]*1000),
					(int)(event->data.mag.vect[2]*1000));
			break;
		case INV_SENSOR_TYPE_UNCAL_GYROSCOPE:
			INV_MSG(INV_MSG_LEVEL_INFO, "data event %s (mdps): %d %d %d %d %d %d", inv_sensor_str(event->sensor),
					(int)(event->data.gyr.vect[0]*1000),
					(int)(event->data.gyr.vect[1]*1000),
					(int)(event->data.gyr.vect[2]*1000),
					(int)(event->data.gyr.bias[0]*1000),
					(int)(event->data.gyr.bias[1]*1000),
					(int)(event->data.gyr.bias[2]*1000));
			break;
		case INV_SENSOR_TYPE_UNCAL_MAGNETOMETER:
			INV_MSG(INV_MSG_LEVEL_INFO, "data event %s (nT): %d %d %d %d %d %d", inv_sensor_str(event->sensor),
					(int)(event->data.mag.vect[0]*1000),
					(int)(event->data.mag.vect[1]*1000),
					(int)(event->data.mag.vect[2]*1000),
					(int)(event->data.mag.bias[0]*1000),
					(int)(event->data.mag.bias[1]*1000),
					(int)(event->data.mag.bias[2]*1000));
			break;
		case INV_SENSOR_TYPE_GAME_ROTATION_VECTOR:
		case INV_SENSOR_TYPE_ROTATION_VECTOR:
		case INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR:
			INV_MSG(INV_MSG_LEVEL_INFO, "data event %s (e-3): %d %d %d %d ", inv_sensor_str(event->sensor),
					(int)(event->data.quaternion.quat[0]*1000),
					(int)(event->data.quaternion.quat[1]*1000),
					(int)(event->data.quaternion.quat[2]*1000),
					(int)(event->data.quaternion.quat[3]*1000));
			break;
		case INV_SENSOR_TYPE_ORIENTATION:
			INV_MSG(INV_MSG_LEVEL_INFO, "data event %s (e-3): %d %d %d %d ", inv_sensor_str(event->sensor),
					(int)(event->data.orientation.x*1000),
					(int)(event->data.orientation.y*1000),
					(int)(event->data.orientation.z*1000));
			break;
		case INV_SENSOR_TYPE_BAC:
			INV_MSG(INV_MSG_LEVEL_INFO, "data event %s : %d %s", inv_sensor_str(event->sensor),
					event->data.bac.event, activityName(event->data.bac.event));
			break;
		case INV_SENSOR_TYPE_STEP_COUNTER:
			INV_MSG(INV_MSG_LEVEL_INFO, "data event %s : %lu", inv_sensor_str(event->sensor),
					(unsigned long)event->data.step.count);
			break;
		case INV_SENSOR_TYPE_PICK_UP_GESTURE:
		case INV_SENSOR_TYPE_STEP_DETECTOR:
		case INV_SENSOR_TYPE_SMD:
		case INV_SENSOR_TYPE_B2S:
		case INV_SENSOR_TYPE_TILT_DETECTOR:
		default:
			INV_MSG(INV_MSG_LEVEL_INFO, "data event %s : ...", inv_sensor_str(event->sensor));
			break;
		}
	}

#endif
}
#if !USE_IDDWRAPPER
/*
 * Function to return activity name in printable char
 */
const char * activityName(int act)
{
	switch(act) {
	case INV_SENSOR_BAC_EVENT_ACT_IN_VEHICLE_BEGIN:          return "BEGIN IN_VEHICLE";
	case INV_SENSOR_BAC_EVENT_ACT_WALKING_BEGIN:             return "BEGIN WALKING";
	case INV_SENSOR_BAC_EVENT_ACT_RUNNING_BEGIN:             return "BEGIN RUNNING";
	case INV_SENSOR_BAC_EVENT_ACT_ON_BICYCLE_BEGIN:          return "BEGIN ON_BICYCLE";
	case INV_SENSOR_BAC_EVENT_ACT_TILT_BEGIN:                return "BEGIN TILT";
	case INV_SENSOR_BAC_EVENT_ACT_STILL_BEGIN:               return "BEGIN STILL";
	case INV_SENSOR_BAC_EVENT_ACT_IN_VEHICLE_END:            return "END IN_VEHICLE";
	case INV_SENSOR_BAC_EVENT_ACT_WALKING_END:               return "END WALKING";
	case INV_SENSOR_BAC_EVENT_ACT_RUNNING_END:               return "END RUNNING";
	case INV_SENSOR_BAC_EVENT_ACT_ON_BICYCLE_END:            return "END ON_BICYCLE";
	case INV_SENSOR_BAC_EVENT_ACT_TILT_END:                  return "END TILT";
	case INV_SENSOR_BAC_EVENT_ACT_STILL_END:                 return "END STILL";
	default:                                                 return "unknown activity!";
	}
}
#endif

void inv_icm20948_sleep_us(int us)
{
	delay_us(us);
}

uint64_t inv_icm20948_get_time_us(void) {
	return timer_get_counter(TIMEBASE_TIMER);
}

uint64_t inv_icm20948_get_dataready_interrupt_time_us(void)
{
	return last_irq_time;
}

static void check_rc(int rc)
{
	if(rc == -1) {
		INV_MSG(INV_MSG_LEVEL_INFO, "BAD RC=%d", rc);
		while(1);
	}
}

/*
 * Printer function for IDD message facility
 */
static void msg_printer(int level, const char * str, va_list ap)
{
#ifdef INV_MSG_ENABLE
	static char out_str[256]; /* static to limit stack usage */
	unsigned idx = 0;
	const char * ptr = out_str;
	const char * s[INV_MSG_LEVEL_MAX] = {
		"",    // INV_MSG_LEVEL_OFF
		"[E] ", // INV_MSG_LEVEL_ERROR
		"[W] ", // INV_MSG_LEVEL_WARNING
		"[I] ", // INV_MSG_LEVEL_INFO
		"[V] ", // INV_MSG_LEVEL_VERBOSE
		"[D] ", // INV_MSG_LEVEL_DEBUG
	};

	idx += snprintf(&out_str[idx], sizeof(out_str) - idx, "%s", s[level]);
	if(idx >= (sizeof(out_str)))
		return;
	idx += vsnprintf(&out_str[idx], sizeof(out_str) - idx, str, ap);
	if(idx >= (sizeof(out_str)))
		return;
	idx += snprintf(&out_str[idx], sizeof(out_str) - idx, "\r\n");
	if(idx >= (sizeof(out_str)))
		return;

	while(*ptr != '\0') {
		uart_putc(LOG_UART_ID, *ptr);
		++ptr;
	}

#else
	(void)level, (void)str, (void)ap;
#endif
}


#if USE_IDDWRAPPER
/** @brief Dispatch received IddWrapper command
 *  @param[in]  eid             event id
 *  @param[in]  edata           event data
 *  @return     none
 */
int handle_command(enum DynProtocolEid eid,
		const DynProtocolEdata_t * edata, DynProtocolEdata_t * respdata)
{
	int rc = 0;
	uint8_t whoami;
	const int sensor = edata->sensor_id;
	inv_sensor_event_t sensor_event;

	switch(eid) {
	case DYN_PROTOCOL_EID_GET_SW_REG:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command 'get_sw_reg'(%s)", inv_sensor_2str(sensor));
		if(edata->d.command.regAddr == DYN_PROTOCOL_EREG_HANDSHAKE_SUPPORT)
			return uart_get_hw_flow_control_configuration(MAIN_UART_ID);
		else
			return 0;

	case DYN_PROTOCOL_EID_WHO_AM_I:
		rc = inv_device_whoami(device, &whoami);
		return (rc == 0) ? whoami : rc;

	case DYN_PROTOCOL_EID_RESET:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command reset(%s)", inv_sensor_2str(sensor));
		return inv_device_reset(device);

	case DYN_PROTOCOL_EID_SETUP:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command setup(%s)", inv_sensor_2str(sensor));
		rc += inv_device_setup(device);
		/*Icm20948 is set by default to 4g(Q13) and 2000dps(Q4)*/
		DynProtocol_setPrecision(&protocol, DYN_PRO_SENSOR_TYPE_ACCELEROMETER, 13);
		DynProtocol_setPrecision(&protocol, DYN_PRO_SENSOR_TYPE_GYROSCOPE, 4);
		rc += inv_device_load(device, NULL, dmp3_image, sizeof(dmp3_image), true /* verify */, NULL);
		/* Retrieve offsets stored in NV memory */
		apply_stored_offsets();
		return rc;

	case DYN_PROTOCOL_EID_CLEANUP:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command cleanup(%s)", inv_sensor_2str(sensor));
		store_offsets();
		return inv_device_cleanup(device);

	case DYN_PROTOCOL_EID_PING_SENSOR:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command ping(%s)", inv_sensor_2str(sensor));
		return inv_device_ping_sensor(device, sensor);

	case DYN_PROTOCOL_EID_START_SENSOR:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command start(%s)", inv_sensor_2str(sensor));
		return inv_device_start_sensor(device, sensor);

	case DYN_PROTOCOL_EID_STOP_SENSOR:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command stop(%s)", inv_sensor_2str(sensor));
		return inv_device_stop_sensor(device, sensor);

	case DYN_PROTOCOL_EID_SET_SENSOR_PERIOD:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command set_period(%s, %d us)",
				inv_sensor_2str(sensor), edata->d.command.period);
		return inv_device_set_sensor_period_us(device, sensor, edata->d.command.period);
		
	case DYN_PROTOCOL_EID_SET_SENSOR_TIMEOUT:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command set_timeout(%s, %d us)",
				inv_sensor_2str(sensor), edata->d.command.timeout);
		return inv_device_set_sensor_timeout(device, sensor, edata->d.command.timeout);
		
	case DYN_PROTOCOL_EID_GET_SENSOR_DATA:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command get_sensor_data(%s)", inv_sensor_2str(sensor));
		rc = inv_device_get_sensor_data(device, sensor, &sensor_event);
		respdata->sensor_id = sensor;
		convert_sensor_event_to_dyn_prot_data(&sensor_event, &respdata->d.async.sensorEvent.vdata);
		return rc;
		
	case DYN_PROTOCOL_EID_SELF_TEST:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command selft_test(%s)", inv_sensor_2str(sensor));
		return inv_device_self_test(device, sensor);
	
	case DYN_PROTOCOL_EID_SET_SENSOR_CFG:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command set_sensor_cfg(%s)", inv_sensor_2str(sensor));
		if(edata->d.command.cfg.base.type == VSENSOR_CONFIG_TYPE_REFERANCE_FRAME) {
			float ref_frame[9];
			if(edata->d.command.cfg.size > sizeof(ref_frame))
				return INV_ERROR;
			memcpy(ref_frame, &edata->d.command.cfg.buffer[0], edata->d.command.cfg.size);
			return inv_device_set_sensor_mounting_matrix(device, sensor, ref_frame);		
		} else {
			if(edata->d.command.cfg.base.type == INV_SENSOR_CONFIG_FSR) {
				// Transmit the new encoding format for the protocol
				iddwrapper_set_encoding_format(sensor, &edata->d.command.cfg.buffer);
			}
			return inv_device_set_sensor_config(device, sensor, edata->d.command.cfg.base.type,
					edata->d.command.cfg.buffer, edata->d.command.cfg.size);
		}

	case DYN_PROTOCOL_EID_GET_SENSOR_CFG:
		INV_MSG(INV_MSG_LEVEL_DEBUG, "DeviceEmdWrapper: received command get_sensor_cfg(%s)", inv_sensor_2str(sensor));
		rc = inv_device_get_sensor_config(device, sensor, edata->d.command.cfg.base.type, 
				respdata->d.response.sensorcfg.cfg.buffer, sizeof(respdata->d.response.sensorcfg.cfg.buffer));
		if(rc > 0) {
			respdata->d.response.sensorcfg.cfg.size = rc;
			respdata->d.response.sensorcfg.cfg.base.type = edata->d.command.cfg.base.type;
			respdata->d.response.sensorcfg.rc = 0;
			respdata->sensor_id = sensor;
			return 0;
		}else if(rc == 0) {
			/* should never happen */
			respdata->d.response.sensorcfg.cfg.size = 0;
			respdata->d.response.sensorcfg.cfg.base.type = edata->d.command.cfg.base.type;
			respdata->d.response.sensorcfg.rc = INV_ERROR;
			memset(respdata->d.response.sensorcfg.cfg.buffer, 0, sizeof(respdata->d.response.sensorcfg.cfg.buffer));
			return INV_ERROR;
		}else {
			/* propagate error */
			respdata->d.response.sensorcfg.cfg.size = 0;
			respdata->d.response.sensorcfg.cfg.base.type = edata->d.command.cfg.base.type;
			respdata->d.response.sensorcfg.rc = rc;
			memset(respdata->d.response.sensorcfg.cfg.buffer, 0, sizeof(respdata->d.response.sensorcfg.cfg.buffer));
			return rc;
		}

	default:
		return INV_ERROR_NIMPL;
	}
}

/*
 * Convert sensor_event to VSensorData because dynamic protocol transports VSensorData
 */
static void convert_sensor_event_to_dyn_prot_data(const inv_sensor_event_t * event, VSensorDataAny * vsensor_data)
{
	vsensor_data->base.timestamp = event->timestamp;

	switch(event->sensor) {
	case DYN_PRO_SENSOR_TYPE_RESERVED:
		break;

	case DYN_PRO_SENSOR_TYPE_ACCELEROMETER:
	case DYN_PRO_SENSOR_TYPE_GRAVITY:
	case DYN_PRO_SENSOR_TYPE_LINEAR_ACCELERATION:
		inv_dc_float_to_sfix32(&event->data.acc.vect[0], 3, 16, (int32_t *)&vsensor_data->data.u32[0]);
		vsensor_data->base.meta_data = event->data.acc.accuracy_flag;
		break;
	case DYN_PRO_SENSOR_TYPE_GYROSCOPE:
		inv_dc_float_to_sfix32(&event->data.gyr.vect[0], 3, 16, (int32_t *)&vsensor_data->data.u32[0]);
		vsensor_data->base.meta_data = event->data.gyr.accuracy_flag;
		break;
	case DYN_PRO_SENSOR_TYPE_MAGNETOMETER:
		inv_dc_float_to_sfix32(&event->data.mag.vect[0], 3, 16, (int32_t *)&vsensor_data->data.u32[0]);
		vsensor_data->base.meta_data = event->data.mag.accuracy_flag;
		break;
	case DYN_PRO_SENSOR_TYPE_UNCAL_GYROSCOPE:
		inv_dc_float_to_sfix32(&event->data.gyr.vect[0], 3, 16, (int32_t *)&vsensor_data->data.u32[0]);
		inv_dc_float_to_sfix32(&event->data.gyr.bias[0], 3, 16, (int32_t *)&vsensor_data->data.u32[3]);
		vsensor_data->base.meta_data = event->data.gyr.accuracy_flag;
		break;
	case DYN_PRO_SENSOR_TYPE_UNCAL_MAGNETOMETER:
		inv_dc_float_to_sfix32(&event->data.mag.vect[0], 3, 16, (int32_t *)&vsensor_data->data.u32[0]);
		inv_dc_float_to_sfix32(&event->data.mag.bias[0], 3, 16, (int32_t *)&vsensor_data->data.u32[3]);
		vsensor_data->base.meta_data = event->data.mag.accuracy_flag;
		break;
	case DYN_PRO_SENSOR_TYPE_GAME_ROTATION_VECTOR:
		inv_dc_float_to_sfix32(&event->data.quaternion.quat[0], 4, 30, (int32_t *)&vsensor_data->data.u32[0]);
		vsensor_data->base.meta_data = event->data.quaternion.accuracy_flag;
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

/*
 * Function to set sensor accuracy according to fsr
 */
static int iddwrapper_set_encoding_format(int sensor, const void * buffer)
{
	int rc = 0;
	if(sensor == INV_SENSOR_TYPE_ACCELEROMETER) {
		int afsr;
		memcpy(&afsr, buffer, sizeof(int));
		switch (afsr) {
		case 2: //set Q14
			rc = DynProtocol_setPrecision(&protocol, DYN_PRO_SENSOR_TYPE_ACCELEROMETER, 14);
			break;
			
		case 4: //set Q13
			rc = DynProtocol_setPrecision(&protocol, DYN_PRO_SENSOR_TYPE_ACCELEROMETER, 13);
			break;

		case 8: //set Q12
			rc = DynProtocol_setPrecision(&protocol, DYN_PRO_SENSOR_TYPE_ACCELEROMETER, 12);
			break;

		case 16: //set Q11
			rc = DynProtocol_setPrecision(&protocol, DYN_PRO_SENSOR_TYPE_ACCELEROMETER, 11);
			break;

		default:
			rc = -1;
			break;
		}
	}
	else if(sensor == INV_SENSOR_TYPE_GYROSCOPE) {
		int gfsr;
		memcpy(&gfsr, buffer, sizeof(int));
		switch (gfsr) {
		case 250: //set Q7
			rc = DynProtocol_setPrecision(&protocol, DYN_PRO_SENSOR_TYPE_GYROSCOPE, 7);
			break;
			
		case 500: //set Q6
			rc = DynProtocol_setPrecision(&protocol, DYN_PRO_SENSOR_TYPE_GYROSCOPE, 6);
			break;

		case 1000: //set Q5
			rc = DynProtocol_setPrecision(&protocol, DYN_PRO_SENSOR_TYPE_GYROSCOPE, 5);
			break;

		case 2000: //set Q4
			rc = DynProtocol_setPrecision(&protocol, DYN_PRO_SENSOR_TYPE_GYROSCOPE, 4);
			break;

		default:
			rc = -1;
			break;
		}
	}
	return rc;
}

/*
 * IddWrapper protocol handler function
 *
 * Will dispatch command and send response back
 */
static void iddwrapper_protocol_event_cb(enum DynProtocolEtype etype, enum DynProtocolEid eid,
		const DynProtocolEdata_t * edata, void * cookie)
{
	(void)cookie;

	static DynProtocolEdata_t resp_edata; /* static to take on .bss */
	static uart_dma_tx_buffer_t * dma_buffer;
	DynProTransportUartFrame_t uart_frame;
	int rc;
	int timeout = 5000; /* us */

	switch(etype) {
	case DYN_PROTOCOL_ETYPE_CMD:
		rc = handle_command(eid, edata, &resp_edata);

		/* store responses */
		switch(eid) {
		case DYN_PROTOCOL_EID_GET_FW_INFO:
			/* nothing to do. Fw version is ready to be encoded */
			break;

		case DYN_PROTOCOL_EID_GET_SENSOR_DATA:
			resp_edata.d.response.rc = rc;
			resp_edata.d.response.sensorData.status = DYN_PRO_SENSOR_STATUS_POLLED_DATA;
			/* if service returned an error reset vdata response field */
			if(resp_edata.d.response.rc)
				memset(&resp_edata.d.response.sensorData.vdata, 0, sizeof(resp_edata.d.response.sensorData.vdata));
			break;

		default:
			resp_edata.d.response.rc = rc;
			break;
		}

		while((uart_dma_tx_take_buffer(&dma_buffer) != 0) && (timeout > 0)) {
			inv_icm20948_sleep_us(10);
			timeout -= 10;
		}

		if (timeout <= 0) {
			/* if no available buffer, can't respond to the command */
			INV_MSG(INV_MSG_LEVEL_WARNING, "iddwrapper_protocol_event_cb: 'uart_dma_tx_take_buffer()' timeouted, response dropped");
			break;
		}

		/* get memory location for the iddwrapper protocol payload */
		if(DynProTransportUart_txAssignBuffer(&transport, &uart_frame, dma_buffer->data, sizeof(dma_buffer->data)) != 0) {
			goto error_dma_buffer;
		}
		if(DynProtocol_encodeResponse(&protocol, eid, &resp_edata,
				uart_frame.payload_data, uart_frame.max_payload_len, &uart_frame.payload_len) != 0) {
			goto error_dma_buffer;
		}
		if(DynProTransportUart_txEncodeFrame(&transport, &uart_frame) != 0) {
			goto error_dma_buffer;
		}
		/* respond to the command */
		dma_buffer->len = uart_frame.len;
		if(uart_dma_tx(MAIN_UART_ID, dma_buffer) != 0) {
			INV_MSG(INV_MSG_LEVEL_DEBUG, "iddwrapper_protocol_event_cb: 'uart_dma_tx()' returned error");
		}
		break;

error_dma_buffer:
		INV_MSG(INV_MSG_LEVEL_WARNING, "iddwrapper_protocol_event_cb: encode error, response dropped");
		uart_dma_tx_release_buffer(&dma_buffer);
		break;

	default:
		INV_MSG(INV_MSG_LEVEL_WARNING, "DeviceEmdWrapper: unexpected packet received. Ignored.");
		break; /* not suppose to happen */
	}
}

/*
 * EmdWrapper transport handler function
 *
 * This function will:
 *  - feed the IddWrapper protocol layer to analyse incomming CMD packets
 *  - forward byte comming from transport layer to be send over uart to the host
 */
static void iddwrapper_transport_event_cb(enum DynProTransportEvent e,
	union DynProTransportEventData data, void * cookie)
{
	(void)cookie;

	int rc;

	switch(e) {
	case DYN_PRO_TRANSPORT_EVENT_ERROR:
		INV_MSG(INV_MSG_LEVEL_ERROR, "ERROR event with value %d received from IddWrapper transport", data.error);
		break;

	case DYN_PRO_TRANSPORT_EVENT_PKT_SIZE:
		break;

	case DYN_PRO_TRANSPORT_EVENT_PKT_BYTE:
		/* Feed Dynamic protocol with bytes */
		rc = DynProtocol_processPktByte(&protocol, data.pkt_byte);
		if(rc < 0) {
			INV_MSG(INV_MSG_LEVEL_DEBUG, "DynProtocol_processPktByte(%02x) returned %d", data.pkt_byte, rc);
		}
		break;

	case DYN_PRO_TRANSPORT_EVENT_PKT_END:
		break;

	case DYN_PRO_TRANSPORT_EVENT_TX_START:
		break;

	case DYN_PRO_TRANSPORT_EVENT_TX_BYTE:
		break;

	case DYN_PRO_TRANSPORT_EVENT_TX_END:
		break;
	}
}

void apply_stored_offsets(void)
{
	uint8_t sensor_bias[84];
	int32_t acc_bias_q16[6] = {0}, gyro_bias_q16[6] = {0};
	uint8_t i, idx = 0;
	int rc;
	
	/* Retrieve Sel-test offsets stored in NV memory */
	if(flash_manager_readData(sensor_bias) != 0) {
		INV_MSG(INV_MSG_LEVEL_WARNING, "No bias values retrieved from NV memory !");
		return;
	}
	
	for(i = 0; i < 6; i++)
		gyro_bias_q16[i] = inv_dc_little8_to_int32((const uint8_t *)(&sensor_bias[i * sizeof(uint32_t)]));
	idx += sizeof(gyro_bias_q16);
	rc = inv_device_set_sensor_config(device, INV_SENSOR_TYPE_GYROSCOPE,
		VSENSOR_CONFIG_TYPE_OFFSET, gyro_bias_q16, sizeof(gyro_bias_q16));
	check_rc(rc);
	
	for(i = 0; i < 6; i++)
		acc_bias_q16[i] = inv_dc_little8_to_int32((const uint8_t *)(&sensor_bias[idx + i * sizeof(uint32_t)]));
	idx += sizeof(acc_bias_q16);
	rc = inv_device_set_sensor_config(device, INV_SENSOR_TYPE_ACCELEROMETER,
		VSENSOR_CONFIG_TYPE_OFFSET, acc_bias_q16, sizeof(acc_bias_q16));

}

void store_offsets(void)
{
	int rc = 0;
	uint8_t i, idx = 0;
	int gyro_bias_q16[6] = {0}, acc_bias_q16[6] = {0};

	uint8_t sensor_bias[84] = {0};
	
	/* Strore Self-test bias in NV memory */
	rc = inv_device_get_sensor_config(device, INV_SENSOR_TYPE_GYROSCOPE,
			VSENSOR_CONFIG_TYPE_OFFSET, gyro_bias_q16, sizeof(gyro_bias_q16));
	check_rc(rc);
	for(i = 0; i < 6; i++)
		inv_dc_int32_to_little8(gyro_bias_q16[i], &sensor_bias[i * sizeof(uint32_t)]);
	idx += sizeof(gyro_bias_q16);
	
	rc = inv_device_get_sensor_config(device, INV_SENSOR_TYPE_ACCELEROMETER,
			VSENSOR_CONFIG_TYPE_OFFSET, acc_bias_q16, sizeof(acc_bias_q16));
	check_rc(rc);
	for(i = 0; i < 6; i++)
		inv_dc_int32_to_little8(acc_bias_q16[i], &sensor_bias[idx + i * sizeof(uint32_t)]);
	idx += sizeof(acc_bias_q16);

	flash_manager_writeData(sensor_bias);
}
#endif
