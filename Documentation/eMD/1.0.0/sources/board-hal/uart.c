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

#include "common.h"
#include "uart.h"

#include "nvic_config.h"

#include <stdio.h>

#include "stm32f4xx.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h" 
#include "stm32f4xx_exti.h"
#include "stm32f4xx_rcc.h"

#include "Invn/EmbUtils/RingBuffer.h"

/********************************* Defines ************************************/

//////////////////  USART1
#define USART1_CLK                  RCC_APB2Periph_USART1
#define USART1_CLK_INIT             RCC_APB2PeriphClockCmd
#define USART1_IRQn                 USART1_IRQn

#define USART1_TX_PIN               GPIO_Pin_9
#define USART1_TX_GPIO_PORT         GPIOA
#define USART1_TX_GPIO_CLK          RCC_AHB1Periph_GPIOA
#define USART1_TX_SOURCE            GPIO_PinSource9
#define USART1_TX_AF                GPIO_AF_USART1

#define USART1_RX_PIN               GPIO_Pin_10
#define USART1_RX_GPIO_PORT         GPIOA
#define USART1_RX_GPIO_CLK          RCC_AHB1Periph_GPIOA
#define USART1_RX_SOURCE            GPIO_PinSource10
#define USART1_RX_AF                GPIO_AF_USART1

#define USART1_CTS_PIN              GPIO_Pin_11
#define USART1_CTS_GPIO_PORT        GPIOA
#define USART1_CTS_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define USART1_CTS_SOURCE           GPIO_PinSource11
#define USART1_CTS_AF               GPIO_AF_USART1

#define USART1_RTS_PIN              GPIO_Pin_12
#define USART1_RTS_GPIO_PORT        GPIOA
#define USART1_RTS_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define USART1_RTS_SOURCE           GPIO_PinSource12
#define USART1_RTS_AF               GPIO_AF_USART1

#define USART1_DMAx_CLK             RCC_AHB1Periph_DMA2

#define USART1_TX_DMA               DMA2
#define USART1_TX_DMA_CHANNEL       DMA_Channel_4
#define USART1_TX_DMA_STREAM        DMA2_Stream7
#define USART1_TX_DMA_IRQ_CHANNEL   DMA2_Stream7_IRQn
#define USART1_TX_DMA_IRQ_HANDLER   DMA2_Stream7_IRQHandler
#define USART1_TX_DMA_ISR           HISR
#define USART1_TX_DMA_IFCR          HIFCR
#define USART1_TX_DMA_FLAG_TCIF     DMA_FLAG_TCIF7
#define USART1_TX_DMA_IT_TCIF       DMA_IT_TCIF7

#define USART1_RX_DMA               DMA2
#define USART1_RX_DMA_CHANNEL       DMA_Channel_4
#define USART1_RX_DMA_STREAM        DMA2_Stream5
#define USART1_RX_DMA_IRQ_CHANNEL   DMA2_Stream5_IRQn
#define USART1_RX_DMA_IRQ_HANDLER   DMA2_Stream5_IRQHandler
#define USART1_RX_DMA_ISR           HISR
#define USART1_RX_DMA_IFCR          HIFCR
#define USART1_RX_DMA_FLAG_TCIF     DMA_FLAG_TCIF5
#define USART1_RX_DMA_FLAG_TEIF     DMA_FLAG_TEIF5
#define USART1_RX_DMA_IT_TCIF       DMA_IT_TCIF5
#define USART1_RX_DMA_IT_TEIF       DMA_IT_TEIF5

//////////////////  USART2
#define USART2_CLK                  RCC_APB1Periph_USART2
#define USART2_CLK_INIT             RCC_APB1PeriphClockCmd
#define USART2_IRQn                 USART2_IRQn
#define USART2_IRQHandler           USART2_IRQHandler

#define USART2_TX_PIN               GPIO_Pin_2
#define USART2_TX_GPIO_PORT         GPIOA
#define USART2_TX_GPIO_CLK          RCC_AHB1Periph_GPIOA
#define USART2_TX_SOURCE            GPIO_PinSource2
#define USART2_TX_AF                GPIO_AF_USART2

#define USART2_RX_PIN               GPIO_Pin_3
#define USART2_RX_GPIO_PORT         GPIOA
#define USART2_RX_GPIO_CLK          RCC_AHB1Periph_GPIOA
#define USART2_RX_SOURCE            GPIO_PinSource3
#define USART2_RX_AF                GPIO_AF_USART2

#define USART2_DMAx_CLK             RCC_AHB1Periph_DMA1

#define USART2_TX_DMA               DMA1
#define USART2_TX_DMA_CHANNEL       DMA_Channel_4
#define USART2_TX_DMA_STREAM        DMA1_Stream6
#define USART2_TX_DMA_IRQ_CHANNEL   DMA1_Stream6_IRQn
#define USART2_TX_DMA_IRQ_HANDLER   DMA1_Stream6_IRQHandler
#define USART2_TX_DMA_ISR           HISR
#define USART2_TX_DMA_IFCR          HIFCR
#define USART2_TX_DMA_FLAG_TCIF     DMA_FLAG_TCIF6
#define USART2_TX_DMA_IT_TCIF       DMA_IT_TCIF6

#define USART2_RX_DMA               DMA1
#define USART2_RX_DMA_CHANNEL       DMA_Channel_6
#define USART2_RX_DMA_STREAM        DMA1_Stream7
#define USART2_RX_DMA_IRQ_CHANNEL   DMA1_Stream7_IRQn
#define USART2_RX_DMA_IRQ_HANDLER   DMA1_Stream7_IRQHandler
#define USART2_RX_DMA_ISR           HISR
#define USART2_RX_DMA_IFCR          HIFCR
#define USART2_RX_DMA_FLAG_TCIF     DMA_FLAG_TCIF7
#define USART2_RX_DMA_FLAG_TEIF     DMA_FLAG_TEIF7
#define USART2_RX_DMA_IT_TCIF       DMA_IT_TCIF7
#define USART2_RX_DMA_IT_TEIF       DMA_IT_TEIF7

//////////////////  USART6
#define USART6_CLK                  RCC_APB2Periph_USART6
#define USART6_CLK_INIT             RCC_APB2PeriphClockCmd
#define USART6_IRQn                 USART6_IRQn

#define USART6_TX_PIN               GPIO_Pin_11
#define USART6_TX_GPIO_PORT         GPIOA
#define USART6_TX_GPIO_CLK          RCC_AHB1Periph_GPIOA
#define USART6_TX_SOURCE            GPIO_PinSource11
#define USART6_TX_AF                GPIO_AF_USART6

