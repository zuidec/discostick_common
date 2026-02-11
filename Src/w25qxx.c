/*
 * w25qxx.c
 *
 *  Created on: Jan 2, 2025
 *      Author: zuidec
 */

#include <stdint.h>
#include <stdbool.h>
#include "w25qxx.h"
#include "logger.h"

#define LOG_PREFIX "[w25qxx]"


/******************************************************************************
 *  Variables
 *****************************************************************************/
static const uint32_t CHIP_ERASE_CODE = 0xDEADC0DE;

/******************************************************************************
 * Static functions 
 *****************************************************************************/

static bool w25q_busy(bus_t* flash_dev) {
    bool ret = true;

	uint8_t busy_flag = w25q_read_SR1(flash_dev)&0x01;
    if(false==busy_flag)    {
        ret = false;
    }
    return ret;
}

/******************************************************************************
 * Main functions 
 *****************************************************************************/

uint32_t w25q_get_chip_erase_confirmation(void)   {
    return CHIP_ERASE_CODE;
}

int32_t w25q_init(bus_t* flash_dev)   {
    int32_t ret = W25Q_ERR_OK;
    w25q_reset(flash_dev);
    while(w25q_busy(flash_dev)) { ;; }
    volatile uint32_t jedec_id = w25q_get_jedec_id(flash_dev);
    if(jedec_id != W25Q_JEDEC_ID)	{
      ret = W25Q_ERR_BADID;
    }
    uint8_t sr = 0x00;

    sr = w25q_read_SR1(flash_dev);
    log_debug("%sinit - SR1 = 0x%x",LOG_PREFIX, sr);
    sr = w25q_read_SR2(flash_dev);
    log_debug("%sinit - SR2 = 0x%x",LOG_PREFIX, sr);
    sr = w25q_read_SR3(flash_dev);
    log_debug("%sinit - SR3 = 0x%x",LOG_PREFIX, sr);
    return ret;
}

int32_t w25q_read(bus_t* flash_dev, uint32_t address, uint8_t* buffer, uint16_t buffer_size)	{
#ifdef W25Q_TRACE
    log_trace("%sReading %u bytes at 0x%08lx",LOG_PREFIX,buffer_size, address);
#endif
    if (address >= (W25Q_BLOCK_COUNT * W25Q_BLOCK_SIZE)) {
        __asm__("bkpt 0");
    }
    int32_t ret = 0;
    ret = flash_dev->read(flash_dev, W25Q_READ_DATA, address, ADDR_24_BIT, DUMMY_NONE, buffer, buffer_size);
    while(w25q_busy(flash_dev))   {
        ;; // Block while waiting for busy to clear
    }
    return ret;
}

int32_t w25q_write(bus_t* flash_dev, uint32_t address, uint8_t* buffer, uint16_t bytes_to_write)	{
    int32_t ret = 0;
#ifdef W25Q_TRACE
    log_trace("%s%u bytes to write",LOG_PREFIX,bytes_to_write);
#endif
    if (address >= (W25Q_BLOCK_COUNT * W25Q_BLOCK_SIZE)) {
        __asm__("bkpt 0");
    }
    uint32_t last_write_addr = bytes_to_write + address - 1;

    while(bytes_to_write > 0)   {
        uint32_t page_boundary = 0;
        if((address%W25Q_PAGE_SIZE)==0)  {
            page_boundary = address + W25Q_PAGE_SIZE;
        }
        else    {
            page_boundary = (address - (address%W25Q_PAGE_SIZE)) + W25Q_PAGE_SIZE;
        }
#ifdef W25Q_TRACE
        log_trace("%sStart address is 0x%08lx, final write at 0x%08lx. Next page boundary start at 0x%08lx",LOG_PREFIX,address,last_write_addr,page_boundary);
#endif
        w25q_enable_write(flash_dev);
        // Check if data exceeds the max amount that can be written at once
        if(last_write_addr >= page_boundary){
            uint16_t this_write = page_boundary - address;
#ifdef W25Q_TRACE
            log_trace("%sWrite will pass boundary, writing only %u bytes",LOG_PREFIX,this_write);
#endif
            flash_dev->write(flash_dev, W25Q_PAGE_PROGRAM, address, ADDR_24_BIT, DUMMY_NONE, buffer, this_write); 
            ret += this_write;
            buffer += this_write;
            bytes_to_write -= this_write;
            address += this_write;
        }
        else 	{
#ifdef W25Q_TRACE
            log_trace("%sWriting %u bytes",LOG_PREFIX, bytes_to_write);
#endif
            flash_dev->write(flash_dev, W25Q_PAGE_PROGRAM, address, ADDR_24_BIT, DUMMY_NONE, buffer, bytes_to_write); 
            ret+=bytes_to_write;
            bytes_to_write -=bytes_to_write;
        }
        while(w25q_busy(flash_dev))   {
            ;;
        }
    }
    
#ifdef W25Q_TRACE
    log_trace("%sFinal write amount: %d bytes",LOG_PREFIX,ret);
#endif
    return ret;
	// Write is disabled by chip once programming page is complete
}

