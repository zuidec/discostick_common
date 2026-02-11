/*
 *	qspi_bus.h
 *	(enter file description here)
 *
 *	Created by zuidec on 01/27/26
 */

#ifndef QSPI_BUS_H
#define QSPI_BUS_H	// BEGIN QSPI_BUS_H

/*
 *	Includes
 */
#include <stdint.h>
#include <stddef.h>
#include "bus.h"


/*
 *	Defines
 */
#define QSPI_BUS_TIMEOUT_MS (100)


/*
 *	Structs, unions, etc...
 */
typedef struct  {
    void* hqspi; /* QSPI_HandleTypeDef* */
    uint32_t timeout;
} qspi_bus_ctx_t;


/*
 *	Function prototypes
 */


int qspi_bus_init(bus_t* bus, qspi_bus_ctx_t* ctx);
void qspi_bus_set_timeout(qspi_bus_ctx_t* ctx, uint32_t millis);


#endif	// END QSPI_BUS_H
