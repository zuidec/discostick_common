/*
 * w25q16jv.h
 *
 *  Created on: Jan 2, 2025
 *      Author: zuidec
 */

#ifndef INC_W25Q16JV_H_
#define INC_W25Q16JV_H_

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

#define W25Q16_WRITE_ENABLE					(0x06)
#define W25Q16_WRITE_DISABLE				(0x04)
#define W25Q16_READ_DATA					(0x03)
#define W25Q16_FAST_READ					(0x0B)
#define W25Q16_PAGE_PROGRAM					(0x02)
#define W25Q16_WRITE_SR1					(0x01)
#define W25Q16_READ_SR1						(0x05)
#define W25Q16_WRITE_SR2					(0x31)
#define W25Q16_READ_SR2						(0x35)
#define W25Q16_WRITE_SR3					(0x11)
#define W25Q16_READ_SR3						(0x15)
#define W25Q16_SECTOR_ERASE_4K				(0x20)
#define W25Q16_BLOCK_ERASE_32K				(0x52)
#define W25Q16_BLOCK_ERASE_64K				(0xD8)
#define W25Q16_CHIP_ERASE					(0xC7)
#define W25Q16_REL_PWR_DOWN					(0xAB)
#define W25Q16_READ_MFR_ID					(0x90)
#define W25Q16_READ_UNIQUE_ID				(0x4B)
#define W25Q16_READ_JEDEC_ID				(0x9F)
#define W25Q16_ERASE_SECURITY_REGS			(0x44)
#define W25Q16_PROG_SECURITY_REGS			(0x42)
#define W25Q16_READ_SECURITY_REGS			(0x48)
#define W25Q16_ENABLE_RESET					(0x66)
#define W25Q16_RESET_DEVICE					(0x99)

#define W25Q16_SPI_TIMEOUT					(500)
#define W25Q16_MFR_ID						(0xEF)
#define W25Q16_BLOCK_SIZE                   (4096) 
#define W25Q16_BLOCK_COUNT                  (512)
#define W25Q16_DEV_ID						(0x4015)
#define W25Q16_MEMTYPE						(0x14)
#define W25Q16_CAPACITY						(0x4015)
#define W25Q16_JEDEC_ID						((uint32_t)(W25Q16_MFR_ID << 16) | (W25Q16_DEV_ID))

#define FLASH_PAGE_SIZE                     (256)
#define FLASH_MAGIC                         (0x0E0FC0DE) 
#ifndef FLASH_MAX_PAYLOAD
#define FLASH_MAX_PAYLOAD                   (1024) // 1KB 
#endif

enum w25q16_error   {
    W25Q16_ERR_OK   = 0,
    W25Q16_ERR_NULL = -255,
    W25Q16_ERR_CRC  = -256,
    W25Q16_ERR_BADID= -257
};

typedef struct	{
	SPI_HandleTypeDef* spi;
	GPIO_TypeDef* cs_port;
	uint32_t cs_pin;
    uint8_t hw_cs;
}w25q16_handle_t;


uint32_t w25q16_get_chip_erase_confirmation(void);
int32_t w25q16_init(w25q16_handle_t* flash_dev);

void w25q16_read(w25q16_handle_t* flash_dev, uint32_t address, uint8_t* buffer, uint16_t buffer_size);
int32_t w25q16_write(w25q16_handle_t* flash_dev, uint32_t address, uint8_t* buffer, uint16_t bytes_to_write);

void w25q16_enable_write(w25q16_handle_t* flash_dev);
void w25q16_disable_write(w25q16_handle_t* flash_dev);

void w25q16_sector_erase_4k(w25q16_handle_t* flash_dev, uint32_t address);
void w25q16_block_erase_32k(w25q16_handle_t* flash_dev, uint32_t address);
void w25q16_block_erase_64k(w25q16_handle_t* flash_dev, uint32_t address);
bool w25q16_chip_erase(w25q16_handle_t* flash_dev, uint32_t confirmation_code);

uint8_t w25q16_read_SR1(w25q16_handle_t* flash_dev);
uint8_t w25q16_read_SR2(w25q16_handle_t* flash_dev);
uint8_t w25q16_read_SR3(w25q16_handle_t* flash_dev);

uint64_t w25q16_get_id(w25q16_handle_t* flash_dev);
uint32_t w25q16_get_jedec_id(w25q16_handle_t* flash_dev);

void w25q16_reset(w25q16_handle_t* flash_dev);

#endif /* INC_W25Q16JV_H_ */