#define USART6_RX_PIN               GPIO_Pin_12
#define USART6_RX_GPIO_PORT         GPIOA
#define USART6_RX_GPIO_CLK          RCC_AHB1Periph_GPIOA
#define USART6_RX_SOURCE            GPIO_PinSource12
#define USART6_RX_AF                GPIO_AF_USART6

#define USART6_DMAx_CLK             RCC_AHB1Periph_DMA2

#define USART6_TX_DMA               DMA2
#define USART6_TX_DMA_CHANNEL       DMA_Channel_5
#define USART6_TX_DMA_STREAM        DMA2_Stream6
#define USART6_TX_DMA_IRQ_CHANNEL   DMA2_Stream6_IRQn
#define USART6_TX_DMA_IRQ_HANDLER   DMA2_Stream6_IRQHandler
#define USART6_TX_DMA_ISR           HISR
#define USART6_TX_DMA_IFCR          HIFCR
#define USART6_TX_DMA_FLAG_TCIF     DMA_FLAG_TCIF6
#define USART6_TX_DMA_IT_TCIF       DMA_IT_TCIF6

#define USART6_RX_DMA               DMA2
#define USART6_RX_DMA_CHANNEL       DMA_Channel_5
#define USART6_RX_DMA_STREAM        DMA2_Stream2
#define USART6_RX_DMA_IRQ_CHANNEL   DMA2_Stream2_IRQn
#define USART6_RX_DMA_IRQ_HANDLER   DMA2_Stream2_IRQHandler
#define USART6_RX_DMA_ISR           LISR
#define USART6_RX_DMA_IFCR          LIFCR
#define USART6_RX_DMA_FLAG_TCIF     DMA_FLAG_TCIF2
#define USART6_RX_DMA_FLAG_TEIF     DMA_FLAG_TEIF2
#define USART6_RX_DMA_IT_TCIF       DMA_IT_TCIF2
#define USART6_RX_DMA_IT_TEIF       DMA_IT_TEIF2

/********************************* Globals ************************************/

static volatile RingByteBuffer sUART1BufferTx;
static volatile RingByteBuffer sUART1BufferRx;

static volatile RingByteBuffer sUART2BufferRx;
static volatile RingByteBuffer sUART2BufferTx;

static volatile RingByteBuffer sUART6BufferTx;
static volatile RingByteBuffer sUART6BufferRx;


static uart_struct_t UART_1, UART_2, UART_6;

static struct {
	uart_dma_tx_buffer_t buffers[UART_DMA_BUF_TX_NR];
	volatile uart_dma_tx_buffer_t * ongoing;
	volatile RINGBUFFER(empty_r_buf, UART_DMA_BUF_TX_NR, uart_dma_tx_buffer_t *);
	volatile RINGBUFFER(ready_r_buf, UART_DMA_BUF_TX_NR, uart_dma_tx_buffer_t *);
}uart_dma_tx_buffers_struct;

static struct {
	uart_dma_rx_buffer_t buffers[UART_DMA_BUF_RX_NR];
	volatile uart_dma_rx_buffer_t * ongoing;
	volatile RINGBUFFER(empty_r_buf, UART_DMA_BUF_RX_NR, uart_dma_rx_buffer_t *);
	volatile RINGBUFFER(ready_r_buf, UART_DMA_BUF_RX_NR, uart_dma_rx_buffer_t *);
}uart_dma_rx_buffers_struct;


/* Configured though an API call */
static int irq_channel_premption_priority = NVIC_PRIORITY_UART;
static int irq_channel_subpriority = 0;

/* UART number used when syscalls to printf are done */
static USART_TypeDef* usart_for_printf = USART6;

/********************************* Prototypes *********************************/

static uart_struct_t * uart_get_instance(uart_num_t uart)
{
	void * pInstance = NULL;

	if(uart == UART1)
		pInstance = &UART_1;
	else if(uart == UART2)
		pInstance = &UART_2;
	else if(uart == UART6)
		pInstance = &UART_6;

	return pInstance;
}

USART_TypeDef * uart_get_uart_for_printf(void)
{
	return usart_for_printf;
}

static void uart_dma_tx_config_uartid(uart_num_t uart, uint32_t DMA_Channel, uint32_t RCC_AHB1Periph, DMA_Stream_TypeDef* DMAy_Streamx, uint8_t NVIC_IRQChannel, DMA_InitTypeDef * DMA_InitStructure, NVIC_InitTypeDef * NVIC_InitStructure)
{
	DMA_InitStructure->DMA_PeripheralBaseAddr =(uint32_t) (&(uart_get_instance(uart)->usartx->DR)) ;
	DMA_InitStructure->DMA_Channel = DMA_Channel;
	DMA_InitStructure->DMA_DIR = DMA_DIR_MemoryToPeripheral;
	// Enable the DMA clock 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph, ENABLE);
	// Configure TX DMA 
	DMA_Init(DMAy_Streamx, DMA_InitStructure);
	// Enable DMA NVIC interrupt 
	NVIC_InitStructure->NVIC_IRQChannel = NVIC_IRQChannel;
	NVIC_Init(NVIC_InitStructure);
	// Enable transfer complete interrupt 
	DMA_ITConfig(DMAy_Streamx, DMA_IT_TC, ENABLE);
	
	uart_get_instance(uart)->state_tx = UART_STATE_IDLE;
	
	uart_get_instance(uart)->TxDMAy_Streamx = DMAy_Streamx;
}

