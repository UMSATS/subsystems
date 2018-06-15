// UMSATS - CDH Scheduler 
//
// File Description:
//   Contains definitions and prototypes for implementing the power monitor.
//
// History
// 2018-05-12 by Tamkin Rahman
// - Removed power monitor flags.

// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "PowerMonitor.h"

#include "SerialPrint.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTION PROTOTYPES ---------------------------------------------------
// -----------------------------------------------------------------------------------------------
void PowerMonitor(void *pvParameters);

// -----------------------------------------------------------------------------------------------
// ----------------------- VARIABLES -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// Any task parameters needed? Maybe pass a pointer to CDHPeriodicTaskTable instead of using the global variable?
TaskInfo PowerMonitorParams =
//                       name,                 priority,  stack depth,   task function,    task parameters,                task handle , power priority
    { (const char *)"Power Monitor",  (UBaseType_t)        1,          128,    PowerMonitor,               NULL,        (TaskHandle_t) NULL  , ALWAYS_ON     };

unsigned int volatile powerReading = 20;

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTIONS -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// Get the current power level of the system (normalized from 0 to 100).
int GetPower(){
  return powerReading;
}

// ----------------------------------------------------------------------
void PowerMonitorInit(){
  TaskInfo info = PowerMonitorParams;
  xTaskCreate(info.taskFunction, info.name, info.stackDepth, info.taskParams, info.priority, &info.taskHandle);
}

// ----------------------------------------------------------------------
void Disable(TaskHandle_t handle){
  // If NULL is passed to vTaskSuspend, the current task is suspended.
  if (handle != NULL){
    vTaskSuspend(handle);
  }
}

// ----------------------------------------------------------------------
void Enable(TaskHandle_t handle){
  if (handle != NULL){
    vTaskResume(handle);
  }  
}

// ----------------------------------------------------------------------
// The power monitor should regularly:
// - Monitor the power level.
// - Disable/enable periodic tasks with "powerPriority" greater than the current power.
void PowerMonitor(void *pvParameters){
  UNUSED(pvParameters);
  
  TickType_t lastWakeTime;
  const TickType_t frequency = pdMS_TO_TICKS( 1000 ); // Run every 1000 ms.

  lastWakeTime = xTaskGetTickCount();
  
  while (1) {
    int current_power = GetPower(); // Assume it returns 0-100.
    for (int ix = 0; ix < TOTAL_NUMBER_OF_TASKS; ix++) {
      if (CDH_PeriodicTaskTable[ix].powerPriority > current_power){
		Disable(CDH_PeriodicTaskTable[ix].taskHandle);
      } else {
		Enable(CDH_PeriodicTaskTable[ix].taskHandle);
      }
    } 
    vTaskDelayUntil(&lastWakeTime, frequency);
  }
}

