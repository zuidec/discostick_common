/*
 * fifo.c
 *
 *  Created on: Mar 3, 2025
 *      Author: zuidec
 */

#include "fifo.h"
#include <stdint.h>

void fifo_init(fifo_buffer_t* fifo, uint8_t* buffer, uint32_t size) {

    fifo->buffer = buffer;
    fifo->read_index = 0;
    fifo->write_index = 0;
    fifo->mask = size - 1;
    fifo->size = size;

}

bool fifo_is_empty(fifo_buffer_t* fifo) {

    // Return true if read index and write index are the same
    return fifo->read_index == fifo->write_index;
}

uint32_t fifo_bytes_available(fifo_buffer_t* fifo)	{
	if(fifo->write_index >= fifo->read_index)	{
		return fifo->write_index-fifo->read_index;
	}
	else	{
		return (fifo->size - fifo->read_index) + fifo->write_index;
	}
}

uint32_t fifo_write(fifo_buffer_t* fifo, uint8_t* data, uint32_t size)   {

    // Copy the indices locally in case they change
    uint32_t local_read_index = fifo->read_index;
    uint32_t local_write_index = fifo->write_index;
    uint32_t next_write_index = local_write_index;
    uint32_t bytes_written = 0;

    while(bytes_written < size){
        local_write_index = next_write_index;
        fifo->buffer[local_write_index] = data[bytes_written];
        bytes_written++;
        next_write_index = (local_write_index + 1) & fifo->mask;
        if(next_write_index == local_read_index){
        	local_read_index = (local_read_index + 1) & fifo->mask; // Push read index
        }
    }

    fifo->write_index = next_write_index;
    fifo->read_index = local_read_index;

    return bytes_written;

}

uint32_t fifo_read(fifo_buffer_t* fifo, uint8_t* data, uint32_t size) {

    // Copy the indices locally in case they change
    uint32_t local_read_index = fifo->read_index;
    uint32_t local_write_index = fifo->write_index;

    // return if theres no data to read
    if(local_read_index == local_write_index)   {
        return 0;
    }

    uint32_t bytes_read = 0;
    for(uint32_t i=0; (i < size) && (local_read_index != local_write_index); i++)	{
//    while(bytes_read < size && local_read_index != local_write_index)	{
    	data[i] = fifo->buffer[local_read_index];
    	bytes_read++;
        local_read_index = (local_read_index + 1) & fifo->mask;
    }
    fifo->read_index = local_read_index;

    return bytes_read;
}

void fifo_flush_unread(fifo_buffer_t* fifo){
	fifo->read_index = fifo->write_index;
}

uint8_t fifo_peek(fifo_buffer_t* fifo, uint32_t peek_distance)    {

    // Copy the indices locally in case they change
    uint32_t local_read_index = fifo->read_index;
    uint32_t local_write_index = fifo->write_index;

    // return if theres no data to read
    if(local_read_index + peek_distance >= local_write_index)   {
        return '\0';
    }

    return fifo->buffer[local_read_index+peek_distance];
}


uint32_t fifo_peek_continuous(fifo_buffer_t* fifo, uint8_t* data, uint32_t distance, uint32_t size)    {

    // Copy the indices locally in case they change
    uint32_t local_read_index = fifo->read_index;
    uint32_t local_write_index = fifo->write_index;

    // return if theres no data to read
    if(local_read_index == local_write_index)   {
        return 0;
    }

    if(distance > size) {
        distance = size;
    }

    uint32_t bytes_read = 0;
    while(bytes_read < distance && local_read_index != local_write_index)	{
    	data[bytes_read] = fifo->buffer[local_read_index];
    	bytes_read++;
        local_read_index = (local_read_index + 1) & fifo->mask;
    }

    return bytes_read;
}

void fifo_push_read_index(fifo_buffer_t* fifo, uint32_t distance)	{
    // Copy the indices locally in case they change
    uint32_t local_read_index = fifo->read_index;
    uint32_t local_write_index = fifo->write_index;

    // return if theres no data to read
    if(local_read_index == local_write_index)   {
        return;
    }

    for(uint32_t i = 0; (i < distance) && (local_read_index != local_write_index); i++)	{
        local_read_index = (local_read_index + 1) & fifo->mask;
    }

    fifo->read_index = local_read_index;
}
