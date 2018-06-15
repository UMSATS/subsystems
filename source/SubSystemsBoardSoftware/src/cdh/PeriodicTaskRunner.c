// UMSATS - CDH Scheduler 
//
// File Description:
//   Defines the main startup code for periodic tasks.
//
// History
// 2018-05-12 by Tamkin Rahman
// - Added Cpu monitor.
// 2018-06-03 by Tamkin Rahman
// - Added Time Delay Task manager task.
// 2018-06-11 by Tamkin Rahman
// - Rename "changeMockPower" to "MockInput".
// - Correct type of "rc" on initializing periodic tasks.
// - Increase stack size of tasks that make liberal use of SerialPrint to make the Atmel Studio
//   build happy.

// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "PeriodicTaskRunner.h"

#include "CANManager.h"
#include "PowerMonitor.h"
#include "SerialPrint.h"
#include "TaskWrite.h"
#include "TimeDelayedTasks.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- DEFINES ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// Cpu monitor defines.
#define MAX_TICKS 10000
#define MAX_ASSUMED_COUNT 380400

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTION PROTOTYPES ---------------------------------------------------
// -----------------------------------------------------------------------------------------------
static void CpuMonitor(void *pvParameters);

// -----------------------------------------------------------------------------------------------
// ----------------------- VARIABLES -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
  /*   When registering a task in this table, also update the CDH_TASK_TABLE_INDICES enum found
   *   in the header file for this module.
   *   
   *   Registering a task in this table means your periodic task should follow this template:
   *  
   *  void TaskToRun(void * params)
   *  {
   *    TickType_t lastWakeTime;
   *    const TickType_t frequency = pdMS_TO_TICKS( 1000 (desired period (ms)) );
   *
   *    for(;;) {
   *      lastWakeTime = xTaskGetTickCount();
   *      // Place your task code here...
   *      vTaskDelayUntil(&lastWakeTime, frequency);
   *    }
   *   }
   *  }
   *   
   *   
   *  According to https://www.freertos.org/RTOS-task-states.html, if a task becomes blocked and then becomes
   *  suspended, when the task is resumed, it enters the "Ready" or "Running" state. When vTaskDelayUntil is used
   *  we specific our current tick count and frequency, and our "lastwaketime" variable gets set to our target wake
   *  tick count. If we suspend for some time, then our target tick count has long passed, so vTaskDelayUntil will
   *  return immediately because we've specified a wake time that is in the past. This will occur until "lastWakeTime"
   *  is specified to a value in the future, at which point the task will delay like normal.
   *  
   *  There are 2 ways around this issue, 1 is to get the tick count on each iteration of the loop, and the other is 
   *  to use vTaskDelay. The first way is chosen so that the task will run as close to "frequency" as possible.
    */
    

// For tasks that make liberal use of SerialPrint, it seems they are fairly happy (i.e. no crashes) if they are allocated a lot of stack space...
TaskInfo CDH_PeriodicTaskTable[TOTAL_NUMBER_OF_TASKS] =
//				                      name,             priority,  stack depth,               task function,    task parameters,                      task handle,   power priority
{
  { (const char *)"Hello World high",                 (UBaseType_t) 2,          256,      TaskWrite_high_priority,              NULL,            (TaskHandle_t) NULL,  ALWAYS_ON    },
  { (const char *)"Hello World low",                  (UBaseType_t) 2,          256,      TaskWrite_low_priority,               NULL,            (TaskHandle_t) NULL,  SOMETIMES_ON },
  { (const char *)"Change Mock Power",                (UBaseType_t) 2,          256,      MockInput,                            NULL,            (TaskHandle_t) NULL,  ALWAYS_ON    },
  
  { (const char *)"CAN Message Manager",              (UBaseType_t) 2,          1024,     CANManager,                           NULL,            (TaskHandle_t) NULL,  ALWAYS_ON    },
  { (const char *)"CAN Monitor",                      (UBaseType_t) 2,          512,      CANMonitor,                           NULL,            (TaskHandle_t) NULL,  ALWAYS_ON    },
  { (const char *)"Time Delay Task Manager",          (UBaseType_t) 2,          1024,     TimeDelayedTaskManager,               NULL,            (TaskHandle_t) NULL,  ALWAYS_ON    },
  
  { (const char *)"CPU Monitor",                      (UBaseType_t) 1,          256,      CpuMonitor,                           NULL,            (TaskHandle_t) NULL,  ALWAYS_ON    },
};


// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTIONS -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
void startPeriodicTasks() {
	signed portBASE_TYPE rc;
	for(int i = 0; i < TOTAL_NUMBER_OF_TASKS; i++) 
	{
  
		SerialPrint("Creating task ");
		SerialPrint(CDH_PeriodicTaskTable[i].name);
		SerialPrint(".\r\n");

		rc = xTaskCreate(CDH_PeriodicTaskTable[i].taskFunction, 
						 CDH_PeriodicTaskTable[i].name, 
						 CDH_PeriodicTaskTable[i].stackDepth, 
						 CDH_PeriodicTaskTable[i].taskParams, 
						 CDH_PeriodicTaskTable[i].priority, 
						 &CDH_PeriodicTaskTable[i].taskHandle
						 );
			if (errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == rc)
		{
			  SerialPrint("Failed to create task.");
			  break;
		}
		else
		{
		  vTaskSuspend( CDH_PeriodicTaskTable[i].taskHandle );
		}
	}

  PowerMonitorInit();
}

// --------------------------------------------------------------------------------
// This task should be run at lower priority than all the other tasks in order to correctly get an estimate of
// how busy the CPU is (i.e. it must be idle, relative to other tasks).
//
// A taskYield should be appended at the end if non-cooperative multitasking is used.
//
// If non-Due is used, then the assumed MAX_TICKS (262000) should be updated.
static void CpuMonitor(void *pvParameters)
{
  UNUSED(pvParameters);
  TickType_t previousCheck = 0;
  TickType_t currentCheck;
  
  unsigned int count = 0;

  while (1)
  {
    count++;
    currentCheck = xTaskGetTickCount();

    if ((currentCheck - previousCheck) >= MAX_TICKS )  // Report every ~1000 ticks.
    {

      SerialPrint("CPU MONITOR: Load is approx ");
      SerialPrintInt( 100 - (count/ (MAX_ASSUMED_COUNT)) );  // Based on bench testing, this will give a rough calculation of the CPU load.
      //SerialPrintInt( count );
      SerialPrint("%\r\n");    

      previousCheck = currentCheck;
      count = 0;
    }
  }
}
