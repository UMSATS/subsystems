// UMSATS - CDH Scheduler 
//
// File Description:
//   Implements the the main Time Delayed Task manager.
//
// History
// 2018-06-03 by Tamkin Rahman
// - Created.

// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "TimeDelayedTasks.h"

#include "CANManager.h"
#include "SerialPrint.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- DEFINES ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#define MAX_QUEUE_SIZE         512
#define MAX_COMMAND_DATA_BYTES 8

// -----------------------------------------------------------------------------------------------
// ----------------------- STRUCTURES AND STRUCTURE TYPEDEFS -------------------------------------
// -----------------------------------------------------------------------------------------------
typedef struct 
{
  unsigned char command;
  unsigned int secondsUntilTaskStart;
  union 
  {
    unsigned char wellNumber;
    unsigned char bytes[MAX_COMMAND_DATA_BYTES];
  } CommandData;
} TimeDelayedTask;


// -----------------------------------------------------------------------------------------------
// ----------------------- VARIABLES -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
TimeDelayedTask TaskQueue[MAX_QUEUE_SIZE];

int Queue_index = 0;  // Points to the next message in the queue.

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTION PROTOTYPES ---------------------------------------------------
// -----------------------------------------------------------------------------------------------
// Description - Turn on the specified well.
void TurnOnWellCommand(unsigned char wellNumber);

// Description - Remove the specified element from the Task Queue.
void RemoveFromQueue(int index);

// -----------------------------------------------------------------------------------------------
// ----------------------- FUNCTIONS -------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
int AddToTimeDelayedTaskQueue(CAN_Message * message)
{
  int rc = 0;

  WaitForSemaphore( taskQueueLock );
  
  if (Queue_index < MAX_QUEUE_SIZE)
  {
    switch(message->data.GroundStationData.command)
    {
      case(TURN_ON_WELL):
      {
        TaskQueue[Queue_index].command = message->data.GroundStationData.command;
        TaskQueue[Queue_index].CommandData.wellNumber = message->data.GroundStationData.dataBytes.payLoadCommand.wellNumber;

        TaskQueue[Queue_index].secondsUntilTaskStart = 0;
        TaskQueue[Queue_index].secondsUntilTaskStart += message->data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[0];
        TaskQueue[Queue_index].secondsUntilTaskStart += (message->data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[1] << 8);
        TaskQueue[Queue_index].secondsUntilTaskStart += (message->data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[2] << 16);
        TaskQueue[Queue_index].secondsUntilTaskStart += (message->data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[3] << 24);
        
        SerialPrint("Added TURN ON WELL command to task queue, will be delayed for ");
        SerialPrintInt(TaskQueue[Queue_index].secondsUntilTaskStart);
        SerialPrint(" seconds.\r\n");
        
        Queue_index++;
        rc = 1;
        break;
      }
      case(TURN_OFF_WELL):
      {
        TaskQueue[Queue_index].command = message->data.GroundStationData.command;
        TaskQueue[Queue_index].CommandData.wellNumber = message->data.GroundStationData.dataBytes.payLoadCommand.wellNumber;

        TaskQueue[Queue_index].secondsUntilTaskStart = 0;
        TaskQueue[Queue_index].secondsUntilTaskStart += message->data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[0];
        TaskQueue[Queue_index].secondsUntilTaskStart += (message->data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[1] << 8);
        TaskQueue[Queue_index].secondsUntilTaskStart += (message->data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[2] << 16);
        TaskQueue[Queue_index].secondsUntilTaskStart += (message->data.GroundStationData.dataBytes.payLoadCommand.secondsUntilCommandLSB[3] << 24);
        
        SerialPrint("Added TURN OFF WELL command to task queue.\r\n");
    
        Queue_index++;
        rc = 1;
        break;
      }
    }
  }
  else
  {
    rc = 0;
  }

  xSemaphoreGive( taskQueueLock );

  return rc;
}

// -----------------------------------------------------------------------------------------------
void TimeDelayedTaskManager(void *pvParameters)
{
  UNUSED(pvParameters);

  int ix;
  TickType_t lastWakeTime;
  const TickType_t frequency = pdMS_TO_TICKS( 1000 ); // Run every 1000 ms.
  
  lastWakeTime = xTaskGetTickCount();
  
  while (1)
  {
      SerialPrint("Time Delay Task Manager running\r\n");

      if (Queue_index > 0)
      {
        // Need to traverse backwards through the array, so that removing
        // an element from the Queue doesn't affect ix.
        for (ix = (Queue_index - 1); ix >= 0; ix--)
        {
          if (TaskQueue[ix].secondsUntilTaskStart <= 0)
          {
            switch(TaskQueue[ix].command)
            {
              case(TURN_ON_WELL):
              {
                TurnOnWellCommand(TaskQueue[ix].CommandData.wellNumber);
                RemoveFromQueue(ix);
                break;
              }
            }
          }
          else
          {
            TaskQueue[ix].secondsUntilTaskStart -= 1;
            SerialPrint("Reduced seconds until task index (");
            SerialPrintInt(TaskQueue[ix].secondsUntilTaskStart);
            SerialPrint(")\r\n");
          }
        }
      }
      
      vTaskDelayUntil(&lastWakeTime, frequency);
  }
}

// -----------------------------------------------------------------------------------------------
void TurnOnWellCommand(unsigned char wellNumber)
{
  CAN_Message msg;
  msg.id = CDH_ID;
  msg.length = 8;
  msg.data.CdhMessage.command = TURN_ON_WELL;
  msg.data.CdhMessage.wellNumber = wellNumber;

  if (!AddToTXQueue(&msg))
  {
    SerialPrint("ERROR: Failed to add TURN ON WELL command to TX Queue\r\n");
  }
}

// -----------------------------------------------------------------------------------------------
void RemoveFromQueue(int index)
{
  int ix;
  int ixCommand;
  if (index < Queue_index)
  {
    if (Queue_index > 1)
    {
      for (ix = (index + 1); ix < Queue_index; ix++)
      { 
         TaskQueue[ix - 1].command = TaskQueue[ix].command;
         TaskQueue[ix - 1].secondsUntilTaskStart = TaskQueue[ix].secondsUntilTaskStart;
  
         for (ixCommand = 0; ixCommand < MAX_COMMAND_DATA_BYTES; ix++)
         {
           TaskQueue[ix - 1].CommandData.bytes[ixCommand] = TaskQueue[ix].CommandData.bytes[ixCommand];
         }
      }
    }
    Queue_index--;

    SerialPrint("Removed queue element at index ");
    SerialPrintInt(index);
    SerialPrint(".\r\n");
  }
}

