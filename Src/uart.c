/*
 * uart.c
 *
 *  Created on: Mar 3, 2025
 *      Author: zuidec
 */

#include "uart.h"
#include "bitutils.h"
#include <string.h>
#include <stdint.h>

static uart_handle_t *uart_instances[MAX_UART_COUNT];
static uint8_t uart_instance_id = 0;

#define NULL_UART_INST	(255)	
/*
 static void uart_dma_tx_callback(uart_handle_t* uart);
 static void uart_dma_rx_callback(uart_handle_t* uart, uint16_t* new_index);
 */

uart_status_t uart_init(uart_handle_t *uart, UART_HandleTypeDef *h_uart) {

	uart->instance_id = uart_register_instance(uart);
	if (h_uart == NULL || uart->instance_id == NULL_UART_INST) {
		uart->status = UART_INIT_FAIL;
		return uart->status;
	}

	uart->huart = h_uart;
	uart->unread_bytes = 0;
	uart->tx_busy = false;
	uart->tx_remaining = 0;
	uart->tx_size = 0;
	memset((void*)uart->rx_buffer, 0, UART_BUFFER_SIZE);
	memset((void*)uart->tx_buffer, 0, UART_BUFFER_SIZE);
	fifo_init(&uart->rx_fifo, (uint8_t*)uart->rx_buffer, UART_BUFFER_SIZE);
	fifo_init(&uart->tx_fifo, (uint8_t*)uart->tx_buffer, UART_BUFFER_SIZE);
	memset((void*)uart->dma_buffer, 0, DMA_BUF_SIZE);
	uart->dma_index = 0;
	uart->rxlock = false;
	HAL_UARTEx_ReceiveToIdle_DMA(h_uart, (uint8_t*)uart->dma_buffer, DMA_BUF_SIZE);
	uart->status = UART_OK;

	return uart->status;
}

void uart_deinit(uart_handle_t* uart)   {

    uart_unregister_instance(uart);
    HAL_UART_Abort(uart->huart);
    //HAL_UART_AbortTransmit(uart->huart);
    uart->huart = NULL;
}

uint8_t uart_register_instance(uart_handle_t *uart) {

	for (uint8_t i = 0; i < MAX_UART_COUNT; i++) {
		if (!(uart_instance_id & (1 << i))) {
			uart_instance_id |= (1 << i);
			uart_instances[i] = uart;
			return i;
		}
	}
	return NULL_UART_INST;
}

void uart_unregister_instance(uart_handle_t *uart) {
	uart_instance_id = uart_instance_id & ~(1 << uart->instance_id);
}

uart_status_t uart_update(uart_handle_t *uart) {
	if (uart == NULL) {
		uart->status = UART_BAD_HANDLE;
		return uart->status;
	}

	uart->unread_bytes = fifo_bytes_available(&uart->rx_fifo);
	if (uart->unread_bytes > UART_BUFFER_SIZE) {
		uart->status = UART_RX_FULL;
	}
	if (uart->tx_remaining > 0 && uart->tx_busy != true) {
		///
		///	(TODO) Figure out how to handle a missed transmission
		///
		uart->tx_busy = true;
		if (uart->tx_remaining > UART_BUFFER_SIZE - uart->tx_fifo.read_index) {
			uart->tx_size = UART_BUFFER_SIZE - uart->tx_fifo.read_index;
			uart->tx_waiting = true;
		} else {
			uart->tx_size = uart->tx_remaining;
			uart->tx_waiting = false;
		}

		uart->tx_remaining -= uart->tx_size;

		if (uart->tx_size > 0) {
			HAL_UART_Transmit_DMA(uart->huart,
			        &uart->tx_buffer[uart->tx_fifo.read_index], uart->tx_size);
		} else {
			uart->tx_busy = false;
		}

	}

	return uart->status;
}

uint32_t uart_read(uart_handle_t *uart, uint8_t *data, uint32_t size) {
	if (uart == NULL) {
		return 0;
	}
	uint32_t ret = 0;
	if (uart->unread_bytes > 0) {
		ret = fifo_read(&uart->rx_fifo, data, size);
		uart->unread_bytes -= ret;
	}
	return ret;
}

