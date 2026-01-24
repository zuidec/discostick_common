#include "com_packet.h"
#include "fifo.h"
#include "bitutils.h"
#include "string.h"
#include <stdint.h>
#include <stdbool.h>

/*
 * Buffer offsets (see com_packet.h for struct definition)
 * index + 0:	version
 * index + 1: 	src_addr
 * index + 2:   dest_addr
 * index + 3:   packet_type
 * index + 4: 	payload_length
 * index + 5: 	padding
 * index + 6:	packet_size
 * index + 10: 	crc32
 * index + 14:	beginning of payload
 */
#define CP_VER_OFFSET   (0)
#define CP_SADDR_OFFSET (1)
#define CP_DADDR_OFFSET (2)
#define CP_PTYPE_OFFSET (3)
#define CP_PLEN_OFFSET  (4)
#define CP_PAD_OFFSET   (5)
#define CP_PSIZE_OFFSET (6)
#define CP_CRC32_OFFSET (10)
#define CP_PLOAD_OFFSET (14)

static uint32_t(*crc32_calc)(uint32_t*,uint32_t) = NULL ;
static uint32_t(*get_tick)(void) = NULL;
static void(*cp_log)(const char*,...) = NULL;

static bool log_cb_registered = false;
static bool required_cb_registered = false;
static uint32_t cp_timeout = PARSE_TIMEOUT_MS;
static uint8_t source_address = COM_ADDR_NONE;

#define cp_assert() _cp_assert(__FILE__,__LINE__)

static void _cp_assert(const char* file, int line);
static void cp_buf_to_packet(com_packet_t* packet, uint8_t* data);


static void _cp_assert(const char* file, int line)  {
    if(log_cb_registered)   {
        cp_log("[com_packet] Assert failed in %s at line: %d", file, line);
    }
}

static void cp_buf_to_packet(com_packet_t* packet, uint8_t* data)   {

     packet->version 			= data[CP_VER_OFFSET];
     packet->src_addr           = data[CP_SADDR_OFFSET];
     packet->dest_addr          = data[CP_DADDR_OFFSET];
     packet->packet_type 		= data[CP_PTYPE_OFFSET];
     packet->payload_length 	= data[CP_PLEN_OFFSET];
     packet->padding 			= data[CP_PAD_OFFSET];
     memcpy(&packet->packet_size,&data[CP_PSIZE_OFFSET],sizeof(uint32_t));
     memcpy(&packet->crc32,&data[CP_CRC32_OFFSET],sizeof(uint32_t));
     //packet->packet_size.value	= u8_to_u32(&data[CP_PSIZE_OFFSET]);
     //packet->crc32.value		= u8_to_u32(&data[CP_CRC32_OFFSET]);
     if(packet->payload_length > COM_PACKET_PAYLOAD_SIZE)	{
    	 cp_assert();
     }
     else	{
    	 memcpy(packet->payload, &data[CP_PLOAD_OFFSET], packet->payload_length);
     }
}

void com_packet_logger_cb(void(*log_cb)(const char*,...))   {
    if(log_cb!=NULL)    {
        cp_log = log_cb;
        log_cb_registered = true;
    }
}

bool com_packet_init(com_addr_t src_address,uint32_t(*crc32_cb)(uint32_t*,uint32_t), uint32_t(*gettick_cb)(void))	{
    bool ret = false;
    if(crc32_cb!=NULL && gettick_cb!=NULL)  {
        crc32_calc = crc32_cb;
        get_tick = gettick_cb;
        required_cb_registered = true;
        ret = true;
    }
    if(required_cb_registered != true)   {
        cp_assert();
    }
    else    {
        source_address = src_address;
    }

    return ret;
}

void com_packet_set_timeout(uint32_t millis)    {
    cp_timeout = millis;
}

void com_packet_clear(com_packet_t* packet)	{
	packet->version = COM_PACKET_VERSION;
    packet->src_addr = source_address;
    packet->dest_addr = COM_ADDR_NONE;
	packet->packet_type = (uint8_t)COM_PACKET_NORMAL;
	packet->payload_length = 0;
	packet->padding = 0;
    memset(&packet->packet_size,0,sizeof(uint32_t));
    memset(&packet->crc32,0,sizeof(uint32_t));
	//packet->packet_size.value = 0;
	//packet->crc32.value = 0;
	memset(packet->payload, 0, COM_PACKET_PAYLOAD_SIZE);
}

