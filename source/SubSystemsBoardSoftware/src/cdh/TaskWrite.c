// UMSATS - CDH Scheduler 
//
// File Description:
//   Contains the mock tasks for the scheduler.
//
// History
// 2018-05-12 by Tamkin Rahman
// - Removed power monitor flags.
// 2018-06-11 by Tamkin Rahman
// - Move input mocking to "changeMockPower" task (renamed to MockInput).

// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "TaskWrite.h"

#include "CANManager.h"
#include "PeriodicTaskRunner.h"
#include "PowerMonitor.h"
#include "SerialPrint.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTIONS -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// Example of a periodic task.
void TaskWrite_low_priority(void *params) {
  UNUSED(params);
  
  TickType_t lastWakeTime;
  const TickType_t frequency = pdMS_TO_TICKS( 5000 ); // Run every 1000 ms.
   
  for(;;) 
  {
    lastWakeTime = xTaskGetTickCount();
	  SerialPrint("low priority hello world\r\n");
	
	  vTaskDelayUntil(&lastWakeTime, frequency);
  }
}

// --------------------------------------------------------------------------------
void TaskWrite_high_priority(void *params) {
  UNUSED(params);
  
  TickType_t lastWakeTime;
  const TickType_t frequency = pdMS_TO_TICKS( 5000 ); // Run every 1000 ms.

  for(;;) {
    lastWakeTime = xTaskGetTickCount();
	
    SerialPrint("high priority hello world\r\n");
    vTaskDelayUntil(&lastWakeTime, frequency);
  }
}

// --------------------------------------------------------------------------------
// Mock function to fake various tasks based on various inputs.
void MockInput(void *params) {
  UNUSED(params);
    
  TickType_t lastWakeTime;
  const TickType_t frequency = pdMS_TO_TICKS( 100 ); // Run every 100 ms.
  int valueRead = 0;
  int received = 0;
  
  CAN_Message message;
  
  message.id = POWER_ID;
  message.length = 8;
  message.data.bytes[0] = 0x50;
  message.data.bytes[1] = 0x00;
  message.data.bytes[2] = 0x00;
  message.data.bytes[3] = 0x00;
  
  while(1) {
	lastWakeTime = xTaskGetTickCount();  
	vTaskDelayUntil(&lastWakeTime, frequency);	 
	received = SerialRead(powerReading);
  
	if (received == 'A') 
	{
		valueRead = 10;
	} 
	else if (received == 'B') 
	{
		valueRead = 90;
	}
	else if (received == 'C') 
	{
		AddToTXQueue(&message);
		SerialPrint("Added to TX queue\r\n");
	}
	else if (received == 'D') 
	{
		message.id = POWER_ID;
		message.length = 8;
		message.data.bytes[0] = 0x50;
		message.data.bytes[1] = 0x00;
		message.data.bytes[2] = 0x00;
		message.data.bytes[3] = 0x00;
    
		AddToRXQueue(&message);
		SerialPrint("Added to RX queue\r\n");
	}
	else if (received == 'E')
	{
		message.id = PAYLOAD_ID;
		message.length = 8;
		message.data.bytes[0] = 1;  // Well number
		// Reading and reserved bytes not set.
    
		AddToRXQueue(&message);
		SerialPrint("Added to RX queue\r\n");
	}
	else if (received == 'F')
	{
		message.id = GROUND_STATION;
		message.length = 8;
		message.data.GroundStationData.command = TURN_ON_WELL; // Command.
		message.data.GroundStationData.dataBytes.payLoadCommand.wellNumber = 1; // Well number.

		// Wait 1 second until using this command.
		message.data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[0] = 1;
		message.data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[1] = 0;
		message.data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[2] = 0;
		message.data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[3] = 0;
    
		// Reading and reserved bytes not set.
    
		AddToRXQueue(&message);
		SerialPrint("Added to RX queue\r\n");
	}    

    
	if ( valueRead != powerReading ) {
		if ( (valueRead > POWER_MIN) && (valueRead <= POWER_MAX) ){
			powerReading = valueRead;
			SerialPrint("Changed the power to ");
			SerialPrintInt(powerReading);
				SerialPrint("\r\n");
		}
	}
  }
}

