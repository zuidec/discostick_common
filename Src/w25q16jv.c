/*
 * w25q16jv.c
 *
 *  Created on: Jan 2, 2025
 *      Author: zuidec
 */

#include <stdint.h>
#include <stdbool.h>
#include "w25q16jv.h"
#include "logger.h"

    
#define LOG_PREFIX "[w25q16]"

/******************************************************************************
 *  Variables
 *****************************************************************************/
static const uint32_t CHIP_ERASE_CODE = 0xDEADC0DE;

/******************************************************************************
 * Static functions 
 *****************************************************************************/
static void cs_enable(w25q16_handle_t* flash_dev)	{
    if(false == flash_dev->hw_cs)    {
        HAL_GPIO_WritePin(flash_dev->cs_port, flash_dev->cs_pin, GPIO_PIN_RESET);
    }
}

static void cs_release(w25q16_handle_t* flash_dev)	{
    if(false == flash_dev->hw_cs)    {
        HAL_GPIO_WritePin(flash_dev->cs_port, flash_dev->cs_pin, GPIO_PIN_SET);
    }
}

static bool w25q16_busy(w25q16_handle_t* flash_dev) {
    bool ret = true;

    cs_enable(flash_dev);
	uint8_t busy_flag = w25q16_read_SR1(flash_dev)&0x01;
    if(false==busy_flag)    {
        ret = false;
    }
    cs_release(flash_dev);
    return ret;
}

/******************************************************************************
 * Main functions 
 *****************************************************************************/

uint32_t w25q16_get_chip_erase_confirmation(void)   {
    return CHIP_ERASE_CODE;
}

int32_t w25q16_init(w25q16_handle_t* flash_dev)   {
    int32_t ret = W25Q16_ERR_OK;
    w25q16_reset(flash_dev);
    HAL_Delay(50);
    volatile uint32_t jedec_id = w25q16_get_jedec_id(flash_dev);
    if(jedec_id != W25Q16_JEDEC_ID)	{
      ret = W25Q16_ERR_BADID;
    }
    return ret;
}

void w25q16_read(w25q16_handle_t* flash_dev, uint32_t address, uint8_t* buffer, uint16_t buffer_size)	{
#ifdef W25Q16_TRACE
    log_trace("%sReading %u bytes at 0x%08lx",LOG_PREFIX,buffer_size, address);
#endif
    uint8_t data[4] = {W25Q16_READ_DATA, (uint8_t)((address >> 16)&0xFF), (uint8_t)((address >> 8)&0xFF), (uint8_t)(address)&0xFF};
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, data, sizeof(data),W25Q16_SPI_TIMEOUT);
	HAL_SPI_Receive(flash_dev->spi, buffer, buffer_size, W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);
    while(w25q16_busy(flash_dev))   {
        ;;
    }
}

int32_t w25q16_write(w25q16_handle_t* flash_dev, uint32_t address, uint8_t* buffer, uint16_t bytes_to_write)	{
    int32_t ret = 0;
#ifdef W25Q16_TRACE
    log_trace("%s%u bytes to write",LOG_PREFIX,bytes_to_write);
#endif
    uint32_t last_write_addr = bytes_to_write + address - 1;

    while(bytes_to_write > 0)   {
        uint32_t page_boundary = 0;
        if((address%FLASH_PAGE_SIZE)==0)  {
            page_boundary = address + FLASH_PAGE_SIZE;
        }
        else    {
            page_boundary = (address - (address%FLASH_PAGE_SIZE)) + FLASH_PAGE_SIZE;
        }
#ifdef W25Q16_TRACE
        log_trace("%sStart address is 0x%08lx, final write at 0x%08lx. Next page boundary start at 0x%08lx",LOG_PREFIX,address,last_write_addr,page_boundary);
#endif
        w25q16_enable_write(flash_dev);
        // Check if data exceeds the max amount that can be written at once
        if(last_write_addr >= page_boundary){
            uint16_t this_write = page_boundary - address;
#ifdef W25Q16_TRACE
            log_trace("%sWrite will pass boundary, writing only %u bytes",LOG_PREFIX,this_write);
#endif
            uint8_t data[4] = {W25Q16_PAGE_PROGRAM, (uint8_t)((address >> 16)&0xFF), (uint8_t)((address >> 8)&0xFF), (uint8_t)(address)&0xFF};
            cs_enable(flash_dev);
            HAL_SPI_Transmit(flash_dev->spi, data, sizeof(data), W25Q16_SPI_TIMEOUT);
            HAL_SPI_Transmit(flash_dev->spi, buffer, this_write, W25Q16_SPI_TIMEOUT);
            cs_release(flash_dev);
            ret += this_write;
            buffer += this_write;
            bytes_to_write -= this_write;
            address += this_write;
        }
        else 	{
#ifdef W25Q16_TRACE
            log_trace("%sWriting %u bytes",LOG_PREFIX, bytes_to_write);
#endif
            uint8_t data[4] = {W25Q16_PAGE_PROGRAM, (uint8_t)((address >> 16)&0xFF), (uint8_t)((address >> 8)&0xFF), (uint8_t)(address)&0xFF};
            cs_enable(flash_dev);
            HAL_SPI_Transmit(flash_dev->spi, data, sizeof(data), W25Q16_SPI_TIMEOUT);
            HAL_SPI_Transmit(flash_dev->spi, buffer, bytes_to_write, W25Q16_SPI_TIMEOUT);
            cs_release(flash_dev);
            ret+=bytes_to_write;
            bytes_to_write -=bytes_to_write;
        }
        while(w25q16_busy(flash_dev))   {
            ;;
        }
    }
    
#ifdef W25Q16_TRACE
    log_trace("%sFinal write amount: %d bytes",LOG_PREFIX,ret);
#endif
    return ret;
	// Write is disabled by chip once programming page is complete
}

