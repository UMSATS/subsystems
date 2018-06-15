#ifndef GENERIC_FREERTOS_H
#define GENERIC_FREERTOS_H
// UMSATS - CDH Scheduler 
//
// File Description:
//   Generic include file that is compatible between Arduino Due and Arduino Uno.
//
// History
// 2018-05-12 by Tamkin Rahman
// - Created.
// 2018-06-11 by Tamkin Rahman
// - Move Atmel Studio related defines to this file.

// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#ifdef ATMEL_STUDIO
  #include "FreeRTOS.h"
  #include "semphr.h"
  #include "task.h"
#else
  #ifndef __arm__
    #include <Arduino_FreeRTOS.h>
  #else
    // For SAM3X8E (i.e. Arduino Due) chips, the stock Arduino library (v9.0.0) isn't compatible.
    // This library can be obtained from: https://github.com/greiman/FreeRTOS-Arduino
    #include <FreeRTOS_ARM.h>
  #endif  // ifdef __arm__
#endif // ifdef ATMEL_STUDIO

// -----------------------------------------------------------------------------------------------
// ----------------------- DEFINES AND TYPEDEFS --------------------------------------------------
// -----------------------------------------------------------------------------------------------
// Used if this file is using Atmel Studio.
#ifdef ATMEL_STUDIO
  // These are typedefs/defines present in FreeRTOS v8.0 but not present in FreeRTOS v7.0 (needed for compatibility with Atmel Studio).
  typedef pdTASK_CODE TaskFunction_t;
  typedef xTaskHandle TaskHandle_t;
  typedef xQueueHandle SemaphoreHandle_t;
  typedef portTickType TickType_t;
  typedef unsigned long UBaseType_t;
  #define pdMS_TO_TICKS( xTimeInMs ) ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeInMs ) * ( TickType_t ) configTICK_RATE_HZ ) / ( TickType_t ) 1000 ) )
  
  // Used to denote whether the scheduler has started.
  extern int schedulerStarted;
  
  #define WaitForSemaphore( lock ) while (!schedulerStarted){} while( xSemaphoreTake( lock, portMAX_DELAY ) != pdTRUE ){} 
#else
  #define WaitForSemaphore( lock ) while( xSemaphoreTake( lock, portMAX_DELAY ) != pdTRUE ){} 
#endif


#endif // GENERIC_FREERTOS_H
