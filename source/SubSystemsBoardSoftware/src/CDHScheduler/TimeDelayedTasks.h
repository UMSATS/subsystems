#ifndef TIME_DELAYED_TASKS_H
#define TIME_DELAYED_TASKS_H
// UMSATS - CDH Scheduler 
//
// File Description:
//   Implements the interface for the main Time Delayed Task manager.
//
// History
// 2018-06-03 by Tamkin Rahman
// - Created.

// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "PeriodicTaskRunner.h"
#include "UMSATSCommon.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- VARIABLES -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
extern SemaphoreHandle_t taskQueueLock;

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTION PROTOTYPES ---------------------------------------------------
// -----------------------------------------------------------------------------------------------

//------------
// Function Description: The task that initializes and runs the CAN manager.
void TimeDelayedTaskManager(void *pvParameters);

//------------
// Function Description: Add the given message to the internal Task queue. If a failure occurs, return 0.
int AddToTimeDelayedTaskQueue(CAN_Message * message);

#endif // TIME_DELAYED_TASKS_H
