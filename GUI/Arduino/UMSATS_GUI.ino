 //Reads all traffic on CAN0 and forwards it to CAN1 (and in the reverse direction) but modifies some frames first.
// Required libraries
#include "variant.h"
#include <due_can.h>
#include "UMSATSCommon.h"
#define POWER 1
#define PAYLOAD 2
#define GROUND 3
#define COMMS 4
//#define ON 99
//#define OFF 100
#include "Si446x.h"
#define CHANNEL 1
#define MAX_PACKET_SIZE 7
#define PACKET_NONE    0
#define PACKET_OK   1
#define PACKET_INVALID  2
int rx;
int value;
int well;
int command;
const int ledPin =13;
char CommsStateMachineInput= 'b';
//Leave defined if you use native port, comment if using programming port
//#define Serial SerialUSB
 
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
  pinMode(ledPin,OUTPUT);
  
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
//=================================================================================
void sendData()
{
  CAN_FRAME outgoing;
  outgoing.id = 0x500;
  outgoing.extended = true;
  outgoing.priority = 4; //0-15 lower is higher priority
  outgoing.length = 8;
  
  
  outgoing.data.byte[0] = 0x00;
  outgoing.data.byte[1] = 0xA3;
  outgoing.data.byte[2] = 0x00;
  outgoing.data.byte[3] = 0x00;
  outgoing.data.byte[4] = 0x00;
  outgoing.data.byte[5] = 0x00;
  outgoing.data.byte[6] = 0x00;
  outgoing.data.byte[7] = 0x00;
  Serial.print("Running!\n");
  Can0.sendFrame(outgoing);  
}
void sendPower(int value){
  CAN_FRAME outgoing;
  outgoing.id = POWER_ID;
  outgoing.extended = true;
  outgoing.priority = 4; //0-15 lower is higher priority
  outgoing.length = 8;
  
  outgoing.data.byte[0] = value & 0xFF;
  outgoing.data.byte[1] = (value & 0xFF00)>>8;
  outgoing.data.byte[2] = (value & 0xFF0000)>>16;
  outgoing.data.byte[3] = (value & 0xFF000000)>>24;
  outgoing.data.byte[4] = 0x00;
  outgoing.data.byte[5] = 0x00;
  outgoing.data.byte[6] = 0x00;
  outgoing.data.byte[7] = 0x00;
  
  Serial.print("Sent power message with value: ");
  Serial.println(value);
  Can0.sendFrame(outgoing);  }
 void sendPayload( int well,int value){
  
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
  
  Serial.print("Sent Payload Message from well ");
  Serial.print(well);
  Serial.print(" With value : ");
  Serial.print(value);
  Serial.println();
  Can0.sendFrame(outgoing);  }
void sendGround(int command, int well,int seconds){
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
  
  
if(command != DUMP_PAYLOAD_DATA){
           Serial.print("Sent ground Message to ");
          
          if(command == TURN_ON_WELL) Serial.print("turn on well ");
          else if(command == TURN_OFF_WELL) Serial.print("turn off well ");
          else{Serial.print("ERROR !");}
          
          Serial.print(well);
          Serial.print(" in ");
          Serial.print(value);
          Serial.print(" seconds.");
          Serial.println();
            }
else{
  
  Serial.print("Sent ground message to dump payload data in ");
  Serial.print(value);
  Serial.println(" seconds.");
  
  }
  Can0.sendFrame(outgoing);  }
void printFrame(CAN_FRAME &frame) {
  
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
  static unsigned long lastTime = 0;
//  STATE = NOT_CONNECTED;
//  Serial.println(CommsStateMachineInput);
if(CommsStateMachineInput == 'a'){
  radio();}
//  while(Can0.available() <= 0){}
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
          
        case 0: CommsStateMachineInput = 'a';
                Serial.println("Starting radio demo.");
                break;
        case 1: CommsStateMachineInput = 'b';
                Serial.println("Stopping radio demo..");
                break;
    }
    
    break;
    

  
  }
  
}