uint32_t uart_write(uart_handle_t *uart, uint8_t *data, uint32_t size) {
	if (uart == NULL) {
		return 0;
	}
	uint32_t ret = 0;
	if (true != (uart->tx_busy)) {
		uart->tx_busy = true;
		if (size >= UART_BUFFER_SIZE) {
			ret = fifo_write(&uart->tx_fifo, data, UART_BUFFER_SIZE);
			uint32_t bytes_to_end = UART_BUFFER_SIZE - uart->tx_fifo.read_index;
			if (ret > bytes_to_end) {
				uart->tx_waiting = true;
				uart->tx_remaining += (ret - bytes_to_end);
				uart->tx_size = bytes_to_end;

			} else {
				uart->tx_size = ret;
			}
		} else {
			ret = fifo_write(&uart->tx_fifo, data, size);
			uint32_t bytes_to_end = UART_BUFFER_SIZE - uart->tx_fifo.read_index;
			if (ret > bytes_to_end) {
				uart->tx_waiting = true;
				uart->tx_remaining += (ret - bytes_to_end);
				uart->tx_size = bytes_to_end;
			} else {
				uart->tx_size = ret;
			}
		}
		HAL_UART_Transmit_DMA(uart->huart,
		        &uart->tx_buffer[uart->tx_fifo.read_index], uart->tx_size);
	} else {
		// (TODO) handle this, could get goofy if dma is running and changing things in interrupt
		if (size > UART_BUFFER_SIZE) {
			ret = fifo_write(&uart->tx_fifo, data, UART_BUFFER_SIZE);
		} else {
			ret = fifo_write(&uart->tx_fifo, data, size);
		}
		uart->tx_remaining += ret;
        if (uart->tx_remaining > UART_BUFFER_SIZE)  {
            uart->tx_remaining = UART_BUFFER_SIZE;
        }
		uart->tx_waiting = true;
	}
	return ret;
}

void uart_write_packet(uart_handle_t *uart, com_packet_t *packet) {
	if (uart == NULL) {
		return;
	}
	/*memset(uart->tx_buffer, 0, packet->packet_size.value);
	 uart->tx_buffer[0]	= packet->version;
	 uart->tx_buffer[1]	= packet->packet_type;
	 uart->tx_buffer[2]	= packet->payload_length;
	 uart->tx_buffer[3]	= packet->padding;
	 memcpy(&uart->tx_buffer[4], packet->packet_size.bytes, sizeof(uint32_t));
	 memcpy(&uart->tx_buffer[8], packet->crc32.bytes, sizeof(uint32_t));
	 memcpy(&uart->tx_buffer[12],packet->payload,packet->payload_length);*/
	/*if(!(uart->tx_busy)){
	 uart->tx_busy = 1;
	 HAL_UART_Transmit_DMA(uart->huart, uart->tx_buffer, packet->packet_size.value);
	 }*/
	uint32_t ret = 0;
	if (true != (uart->tx_busy)) {
		if (u8_to_u32(packet->packet_size) >= UART_BUFFER_SIZE) {
			ret = fifo_write(&uart->tx_fifo, (uint8_t*) packet,
			        UART_BUFFER_SIZE);
			uint32_t bytes_to_end = UART_BUFFER_SIZE - uart->tx_fifo.read_index;
			if (ret > bytes_to_end) {
				uart->tx_waiting = true;
				uart->tx_remaining += (ret - bytes_to_end);
				uart->tx_size = bytes_to_end;

			} else {
				uart->tx_size = ret;
			}
		} else {
			ret = fifo_write(&uart->tx_fifo, (uint8_t*) packet,
			        u8_to_u32(packet->packet_size));
			uint32_t bytes_to_end = UART_BUFFER_SIZE - uart->tx_fifo.read_index;
			if (ret > bytes_to_end) {
				uart->tx_remaining += (ret - bytes_to_end);
				uart->tx_size = bytes_to_end;
				uart->tx_waiting = true;
			} else {
				uart->tx_size = ret;
			}
		}
		uart->tx_busy = true;
		HAL_UART_Transmit_DMA(uart->huart,
		        &uart->tx_buffer[uart->tx_fifo.read_index], uart->tx_size);
	} else {
		// (TODO) handle this, could get goofy if dma is running and changing things in interrupt
		if (u8_to_u32(packet->packet_size)> UART_BUFFER_SIZE) {
			ret = fifo_write(&uart->tx_fifo, (uint8_t*) packet,
			        UART_BUFFER_SIZE);
		} else {
			ret = fifo_write(&uart->tx_fifo, (uint8_t*) packet,
			        u8_to_u32(packet->packet_size));
		}
		uart->tx_remaining += ret;
		if (uart->tx_remaining >= uart->tx_fifo.size) {
			uart->tx_remaining = uart->tx_fifo.size;
		}
		uart->tx_waiting = true;
	}
	return;
}

