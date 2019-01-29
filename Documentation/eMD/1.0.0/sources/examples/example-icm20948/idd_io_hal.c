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
 
#include "idd_io_hal.h"

// board drivers
#include "i2c_master.h"
#include "i2c_slave.h"
#include "spi_master.h"
#include "delay.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* Host Serif object definition for SPI ***************************************/

static int idd_io_hal_init_spi(void)
{
	spi_master_init(SPI_NUM1, SPI_1562KHZ);
	return 0;
}

static int idd_io_hal_read_reg_spi(uint8_t reg, uint8_t * rbuffer, uint32_t rlen)
{
	return spi_master_read_register(SPI_NUM1, reg, rlen, rbuffer);
}

static int idd_io_hal_write_reg_spi(uint8_t reg, const uint8_t * wbuffer, uint32_t wlen)
{
	return spi_master_write_register(SPI_NUM1, reg, wlen, wbuffer);
}

static const inv_host_serif_t serif_instance_spi = {
	idd_io_hal_init_spi,
	0,
	idd_io_hal_read_reg_spi,
	idd_io_hal_write_reg_spi,
	0,
	1024*32, /* max transaction size */
	1024*32, /* max transaction size */
	INV_HOST_SERIF_TYPE_SPI,
};

const inv_host_serif_t * idd_io_hal_get_serif_instance_spi(void)
{
	return &serif_instance_spi;
}