static int uart_dma_tx_config(uart_num_t uart)
{
	unsigned int i;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	uart_struct_t * uart_inst = uart_get_instance(uart);

	// Configure NVIC DMA Interrupt structure 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = irq_channel_premption_priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = irq_channel_subpriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	// Configure DMA Initialization Structure 
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;

	if(uart == UART2) {
		uart_inst->TxDMAy = USART2_TX_DMA;
		uart_inst->TxDMA_ItStatusRegAddr = &USART2_TX_DMA->USART2_TX_DMA_ISR;
		uart_inst->TxDMA_ClearItStatusRegAddr = &USART2_TX_DMA->USART2_TX_DMA_IFCR;
		uart_inst->TxDMA_ItTcIf = USART2_TX_DMA_IT_TCIF;
		uart_inst->TxDMA_FlagItTcIf = USART2_TX_DMA_FLAG_TCIF;
		uart_dma_tx_config_uartid(uart, USART2_TX_DMA_CHANNEL, USART2_DMAx_CLK, USART2_TX_DMA_STREAM, USART2_TX_DMA_IRQ_CHANNEL, &DMA_InitStructure, &NVIC_InitStructure);
	} else if(uart == UART6) {
		uart_inst->TxDMAy = USART6_TX_DMA;
		uart_inst->TxDMA_ItStatusRegAddr = &USART6_TX_DMA->USART6_TX_DMA_ISR;
		uart_inst->TxDMA_ClearItStatusRegAddr = &USART6_TX_DMA->USART6_TX_DMA_IFCR;
		uart_inst->TxDMA_ItTcIf = USART6_TX_DMA_IT_TCIF;
		uart_inst->TxDMA_FlagItTcIf = USART6_TX_DMA_FLAG_TCIF;
		uart_dma_tx_config_uartid(uart, USART6_TX_DMA_CHANNEL, USART6_DMAx_CLK, USART6_TX_DMA_STREAM, USART6_TX_DMA_IRQ_CHANNEL, &DMA_InitStructure, &NVIC_InitStructure);
	} else if(uart == UART1) {
		uart_inst->TxDMAy = USART1_TX_DMA;
		uart_inst->TxDMA_ItStatusRegAddr = &USART1_TX_DMA->USART1_TX_DMA_ISR;
		uart_inst->TxDMA_ClearItStatusRegAddr = &USART1_TX_DMA->USART1_TX_DMA_IFCR;
		uart_inst->TxDMA_ItTcIf = USART1_TX_DMA_IT_TCIF;
		uart_inst->TxDMA_FlagItTcIf = USART1_TX_DMA_FLAG_TCIF;
		uart_dma_tx_config_uartid(uart, USART1_TX_DMA_CHANNEL, USART1_DMAx_CLK, USART1_TX_DMA_STREAM, USART1_TX_DMA_IRQ_CHANNEL, &DMA_InitStructure, &NVIC_InitStructure);
	}

	RINGBUFFER_CLEAR(&uart_dma_tx_buffers_struct.empty_r_buf);
	RINGBUFFER_CLEAR(&uart_dma_tx_buffers_struct.ready_r_buf);
	for(i=0; i<(sizeof(uart_dma_tx_buffers_struct.buffers)/sizeof(uart_dma_tx_buffers_struct.buffers[0])); i++) {
		uart_dma_tx_buffer_t * temp = &uart_dma_tx_buffers_struct.buffers[i];
		RINGBUFFER_PUSH(&uart_dma_tx_buffers_struct.empty_r_buf, &temp);
	}
	uart_dma_tx_buffers_struct.ongoing = 0;

	return 0;
}

static void uart_dma_rx_config_uartid(uart_num_t uart, uint32_t DMA_Channel, uint32_t RCC_AHB1Periph, DMA_Stream_TypeDef* DMAy_Streamx, uint8_t NVIC_IRQChannel, DMA_InitTypeDef * DMA_InitStructure, NVIC_InitTypeDef * NVIC_InitStructure)
{
	uart_struct_t * uart_inst = uart_get_instance(uart);

	DMA_InitStructure->DMA_PeripheralBaseAddr =(uint32_t) (&(uart_inst->usartx->DR)) ;
	DMA_InitStructure->DMA_Channel = DMA_Channel;
	DMA_InitStructure->DMA_DIR = DMA_DIR_PeripheralToMemory;
	// Enable the DMA clock 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph, ENABLE);
	// Configure RX DMA 
	DMA_Init(DMAy_Streamx, DMA_InitStructure);
	// Enable DMA NVIC interrupt 
	NVIC_InitStructure->NVIC_IRQChannel = NVIC_IRQChannel;
	NVIC_Init(NVIC_InitStructure);
	// Enable transfer complete interrupt 
	DMA_ITConfig(DMAy_Streamx, DMA_IT_TC|DMA_IT_TE, ENABLE);

	uart_inst->RxDMAy_Streamx = DMAy_Streamx;
	USART_ITConfig(uart_inst->usartx, USART_IT_ERR, ENABLE);
	USART_DMACmd(uart_inst->usartx, USART_DMAReq_Rx, ENABLE);

	uart_inst->state_rx = UART_STATE_IDLE;
}

static int uart_dma_rx_config(uart_num_t uart)
{
	unsigned int i;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	uart_struct_t * uart_inst = uart_get_instance(uart);

	// Configure NVIC DMA Interrupt structure 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = irq_channel_premption_priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = irq_channel_subpriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	// Configure DMA Initialization Structure 
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;

	if(uart == UART2) {
		uart_inst->RxDMAy=USART2_RX_DMA;
		uart_inst->RxDMA_ItStatusRegAddr=&USART2_RX_DMA->USART2_RX_DMA_ISR;
		uart_inst->RxDMA_ClearItStatusRegAddr=&USART2_RX_DMA->USART2_RX_DMA_IFCR;
		uart_inst->RxDMA_ItTcIf=USART2_RX_DMA_IT_TCIF;
		uart_inst->RxDMA_FlagItTcIf=USART2_RX_DMA_FLAG_TCIF;
		uart_inst->RxDMA_ItTeIf=USART2_RX_DMA_IT_TEIF;
		uart_inst->RxDMA_FlagItTeIf=USART2_RX_DMA_FLAG_TEIF;
		uart_dma_rx_config_uartid(uart, USART2_RX_DMA_CHANNEL, USART2_DMAx_CLK, USART2_RX_DMA_STREAM, USART2_RX_DMA_IRQ_CHANNEL, &DMA_InitStructure, &NVIC_InitStructure);
	} else if(uart == UART6) {
		uart_inst->RxDMAy=USART6_RX_DMA;
		uart_inst->RxDMA_ItStatusRegAddr=&USART6_RX_DMA->USART6_RX_DMA_ISR;
		uart_inst->RxDMA_ClearItStatusRegAddr=&USART6_RX_DMA->USART6_RX_DMA_IFCR;
		uart_inst->RxDMA_ItTcIf=USART6_RX_DMA_IT_TCIF;
		uart_inst->RxDMA_FlagItTcIf=USART6_RX_DMA_FLAG_TCIF;
		uart_inst->RxDMA_ItTeIf=USART6_RX_DMA_IT_TEIF;
		uart_inst->RxDMA_FlagItTeIf=USART6_RX_DMA_FLAG_TEIF;
		uart_dma_rx_config_uartid(uart, USART6_RX_DMA_CHANNEL, USART6_DMAx_CLK, USART6_RX_DMA_STREAM, USART6_RX_DMA_IRQ_CHANNEL, &DMA_InitStructure, &NVIC_InitStructure);
	} else if(uart == UART1) {
		uart_inst->RxDMAy=USART1_RX_DMA;
		uart_inst->RxDMA_ItStatusRegAddr=&USART1_RX_DMA->USART1_RX_DMA_ISR;
		uart_inst->RxDMA_ClearItStatusRegAddr=&USART1_RX_DMA->USART1_RX_DMA_IFCR;
		uart_inst->RxDMA_ItTcIf=USART1_RX_DMA_IT_TCIF;
		uart_inst->RxDMA_FlagItTcIf=USART1_RX_DMA_FLAG_TCIF;
		uart_inst->RxDMA_ItTeIf=USART1_RX_DMA_IT_TEIF;
		uart_inst->RxDMA_FlagItTeIf=USART1_RX_DMA_FLAG_TEIF;
		uart_dma_rx_config_uartid(uart, USART1_RX_DMA_CHANNEL, USART1_DMAx_CLK, USART1_RX_DMA_STREAM, USART1_RX_DMA_IRQ_CHANNEL, &DMA_InitStructure, &NVIC_InitStructure);
	}

	RINGBUFFER_CLEAR(&uart_dma_rx_buffers_struct.empty_r_buf);
	RINGBUFFER_CLEAR(&uart_dma_rx_buffers_struct.ready_r_buf);
	for(i=0; i<(sizeof(uart_dma_rx_buffers_struct.buffers)/sizeof(uart_dma_rx_buffers_struct.buffers[0])); i++) {
		uart_dma_rx_buffer_t * temp = &uart_dma_rx_buffers_struct.buffers[i];
		RINGBUFFER_PUSH(&uart_dma_rx_buffers_struct.empty_r_buf, &temp);
	}
	uart_dma_rx_buffers_struct.ongoing = 0;

	return 0;
}