uart_handle_t* get_uart_handle(UART_HandleTypeDef *huart) {
	for (uint8_t i = 0; i < MAX_UART_COUNT; i++) {
		if (huart == uart_instances[i]->huart) {
			return uart_instances[i];
		}
	}
	return (uart_handle_t*) NULL;
}

/*
 static void uart_dma_tx_callback(uart_handle_t* uart){
 if(uart != NULL)	{
 uart->tx_busy = 0;
 fifo_flush_unread(&uart->tx_fifo);
 }
 }

 static void uart_dma_rx_callback(uart_handle_t* uart, uint16_t* new_index)	{
 if(uart != NULL)	{
 uart->dma_index = (uint32_t)*new_index;
 HAL_UARTEx_ReceiveToIdle_DMA(uart->huart, uart->dma_buffer, DMA_BUF_SIZE);
 }
 }
 */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {

	switch (huart->RxEventType) {
	case HAL_UART_RXEVENT_HT: {
		// Do nothing
		break;
	}
	case HAL_UART_RXEVENT_TC: {
		uart_handle_t *uart = get_uart_handle(huart);
		if (uart != NULL) {
			if (!uart->rxlock && uart->dma_index != DMA_BUF_SIZE) {
				uart->rxlock = true;
				(void) fifo_write(&uart->rx_fifo,
				        (uint8_t*) &uart->dma_buffer[uart->dma_index],
				        (DMA_BUF_SIZE - uart->dma_index));
				uart->dma_index = Size;
				uart->rxlock = false;
			}
		}
		break;
	}
	default: {
		uart_handle_t *uart = get_uart_handle(huart);
		if (uart != NULL) {
			if (!uart->rxlock) {
				uart->rxlock = true;
				uint32_t new_bytes = 0;
                if (Size > uart->dma_index && Size != 0) {
                    new_bytes += fifo_write(&uart->rx_fifo,
                            (uint8_t*) &uart->dma_buffer[uart->dma_index],
                            (Size - uart->dma_index));
                    uart->dma_index += new_bytes;
                } 
                else if (Size <= uart->dma_index && Size !=0) {
                    if (uart->dma_index < DMA_BUF_SIZE) {
                        new_bytes +=
                                fifo_write(&uart->rx_fifo,
                                        (uint8_t*) &uart->dma_buffer[uart->dma_index],
                                        (DMA_BUF_SIZE - uart->dma_index));
                    }
                    new_bytes += fifo_write(&uart->rx_fifo,
                            (uint8_t*) uart->dma_buffer, Size);
                    uart->dma_index = Size;
                }
				
				uart->rxlock = false;
			}
		}
	}
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	uart_handle_t *uart = get_uart_handle(huart);
	if (uart != NULL) {

        fifo_push_read_index(&uart->tx_fifo, uart->tx_size);
		if (uart->tx_waiting == false) {
			uart->tx_remaining = 0;
			uart->tx_size = 0;
			uart->tx_busy = false;
		} 
        else if (uart->tx_remaining > UART_BUFFER_SIZE - uart->tx_fifo.read_index) {
            uart->tx_size = UART_BUFFER_SIZE - uart->tx_fifo.read_index;
            uart->tx_remaining -= uart->tx_size;
        } 
        else {
            uart->tx_size = uart->tx_remaining;
            uart->tx_remaining = 0;
            uart->tx_waiting = false;
        }

        if (uart->tx_size > 0) {
            HAL_UART_Transmit_DMA(uart->huart,
                    &uart->tx_buffer[uart->tx_fifo.read_index],
                    uart->tx_size);
        } 
        else {

            uart->tx_busy = false;
        }
    }
}

