/*
 * w25qxx.h
 *
 *  Created on: Jan 2, 2025
 *      Author: zuidec
 */

#ifndef INC_W25Qxx_H_
#define INC_W25Qxx_H_

#include <stdint.h>
#include <stdbool.h>
#include "bus.h"

/******************************************************************************
 *  Device selection
 *****************************************************************************/

    /********************************************
     * In the datasheet, the IM/JM variants
     * have a different prefix in the device ID
     * which is reflected here
     ********************************************/
//#define M_VARIANT
//#define W25Q16
#define W25Q64

#if (!defined W25Q16) && (!defined W25Q64)
#error Flash device not defined, please select the desired device type
#endif

#if (defined M_VARIANT)
#define ID_TOP                          (0x7000)
#else
#define ID_TOP                          (0x4000)
#endif

#if (defined W25Q16)
#define W25Q_BLOCK_COUNT                (512)
#define W25Q_DEV_ID						(0x15 | ID_TOP)
#elif (defined W25Q64)
#define W25Q_BLOCK_COUNT                (2048)
#define W25Q_DEV_ID						(0x17 | ID_TOP)
#endif


/******************************************************************************
 * Commands 
 *****************************************************************************/
#define W25Q_WRITE_ENABLE					(0x06)
#define W25Q_WRITE_ENABLE_VSR               (0x50)
#define W25Q_WRITE_DISABLE				    (0x04)

#define W25Q_REL_PWR_DOWN					(0xAB)
#define W25Q_ID					            (0xAB)
#define W25Q_READ_MFR_ID					(0x90)
#define W25Q_READ_JEDEC_ID				    (0x9F)
#define W25Q_READ_UNIQUE_ID				    (0x4B)

#define W25Q_READ_DATA					    (0x03)
#define W25Q_FAST_READ					    (0x0B)
#define W25Q_FAST_READ_DO				    (0x3B)
#define W25Q_FAST_READ_DIO				    (0xBB)
#define W25Q_FAST_READ_QO				    (0x6B)
#define W25Q_FAST_READ_QIO				    (0xEB)

#define W25Q_PAGE_PROGRAM					(0x02)
#define W25Q_PAGE_PROGRAM_QI				(0x32)

#define W25Q_SECTOR_ERASE_4K				(0x20)
#define W25Q_BLOCK_ERASE_32K				(0x52)
#define W25Q_BLOCK_ERASE_64K				(0xD8)
#define W25Q_CHIP_ERASE					    (0xC7)

#define W25Q_READ_SR1						(0x05)
#define W25Q_WRITE_SR1					    (0x01)
#define W25Q_READ_SR2						(0x35)
#define W25Q_WRITE_SR2					    (0x31)
#define W25Q_READ_SR3						(0x15)
#define W25Q_WRITE_SR3					    (0x11)

#define W25Q_READ_SFDP                      (0x5A)
#define W25Q_ERASE_SECURITY_REG 			(0x44)
#define W25Q_PROG_SECURITY_REG			    (0x42)
#define W25Q_READ_SECURITY_REG			    (0x48)

#define W25Q_GLOBAL_BLK_LOCK                (0x7E)
#define W25Q_GLOBAL_BLK_UNLOCK              (0x98)
#define W25Q_READ_BLOCK_LOCK                (0x3D)
#define W25Q_IND_BLOCK_LOCK                 (0x36)
#define W25Q_IND_BLOCK_UNLOCK               (0x39)

#define W25Q_SUSPEND                        (0x75)
#define W25Q_RESUME                         (0x7A)
#define W25Q_PWR_DOWN                       (0xB9)

#define W25Q_ENABLE_RESET					(0x66)
#define W25Q_RESET_DEVICE					(0x99)

/******************************************************************************
 * Register bitmasks 
 *****************************************************************************/

