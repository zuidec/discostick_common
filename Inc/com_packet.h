/*
 * uart_comms.h
 *
 *  Created on: Mar 12, 2025
 *      Author: zuidec
 */

#ifndef INC_COM_PACKET_H_
#define INC_COM_PACKET_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

#define COM_PACKET_PAYLOAD_SIZE		(128) // Must be divisible by 4 to align with 32bit crc
#define COM_PACKET_HEADER_SIZE		(14)
#define COM_PACKET_T_SIZE			(COM_PACKET_PAYLOAD_SIZE + COM_PACKET_HEADER_SIZE)
#define COM_PACKET_VERSION			(2)
#define PARSE_TIMEOUT_MS			(5)

typedef enum packet_type_t	{
	COM_PACKET_NORMAL,
	COM_PACKET_CMD,
	COM_PACKET_ACK,
	COM_PACKET_NACK,
	COM_PACKET_FALSE,
    COM_PACKET_BAD_CRC,
    COM_PACKET_POS,
    COM_PACKET_CAL_FACTOR,
    COM_PACKET_DEVICES,
    COM_PACKET_TIMEOUT,
    COM_PACKET_NOINIT
}packet_type_t;

typedef enum cmd_type_t {
    CMD_BAD_CMD     = 0xFF,
    CMD_CDR_EN      = 0x0A,
    CMD_CDR_DS      = 0x0B,
    CMD_GET_POS     = 0x0C,
    CMD_GET_CAL     = 0x0D,
    CMD_SET_CAL     = 0x0E,
    CMD_STEP_ON     = 0x0F,
    CMD_STEP_OFF    = 0x11,
    CMD_GET_DEVS    = 0x12,
    CMD_GET_ALPHA   = 0x13,
    CMD_SET_ALPHA   = 0x14
}cmd_type_t;

typedef enum com_addr_t {
    COM_ADDR_NONE   = 0x00,
    COM_ADDR_CYCLIC = 0x1A,
    COM_ADDR_COLL   = 0x1B,
    COM_ADDR_PEDAL  = 0x1C,
    COM_ADDR_CTRL   = 0x1D
}com_addr_t;

typedef struct __attribute__((packed)) com_packet_t	{
	uint8_t version;
    uint8_t src_addr;
    uint8_t dest_addr;
	uint8_t packet_type;
	uint8_t payload_length;
	uint8_t padding;
	uint8_t packet_size [4];
	uint8_t crc32[4];
	uint8_t payload[COM_PACKET_PAYLOAD_SIZE];
}com_packet_t;

typedef enum standard_crc32_t	{
	ACK_CRC32 = 0xABCD001,
	NACK_CRC32 = 0xABCD002
}standard_crc_32_t;

void com_packet_logger_cb(void(*log_cb)(const char*,...));
bool com_packet_init(com_addr_t src_address, uint32_t(*crc32_cb)(uint32_t*,uint32_t), uint32_t(*gettick_cb)(void));
void com_packet_create(com_packet_t* packet, com_addr_t addr, uint8_t* data, uint32_t size);
void com_packet_create_cmd(com_packet_t* packet, com_addr_t addr, cmd_type_t command, uint8_t* para_data, uint32_t para_size);
cmd_type_t com_packet_get_cmd(com_packet_t* packet);
void com_packet_create_special(com_packet_t* packet, com_addr_t addr, packet_type_t packet_type, uint8_t* para_data, uint32_t para_size);
void com_packet_clear(com_packet_t* packet);
packet_type_t com_packet_parse(com_packet_t* packet, uint8_t* data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* INC_COM_PACKET_H_ */