void w25q_enable_write(bus_t* flash_dev)	{
    flash_dev->write(flash_dev, W25Q_WRITE_ENABLE, ADDR_NONE, ADDR_NONE, DUMMY_NONE, NULL, 0); 
}

void w25q_enable_vsr_write(bus_t* flash_dev)    {
    flash_dev->write(flash_dev, W25Q_WRITE_ENABLE_VSR, ADDR_NONE, ADDR_NONE, DUMMY_NONE, NULL, 0); 
}

void w25q_disable_write(bus_t* flash_dev)	{
    flash_dev->write(flash_dev, W25Q_WRITE_DISABLE, ADDR_NONE, ADDR_NONE, DUMMY_NONE, NULL, 0); 
}

void w25q_sector_erase_4k(bus_t* flash_dev, uint32_t address)	{
#ifdef W25Q_TRACE
    log_trace("%sErasing 4k sector at 0x%08lx",LOG_PREFIX,address);
#endif
    if ((address % 4096) != 0) {
        __asm__("bkpt 0");
    }
	w25q_enable_write(flash_dev);
    flash_dev->write(flash_dev, W25Q_SECTOR_ERASE_4K, address, ADDR_24_BIT, DUMMY_NONE, NULL, 0); 
    while(w25q_busy(flash_dev))   {;;}
	// Write is disabled by chip once erase is complete

}

void w25q_block_erase_32k(bus_t* flash_dev, uint32_t address)	{
#ifdef W25Q_TRACE
    log_trace("%sErasing 32k block starting at 0x%08lx",LOG_PREFIX,address);
#endif
    if ((address % 4096) != 0) {
        __asm__("bkpt 0");
    }
    flash_dev->write(flash_dev, W25Q_BLOCK_ERASE_32K, address, ADDR_24_BIT, DUMMY_NONE, NULL, 0); 
    while(w25q_busy(flash_dev))   {;;}
	// Write is disabled by chip once erase is complete

}

void w25q_block_erase_64k(bus_t* flash_dev, uint32_t address)	{
#ifdef W25Q_TRACE
    log_trace("%sErasing 32k block starting at 0x%08lx",LOG_PREFIX,address);
#endif
    if ((address % 4096) != 0) {
        __asm__("bkpt 0");
    }
    flash_dev->write(flash_dev, W25Q_BLOCK_ERASE_64K, address, ADDR_24_BIT, DUMMY_NONE, NULL, 0); 
    while(w25q_busy(flash_dev))   {;;}
	// Write is disabled by chip once erase is complete


}