int uart_init(uart_init_struct_t * uart)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	uart_struct_t * pInstance = uart_get_instance(uart->uart_num);
	
        /* interrupt handling needs memory buffer */
        if ((uart->irqs_on) && ((uart->tx_size == 0) && (uart->rx_size == 0)))
                return -1;

	/* current implementation only support :
		- TX IRQ and RX IRQ
		- TX DMA and RX IRQ
		- TX DMA and RX DMA
	If TX IRQ and RX DMA configuration is to be supported, then UART IRQ handler must be extended
	and is not for now for a matter of simplicity and footprint */
	if ((uart->use_dma_for_rx) && !(uart->use_dma_for_tx))
		return -1;
	
        /* Configure USART GPIO as alternate function push-pull */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

        /* Configure USART TX/RX - 8bits - 1Stop - No parity */
        USART_InitStructure.USART_BaudRate = uart->baudrate;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

        /* Configure USART interrupt */
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = irq_channel_premption_priority;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = irq_channel_subpriority;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	/* Initialize fields with same value for any UART nb */
	if (uart->irqs_on) {
		pInstance->rx_interrupt_cb = uart->rx_interrupt_cb;
		pInstance->tx_interrupt_cb = uart->tx_interrupt_cb;
		pInstance->rx_context = uart->rx_context;
		pInstance->tx_context = uart->tx_context;
	} else {
		pInstance->rx_interrupt_cb = NULL;
		pInstance->tx_interrupt_cb = NULL;
		pInstance->rx_context = NULL;
		pInstance->tx_context = NULL;
	}
	
	if(uart->uart_num == UART2) {
		/* init structure */
		UART_2.irqs_on = uart->irqs_on;
		UART_2.use_dma_for_rx = uart->use_dma_for_rx;
		UART_2.usartx = USART2;
		UART_2.hw_flowcontrol_on = 0;
		RingByteBuffer_init((RingByteBuffer *)&sUART2BufferRx, uart->rx_buffer, uart->rx_size);
		RingByteBuffer_init((RingByteBuffer *)&sUART2BufferTx, uart->tx_buffer, uart->tx_size);
		UART_2.ptr_RxBuffer = &sUART2BufferRx;
		UART_2.ptr_TxBuffer = &sUART2BufferTx;

		/* Peripheral Clock Enable -------------------------------------------------*/
		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(USART2_TX_GPIO_CLK | USART2_RX_GPIO_CLK, ENABLE);

		/* Enable USART clock */
		USART2_CLK_INIT(USART2_CLK, ENABLE);

		/* USARTx GPIO configuration -----------------------------------------------*/ 
		/* Connect USART pins to corresponding AF */
		GPIO_PinAFConfig(USART2_TX_GPIO_PORT, USART2_TX_SOURCE, USART2_TX_AF);
		GPIO_PinAFConfig(USART2_RX_GPIO_PORT, USART2_RX_SOURCE, USART2_RX_AF);

		/* Configure USART Tx and Rx as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin = USART2_TX_PIN;
		GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = USART2_RX_PIN;
		GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStructure);

		if(UART_2.irqs_on) {
			/* Enable the USARTx Interrupt */
			NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
			NVIC_Init(&NVIC_InitStructure);
		}

		USART_Init(USART2, &USART_InitStructure);
		// RXNEIE bit must not be set if DMA is used
		if (!uart->use_dma_for_rx)
			USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

		/* Enable USART */
		USART_Cmd(USART2, ENABLE);

	} else if(uart->uart_num == UART6) {
		/* init structure */
		UART_6.irqs_on = uart->irqs_on;
		UART_6.use_dma_for_rx = uart->use_dma_for_rx;
		UART_6.hw_flowcontrol_on = 0;
		UART_6.usartx = USART6;
		RingByteBuffer_init((RingByteBuffer *)&sUART6BufferRx, uart->rx_buffer, uart->rx_size);
		RingByteBuffer_init((RingByteBuffer *)&sUART6BufferTx, uart->tx_buffer, uart->tx_size);
		UART_6.ptr_RxBuffer = &sUART6BufferRx;
		UART_6.ptr_TxBuffer = &sUART6BufferTx;

		/* Peripheral Clock Enable -------------------------------------------------*/
		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(USART6_TX_GPIO_CLK | USART6_RX_GPIO_CLK, ENABLE);

		/* Enable USART clock */
		USART6_CLK_INIT(USART6_CLK, ENABLE);

		/* USARTx GPIO configuration -----------------------------------------------*/ 
		/* Connect USART pins to corresponding AF */
		GPIO_PinAFConfig(USART6_TX_GPIO_PORT, USART6_TX_SOURCE, USART6_TX_AF);
		GPIO_PinAFConfig(USART6_RX_GPIO_PORT, USART6_RX_SOURCE, USART6_RX_AF);

		GPIO_InitStructure.GPIO_Pin = USART6_TX_PIN;
		GPIO_Init(USART6_TX_GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = USART6_RX_PIN;
		GPIO_Init(USART6_RX_GPIO_PORT, &GPIO_InitStructure);

		if(UART_6.irqs_on) {
			/* Enable the USARTx Interrupt */
			NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
			NVIC_Init(&NVIC_InitStructure);
		}

		USART_Init(USART6, &USART_InitStructure);
		// RXNEIE bit must not be set if DMA is used
		if (!uart->use_dma_for_rx)
			USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);

		/* Enable USART */
		USART_Cmd(USART6, ENABLE);

	} else if(uart->uart_num == UART1) {
		/* init structure */
		UART_1.irqs_on = uart->irqs_on;
		UART_1.use_dma_for_rx = uart->use_dma_for_rx;
		UART_1.hw_flowcontrol_on = 1;
		UART_1.usartx = USART1;
		RingByteBuffer_init((RingByteBuffer *)&sUART1BufferRx, uart->rx_buffer, uart->rx_size);
		RingByteBuffer_init((RingByteBuffer *)&sUART1BufferTx, uart->tx_buffer, uart->tx_size);
		UART_1.ptr_RxBuffer = &sUART1BufferRx;
		UART_1.ptr_TxBuffer = &sUART1BufferTx;

		/* Peripheral Clock Enable -------------------------------------------------*/
		/* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(USART1_TX_GPIO_CLK | USART1_RX_GPIO_CLK 
				| USART1_CTS_GPIO_CLK | USART1_RTS_GPIO_CLK, ENABLE);

		/* Enable USART clock */
		USART1_CLK_INIT(USART1_CLK, ENABLE);

		/* USARTx GPIO configuration -----------------------------------------------*/ 
		/* Connect USART pins to corresponding AF */
		GPIO_PinAFConfig(USART1_TX_GPIO_PORT, USART1_TX_SOURCE, USART1_TX_AF);
		GPIO_PinAFConfig(USART1_RX_GPIO_PORT, USART1_RX_SOURCE, USART1_RX_AF);
		GPIO_PinAFConfig(USART1_CTS_GPIO_PORT, USART1_CTS_SOURCE, USART1_RX_AF);
		GPIO_PinAFConfig(USART1_RTS_GPIO_PORT, USART1_RTS_SOURCE, USART1_RX_AF);

		GPIO_InitStructure.GPIO_Pin = USART1_TX_PIN;
		GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = USART1_RX_PIN;
		GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = USART1_CTS_PIN;
		GPIO_Init(USART1_CTS_GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = USART1_RTS_PIN;
		GPIO_Init(USART1_RTS_GPIO_PORT, &GPIO_InitStructure);

		if(UART_1.irqs_on) {
			/* Enable the USARTx Interrupt */
			NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
			NVIC_Init(&NVIC_InitStructure);
		}

		/* Enable Hardware Flow Control */
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;

		USART_Init(USART1, &USART_InitStructure);
		
		// RXNEIE bit must not be set if DMA is used
		if (!uart->use_dma_for_rx)
			USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

		/* Enable USART */
		USART_Cmd(USART1, ENABLE);
	}
	
	if(uart->use_for_printf)
		usart_for_printf = pInstance->usartx;
	
	if (uart->use_dma_for_tx)
		uart_dma_tx_config(uart->uart_num);
	
	if (uart->use_dma_for_rx)
		uart_dma_rx_config(uart->uart_num);

	return 0;
}

