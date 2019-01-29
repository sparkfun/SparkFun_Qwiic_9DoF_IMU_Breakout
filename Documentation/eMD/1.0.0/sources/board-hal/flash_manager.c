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

#include "flash_manager.h"

#include "stm32f4xx.h"
#include "stm32f4xx_flash.h"

#define FLASH_SECTOR		FLASH_Sector_1
#define FLASH_DATA_SIZE		84
#define FLASH_HEADER_SIZE	4
#define FLASH_HEADER		0x0A0B0B0A

static uint32_t* start_sector_address;
static uint32_t* end_sector_address;

typedef enum flash_manager_status
{
	FLASH_MANAGER_STATUS_NOERROR = 0,
	FLASH_MANAGER_STATUS_EMPTY,
	FLASH_MANAGER_STATUS_FULL,
	FLASH_MANAGER_STATUS_ERASE_ERROR,
	FLASH_MANAGER_STATUS_WRITE_ERROR
} flash_manager_status;

static flash_manager_status flash_manager_getEmptySlotAddr(uint32_t * pAddress)
{
	uint32_t addr = (uint32_t)(start_sector_address);
	
	/* Check page address starting from begining */
	while (addr <= (uint32_t)(end_sector_address) - (FLASH_DATA_SIZE + FLASH_HEADER_SIZE))
	{
		/* Verify if address contents are 0xFFFFFFFF */
		if ((*(volatile uint32_t*)addr) == 0xFFFFFFFF)
		{
			*pAddress = addr;
			return FLASH_MANAGER_STATUS_NOERROR;
		}
		else
		{
			/* Next block address location */
			addr = addr + FLASH_DATA_SIZE + FLASH_HEADER_SIZE;
		}
	}
	return FLASH_MANAGER_STATUS_FULL;
}

static flash_manager_status flash_manager_eraseUnsafe(void)
{
	/* Clear pending flags (if any) */  
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
			  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

	/* Device voltage range supposed to be [1.8V to 2.1V], the operation will
	   be done by byte */ 
	if (FLASH_EraseSector(FLASH_SECTOR, VoltageRange_1) != FLASH_COMPLETE)
	{
		/* Erase sector error */
		return FLASH_MANAGER_STATUS_ERASE_ERROR;
	}
	return FLASH_MANAGER_STATUS_NOERROR;
}

/** Public functions **/

int flash_manager_eraseData(void)
{
	flash_manager_status status;
	
	/* Unlock the Flash */
	/* Enable the flash control register access */
	FLASH_Unlock();
	
	status = flash_manager_eraseUnsafe();
	
	/* Lock the Flash to disable the flash control register access (recommended
	to protect the FLASH memory against possible unwanted operation) */
	FLASH_Lock();

	return status;
}

int flash_manager_readData(uint8_t* pData)
{
	uint8_t i;
	uint32_t address;
	uint32_t header;
	uint8_t readStatus = 0;
	
	/* Get the valid Page end address */
	address = (uint32_t)(end_sector_address);
	
	/* Check each active page address starting from end */
	while (address > (uint32_t)(start_sector_address))
	{
		/* Get the header to delimitate context data */
		header = (*(volatile uint32_t*)address);
		
		/* Compare the read header */
		if (header == FLASH_HEADER)
		{
			address = address - FLASH_DATA_SIZE;
			/* Get content of data in the correct order */
			for(i = 0; i < FLASH_DATA_SIZE; i++)
			{
				pData[i] = (*(volatile uint8_t*)(address));
				address ++;
			}
			readStatus = 1;
			break;
		}
		else
		{
			address = address - sizeof(uint8_t);
		}
	}
	if(readStatus)
		return FLASH_MANAGER_STATUS_NOERROR;
	else
		return FLASH_MANAGER_STATUS_EMPTY;
}

int flash_manager_writeData(const uint8_t* pData)
{
	uint32_t i;
	FLASH_Status FlashStatus;
	uint32_t address = (uint32_t)(start_sector_address);
	static const uint8_t cFlashHeader[4] = {0xA, 0xB, 0xB, 0xA};

	/* Unlock the Flash *********************************************************/
	/* Enable the flash control register access */
	FLASH_Unlock();

	 /* Get the next empty address to write data */
	if(flash_manager_getEmptySlotAddr(&address) == FLASH_MANAGER_STATUS_FULL)
	{
		/* Erase the Flash area */
		flash_manager_eraseUnsafe();

		/* Get the start Address */
		address = (uint32_t)(start_sector_address);
	}
	
	/* Program the Flash area byte by byte */
	for(i = 0; i < FLASH_DATA_SIZE; i++)
	{
		FlashStatus = FLASH_ProgramByte(address, pData[i]);
		/* If program operation was failed, a Flash error code is returned */
		if (FlashStatus != FLASH_COMPLETE)
		{
			/* Flash write operation error */
			FLASH_Lock();
			return FLASH_MANAGER_STATUS_WRITE_ERROR;
		}
		address++;
	}
	
	/* Program the header */
	for(i = 0; i < FLASH_HEADER_SIZE; i++)
	{
		FlashStatus = FLASH_ProgramByte(address, cFlashHeader[i]);
		/* If program operation was failed, a Flash error code is returned */
		if (FlashStatus != FLASH_COMPLETE)
		{
			/* Flash write operation error */
			FLASH_Lock();
			return FLASH_MANAGER_STATUS_WRITE_ERROR;
		}
		address++;
	}
	
	/* Lock the Flash to disable the flash control register access (recommended
	to protect the FLASH memory against possible unwanted operation) ****************/
	FLASH_Lock(); 
	
	return FLASH_MANAGER_STATUS_NOERROR;
}

uint32_t** flash_manager_get_start_sector_address(void)
{
	return &start_sector_address;
}

uint32_t** flash_manager_get_end_sector_address(void)
{
	return &end_sector_address;
}