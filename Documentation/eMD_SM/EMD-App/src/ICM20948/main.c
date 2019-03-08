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

#include "ASF/sam/drivers/pio/pio.h"
#include "ASF/sam/drivers/pio/pio_handler.h"
#include "ASF/sam/drivers/twi/twi.h"
#include "ASF/sam/drivers/tc/tc.h"

#include "main.h"
#include "system.h"
#include "sensor.h"
#include "fw_version.h"

/* Flag set from device irq handler */
volatile int irq_from_device = 0;

static InvSchedulerTask blinkerLedTask;
static InvSchedulerTask commandHandlerTask;

#ifdef INV_MSG_ENABLE
static void msg_printer(int level, const char * str, va_list ap);
#endif

/*
* BlinkerLedTaskMain - Task that blinks the LED.
*/
static void BlinkerLedTaskMain(void * arg){
	(void)arg;

	ioport_toggle_pin_level(LED_0_PIN);
}

uint64_t inv_icm20948_get_time_us(void){
	return InvEMDFrontEnd_getTimestampUs();
}

/*
* CommandHandlerTaskMain - Task that monitors the UART
*/
static void CommandHandlerTaskMain(void * arg){
	(void)arg;

	int byte;
	do {
		byte = EOF;
		__disable_irq();

		if(!RingByteBuffer_isEmpty(&uart_rx_rb)) {
			byte = RingByteBuffer_popByte(&uart_rx_rb);
		}
		__enable_irq();

		if(byte != EOF) {
			DynProTransportUart_rxProcessByte(&transport, (uint8_t)byte);
		}
	} while(byte != EOF);
}

int main (void){

	int rc = 0;

	/* Hardware initialization */
	sysclk_init();
	board_init();
	sysclk_enable_peripheral_clock(ID_TC0);

	/* Configure Device - Host Interface */
	configure_console();

#ifdef INV_MSG_ENABLE
	/* Setup message logging */
	INV_MSG_SETUP(INV_MSG_ENABLE, msg_printer);
#endif

	INV_MSG(INV_MSG_LEVEL_INFO, "###################################");
	INV_MSG(INV_MSG_LEVEL_INFO, "#   ICM20948 example              #");
	INV_MSG(INV_MSG_LEVEL_INFO, "###################################");

	/* Initialize External Sensor Interrupt */
	ext_int_initialize(&ext_interrupt_handler);
	interface_initialize();

	/* Configure sysTick Timer */
	SysTick_Config(sysclk_get_cpu_hz() / MILLISECONDS_PER_SECOND);

	/*
	* Initialize icm20948 serif structure
	*/
	struct inv_icm20948_serif icm20948_serif;
	icm20948_serif.context   = 0; /* no need */
	icm20948_serif.read_reg  = idd_io_hal_read_reg;
	icm20948_serif.write_reg = idd_io_hal_write_reg;
	icm20948_serif.max_read  = 1024*16; /* maximum number of bytes allowed per serial read */
	icm20948_serif.max_write = 1024*16; /* maximum number of bytes allowed per serial write */

	icm20948_serif.is_spi = interface_is_SPI();

	/*
	* Reset icm20948 driver states
	*/
	inv_icm20948_reset_states(&icm_device, &icm20948_serif);

	inv_icm20948_register_aux_compass(&icm_device, INV_ICM20948_COMPASS_ID_AK09916, AK0991x_DEFAULT_I2C_ADDR);

	/*
	* Setup the icm20948 device
	*/
	rc = icm20948_sensor_setup();

	/*
	* Now that Icm20948 device was initialized, we can proceed with DMP image loading
	* This step is mandatory as DMP image are not store in non volatile memory
	*/
	rc += load_dmp3();
	check_rc(rc, "Error sensor_setup/DMP loading.");

	/*
	* Initialize Dynamic protocol stuff
	*/
	DynProTransportUart_init(&transport, iddwrapper_transport_event_cb, 0);
	DynProtocol_init(&protocol, iddwrapper_protocol_event_cb, 0);

	InvScheduler_init(&scheduler);
	InvScheduler_initTask(&scheduler, &commandHandlerTask, "commandHandlerTask", CommandHandlerTaskMain, 0, INVSCHEDULER_TASK_PRIO_MIN, 1);
	InvScheduler_initTask(&scheduler, &blinkerLedTask, "blinkerLedTask", BlinkerLedTaskMain, 0, INVSCHEDULER_TASK_PRIO_MIN+1, 1000000/SCHEDULER_PERIOD);
	InvScheduler_startTask(&blinkerLedTask, 0);
	InvScheduler_startTask(&commandHandlerTask, 0);

	hw_timer_start(20);		// Start the timestamp timer at 20 Hz.
	while (1) {
		InvScheduler_dispatchTasks(&scheduler);

		if (irq_from_device == 1) {
			inv_icm20948_poll_sensor(&icm_device, (void *)0, build_sensor_event_data);

			__disable_irq();
			irq_from_device = 0;
			__enable_irq();
		}
	}

	return 0;
}

#ifdef INV_MSG_ENABLE
/*
* Printer function for message facility
*/
static void msg_printer(int level, const char * str, va_list ap){

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
		usart_serial_putchar(DEBUG_UART, *ptr);
		++ptr;
	}
}
#endif

void InvEMDFrontEnd_busyWaitUsHook(uint32_t us){
	delay_us(us);
}

int InvEMDFrontEnd_isHwFlowCtrlSupportedHook(void){
	return 0;
}

int InvEMDFrontEnd_putcharHook(int c){
	if(usart_serial_putchar(CONF_UART, (uint8_t)c))
		return c;
	else
		return -1;
}