#define W25Q_SR1_SRP                        (1<<7)
#define W25Q_SR1_SEC                        (1<<6)
#define W25Q_SR1_TB                         (1<<5)
#define W25Q_SR1_BP2                        (1<<4)
#define W25Q_SR1_BP1                        (1<<3)
#define W25Q_SR1_BP0                        (1<<2)
#define W25Q_SR1_WEL                        (1<<1)
#define W25Q_SR1_BUSY                       (1<<0)

#define W25Q_SR2_SUS                        (1<<7)
#define W25Q_SR2_CMP                        (1<<6)
#define W25Q_SR2_LB3                        (1<<5)
#define W25Q_SR2_LB2                        (1<<4)
#define W25Q_SR2_LB1                        (1<<3)
#define W25Q_SR2_RESERVED                   (1<<2)
#define W25Q_SR2_QE                         (1<<1)
#define W25Q_SR2_SRL                        (1<<0)

#define W25Q_SR3_RESERVED5                  (1<<7)
#define W25Q_SR3_DRV1                       (1<<6)
#define W25Q_SR3_DRV0                       (1<<5)
#define W25Q_SR3_RESERVED4                  (1<<4)
#define W25Q_SR3_RESERVED3                  (1<<3)
#define W25Q_SR3_WPS                        (1<<2)
#define W25Q_SR3_RESERVED2                  (1<<1)
#define W25Q_SR3_RESERVED1                  (1<<0)

/******************************************************************************
 * Device parameters
 *****************************************************************************/
#define W25Q_MFR_ID						    (0xEF)
#define W25Q_JEDEC_ID						((uint32_t)(W25Q_MFR_ID << 16) | (W25Q_DEV_ID)) // DEV_ID is model-dependent
#define W25Q_PAGE_SIZE                      (256)
#define W25Q_BLOCK_SIZE                     (4096) 
#define W25Q_MEMTYPE						(0x14)
#define W25Q_CAPACITY                       (W25Q_BLOCK_COUNT * W25Q_BLOCK_SIZE)

#ifndef FLASH_MAX_PAYLOAD
#define FLASH_MAX_PAYLOAD                   (1024) // 1KB 
#endif


enum w25q_error   {
    W25Q_ERR_OK   = 0,
    W25Q_ERR_NULL = -255,
    W25Q_ERR_CRC  = -256,
    W25Q_ERR_BADID= -257
};


uint32_t w25q_get_chip_erase_confirmation(void);
int32_t w25q_init(bus_t* flash_dev);

int32_t w25q_read(bus_t* flash_dev, uint32_t address, uint8_t* buffer, uint16_t buffer_size);
int32_t w25q_write(bus_t* flash_dev, uint32_t address, uint8_t* buffer, uint16_t bytes_to_write);

void w25q_enable_write(bus_t* flash_dev);
void w25q_enable_vsr_write(bus_t* flash_dev);
void w25q_disable_write(bus_t* flash_dev);

void w25q_sector_erase_4k(bus_t* flash_dev, uint32_t address);
void w25q_block_erase_32k(bus_t* flash_dev, uint32_t address);
void w25q_block_erase_64k(bus_t* flash_dev, uint32_t address);
bool w25q_chip_erase(bus_t* flash_dev, uint32_t confirmation_code);

uint8_t w25q_read_SR1(bus_t* flash_dev);
uint8_t w25q_read_SR2(bus_t* flash_dev);
uint8_t w25q_read_SR3(bus_t* flash_dev);
void w25q_write_SR1(bus_t* flash_dev,  uint8_t state, uint8_t mask);
void w25q_write_SR2(bus_t* flash_dev,  uint8_t state, uint8_t mask);
void w25q_write_SR3(bus_t* flash_dev,  uint8_t state, uint8_t mask);

void w25q_enable_qspi(bus_t* flash_dev);
void w25q_disable_qspi(bus_t* flash_dev);

uint64_t w25q_get_id(bus_t* flash_dev);
uint32_t w25q_get_jedec_id(bus_t* flash_dev);

void w25q_reset(bus_t* flash_dev);
void w25q_power_down(bus_t* flash_dev);
void w25_release_power_down(bus_t* flash_dev);

#endif /* INC_W25Q_H_ */
