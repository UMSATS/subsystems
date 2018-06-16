#ifndef SERIAL_PRINT
#define SERIAL_PRINT
// UMSATS - CDH Scheduler 
//
// File Description:
//   Defines the interface for input/output to the serial bus. This is done to avoid dependencies
//   to processor/arduino specific functions.
//
// History
// 2018-05-12 by Tamkin Rahman
// - Added function descriptions.
// 2018-06-11 by Tamkin Rahman
// - Moved printLock here.

// -----------------------------------------------------------------------------------------------
// ----------------------- EXTERNS ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "Generic_FreeRTOS.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- EXTERNS ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
extern SemaphoreHandle_t printLock;
extern SemaphoreHandle_t scanLock;

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTION PROTOTYPES ---------------------------------------------------
// -----------------------------------------------------------------------------------------------
// Description: Reads the serial bus for the first byte of data. If no data is available, return
//              the given "default_value".
int SerialRead(int default_value);

// Description: Print the given string to the serial bus.
void SerialPrint(const char * text);

// Description: Print the given integer to the serial bus.
void SerialPrintInt(int text);

#endif // SERIAL_PRINT
