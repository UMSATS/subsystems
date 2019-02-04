// UMSATS-GUI
//
// File Description:
//   Reads CAN messages and sends them over serial to the GUI, and also
//   gets messages from the GUI and sends over CAN. Also runs the Comms demo.
//
//   Assumptions: All CAN messages are 8 bytes in length, and use standard IDs.
//      
//
// History
// 2018-05 
// - Created. Modified from the Traffic_Modifier example.
// 2018-06 by Joseph Howarth
// - Added comms code, fixed formatting for serial messages.
// 2018-12 by Joseph Howarth
// - Organized and refactored to follow programming standards.

#include <due_can.h>
#include "Si446x.h"
#include "variant.h"
#include "UMSATSCommon.h"

#define POWER 1
#define PAYLOAD 2
#define GROUND 3
#define COMMS 4

#define CHANNEL 1
#define MAX_PACKET_SIZE 7
#define PACKET_NONE    0
#define PACKET_OK   1
#define PACKET_INVALID  2


//Leave defined if you use native port, comment if using programming port
//#define Serial SerialUSB

const int LED_PIN =13;

int  rx;
int  value;
int  well;
int  command;
char comms_sm_input= 'b';

 
void setup()
{

  Serial.begin(9600);


  // initialize radio 
  Si446x_init();
  Si446x_setTxPower(SI446X_MAX_TX_POWER);
  
  // Initialize CAN0 and CAN1, Set the proper baud rates here
  Can0.begin(CAN_BPS_1000K);
  Can1.begin(CAN_BPS_1000K);


  Serial.setTimeout(10);
  pinMode(LED_PIN,OUTPUT);
  
  //By default there are 7 mailboxes for each device that are RX boxes
  //This sets each mailbox to have an open filter that will accept extended
  //or standard frames

  int filter;

  //extended
  for (filter = 0; filter < 3; filter++) {
  Can0.setRXFilter(filter, 0, 0, true);
  Can1.setRXFilter(filter, 0, 0, true);
  }  

  //standard
  for (int filter = 3; filter < 7; filter++) {
  Can0.setRXFilter(filter, 0, 0, false);
  Can1.setRXFilter(filter, 0, 0, false);
  }  
  
  Can0.watchFor();
  Serial.print("Finished setup!\n");
}

// Radio functions =======================================================================
static uint8_t counter;

typedef struct{
  uint8_t ready;
  int16_t rssi;
  uint8_t length;
  uint8_t buffer[MAX_PACKET_SIZE];
} pingInfo_t;

static volatile pingInfo_t pingInfo;

void SI446X_CB_RXCOMPLETE(uint8_t length, int16_t rssi)
{
  if(length > MAX_PACKET_SIZE)
    length = MAX_PACKET_SIZE;
  pingInfo.ready = PACKET_OK;
  pingInfo.rssi = rssi;
  pingInfo.length = length;
  Si446x_read((uint8_t*)pingInfo.buffer, length);
  // Radio will now be in idle mode
}

void SI446X_CB_RXINVALID(int16_t rssi)
{
  pingInfo.ready = PACKET_INVALID;
  pingInfo.rssi = rssi;
}

void radio()
{
  static uint32_t pings;
  static uint32_t invalids;
  Si446x_RX(CHANNEL);
  //Serial.print("pingInfo.ready: ");
  //Serial.println(pingInfo.ready);
  while (pingInfo.ready == PACKET_NONE);
  //Serial.print("pingInfo.ready: ");
  //Serial.println(pingInfo.ready);
  if(pingInfo.ready != PACKET_OK)
  {
    invalids++;
    pingInfo.ready = PACKET_NONE;
    Si446x_RX(CHANNEL);
  }
  else
  {
    //Serial.print("SESES");
    pings++;
    Serial.println();
    Serial.println();
    //Serial.println(pingInfo.ready);
    pingInfo.ready = PACKET_NONE;
    Serial.print(F("Signal strength "));
    Serial.print(pingInfo.rssi);
    Serial.println(F("dBm"));
    // Print out ping contents
    //Serial.print(F("Data from server: "));
    //Serial.write((uint8_t*)pingInfo.buffer, sizeof(pingInfo.buffer));
    //Serial.print("SESES");
    Serial.print(char(pingInfo.buffer[0]));
    Serial.print(char(pingInfo.buffer[1]));
    Serial.print(char(pingInfo.buffer[2]));
    Serial.print(char(pingInfo.buffer[3]));
    Serial.print(char(pingInfo.buffer[4]));
    Serial.print(char(pingInfo.buffer[5]));
    Serial.print(char(pingInfo.buffer[6]));
    Serial.println();
  }
}

//GUI Functions ====================================================================