int uart_putc(uart_num_t uart, int ch)
{
	uart_struct_t * pInstance = uart_get_instance(uart);

	if(pInstance->irqs_on) {
		disable_irq();
		if(!RingByteBuffer_isFull((RingByteBuffer *)pInstance->ptr_TxBuffer))
		{
			RingByteBuffer_pushByte((RingByteBuffer *)pInstance->ptr_TxBuffer, (uint8_t) ch);
			
			if(USART_GetITStatus(pInstance->usartx, USART_IT_TXE) == RESET)
				USART_ITConfig(pInstance->usartx, USART_IT_TXE, ENABLE);
		}
		else
		{
			//return error code or you will push incomplete packets into the buffer
			ch = EOF;
		}
		enable_irq();
	} else {
		USART_SendData(pInstance->usartx, (uint8_t) ch);

		// Loop until the end of transmission 
		while (USART_GetFlagStatus(pInstance->usartx, USART_FLAG_TC) == RESET)
		{}
	}
	return ch;
}

int uart_getc(uart_num_t uart)
{
	uart_struct_t * pInstance = uart_get_instance(uart);

	if(pInstance->irqs_on) {
		int data = EOF;
		disable_irq();
		if(!RingByteBuffer_isEmpty((RingByteBuffer *)pInstance->ptr_RxBuffer))
		{
			data = RingByteBuffer_popByte((RingByteBuffer *)pInstance->ptr_RxBuffer);
		}
		enable_irq();
		return data;
	} else {
		if (USART_GetFlagStatus(pInstance->usartx, USART_FLAG_RXNE) == SET)
			return USART_ReceiveData(pInstance->usartx);
	}
	return EOF;
}

uint8_t uart_available(uart_num_t uart)
{
	uart_struct_t * pInstance = uart_get_instance(uart);

	if(pInstance->irqs_on) {
		uint8_t size;
		disable_irq();
		size = RingByteBuffer_size((RingByteBuffer *)pInstance->ptr_RxBuffer);
		enable_irq();
		return size;
	} else
		return 0;
}

int uart_puts(uart_num_t uart, const char * s)
{
	int n = 0;
	const char * pc = s;
    
    while(*pc != '\0')
    {
    	uart_putc(uart, *pc);
		++pc;
		++n;
	}

	return n;
}

int uart_get_hw_flow_control_configuration(uart_num_t uart)
{
	uart_struct_t * pInstance = uart_get_instance(uart);

	return pInstance->hw_flowcontrol_on;
}


void uart_config_irq_priority(int ch_premption_prio, int ch_subprio)
{
	irq_channel_premption_priority = ch_premption_prio;
	irq_channel_subpriority = ch_subprio;
}