void w25q16_enable_write(w25q16_handle_t* flash_dev)	{
	uint8_t data = (uint8_t)W25Q16_WRITE_ENABLE;
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, &data, sizeof(data), W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);
}

void w25q16_disable_write(w25q16_handle_t* flash_dev)	{
	uint8_t data = (uint8_t)W25Q16_WRITE_DISABLE;
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, &data, sizeof(data), W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);
}

void w25q16_sector_erase_4k(w25q16_handle_t* flash_dev, uint32_t address)	{
#ifdef W25Q16_TRACE
    log_trace("%sErasing 4k sector at 0x%08lx",LOG_PREFIX,address);
#endif
    uint8_t data[4] = {W25Q16_SECTOR_ERASE_4K, (uint8_t)(address >> 16), (uint8_t)(address >> 8), (uint8_t)address};
	w25q16_enable_write(flash_dev);
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, data, sizeof(data), W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);
    while(w25q16_busy(flash_dev))   {;;}
	// Write is disabled by chip once erase is complete

}

void w25q16_block_erase_32k(w25q16_handle_t* flash_dev, uint32_t address)	{
#ifdef W25Q16_TRACE
    log_trace("%sErasing 32k block starting at 0x%08lx",LOG_PREFIX,address);
#endif
    uint8_t data[4] = {W25Q16_BLOCK_ERASE_32K, (uint8_t)(address >> 16), (uint8_t)(address >> 8), (uint8_t)address};
	w25q16_enable_write(flash_dev);
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, data, sizeof(data), W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);
    while(w25q16_busy(flash_dev))   {;;}
	// Write is disabled by chip once erase is complete

}

void w25q16_block_erase_64k(w25q16_handle_t* flash_dev, uint32_t address)	{

}

bool w25q16_chip_erase(w25q16_handle_t* flash_dev, uint32_t confirmation_code)  {
    bool ret = false;
    if(confirmation_code==CHIP_ERASE_CODE)  {
        log_warning("%sErasing entire flash module!",LOG_PREFIX);
        uint8_t data = W25Q16_CHIP_ERASE;
        w25q16_enable_write(flash_dev);
        cs_enable(flash_dev);
        HAL_SPI_Transmit(flash_dev->spi, &data, sizeof(data), W25Q16_SPI_TIMEOUT);
        cs_release(flash_dev);
        while(w25q16_busy(flash_dev))   {
            ;;
        }
        ret = true;
    }
    else    {
        log_error("%sIncorrect confimation code, flash not erased!",LOG_PREFIX);
        ret = false;
    }
    return ret;
}

uint8_t w25q16_read_SR1(w25q16_handle_t* flash_dev)	{
	uint8_t data = 0;
	uint8_t payload = W25Q16_READ_SR1;
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, &payload, sizeof(payload),W25Q16_SPI_TIMEOUT);
	HAL_SPI_Receive(flash_dev->spi, &data, sizeof(data), W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);

	return data;
}

uint8_t w25q16_read_SR2(w25q16_handle_t* flash_dev)	{
	uint8_t data = 0;
	uint8_t payload = W25Q16_READ_SR2;
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, &payload, sizeof(payload),W25Q16_SPI_TIMEOUT);
	HAL_SPI_Receive(flash_dev->spi, &data, sizeof(data), W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);

	return data;
}

uint8_t w25q16_read_SR3(w25q16_handle_t* flash_dev)	{
	uint8_t data = 0;
	uint8_t payload = W25Q16_READ_SR3;
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, &payload, sizeof(payload),W25Q16_SPI_TIMEOUT);
	HAL_SPI_Receive(flash_dev->spi, &data, sizeof(data), W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);

	return data;
}

uint64_t w25q16_get_id(w25q16_handle_t* flash_dev)	{
	uint8_t payload[5] = {W25Q16_READ_UNIQUE_ID, 0x00, 0x00, 0x00, 0x00};
	uint8_t id[8] = {0};
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, payload, sizeof(payload),W25Q16_SPI_TIMEOUT);
	HAL_SPI_Receive(flash_dev->spi, id, sizeof(id), W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);
	return (uint64_t)(((uint64_t)id[0] << 56) | ((uint64_t)id[1] << 48) | ((uint64_t)id[2] << 40) | ((uint64_t)id[3] << 32) | ((uint64_t)id[4] << 24) | ((uint64_t)id[5] << 16) | ((uint64_t)id[6] << 8) | (uint64_t)id[7]);
}

void w25q16_reset(w25q16_handle_t* flash_dev)	{
	uint8_t payload[2] = {W25Q16_ENABLE_RESET, W25Q16_RESET_DEVICE};
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, &payload[0], 1,W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, &payload[1], 1,W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);
}

uint32_t w25q16_get_jedec_id(w25q16_handle_t* flash_dev)	{
	cs_release(flash_dev);
	uint8_t id[3] = {0};
	uint8_t data = (uint8_t)W25Q16_READ_JEDEC_ID;
	cs_enable(flash_dev);
	HAL_SPI_Transmit(flash_dev->spi, &data, sizeof(data), W25Q16_SPI_TIMEOUT);
	HAL_SPI_Receive(flash_dev->spi, &id[0], sizeof(id), W25Q16_SPI_TIMEOUT);
	cs_release(flash_dev);
	return (uint32_t)((id[0] << 16) | (id[1] << 8) | id[2]);
}