void com_packet_create(com_packet_t* packet, com_addr_t addr, uint8_t* data, uint32_t size){
    if(required_cb_registered != true)   {
        cp_assert();
        return;
    }
    if((uint32_t*)data == (uint32_t*)NULL) {
        cp_assert();
        return;
    }

	com_packet_clear(packet);

	if(size >= COM_PACKET_PAYLOAD_SIZE){
		memcpy(packet->payload, data, COM_PACKET_PAYLOAD_SIZE);
		packet->payload_length = COM_PACKET_PAYLOAD_SIZE;
		packet->padding = 0;
	}
	else	{
		memcpy(packet->payload, data, size);
        packet->padding = (sizeof(uint32_t) - (size%sizeof(uint32_t)));  // check here for size errors
        packet->payload_length = size + packet->padding;        
	}

    packet->src_addr            = source_address;
    packet->dest_addr           = addr;
	packet->packet_type         = COM_PACKET_NORMAL;
    uint32_t temp = COM_PACKET_HEADER_SIZE + packet->payload_length;
    memcpy(&packet->packet_size,&temp,sizeof(uint32_t));
    temp = crc32_calc((uint32_t*)packet->payload, packet->payload_length/sizeof(uint32_t));
    memcpy(&packet->crc32,&temp,sizeof(uint32_t));
}

void com_packet_create_cmd(com_packet_t* packet, com_addr_t addr, cmd_type_t command, uint8_t* para_data, uint32_t para_size)    {
    if(required_cb_registered != true)   {
        cp_assert();
        return;
    }
    com_packet_clear(packet);
    if(command == CMD_SET_CAL)  {
        if((uint32_t)para_data==(uint32_t)NULL) {
            cp_assert();
            return;
        }
        if(para_size >= COM_PACKET_PAYLOAD_SIZE)    {
            memcpy(&packet->payload[1], para_data, COM_PACKET_PAYLOAD_SIZE-1);
            packet->payload_length = COM_PACKET_PAYLOAD_SIZE;
            packet->padding = 0;
        }
        else    {
            // Offset by one to account for the actual command at [0] index
            memcpy(&packet->payload[1], para_data, para_size);
           // packet->padding = para_size + (sizeof(uint32_t) - (para_size%sizeof(uint32_t)));
            packet->padding = (sizeof(uint32_t) - ((para_size+1)%sizeof(uint32_t)));
            packet->payload_length = para_size + 1 + packet->padding;        

        }

    }
    else    {
        (void)para_data;
        (void)para_size;
        packet->payload_length  = sizeof(uint32_t);
        packet->padding         = sizeof(uint32_t)-sizeof(uint8_t);
    }

    packet->src_addr            = source_address;
    packet->dest_addr           = addr;
    packet->packet_type         = COM_PACKET_CMD;
    //packet->packet_size.value   = COM_PACKET_HEADER_SIZE + packet->payload_length;
    //packet->crc32.value         = crc32_calc((uint32_t*)packet->payload, packet->payload_length/sizeof(uint32_t));
    uint32_t temp = COM_PACKET_HEADER_SIZE + packet->payload_length;
    memcpy(&packet->packet_size,&temp,sizeof(uint32_t));
    temp = crc32_calc((uint32_t*)packet->payload, packet->payload_length/sizeof(uint32_t));
    memcpy(&packet->crc32,&temp,sizeof(uint32_t));
    packet->payload[0]          = (uint8_t)command;
}

cmd_type_t com_packet_get_cmd(com_packet_t* packet) {
    cmd_type_t ret;
    if(packet->packet_type != COM_PACKET_CMD)   {
        ret = CMD_BAD_CMD;
    }
    else    {
        ret = packet->payload[0];
    }
    return ret;
}

void com_packet_create_special(com_packet_t* packet, com_addr_t addr, packet_type_t packet_type, uint8_t* para_data, uint32_t para_size){
    if(required_cb_registered != true)   {
        cp_assert();
        return;
    }
	com_packet_clear(packet);
    if(packet_type == COM_PACKET_BAD_CRC || packet_type == COM_PACKET_FALSE || packet_type == COM_PACKET_CMD)    {
        //Do nothing
    }
    else if(packet_type == COM_PACKET_ACK || packet_type == COM_PACKET_NACK) {
        uint32_t temp =0;
        switch(packet_type) {
            case COM_PACKET_ACK:
                temp = ACK_CRC32;
                memcpy(&packet->crc32,&temp,sizeof(uint32_t));
                break;
            case COM_PACKET_NACK:
                temp = NACK_CRC32;
                memcpy(&packet->crc32,&temp,sizeof(uint32_t));
                break;
            default:
                break;
        }

        packet->src_addr            = source_address;
        packet->dest_addr           = addr;
        packet->packet_type         = packet_type;
        packet->payload_length      = 0;
        packet->padding             = 0;
        uint32_t size               = COM_PACKET_HEADER_SIZE;;
        memcpy(&packet->packet_size,&size,sizeof(uint32_t));
    }
    else if(packet_type == COM_PACKET_NORMAL)   {
        com_packet_create(packet, addr, para_data, para_size);
    }
    else    {
        if((uint32_t)para_data == (uint32_t)NULL) {
            cp_assert();
            return;
        }
        if(para_size >= COM_PACKET_PAYLOAD_SIZE)    {
            memcpy(packet->payload, para_data, COM_PACKET_PAYLOAD_SIZE);
            packet->payload_length = COM_PACKET_PAYLOAD_SIZE;
            packet->padding = 0;
        }
        else    {
            memcpy(packet->payload, para_data, para_size);
            packet->padding = para_size + (sizeof(uint32_t) - (para_size%sizeof(uint32_t)));  
            packet->payload_length = para_size + packet->padding;        
        }

        packet->src_addr            = source_address;
        packet->dest_addr           = addr;
        packet->packet_type         = packet_type;
        //packet->packet_size.value   = COM_PACKET_HEADER_SIZE + packet->payload_length;
        //packet->crc32.value         = crc32_calc((uint32_t*)packet->payload, packet->payload_length/sizeof(uint32_t));
        uint32_t temp = COM_PACKET_HEADER_SIZE + packet->payload_length;
        memcpy(&packet->packet_size,&temp,sizeof(uint32_t));
        temp = crc32_calc((uint32_t*)packet->payload, packet->payload_length/sizeof(uint32_t));
        memcpy(&packet->crc32,&temp,sizeof(uint32_t));
    }
}

