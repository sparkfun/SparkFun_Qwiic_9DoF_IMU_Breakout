/**
 * \file
 *
 * \brief Board configuration.
 *
 * Copyright (c) 2014-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef CONF_BOARD_H_INCLUDED
#define CONF_BOARD_H_INCLUDED

#ifndef CONF_BOARD_USART0
#define CONF_BOARD_USART0
#endif

#ifndef CONF_BOARD_UART_CONSOLE
#define CONF_BOARD_UART_CONSOLE
#endif

#ifndef BOARD_FLEXCOM_SPI
/** FLEXCOM base address for SPI mode*/
#define BOARD_FLEXCOM_SPI    FLEXCOM5
#endif

#ifndef BOARD_FLEXCOM_USART
/** FLEXCOM base address for USART mode*/
#define BOARD_FLEXCOM_USART  FLEXCOM6
#endif

/* I2C defines - I2C_FLAG */

/** Configure TWI4 pins */
#define CONF_BOARD_TWI4

/** TWI Bus Clock 400kHz */
#define TWI_CLK					400000

/** Flexcom application to use */
#define BOARD_FLEXCOM_TWI		FLEXCOM4

/** TWI ID for simulated EEPROM application to use */
#define BOARD_ID_TWI			ID_TWI4

/** TWI Base for simulated TWI EEPROM application to use */
#define BOARD_BASE_TWI			TWI4

/** SPI MACRO definition */
#define CONF_BOARD_SPI

/** SPI slave select MACRO definition */
#define CONF_BOARD_SPI_NPCS0
#define CONF_BOARD_SPI_NPCS1

/** Spi Hw ID . */
#define SPI_ID				ID_SPI5

/** SPI base address for SPI master mode*/
#define SPI_MASTER_BASE      SPI5
/** SPI base address for SPI slave mode, (on different board) */
#define SPI_SLAVE_BASE       SPI5
/** FLEXCOM base address for SPI mode*/
#define BOARD_FLEXCOM_SPI    FLEXCOM5

#endif /* CONF_BOARD_H_INCLUDED */
