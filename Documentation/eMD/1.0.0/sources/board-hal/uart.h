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

/** @defgroup Uart Uart
	@ingroup  Low_Level_Driver
	@{
*/
#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>

#include "stm32f4xx.h"
#include "Invn/EmbUtils/RingByteBuffer.h"

/** @brief Size in bytes of data buffers handled by uart-dma */
#ifndef UART_DMA_MAX_TRANSF_SIZE
 #define UART_DMA_MAX_TRANSF_SIZE 128
#endif
/** @brief Amount of buffer driver is able to record before being sent over DMA */
#ifndef UART_DMA_BUF_TX_NR
 #define UART_DMA_BUF_TX_NR          256
#endif
/** @brief Amount of buffer driver is able to record once received from DMA before being processed by MCU */
#ifndef UART_DMA_BUF_RX_NR
 #define UART_DMA_BUF_RX_NR          16
#endif

/**
* UART peripherals available
*/
typedef enum uart_num {
	UART1,      /**< USART1 with handshakes connected to USB on Nucleo Carrier board revB */
	UART2,      /**< USB - USART2 connected to ST LINK */
	UART6       /**< USART6 connected to USB on Nucleo Carrier board revA */
}uart_num_t;

typedef enum {
	UART_STATE_IDLE,
	UART_STATE_BUSY_TX,
	UART_STATE_BUSY_RX
}uart_dma_state_t;

/** @brief Structure defining data buffer handled by uart_dma API
 *
 */
typedef struct {
	uint8_t      data[UART_DMA_MAX_TRANSF_SIZE]; /**< buffer of data */
	uint16_t     len; 						/**< number of bytes used in buffer */
}uart_dma_tx_buffer_t;

/** @brief Same type of buffers are used for rx and tx */
typedef uart_dma_tx_buffer_t uart_dma_rx_buffer_t;

/** @brief UART object definition.
 *
 * Contains all the fields needed by the present API to handle an UART device.
 */
typedef struct uart_struct {
	int irqs_on;								/**< Use IRQ if set */
	int hw_flowcontrol_on;						/**< Use RTS/CTS flowcontrol if set */
	char use_dma_for_rx;						/**< Use DMA for UART RX if set */
	DMA_TypeDef* RxDMAy;						/**< DMA to be used for UART RX */
	DMA_Stream_TypeDef* RxDMAy_Streamx;			/**< DMA stream to be used for UART RX */
	volatile uint32_t * RxDMA_ItStatusRegAddr;		/**< DMA interrupt status register */
	volatile uint32_t * RxDMA_ClearItStatusRegAddr;	/**< DMA clear interrupt status register */
	uint32_t RxDMA_ItTcIf;						/**< Transfer complete DMA interruption to be used for UART RX*/
	uint32_t RxDMA_FlagItTcIf;					/**< Transfer complete DMA flag interruption to be used for UART RX*/
	uint32_t RxDMA_ItTeIf;						/**< Transfer error DMA interruption to be used for UART RX */
	uint32_t RxDMA_FlagItTeIf;					/**< Transfer error DMA flag interruption to be used for UART RX */
	DMA_TypeDef* TxDMAy;						/**< DMA to be used for UART TX */
	DMA_Stream_TypeDef* TxDMAy_Streamx;			/**< DMA stream to be used for UART TX */
	volatile uint32_t * TxDMA_ItStatusRegAddr;		/**< DMA interrupt status register to be used for UART TX*/
	volatile uint32_t * TxDMA_ClearItStatusRegAddr;	/**< DMA clear interrupt status register to be used for UART TX*/
	uint32_t TxDMA_ItTcIf;						/**< Transfer complete DMA interruption to be used for UART TX*/
	uint32_t TxDMA_FlagItTcIf;					/**< Transfer complete DMA flag interruption to be used for UART TX*/
	USART_TypeDef * usartx;
	volatile RingByteBuffer * ptr_TxBuffer;		/**< pointer on circular buffer used to store Tx byte if DMA is off */
	volatile RingByteBuffer * ptr_RxBuffer;		/**< pointer on circular buffer used to store Rx byte if DMA is off */
	volatile uart_dma_state_t state_tx;			/**< Current state of DMA UART TX */
	volatile uart_dma_state_t state_rx;			/**< Current state of DMA UART RX */
	void (*rx_interrupt_cb)(void * context);	/** < Callback executed by UART rx IRQ handler */
	void (*tx_interrupt_cb)(void * context);	/** < Callback executed by UART tx IRQ handler */
	void *rx_context;							/** < Context passed to rx_interrupt_cb */
	void *tx_context;							/** < Context passed to tx_interrupt_cb */
}uart_struct_t;

/** @brief UART initialisation structure.
 *
 * Such a structure should be filled and passed to uart_init function to 
 * allow using an UART.
 */