bool w25q_chip_erase(bus_t* flash_dev, uint32_t confirmation_code)  {
    bool ret = false;
    if(confirmation_code==CHIP_ERASE_CODE)  {
        log_warning("%sErasing entire flash module!",LOG_PREFIX);
        w25q_enable_write(flash_dev);
        flash_dev->write(flash_dev, W25Q_CHIP_ERASE, ADDR_NONE, ADDR_NONE, DUMMY_NONE, NULL, 0); 
        while(w25q_busy(flash_dev))   {
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

uint8_t w25q_read_SR1(bus_t* flash_dev)	{
	uint8_t data = 0;
    flash_dev->read(flash_dev, W25Q_READ_SR1, ADDR_NONE, ADDR_NONE, DUMMY_NONE, &data, sizeof(data)); 

	return data;
}

uint8_t w25q_read_SR2(bus_t* flash_dev)	{
	uint8_t data = 0;
    flash_dev->read(flash_dev, W25Q_READ_SR2, ADDR_NONE, ADDR_NONE, DUMMY_NONE, &data, sizeof(data)); 

	return data;
}

uint8_t w25q_read_SR3(bus_t* flash_dev)	{
	uint8_t data = 0;
    flash_dev->read(flash_dev, W25Q_READ_SR3, ADDR_NONE, ADDR_NONE, DUMMY_NONE, &data, sizeof(data)); 

	return data;
}

void w25q_write_SR1(bus_t* flash_dev, uint8_t state, uint8_t mask)  {
    uint8_t sr = w25q_read_SR1(flash_dev);
    if(state)   {
        sr |= mask;
    }
    else    {
        sr &= ~mask;
    }

    // If status register write access needs to be controlled, a specific 
    // call for that action should be implemented
    sr &= ~W25Q_SR1_SRP;

    w25q_enable_vsr_write(flash_dev);
    flash_dev->write(flash_dev, W25Q_WRITE_SR1, ADDR_NONE, ADDR_NONE, DUMMY_NONE, &sr, sizeof(sr));
    while(w25q_busy(flash_dev))   {;;}

}

void w25q_write_SR2(bus_t* flash_dev, uint8_t state, uint8_t mask)  {

    uint8_t sr = w25q_read_SR2(flash_dev);
    if(state)   {
        sr |= mask;
    }
    else    {
        sr &= ~mask;
    }

    // Here we will make sure these bits are not accidentally set as they are permanent.
    // This SHOULDN'T happen using VSR instruction, but don't want to brick a device.
    // Instead, a dedicated call can be made when that action is desired.
    sr &= ~W25Q_SR2_LB1;
    sr &= ~W25Q_SR2_LB2;
    sr &= ~W25Q_SR2_LB3;

    // Same goes for locking status registers
    sr &= ~W25Q_SR2_SRL;

    // Probably doesn't matter but datasheet specifies writing 0 as safe
    sr &= ~W25Q_SR2_RESERVED;

    w25q_enable_vsr_write(flash_dev);
    flash_dev->write(flash_dev, W25Q_WRITE_SR2, ADDR_NONE, ADDR_NONE, DUMMY_NONE, &sr, sizeof(sr));
    while(w25q_busy(flash_dev))   {;;}
}

void w25q_write_SR3(bus_t* flash_dev, uint8_t state, uint8_t mask)  {
    uint8_t sr = w25q_read_SR3(flash_dev);
    if(state)   {
        sr |= mask;
    }
    else    {
        sr &= ~mask;
    }

    // Probably doesn't matter but datasheet specifies writing 0 as safe
    sr &= ~W25Q_SR3_RESERVED1;
    sr &= ~W25Q_SR3_RESERVED2;
    sr &= ~W25Q_SR3_RESERVED3;
    sr &= ~W25Q_SR3_RESERVED4;
    sr &= ~W25Q_SR3_RESERVED5;

    w25q_enable_vsr_write(flash_dev);
    flash_dev->write(flash_dev, W25Q_WRITE_SR3, ADDR_NONE, ADDR_NONE, DUMMY_NONE, &sr, sizeof(sr));
    while(w25q_busy(flash_dev))   {;;}

}


void w25q_enable_qspi(bus_t* flash_dev) {
    uint8_t sr = w25q_read_SR2(flash_dev);
        sr |= W25Q_SR2_QE;

    // Here we will make sure these bits are not accidentally set as they are permanent.
    sr &= ~W25Q_SR2_LB1;
    sr &= ~W25Q_SR2_LB2;
    sr &= ~W25Q_SR2_LB3;

    // Same goes for locking status registers
    sr &= ~W25Q_SR2_SRL;

    // Probably doesn't matter but datasheet specifies writing 0 as safe
    sr &= ~W25Q_SR2_RESERVED;

    w25q_enable_write(flash_dev);
    flash_dev->write(flash_dev, W25Q_WRITE_SR2, ADDR_NONE, ADDR_NONE, DUMMY_NONE, &sr, sizeof(sr));
    while(w25q_busy(flash_dev))   {;;}
}

void w25q_disable_qspi(bus_t* flash_dev)    {
    uint8_t sr = w25q_read_SR2(flash_dev);
    sr &= ~W25Q_SR2_QE;

    // Here we will make sure these bits are not accidentally set as they are permanent.
    sr &= ~W25Q_SR2_LB1;
    sr &= ~W25Q_SR2_LB2;
    sr &= ~W25Q_SR2_LB3;

    // Same goes for locking status registers
    sr &= ~W25Q_SR2_SRL;

    // Probably doesn't matter but datasheet specifies writing 0 as safe
    sr &= ~W25Q_SR2_RESERVED;

    w25q_enable_write(flash_dev);
    flash_dev->write(flash_dev, W25Q_WRITE_SR2, ADDR_NONE, ADDR_NONE, DUMMY_NONE, &sr, sizeof(sr));
    while(w25q_busy(flash_dev))   {;;}
}

uint64_t w25q_get_id(bus_t* flash_dev)	{
    uint8_t dummy_clocks = 4;
	uint8_t id[8] = {0};
    flash_dev->read(flash_dev, W25Q_READ_UNIQUE_ID, ADDR_NONE, ADDR_NONE, dummy_clocks, id, sizeof(id));
	return (uint64_t)(((uint64_t)id[0] << 56) | ((uint64_t)id[1] << 48) | ((uint64_t)id[2] << 40) | ((uint64_t)id[3] << 32) | ((uint64_t)id[4] << 24) | ((uint64_t)id[5] << 16) | ((uint64_t)id[6] << 8) | (uint64_t)id[7]);
}

void w25q_reset(bus_t* flash_dev)	{
    // Datasheet shows two distinct transactions with CS returning high before the reset command
    flash_dev->write(flash_dev, W25Q_ENABLE_RESET, ADDR_NONE, ADDR_NONE, DUMMY_NONE, NULL, 0);
    flash_dev->write(flash_dev, W25Q_RESET_DEVICE, ADDR_NONE, ADDR_NONE, DUMMY_NONE, NULL, 0);
}

uint32_t w25q_get_jedec_id(bus_t* flash_dev)	{
	uint8_t id[3] = {0};
    flash_dev->read(flash_dev, W25Q_READ_JEDEC_ID, ADDR_NONE, ADDR_NONE, DUMMY_NONE, id, sizeof(id));

	return (uint32_t)(((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | (uint32_t)id[2]);
}

void w25q_power_down(bus_t* flash_dev)  {
    flash_dev->write(flash_dev, W25Q_PWR_DOWN, ADDR_NONE, ADDR_NONE, DUMMY_NONE, NULL, 0);
}

void w25_release_power_down(bus_t* flash_dev)   {
    flash_dev->write(flash_dev, W25Q_REL_PWR_DOWN, ADDR_NONE, ADDR_NONE, DUMMY_NONE, NULL, 0);

}