int uart_dma_tx_take_buffer(uart_dma_tx_buffer_t ** buffer)
{
	int rc;
	uint16_t head, tail;

	disable_irq();
	// Check if the ringbuffer is empty 
	head = uart_dma_tx_buffers_struct.empty_r_buf.read;
	tail = uart_dma_tx_buffers_struct.empty_r_buf.write;
	if(head == tail) {
		rc = -1;
	} else {
		RINGBUFFER_POP(&uart_dma_tx_buffers_struct.empty_r_buf, buffer);
		rc = 0;
	}
	enable_irq();

	return rc;
}

int uart_dma_tx_release_buffer(uart_dma_tx_buffer_t ** buffer)
{
	int rc;
	uint16_t head, tail, size;

	disable_irq();
	// Check if the ringbuffer is full
	head = uart_dma_tx_buffers_struct.empty_r_buf.read;
	tail = uart_dma_tx_buffers_struct.empty_r_buf.write;
	size = tail - head;
	if(size == RINGBUFFER_MAXSIZE(&uart_dma_tx_buffers_struct.empty_r_buf)) {
		rc = -1;
	} else {
		RINGBUFFER_PUSH(&uart_dma_tx_buffers_struct.empty_r_buf, buffer);
		rc = 0;
	}
	enable_irq();

	return rc;
}

int uart_dma_rx_transfer_buffer(uart_dma_rx_buffer_t ** buffer)
{
	int rc;
	uint16_t head, tail, size_ready;

	disable_irq();
	
	// Check if the ringbuffers are full
	head = uart_dma_rx_buffers_struct.ready_r_buf.read;
	tail = uart_dma_rx_buffers_struct.ready_r_buf.write;
	size_ready = tail - head;
	
	if(size_ready == RINGBUFFER_MAXSIZE(&uart_dma_rx_buffers_struct.ready_r_buf)) {
		rc = -1;
	} else {
		// Add current buffer to the list of buffers ready to be processed
		RINGBUFFER_PUSH(&uart_dma_rx_buffers_struct.ready_r_buf, buffer);;
		rc = 0;
	}
	enable_irq();

	return rc;
}

int uart_dma_rx_release_buffer(uart_dma_rx_buffer_t ** buffer)
{
	int rc;
	uint16_t head, tail, size_ready, size_empty;

	disable_irq();
	
	// Check if the 'ready' ringbuffer is empty
	head = uart_dma_rx_buffers_struct.ready_r_buf.read;
	tail = uart_dma_rx_buffers_struct.ready_r_buf.write;
	size_ready = tail - head;
	
	//Check if the 'empty' ringbuffer is full
	head = uart_dma_rx_buffers_struct.empty_r_buf.read;
	tail = uart_dma_rx_buffers_struct.empty_r_buf.write;
	size_empty = tail - head;
	
	if((size_ready == 0) || (size_empty == RINGBUFFER_MAXSIZE(&uart_dma_tx_buffers_struct.empty_r_buf))) {
		rc = -1;
	} else {
		// Remove current buffer from list of buffers ready to be processed
		RINGBUFFER_POP(&uart_dma_rx_buffers_struct.ready_r_buf, buffer);
		// Add back current buffer to the list of buffers DMA can take to fill in 
		RINGBUFFER_PUSH(&uart_dma_rx_buffers_struct.empty_r_buf, buffer);
		rc = 0;
	}
	enable_irq();

	return rc;
}

static int uart_dma_do_tx(uart_num_t uart, uart_dma_tx_buffer_t * buffer)
{
	int timeout = 1000;
	uart_struct_t * uart_inst = uart_get_instance(uart);

	// Set the number of data units to be transfered 
	uart_inst->TxDMAy_Streamx->NDTR = buffer->len;
	// Set the source address of the transfer 
	uart_inst->TxDMAy_Streamx->M0AR = (uint32_t)buffer->data;

	// Enable DMA transfer
	uart_inst->TxDMAy_Streamx->CR |= DMA_SxCR_EN;
	// Enable USART DMA transmit request
	uart_inst->usartx->CR3 |= USART_DMAReq_Tx;

	// Wait for the stream to be enabled 
	while (((uart_inst->TxDMAy_Streamx->CR & DMA_SxCR_EN) == 0) && (timeout-- > 0));
	if (timeout == 0) {
		uart_dma_tx_release_buffer(&buffer);
		uart_inst->state_tx = UART_STATE_IDLE;
		return -1;
	}
	uart_dma_tx_buffers_struct.ongoing = buffer;

	return 0;
}

int uart_dma_tx(uart_num_t uart, uart_dma_tx_buffer_t * buffer)
{
	int rc = 0;
	uart_struct_t * uartx = uart_get_instance(uart);
	
	disable_irq();
	if(uartx) {
		if(uartx->state_tx == UART_STATE_IDLE) {
			uartx->state_tx = UART_STATE_BUSY_TX;
			// Start the DMA transfer 
			rc = uart_dma_do_tx(uart, buffer);
		} else if(uartx->state_tx == UART_STATE_BUSY_TX) {
			RINGBUFFER_PUSH(&uart_dma_tx_buffers_struct.ready_r_buf, &buffer);
		}
	} else {
		rc = -1;
	}
	enable_irq();

	return rc;
}

int uart_dma_do_rx(uart_num_t uart, uart_dma_tx_buffer_t * buffer, uint16_t expected_bytes)
{
	int timeout = 1000;
	uart_struct_t * uart_inst = uart_get_instance(uart);

	// Wait for the stream to be disabled first before reprogrammming DMA
	while (((uart_inst->RxDMAy_Streamx->CR & DMA_SxCR_EN) != 0) && (timeout-- > 0));
	if (timeout == 0)
		return -1;
	
	// Set the number of data units to be transfered 
	uart_inst->RxDMAy_Streamx->NDTR = expected_bytes;
	// Set the destination address of the transfer 
	uart_inst->RxDMAy_Streamx->M0AR = (uint32_t)(buffer->data + buffer->len);
	// Increment buffer length
	buffer->len += expected_bytes;

	uart_inst->state_rx = UART_STATE_BUSY_RX;
	
	// Buffer we are working on is now current buffer manipulated by DMA
	uart_dma_rx_buffers_struct.ongoing = buffer;
	
	// Enable DMA transfer
	uart_inst->RxDMAy_Streamx->CR |= (uint32_t)DMA_SxCR_EN;

	// Wait for the stream to be actually enabled 
	timeout = 1000;
	while (((uart_inst->RxDMAy_Streamx->CR & DMA_SxCR_EN) == 0) && (timeout-- > 0));
	if (timeout == 0) {
		uart_inst->state_rx = UART_STATE_IDLE;
		return -1;
	}

	return 0;
}