typedef struct uart_init_struct {
	uart_num_t uart_num;		/** < UART peripheral */
	char irqs_on;				/** < UART RX/TX IRQs enable option */
	char use_for_printf;		/** < Boolean, use current UART as a pipe for printf, default is UART6 */
	char use_dma_for_tx;		/** < if set, configures the UART to use the DMA for TX */
	char use_dma_for_rx;		/** < if set, configures the UART to use the DMA for RX */
	uint8_t * tx_buffer;		/** < memory buffer allocated for UART TX FIFO */
	uint8_t * rx_buffer;		/** < memory buffer allocated for UART RX FIFO */
	uint16_t tx_size;			/** < buffer size for UART TX FIFO */
	uint16_t rx_size;			/** < buffer size for UART RX FIFO */
	int baudrate;				/** < UART baudrate speed */
	void (*rx_interrupt_cb)(void * context); /** < Callback executed by UART rx IRQ handler */
	void (*tx_interrupt_cb)(void * context); /** < Callback executed by UART tx IRQ handler */
	void *rx_context;       	/** < Context passed to rx_interrupt_cb */
	void *tx_context;       	/** < Context passed to tx_interrupt_cb */
}uart_init_struct_t;


/** @brief UART RX context definition */
typedef struct uart_rx_context {
	uart_num_t uart;						/** < UART ID on which the buffer was received */
	volatile uart_dma_rx_buffer_t * buf;	/** < Buffer received and for which context is to be notified */
}uart_rx_context_t;

/** @brief Initialize UART peripheral to communicate with host
* @param[in] uart       set of parameters to init UART
* @return 0 on success, negative value otherwise
*/
int uart_init(uart_init_struct_t * uart);
	
/** @brief Get UART ID to be used when printf is called
* @param[in] None
* @return USART to be used
*/
USART_TypeDef * uart_get_uart_for_printf(void);

/** @brief Prints a text on UART 
* @param[in] uart UART peripheral
* @param[in] s    the text to be sent to the console
*/
int uart_puts(uart_num_t uart, const char * s);

/** @brief Prints a character on UART 
* @param[in] uart UART peripheral
* @param[in] ch   character be sent to the console
*/
int uart_putc(uart_num_t uart, int ch);

/** @brief Gets a character received on UART 
* @param[in] uart     UART peripheral
*/
int uart_getc(uart_num_t uart);

/** @brief Gets the number of bytes available received on UART. This API is only relevant when using IRQ.
* @param[in] uart     UART peripheral
* @return byte number available 
*/
uint8_t uart_available(uart_num_t uart);

/** @brief Return the HW flow control configuration for the UART
* @param[in] uart     UART peripheral
* @return 0 is HW flow control actived, -1 otherwise 
*/
int uart_get_hw_flow_control_configuration(uart_num_t uart);

/** @brief Gets a pointer to an available DMA buffer
* @param[in] buffer       address of a placeholder for the buffer pointer
* @return 0 on success, negative value if no buffers available
*/
int uart_dma_tx_take_buffer(uart_dma_tx_buffer_t ** buffer);


/** @brief Informs the DMA driver that the buffer is no longer in use
* @Note: This function has to be called if you 'took' a buffer
* that you won't send over the UART.
* @param[in] buffer       address of the buffer pointer
* @return 0 on success, negative value if all the buffers are already released
*/
int uart_dma_tx_release_buffer(uart_dma_tx_buffer_t ** buffer);

/** @brief Triggers the transmit of a DMA buffer.
* @param[in] buffer       pointer to the buffer to send
* @return 0 on success, negative value if an error ocurred
*/
int uart_dma_tx(uart_num_t uart, uart_dma_tx_buffer_t * buffer);

/** @brief Informs the DMA driver that the current buffer DMA is working on
* can now be transfered to the pool of buffers fully filled in and ready to be parsed
* @param[in] buffer       address of the buffer pointer
* @return 0 on success, negative value if no free space remaining in pool of buffers
*/
int uart_dma_rx_transfer_buffer(uart_dma_rx_buffer_t ** buffer);

/** @brief Informs the DMA driver that buffer has been parsed and can be dropped
* @param[in] buffer       address of the buffer pointer
* @return 0 on success, negative value if pool of buffers ready to be parsed is already empty
*/
int uart_dma_rx_release_buffer(uart_dma_rx_buffer_t ** buffer);

/** @brief Triggers the reception from UART in a DMA buffer already allocated for expected_bytes bytes
* @param[in] buffer       pointer to the buffer to send
* @param[in] expected_bytes       number of bytes to receive before a new DMA IRQ is triggered
* @return 0 on success, negative value if an error ocurred
*/
int uart_dma_do_rx(uart_num_t uart, uart_dma_tx_buffer_t * buffer, uint16_t expected_bytes);

/** @brief Triggers the reception of 1 byte from UART in a new DMA buffer
* @param[in] buffer       pointer to the buffer to send
* @return 0 on success, negative value if an error ocurred
*/
int uart_dma_rx(uart_num_t uart);

/** @brief Configure UART preemption priority and subpriority, at global level (same for all UARTs)
* @param[in] ch_premption_prio  Goes in NVIC_IRQChannelPreemptionPriority
* @param[in] ch_subprio         Goes in NVIC_IRQChannelSubPriority
*/
void uart_config_irq_priority(int ch_premption_prio, int ch_subprio);

#endif // __UART_H__

/** @} */
