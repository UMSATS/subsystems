// UMSATS - CDH Scheduler 
//
// File Description:
//   Main entry point for the CDH scheduler.
//
// History
// 2018-06-11 by Tamkin Rahman
// - Created.

// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "CDHSchedulerMain.h"

#include "CANManager.h"
#include "PayloadData.h"
#include "SerialPrint.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- VARIABLES -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// Use a lock to prevent multiple tasks from hogging the serial bus.
SemaphoreHandle_t printLock;
SemaphoreHandle_t scanLock;

SemaphoreHandle_t canRxQueueLock;
SemaphoreHandle_t canTxQueueLock;

SemaphoreHandle_t payloadQueueLock;
SemaphoreHandle_t taskQueueLock;

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTIONS -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
int SchedulerSetup()
{
  int result = 0;

  // Initialise all semaphores.
  printLock = xSemaphoreCreateMutex();
  scanLock = xSemaphoreCreateMutex();
  canRxQueueLock = xSemaphoreCreateMutex();
  canTxQueueLock = xSemaphoreCreateMutex();
  payloadQueueLock = xSemaphoreCreateMutex();
  taskQueueLock = xSemaphoreCreateMutex();

  if (   printLock        != NULL
      && scanLock         != NULL
      && canRxQueueLock   != NULL
      && canTxQueueLock   != NULL
      && payloadQueueLock != NULL
      && taskQueueLock    != NULL                                                                                                                                             
     )
  {
    startPeriodicTasks();
    SerialPrint("Starting Scheduler.\r\n");

    result = 1;
  }

  return result;
}


