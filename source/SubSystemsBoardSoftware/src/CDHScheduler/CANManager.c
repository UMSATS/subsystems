// UMSATS - CDH Scheduler 
//
// File Description:
//   Implements the the main CAN traffic manager.
//
// History
// 2018-05-12 by Tamkin Rahman
// - Created.
// 2018-05-19 by Tamkin Rahman
// - Added payload message handler.
// 2018-06-23 by Tamkin Rahman
// - Added function for inserting an element to the beginning of the TX queue.

// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "CANManager.h"

#include "PayloadData.h"
#include "PowerMonitor.h"
#include "SerialPrint.h"
#include "TimeDelayedTasks.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- DEFINES ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#define MAX_RX_QUEUE_SIZE 512
#define MAX_TX_QUEUE_SIZE 512

// -----------------------------------------------------------------------------------------------
// ----------------------- VARIABLES -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
CAN_Message RX_Queue[MAX_RX_QUEUE_SIZE];
CAN_Message TX_Queue[MAX_TX_QUEUE_SIZE];

int RX_Queue_index = 0;  // Points to the next message in the RX queue.
int TX_Queue_index = 0;  // Points to the next message in the TX queue.

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTION PROTOTYPES ---------------------------------------------------
// -----------------------------------------------------------------------------------------------
// Function Description: Handles the given message.
void HandleMessage(CAN_Message * message);

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTIONS -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
void CANManager(void *pvParameters)
{
    UNUSED(pvParameters);

    int ix;
    TickType_t lastWakeTime;
    const TickType_t frequency = pdMS_TO_TICKS( 1000 ); // Run every 1000 ms.

    lastWakeTime = xTaskGetTickCount();
    
    while (1)
    {
        // SerialPrint("CAN Manager running\r\n");

        while (RX_Queue_index > 0)
        {
            WaitForSemaphore( canRxQueueLock );
            
            RX_Queue_index--;

            HandleMessage(&RX_Queue[RX_Queue_index]);
            
            SerialPrint("Processed message with\r\n    ID: ");
            SerialPrintInt(RX_Queue[RX_Queue_index].id);
            SerialPrint("\r\n    Length: ");
            SerialPrintInt(RX_Queue[RX_Queue_index].length);
            SerialPrint("\r\n    Bytes : ");
            for (ix = 0; ix < RX_Queue[RX_Queue_index].length; ix++)
            {
              SerialPrintInt(RX_Queue[RX_Queue_index].data.bytes[ix]);
              SerialPrint(" ");
            }
            SerialPrint("\r\n");

            xSemaphoreGive( canRxQueueLock );
        }
        
        vTaskDelayUntil(&lastWakeTime, frequency);
    }
}

// --------------------------------------------------------------------------------
int AddToRXQueue(CAN_Message *message)
{
    int ix;
    int rc = 0;
    if (RX_Queue_index < MAX_RX_QUEUE_SIZE)
    {
        WaitForSemaphore( canRxQueueLock );
        
        RX_Queue[RX_Queue_index].id = message->id;
        RX_Queue[RX_Queue_index].length = message->length;
        
        for (ix = 0; ix < message->length; ix++)
        {
            RX_Queue[RX_Queue_index].data.bytes[ix] = message->data.bytes[ix];
        }
        RX_Queue_index++;

        xSemaphoreGive( canRxQueueLock);
        
        rc = 1;
    }
        
    return rc;
}

// --------------------------------------------------------------------------------
int AddToTXQueue(CAN_Message * message)
{
    int ix;
    int rc = 0;

    WaitForSemaphore( canTxQueueLock );
    if (TX_Queue_index < MAX_TX_QUEUE_SIZE)
    {   
        TX_Queue[TX_Queue_index].id = message->id;
        TX_Queue[TX_Queue_index].length = message->length;
        
        for (ix = 0; ix < message->length; ix++)
        {
            TX_Queue[TX_Queue_index].data.bytes[ix] = message->data.bytes[ix];
        }
        
        TX_Queue_index++;     
        
        rc = 1;
    }
	else
	{
		rc = 0;
	}
	xSemaphoreGive( canTxQueueLock);
        
    return rc;
}
// --------------------------------------------------------------------------------
int InsertToBeginningOfTXQueue(CAN_Message * message)
{
    int ix;
	int iy;
    int rc = 0;
	
	WaitForSemaphore( canTxQueueLock );
	        
    if (TX_Queue_index < MAX_TX_QUEUE_SIZE)
    {
		if (TX_Queue_index > 0)
		{
			// Shift each element over.
			for (ix = (TX_Queue_index - 1); ix >= 0; ix--)
			{
				TX_Queue[ix + 1].id = TX_Queue[ix].id;
				TX_Queue[ix + 1].length = TX_Queue[ix].length;
				TX_Queue[ix + 1].extended = TX_Queue[ix].extended;
				
				for (iy = 0; iy < TX_Queue[ix].length; iy++)
				{
					TX_Queue[ix + 1].data.bytes[iy] = TX_Queue[ix].data.bytes[iy];
				}
			}	
		}
		
        TX_Queue[0].id = message->id;
        TX_Queue[0].length = message->length;
        
        for (ix = 0; ix < message->length; ix++)
        {
            TX_Queue[0].data.bytes[ix] = message->data.bytes[ix];
        }
        
        TX_Queue_index++;      
        
        rc = 1;
    }
	else
	{
		rc = 0;
	}
	
	xSemaphoreGive( canTxQueueLock);  
        
    return rc;
}

// --------------------------------------------------------------------------------
int GetNextCANTXMessage(CAN_Message *message)
{
    int ix;
    int rc = 0;
    
	WaitForSemaphore( canTxQueueLock );
    if (TX_Queue_index > 0)
    {   
        TX_Queue_index--;
              
        message->id = TX_Queue[TX_Queue_index].id;
        message->length = TX_Queue[TX_Queue_index].length;
        
        for (ix = 0; ix < message->length; ix++)
        {
            message->data.bytes[ix] = TX_Queue[TX_Queue_index].data.bytes[ix];
        }
        
        rc = 1;
    }
    else
    {
      message = NULL;
    }
	xSemaphoreGive( canTxQueueLock);

    return rc;
}



// --------------------------------------------------------------------------------
void HandleMessage(CAN_Message * message)
{
  int ix;
  unsigned int reading;
  switch(message->id)
  {
    case(POWER_ID):
    {
      reading = 0;
      
      // Reading is LSB first.
      for(ix = 3; ix >= 0; ix--)
      {
        reading <<= 8;
        reading += message->data.PowerData.powerVoltageLSB[ix];
      }

      if (reading != powerReading)
      {
        powerReading = reading;

        SerialPrint("Changed power reading to: ");
        SerialPrintInt(powerReading);
        SerialPrint("\r\n");  
      }
      break;
    }
    case(PAYLOAD_ID):
    {
      HandlePayloadMessage(message);
      break;
    }
    case(GROUND_STATION):
    {
      if (!AddToTimeDelayedTaskQueue(message))
      {
        SerialPrint("ERROR: Task queue full.\r\n");
      }
      break;
    }
  }
}

