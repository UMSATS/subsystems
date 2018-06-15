// UMSATS - CDH Scheduler 
//
// File Description:
//   Manages data from the payload.
//
// History
// 2018-05-19 by Tamkin Rahman
// - Created.

// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "PayloadData.h"

#include "CANManager.h"
#include "SerialPrint.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- DEFINES ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#define MAX_QUEUE_SIZE  512 // Should be dependent on how often data can be transmitted.
#define NUMBER_OF_WELLS 5

// -----------------------------------------------------------------------------------------------
// ----------------------- DEFINES ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
typedef struct
{
  unsigned char wellNumber;
  unsigned int timestamp;
  unsigned char reading[4];
  unsigned char checksum;
} PayLoadData;

// -----------------------------------------------------------------------------------------------
// ----------------------- VARIABLES -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
int QueueIndex = 0;  // Points to next entry in the data queue.

PayLoadData Data_Queue[MAX_QUEUE_SIZE];

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTION PROTOTYPES ---------------------------------------------------
// -----------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTIONS -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
void HandlePayloadMessage(CAN_Message * message)
{
  if (QueueIndex < MAX_QUEUE_SIZE)
  {
      WaitForSemaphore( payloadQueueLock );
      
      Data_Queue[QueueIndex].timestamp = xTaskGetTickCount();
          
      Data_Queue[QueueIndex].wellNumber = message->data.PayloadData.wellNumber;
      
      Data_Queue[QueueIndex].reading[0] = message->data.PayloadData.reading[0];
      Data_Queue[QueueIndex].reading[1] = message->data.PayloadData.reading[1];
      Data_Queue[QueueIndex].reading[2] = message->data.PayloadData.reading[2];
      Data_Queue[QueueIndex].reading[3] = message->data.PayloadData.reading[3];

      QueueIndex++;

      SerialPrint("Processed payload reading from well ");
      SerialPrintInt(message->data.PayloadData.wellNumber);
      SerialPrint("\r\n");

      xSemaphoreGive( payloadQueueLock);
  }
}