int uart_dma_rx(uart_num_t uart)
{
	int rc = 0;
	uart_dma_tx_buffer_t * buffer;
	uint16_t head, tail;
	uart_struct_t * uartx = uart_get_instance(uart);
	
	disable_irq();
	if(uartx) {
		if(uartx->state_rx == UART_STATE_IDLE) {
			// Start the DMA transfer for 1 byte if we have at least 1 empty buffer available for DMA
			head = uart_dma_rx_buffers_struct.empty_r_buf.read;
			tail = uart_dma_rx_buffers_struct.empty_r_buf.write;
			if(head != tail) {
				RINGBUFFER_POP(&uart_dma_rx_buffers_struct.empty_r_buf, &buffer);
				rc = uart_dma_do_rx(uart, buffer, 1);
			} else {
				rc = -1;
			}
		} else {
			rc = -1;
		}
	} else {
		rc = -1;
	}
	enable_irq();

	return rc;
}

int uart_dma_rx_abort(uart_num_t uart)
{
	uart_struct_t * UART_x = uart_get_instance(uart);
	int timeout = 1000;	
	int rc = 0;
	
	disable_irq();

	// First disable DMA
	// There is no particular action to do, except to clear the EN bit in the
	// DMA_SxCR register to disable the stream. The stream may take time to be disabled
	DMA_Cmd(UART_x->RxDMAy_Streamx,DISABLE);

	// Wait for the stream to be disabled 
	while ((DMA_GetCmdStatus(UART_x->RxDMAy_Streamx) != DISABLE) && (timeout-- > 0));
	if (timeout == 0)
		return -1;
	
	// The transfer complete interrupt flag (TCIF in the
	// DMA_LISR or DMA_HISR register) is set in order to indicate the end of transfer. 
	while ((DMA_GetITStatus(UART_x->RxDMAy_Streamx, UART_x->RxDMA_ItTcIf) == RESET) && (timeout-- > 0));
	
	// IRQ are disabled at that point, we must clear flags and interrupt pending bits right now
	// to not end up in DMA IRQ handler once IRQ are unmasked
	DMA_ClearFlag(UART_x->RxDMAy_Streamx, UART_x->RxDMA_FlagItTcIf);
	DMA_ClearITPendingBit(UART_x->RxDMAy_Streamx, DMA_IT_TC);

	// Reset number of bytes received so far in current buffer
	uart_dma_rx_buffers_struct.ongoing->len = 0;
	
	// Start the DMA transfer for 1 byte again
	rc = uart_dma_do_rx(uart, (uart_dma_tx_buffer_t *)uart_dma_rx_buffers_struct.ongoing, 1);
		
	enable_irq();

	return rc;
}


/* Interrupt management ------------------------------------------------------*/
static void commonUSARTx_read_data_IRQHandler(uart_struct_t * UART_x)
{
	if(!RingByteBuffer_isFull((RingByteBuffer *)UART_x->ptr_RxBuffer))
	{
		uint8_t lRxByte = USART_ReceiveData(UART_x->usartx);
		RingByteBuffer_pushByte((RingByteBuffer *)UART_x->ptr_RxBuffer, lRxByte);
	}
	if(UART_x->rx_interrupt_cb)
		UART_x->rx_interrupt_cb(UART_x->rx_context);
}

/**
  * @brief  Checks as quickly as possible whether the specified USART interrupt has occurred or not.
  * No additionall check is done, can be called from an IRQ handler to improve timing and ensure all
  * types of build will behave fine.
  * @param  USARTx: where x can be 1, 2, or 6 to select the USART or 
  *         UART peripheral.
  * @param  USART_IT: specifies the USART interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg USART_IT_TXE:  Transmit Data Register empty interrupt
  *            @arg USART_IT_RXNE: Receive Data register not empty interrupt
  *            @arg USART_IT_ORE_RX : OverRun Error interrupt if the RXNEIE bit is set
  * @warning There is no check done on any parameter, this must stick to description above
  * @retval The new state of USART_IT (SET or RESET).
  */
static ITStatus commonUSARTx_GetITStatus_IRQHandler(USART_TypeDef* USARTx, uint16_t USART_IT)
{
	uint32_t itmask = USARTx->CR1;
	uint32_t bitpos = USARTx->SR;
  
	if( (USART_IT == USART_IT_RXNE) || (USART_IT == USART_IT_ORE_RX) )
	{
		itmask &= USART_FLAG_RXNE;
		if (USART_IT == USART_IT_ORE_RX)
			bitpos &= USART_FLAG_ORE;
		else
			bitpos &= USART_FLAG_RXNE;
	} else
	{
		itmask &= USART_FLAG_TXE;
		bitpos &= USART_FLAG_TXE;
	}
  
  
  if ((itmask != (uint16_t)RESET)&&(bitpos != (uint16_t)RESET))
  {
    return SET;
  }
  else
  {
    return RESET;
  }
}

