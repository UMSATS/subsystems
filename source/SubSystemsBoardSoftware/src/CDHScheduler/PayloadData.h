#ifndef PAYLOAD_DATA_H
#define PAYLOAD_DATA_H
// UMSATS - CDH Scheduler
//
// File Description:
//   Implements an interface for the payload handler.
//
// History
// 2018-05-19 by Tamkin Rahman
// - Created.

// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "Generic_FreeRTOS.h"
#include "UMSATSCommon.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- VARIABLES -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
extern SemaphoreHandle_t payloadQueueLock;

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTION PROTOTYPES ---------------------------------------------------
// -----------------------------------------------------------------------------------------------

//------------
// Function Description: Handle the given payload message and saves the data to memory.
void HandlePayloadMessage(CAN_Message * message);

//------------
// Function Description: Send all payload data to ground station.
void DumpPayloadData(void);

#endif // PAYLOAD_DATA_H
