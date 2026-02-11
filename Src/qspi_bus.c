/*
 *	qspi_bus.c
 *	(enter file description here)
 *
 *	Created by zuidec on 01/27/26
 */
#include "hal_shim.h"
#include "qspi_bus.h"


static int qspi_bus_write(bus_t* bus, uint8_t opcode, uint32_t addr, uint8_t addr_len,
                        uint8_t dummy, const uint8_t* txbuf, size_t len)    {

    qspi_bus_ctx_t* ctx = (qspi_bus_ctx_t*)bus->ctx;
    QSPI_HandleTypeDef* hqspi = (QSPI_HandleTypeDef*)ctx->hqspi;

    int ret = 0;

    QSPI_CommandTypeDef cmd = {0};
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = opcode;

    switch(addr_len)    {
        case ADDR_32_BIT:
            cmd.AddressSize = QSPI_ADDRESS_32_BITS;
            break;
        case ADDR_24_BIT:
            cmd.AddressSize = QSPI_ADDRESS_24_BITS;
            break;
        case ADDR_16_BIT:
            cmd.AddressSize = QSPI_ADDRESS_16_BITS;
            break;
        case ADDR_8_BIT:
            cmd.AddressSize = QSPI_ADDRESS_8_BITS;
            break;
        default:
            cmd.AddressSize = QSPI_ADDRESS_NONE;
            break;
    }
    cmd.Address = addr;
    cmd.AddressMode = addr_len ? QSPI_ADDRESS_1_LINE : QSPI_ADDRESS_NONE;

    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    if(txbuf)  {
        cmd.DataMode = QSPI_DATA_1_LINE;
    }
    else    {
        cmd.DataMode = QSPI_DATA_NONE;
    }
    cmd.NbData = len;
    cmd.DummyCycles = dummy;

    if(len > 0 && txbuf) {
        ret = HAL_QSPI_Command(hqspi, &cmd, ctx->timeout);
        if(HAL_OK!= ret) {
        	__asm__("bkpt 2");
        	goto error;
        }
        ret = HAL_QSPI_Transmit(hqspi, (uint8_t*)txbuf, ctx->timeout);
        if(HAL_OK!= ret) {
        	__asm__("bkpt 2");
        	goto error;
        }
    }
    else    { // Command only
        ret = HAL_QSPI_Command(hqspi, &cmd, ctx->timeout);
        if(HAL_OK!= ret) {goto error;}
    }

error:
    return ret;
}
static int qspi_bus_read(bus_t* bus, uint8_t opcode, uint32_t addr, uint8_t addr_len,
                        uint8_t dummy, uint8_t* rxbuf, size_t len)    {

    qspi_bus_ctx_t* ctx = (qspi_bus_ctx_t*)bus->ctx;
    QSPI_HandleTypeDef* hqspi = (QSPI_HandleTypeDef*)ctx->hqspi;

    int ret = 0;

    QSPI_CommandTypeDef cmd = {0};
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = opcode;

    switch(addr_len)    {
        case ADDR_32_BIT:
            cmd.AddressSize = QSPI_ADDRESS_32_BITS;
            break;
        case ADDR_24_BIT:
            cmd.AddressSize = QSPI_ADDRESS_24_BITS;
            break;
        case ADDR_16_BIT:
            cmd.AddressSize = QSPI_ADDRESS_16_BITS;
            break;
        case ADDR_8_BIT:
            cmd.AddressSize = QSPI_ADDRESS_8_BITS;
            break;
        default:
            cmd.AddressSize = QSPI_ADDRESS_NONE;
            break;
    }
    cmd.Address = addr;
    cmd.AddressMode = addr_len ? QSPI_ADDRESS_1_LINE : QSPI_ADDRESS_NONE;

    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    if(rxbuf)  {
        cmd.DataMode = QSPI_DATA_1_LINE;
    }
    else    {
        cmd.DataMode = QSPI_DATA_NONE;
    }
    cmd.NbData = len;
    cmd.DummyCycles = dummy;

    if(len > 0 && rxbuf) {
        ret = HAL_QSPI_Command(hqspi, &cmd, ctx->timeout);
        if(HAL_OK!= ret) {
        	__asm__("bkpt 2");
        	goto error;
        }
        ret = HAL_QSPI_Receive(hqspi, rxbuf, ctx->timeout);
        if(HAL_OK!= ret) {
        	__asm__("bkpt 2");
        	goto error;
        }
    }
    else    { // Command only
        ret = HAL_QSPI_Command(hqspi, &cmd, ctx->timeout);
        if(HAL_OK!= ret) {goto error;}
    }

error:
    return ret;
}

int qspi_bus_init(bus_t *bus, qspi_bus_ctx_t *ctx)    {
    if(!bus || !ctx)    {
        return -1;
    }
    bus->read= qspi_bus_read;
    bus->write= qspi_bus_write;
    bus->ctx = ctx;
    ctx->timeout = QSPI_BUS_TIMEOUT_MS;
    return 0;
}

void qspi_bus_set_timeout(qspi_bus_ctx_t* ctx, uint32_t millis)   {
    ctx->timeout = millis;
}
