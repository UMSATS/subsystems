// UMSATS - CDH Scheduler 
//
// File Description:
//  Main entry-point for the scheduler. This file should contain the bare minimum w.r.t. working code. 
//  Should only contain Arduino specific code.
//
// The due CAN bus library is used for this file: https://github.com/collin80/due_can
// - Requires this library as well: https://github.com/collin80/can_common
//
// History
// 2018-05-12 by Tamkin Rahman
// - Updated with CAN monitor.
// 2018-06-11 by Tamkin Rahman
// - Separate scheduler initialization to a separate function.
// - Separate "SerialRead" commands into the "MockInput" task.

// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "Generic_FreeRTOS.h"

#include <due_can.h>
  
extern "C" 
{
  #include "CANManager.h"
  #include "CDHSchedulerMain.h"
  #include "SerialPrint.h"
}
// -----------------------------------------------------------------------------------------------
// ----------------------- ARDUINO FUNCTIONS -----------------------------------------------------
// -----------------------------------------------------------------------------------------------
// The setup function runs once when you press reset or power the board
void setup() 
{
  Serial.begin(9600);

  if (SchedulerSetup())
  {
    // Initialize CAN0 and CAN1, Set the proper baud rates here
    Can0.begin(CAN_BPS_250K);
  
    //By default there are 7 mailboxes for each device that are RX boxes
    //This sets each mailbox to have an open filter that will accept extended
    //or standard frames
    int filter;
    
    //extended
    for (filter = 0; filter < 3; filter++) {
      Can0.setRXFilter(filter, 0, 0, true);
    }  
    //standard
    for (filter = 3; filter < 7; filter++) {
      Can0.setRXFilter(filter, 0, 0, false);
    }  
    
    Can0.watchFor();
  
    // Start FreeRTOS
    vTaskStartScheduler();
  }
  else
  {
    Serial.println("Error, Scheduler failed to start.");
  }
  Serial.println("Insufficent RAM.\n");
  while(1){};
}

void loop()
{
  // Empty. Things are done in Tasks.
}

// -------------------------------------------------------------------------------------------
// ----------------------- FUNCTIONS ---------------------------------------------------------
// -------------------------------------------------------------------------------------------
int SerialRead(int default_value) 
{
  int result = default_value;  
  
  while( xSemaphoreTake( printLock, portMAX_DELAY ) != pdTRUE ){}
  if (Serial.available() > 0)
  {
    result = Serial.read();
  }
  xSemaphoreGive( printLock );
  
  return result;
}

// --------------------------------------------------------------------------------
void SerialPrint(const char * text) 
{
  while( xSemaphoreTake( printLock, portMAX_DELAY ) != pdTRUE ){}
  Serial.print(text); 
  xSemaphoreGive( printLock );
}

// --------------------------------------------------------------------------------
void SerialPrintInt(int text) 
{
  while( xSemaphoreTake( printLock, portMAX_DELAY ) != pdTRUE ){} 
  Serial.print(text); 
  xSemaphoreGive( printLock );
}

// --------------------------------------------------------------------------------
void CANMonitor(void *pvParameters)
{
    UNUSED(pvParameters);

    int ix;
    TickType_t lastWakeTime;

    CAN_FRAME incoming;
    CAN_FRAME outgoing;
    CAN_Message currentTxMessage;
    CAN_Message currentRxMessage;
    
    const TickType_t frequency = pdMS_TO_TICKS( 1 ); // Run every 1 ms.

    lastWakeTime = xTaskGetTickCount();
    
    while (1)
    {
        // SerialPrint("Monitoring CAN bus\n");
        
        while (0 != GetNextCANTXMessage(&currentTxMessage))
        {
          // Transmit the message.
          SerialPrint("Transmitted message with\n    ID: ");
          SerialPrintInt(currentTxMessage.id);
          SerialPrint("\n    Length: ");
          SerialPrintInt(currentTxMessage.length);
          SerialPrint("\n    Bytes : ");
          for (ix = 0; ix < currentTxMessage.length; ix++)
          {
            SerialPrintInt(currentTxMessage.data.bytes[ix]);
            SerialPrint(" ");
          }
          SerialPrint("\n");

          outgoing.id = currentTxMessage.id;
          outgoing.length = currentTxMessage.length;
          outgoing.extended = false;

          for (ix = 0; ix < outgoing.length; ix++)
          {
            outgoing.data.bytes[ix] = currentTxMessage.data.bytes[ix];
          }

          Can0.sendFrame(outgoing);
        }
        
        // Also, listen for messages here, and pass them onto the CAN manager, e.g.
        if (Can0.available() > 0) {
          Can0.read(incoming);

          currentRxMessage.id = incoming.id;
          currentRxMessage.length = incoming.length;

          for(ix = 0; ix < currentRxMessage.length; ix++)
          {
            currentRxMessage.data.bytes[ix] = incoming.data.bytes[ix];
          }

          while (AddToRXQueue(&currentRxMessage) == 0)
          {
            // Prefer this to yielding.
            vTaskDelay(0);
          }
        }
        
        vTaskDelayUntil(&lastWakeTime, frequency);
    }
}

