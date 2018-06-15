#ifndef PERIODIC_TASK_RUNNER_H
#define PERIODIC_TASK_RUNNER_H
// UMSATS - CDH Scheduler 
//
// File Description:
//   Contains definitions and prototypes for implementing periodic tasks.
//
// History
// 2018-05-12 by Tamkin Rahman
// - Added CAN and Cpu monitoring tasks.
// 2018-06-03 by Tamkin Rahman
// - Added Time Delay Task manager task.
// 2018-06-11 by Tamkin Rahman
// - Move Atmel Studio related defines to "Generic_FreeRTOS.h".


// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "Generic_FreeRTOS.h"
#include "TaskWrite.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- DEFINES ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// The power is assumed to be some value between 0 and 100. If a task's "powerPriority" falls below
// one of these thresholds, it will be suspended.
#define ALWAYS_ON    0
#define SOMETIMES_ON 25
#define RARELY_ON    50
#define NEVER_ON     100

#ifndef UNUSED
	#define UNUSED(x) (void)(x)
#endif

// -----------------------------------------------------------------------------------------------
// ----------------------- ENUMERATIONS AND ENUMERATION TYPEDEFS ---------------------------------
// -----------------------------------------------------------------------------------------------
// Register your periodic task(s) at the cdh task table in PeriodicTaskRunner.c.
enum CDH_TASK_TABLE_INDICES
{
    HELLO_WORLD_HIGH_IX,
    HELLO_WORLD_LOW_IX,
    CHANGE_MOCK_POWER_IX,
    CAN_MANAGER_IX,
    CAN_MONITOR_IX,
    TIME_DELAYED_TASK_MANAGER_IX,
    CPU_MONITOR_IX,
    TOTAL_NUMBER_OF_TASKS,
};

// -----------------------------------------------------------------------------------------------
// ----------------------- STRUCTURES AND STRUCTURE TYPEDEFS -------------------------------------
// -----------------------------------------------------------------------------------------------
// We will eventually need this to stop tasks when power management wants to.
typedef struct {
	const char *name;
	int priority;
	int stackDepth;
	TaskFunction_t taskFunction;
	void *taskParams;
	TaskHandle_t taskHandle;
	int powerPriority;  // http://www.freertos.org/RTOS-task-states.html
} TaskInfo;

// -----------------------------------------------------------------------------------------------
// ----------------------- EXTERNS ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
extern TaskInfo CDH_PeriodicTaskTable[TOTAL_NUMBER_OF_TASKS];

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTION PROTOTYPES ---------------------------------------------------
// -----------------------------------------------------------------------------------------------
void startPeriodicTasks(void);

#endif