packet_type_t com_packet_parse(com_packet_t* packet, uint8_t* data, uint32_t size){
    if(required_cb_registered != true)   {
        cp_assert();
        return COM_PACKET_NOINIT;
    }
	uint32_t index = 0;
	packet_type_t retval = COM_PACKET_FALSE;
	if(size < COM_PACKET_HEADER_SIZE){
		return retval;
	}

    uint32_t timeout_counter = get_tick();
	while((index + COM_PACKET_HEADER_SIZE) <= size && retval==COM_PACKET_FALSE){
        if(get_tick()-timeout_counter > cp_timeout)  {
            retval = COM_PACKET_TIMEOUT;
        }
		if(data[index+CP_VER_OFFSET] == COM_PACKET_VERSION)	{
			 if((index + u8_to_u32(&data[index+CP_PSIZE_OFFSET])) <= size){
				 packet_type_t type = data[index+CP_PTYPE_OFFSET];
				 uint32_t test_crc = u8_to_u32(&data[index+CP_CRC32_OFFSET]);
				 uint32_t calc_crc = 0;
				 switch(type)	{
				 	 case COM_PACKET_NORMAL:
				 		 calc_crc = crc32_calc((uint32_t*)&data[index+CP_PLOAD_OFFSET],data[index+CP_PLEN_OFFSET]/sizeof(uint32_t));
				 		 if(calc_crc==test_crc)	{
				 			 retval = type;
                             cp_buf_to_packet(packet, &data[index]);
				 		 }
				 		 break;
				 	 case COM_PACKET_CMD:
				 		 calc_crc = crc32_calc((uint32_t*)&data[index+CP_PLOAD_OFFSET],data[index+CP_PLEN_OFFSET]/sizeof(uint32_t));
                        if(calc_crc==test_crc)	{
				 			 retval = type;
                             cp_buf_to_packet(packet, &data[index]);
				 		 }
                        else   {
                             retval = COM_PACKET_BAD_CRC;
                         }
				 		 break;
				 	 case COM_PACKET_ACK:
                        if(test_crc == ACK_CRC32){
				 			 retval = type;
				 		 }
                        else   {
                             retval = COM_PACKET_BAD_CRC;
                         }
				 		 break;
				 	 case COM_PACKET_NACK:
                        if(test_crc == NACK_CRC32){
				 			 retval = type;
				 		 }
                        else   {
                             retval = COM_PACKET_BAD_CRC;
                         }
				 		 break;
                     case COM_PACKET_POS:
                        if(calc_crc == test_crc){
				 			 retval = type;
                             cp_buf_to_packet(packet, &data[index]);
				 		 }
                        else   {
                             retval = COM_PACKET_BAD_CRC;
                         }
				 		 break;
                     case COM_PACKET_CAL_FACTOR:
                        if(calc_crc == test_crc){
				 			 retval = type;
                             cp_buf_to_packet(packet, &data[index]);
				 		 }
                        else   {
                             retval = COM_PACKET_BAD_CRC;
                         }
				 		 break;
                     case COM_PACKET_DEVICES:
                        if(calc_crc == test_crc){
				 			 retval = type;
                             cp_buf_to_packet(packet, &data[index]);
				 		 }
                        else   {
                             retval = COM_PACKET_BAD_CRC;
                         }
				 		 break;
				 	 default:
                         retval = COM_PACKET_FALSE;
					 	 break;
				 }
			 }
		}
		index++;
	}
	return retval;
}
/* delete this function if no weird errors
uint32_t bind_access(uint32_t index, uint32_t buffer_size)	{
	if(index < buffer_size){
		return index;
	}
	 return 0xFFFFFFFF;
}
*/