static void commonUSARTx_IRQHandler(uart_num_t uart)
{
	uart_struct_t * UART_x = uart_get_instance(uart);
	// If we use DMA for RX then only case for which we might end up there is for error management of ORE
	// since TX without DMA while RX uses DMA is forbidden for now
	if (UART_x->use_dma_for_rx) {
		// read SR and then DR to reset ORE bit
		(void)commonUSARTx_GetITStatus_IRQHandler(UART_x->usartx, USART_IT_ORE_RX);
		(void)USART_ReceiveData(UART_x->usartx);
		// and then abort current DMA transfer 
		if( uart_dma_rx_abort(uart) ) {
			// Give hand to the application with special buffer pointer value
			// so that it decides what to do upon such an unrecoverable error
			uart_rx_context_t context;
			context.uart = uart;
			context.buf = 0;
			if(UART_x->rx_interrupt_cb)
				UART_x->rx_interrupt_cb(&context);
		}
		
	// Otherwise this is where we are notified of
	// - new data available for RX
	// - byte transmission on TX is done
	// - any kind of error
	} else {
		uint8_t overrun_error = 0;
		uint8_t rxne_detected = 0;
		uint8_t txe_detected = 0;
		// UART IRQ handling must absolutely be done with sequence :
		// 1. overrun detection, because it must be quickly fixed
		// 2. reception buffer, so that we do not stuck next byte to be received
		// 3. new byte transmission
		// 4. corner case detection of any remaining data to be read in case none of [1-3] is encountered
		
		// Reason for IRQ to be triggered is to be checked and analyzed very quickly,
		// so use speed optimized driver functions when looking at IT status
		
		// Disable IRQ so that we are sure we are not preempted since timing is very important
		disable_irq();
		
		// Handle case of overrun, need to quickly flush RX buffer to not lose any byte
		if(commonUSARTx_GetITStatus_IRQHandler(UART_x->usartx, USART_IT_ORE_RX) != RESET)
		{
			overrun_error = 1;
			commonUSARTx_read_data_IRQHandler(UART_x);
		}
		// Handle case of RX buffer not empty, need to flush it
		if(commonUSARTx_GetITStatus_IRQHandler(UART_x->usartx, USART_IT_RXNE) != RESET)
		{
			rxne_detected = 1;
			commonUSARTx_read_data_IRQHandler(UART_x);
		}
		// Handle case of TX buffer empty, ready to send a new data
		if(commonUSARTx_GetITStatus_IRQHandler(UART_x->usartx, USART_IT_TXE) != RESET)
		{
			txe_detected = 1;
			if(!RingByteBuffer_isEmpty((RingByteBuffer *)UART_x->ptr_TxBuffer))
			{
				uint8_t lTxByte = RingByteBuffer_popByte((RingByteBuffer *)UART_x->ptr_TxBuffer);
				USART_SendData(UART_x->usartx, lTxByte);
			} 
			else
				USART_ITConfig(UART_x->usartx, USART_IT_TXE, DISABLE);
			
			if(UART_x->tx_interrupt_cb)
				UART_x->tx_interrupt_cb(UART_x->tx_context);
		}
		// IRQ was triggered but not reason for IRQ was actually detected so we are in very special case
		// when the last valid data is read in the RDR at the same time as the new (and lost) data is received.
		// It may also occur when the new data is received during the reading sequence
		// (between the USART_SR register read access and the USART_DR read access).
		// In any case, there is something to be read from DR register even if SR does not say so...
		if (!overrun_error && !rxne_detected && !txe_detected) {
			commonUSARTx_read_data_IRQHandler(UART_x);
		}
		
		enable_irq();
	}
}

void USART1_IRQHandler(void)
{
	commonUSARTx_IRQHandler(UART1);
}

void USART2_IRQHandler(void)
{
	commonUSARTx_IRQHandler(UART2);
}

void USART6_IRQHandler(void)
{
	commonUSARTx_IRQHandler(UART6);
}

static void commonUSARTx_TX_DMA_IRQ_HANDLER(uart_num_t uart)
{
	uart_dma_tx_buffer_t * buffer;
	uint16_t head, tail;
	uart_struct_t * uart_inst = uart_get_instance(uart);

	if((*uart_inst->TxDMA_ItStatusRegAddr & uart_inst->TxDMA_FlagItTcIf) != 0)
	{
		// Clear flags and interrupt pending bits 
		*uart_inst->TxDMA_ClearItStatusRegAddr |= uart_inst->TxDMA_ItTcIf;

		// We are done with transmission of current buffer, it can be made available again for any next transfer
		// So we can repopulate empty_r_buf with the current buffer pointer 
		if(uart_dma_tx_buffers_struct.ongoing) {
			RINGBUFFER_PUSH(&uart_dma_tx_buffers_struct.empty_r_buf, (uart_dma_tx_buffer_t **)&uart_dma_tx_buffers_struct.ongoing);
		}		

		// Check if ringbuffer is empty 
		head = uart_dma_tx_buffers_struct.ready_r_buf.read;
		tail = uart_dma_tx_buffers_struct.ready_r_buf.write;
		if(head == tail) {
			// Nothing else to transmist so update state variable 
			uart_inst->state_tx = UART_STATE_IDLE;
			uart_dma_tx_buffers_struct.ongoing = 0;
		} else {
			// Transmit next buffer
			RINGBUFFER_POP(&uart_dma_tx_buffers_struct.ready_r_buf, &buffer);
			uart_dma_do_tx(uart, buffer);
		}
	}
}

void USART2_TX_DMA_IRQ_HANDLER(void)
{
	commonUSARTx_TX_DMA_IRQ_HANDLER(UART2);
}

void USART6_TX_DMA_IRQ_HANDLER(void)
{
	commonUSARTx_TX_DMA_IRQ_HANDLER(UART6);
}

void USART1_TX_DMA_IRQ_HANDLER(void)
{
	commonUSARTx_TX_DMA_IRQ_HANDLER(UART1);
}

static void commonUSARTx_RX_DMA_IRQ_HANDLER(uart_num_t uart)
{
	uart_rx_context_t context;
	uart_struct_t * uart_inst = uart_get_instance(uart);

	// UART DMA full RX transfer is done
	if((*uart_inst->RxDMA_ItStatusRegAddr & uart_inst->RxDMA_FlagItTcIf) != 0)
	{
		// Clear flags and interrupt pending bits 
		*uart_inst->RxDMA_ClearItStatusRegAddr |= uart_inst->RxDMA_ItTcIf;
		
		// we are done with current DMA transfer, reset its state
		uart_inst->state_rx = UART_STATE_IDLE;
		
		// Give hand to the application with the information about bytes we received
		context.uart = uart;
		context.buf = uart_dma_rx_buffers_struct.ongoing;
		if(uart_inst->rx_interrupt_cb)
			uart_inst->rx_interrupt_cb(&context);
	}
	
	// UART DMA RX transfer error happened
	if((*uart_inst->RxDMA_ItStatusRegAddr & uart_inst->RxDMA_FlagItTeIf) != 0)
	{
		// Clear flags and interrupt pending bits 
		*uart_inst->RxDMA_ClearItStatusRegAddr |= uart_inst->RxDMA_ItTeIf;
		
		// Give hand to the application with special buffer pointer value
		// so that it decides what to do upon such an error
		context.uart = uart;
		context.buf = 0;
		if(uart_get_instance(uart)->rx_interrupt_cb)
			uart_get_instance(uart)->rx_interrupt_cb(&context);
	}
}

void USART2_RX_DMA_IRQ_HANDLER(void)
{
	commonUSARTx_RX_DMA_IRQ_HANDLER(UART2);
}

void USART6_RX_DMA_IRQ_HANDLER(void)
{
	commonUSARTx_RX_DMA_IRQ_HANDLER(UART6);
}

void USART1_RX_DMA_IRQ_HANDLER(void)
{
	commonUSARTx_RX_DMA_IRQ_HANDLER(UART1);
}

/* Embedded Utils hook implmentation ------------------------------------------*/

// needed for RingByteBuffer embedded utils
void InvAssert(const char *predicate, const char *file, unsigned line) 
{
	(void)predicate;
	(void)line;
	(void)file;
}