void sendPower(int value){
 //This function sends a power message over the CAN bus to the CDH module.
 //An acknowledgment is sent over serial back to the GUI.
 //   
 //Parameters:
 //  value: an integer that represents the power level.

  
  CAN_FRAME outgoing;
  
  outgoing.id = POWER_ID;
  outgoing.extended = true;
  outgoing.priority = 4; //0-15 lower is higher priority
  outgoing.length = 8;
  
  outgoing.data.byte[0] = (value & 0xFF);
  outgoing.data.byte[1] = (value & 0xFF00)>>8;
  outgoing.data.byte[2] = (value & 0xFF0000)>>16;
  outgoing.data.byte[3] = (value & 0xFF000000)>>24;
  outgoing.data.byte[4] = 0x00;
  outgoing.data.byte[5] = 0x00;
  outgoing.data.byte[6] = 0x00;
  outgoing.data.byte[7] = 0x00;

  Can0.sendFrame(outgoing);
  
  Serial.print("Sent power message with value: ");
  Serial.println(value);
  
  }
  
 void sendPayload( int well,int value){
 //This function sends a payload data message over the CAN bus to the CDH module.
 //An acknowledgment is sent over serial back to the GUI.
 //
 //Parameters:
 //  well:  an integer representing which well the data is from.
 //  value: an integer that represents the payload data.

  
  CAN_FRAME outgoing;
  
  outgoing.id = PAYLOAD_ID;
  outgoing.extended = true;
  outgoing.priority = 4; //0-15 lower is higher priority
  outgoing.length = 8;
  
  outgoing.data.byte[0] = well;
  outgoing.data.byte[1] = value & 0xFF;
  outgoing.data.byte[2] = (value & 0xFF00)>>8;
  outgoing.data.byte[3] = (value & 0xFF0000)>>16;
  outgoing.data.byte[4] = (value & 0xFF000000)>>24;
  outgoing.data.byte[4] = 0x00;
  outgoing.data.byte[5] = 0x00;
  outgoing.data.byte[6] = 0x00;
  outgoing.data.byte[7] = 0x00;

  Can0.sendFrame(outgoing); 
  
  Serial.print("Sent Payload Message from well ");
  Serial.print(well);
  Serial.print(" With value : ");
  Serial.print(value);
  Serial.println();

 }
 
void sendGround(int command, int well,int seconds){
 //This function sends a ground station message over the CAN bus to the CDH module.
 //An acknowledgment is sent over serial back to the GUI.
 //
 //Parameters:
 //  command:  This parameter selects the type of command sent to CDH, 
 //            either turn on/off a payload well, dump the payload data or get adc reading.
 //  well:     An integer that represents the well which should be turned on/off, or which ADC should be read.
 //  seconds:  The number of seconds to delay before executing the specified command. 

  CAN_FRAME outgoing;
  
  outgoing.id = GROUND_STATION;
  outgoing.extended = true;
  outgoing.priority = 4; //0-15 lower is higher priority
  outgoing.length = 8;
  
  outgoing.data.byte[0] = command;
  outgoing.data.byte[1] = well;
  outgoing.data.byte[2] = seconds & 0xFF;
  outgoing.data.byte[3] = (seconds & 0xFF00)>>8;
  outgoing.data.byte[4] = (seconds & 0xFF0000)>>16;
  outgoing.data.byte[5] = (seconds & 0xFF000000)>>24;
  outgoing.data.byte[6] = 0x00;
  outgoing.data.byte[7] = 0x00;
  
  Can0.sendFrame(outgoing);
  
  if(command == TURN_ON_WELL || command == TURN_OFF_WELL){
    
    Serial.print("Sent ground Message to ");
    
    if(command == TURN_ON_WELL){
      Serial.print("turn on well ");
    }
    else if(command == TURN_OFF_WELL){
      Serial.print("turn off well ");
    }
    else{
      Serial.print("ERROR !");
    }
    
    Serial.print(well);
    Serial.print(" in ");
    Serial.print(seconds);
    Serial.print(" seconds.");
    Serial.println();
    
  }
  else if (command == DUMP_PAYLOAD_DATA){
    Serial.print("Sent ground message to dump payload data in ");
    Serial.print(seconds);
    Serial.println(" seconds.");
  }
  else if(command == GET_ADC_READING){
    Serial.print("Sent ground message to get reading from ADC ");
    Serial.print(well);
    Serial.print(" in ");
    Serial.print(seconds);
    Serial.print(" seconds");
    Serial.println();
  }
  
}
  
void printFrame(CAN_FRAME &frame) {
  //This function prints out a CAN frame to the GUI.

   Serial.print("ID: 0x");
   Serial.print(frame.id, HEX);
   Serial.print(" Len: ");
   Serial.print(frame.length);
   Serial.print(" Data: 0x");
   for (int count = 0; count < frame.length; count++) {
       Serial.print(frame.data.bytes[count], HEX);
       Serial.print(" ");
   }
   Serial.print("\r\n");
}


//=========================================================================
//LOOP
//=========================================================================

void loop(){

  CAN_FRAME incoming;

  if(comms_sm_input == 'a'){
    radio();
  }

  if(Can0.available() > 0){
    
    Can0.read(incoming);
    
    printFrame(incoming);  
  }

  rx = Serial.parseInt();

    switch(rx){
        
      case POWER:

        while(Serial.available()<= 0){}
        value = Serial.parseInt();
   
        sendPower(value);

      break;


      case PAYLOAD:
    
        while(Serial.available()<= 0){}
        well = Serial.parseInt();
    
        while(Serial.available()<= 0){}
        value = Serial.parseInt();
    
        sendPayload(well,value);

      break;


      case GROUND:

        while(Serial.available()<= 0){}
        command = Serial.parseInt();
    
        while(Serial.available()<= 0){}
        well = Serial.parseInt();
    
        while(Serial.available()<= 0){}
        value = Serial.parseInt();
    
        sendGround(command,well,value);

      break;


      case COMMS:
    
        while(Serial.available()<= 0){}
        command = Serial.parseInt();

        switch (command){
          
          case 0: 

            comms_sm_input = 'a';
            Serial.println("Starting radio demo.");

          break;


          case 1:
        
            comms_sm_input = 'b';
            Serial.println("Stopping radio demo..");

          break;
        }
    
        break;   
    }
  
}
