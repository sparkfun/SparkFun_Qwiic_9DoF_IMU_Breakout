/*
 * Hook implemetation for low-level routine called from IAR C library
 */

/*
 * Only for IAR compiler
 */ 
#ifdef __IAR_SYSTEMS_ICC__ 

#include <stddef.h>

#include "stm32f4xx.h"

#ifndef INVN_DRIVERS_DEVICE_NUCLEO_REMOVE_UART
#include "uart.h"
#endif
/*
 * Write char to debug UART (USART6 on Nucleo F411-RE)
 */
#pragma weak __write
size_t __write(int handle, const unsigned char * ptr, size_t len)
{
#ifndef INVN_DRIVERS_DEVICE_NUCLEO_REMOVE_UART
	size_t n;

	/* Check for stdout and stderr
	 * (only necessary if file descriptors are enabled.)
	 */
	if (handle != 1 && handle != 2) {
		return -1;
	}
	
	for (n = 0; n < len; n++) {
		int timeout = 0xFFFF;

		/*
		 * Wait for char to be sent
		 */
		while (USART_GetFlagStatus(uart_get_uart_for_printf(), USART_FLAG_TXE) == RESET) {
			if(--timeout <= 0) {
				return n;
			}
		}

		USART_SendData(uart_get_uart_for_printf(), *(ptr++));
	}
#else
	(void)handle;
	(void)ptr;
#endif
	return len;
}

#endif /* defined(__IAR_SYSTEMS_ICC__) */
