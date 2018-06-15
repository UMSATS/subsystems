#ifndef UMSATS_COMMON_H
#define UMSATS_COMMON_H
// UMSATS - CDH Scheduler 
//
// File Description:
//   Common definitions and structures for UMSATS 2018. 
//
//   Assumptions: All CAN messages are 8 bytes in length, and use standard IDs.
//      
//
// History
// 2018-05-14 by Tamkin Rahman
// - Created.

// -----------------------------------------------------------------------------------------------
// ----------------------- DEFINES ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

// CAN IDs for UMSATS modules.
#define POWER_ID       0x100U
#define GROUND_STATION 0x200U
#define CDH_ID         0x300U
#define DIAGNOSTIC_ID  0x400U
#define PAYLOAD_ID     0x500U

// Ground station commands (TURN ON WELL and TURN OFF WELL are also used by CDH messages).
#define TURN_ON_WELL          0x01
#define TURN_OFF_WELL         0x02
#define DUMP_PAYLOAD_DATA     0x03

// -----------------------------------------------------------------------------------------------
// ----------------------- STRUCTURES AND STRUCTURE TYPEDEFS -------------------------------------
// -----------------------------------------------------------------------------------------------
typedef union
{
    // Using ID = POWER_ID - Should only be transmitted by the power module.
    struct
    {
      unsigned char powerVoltageLSB[4];
      unsigned char reserved[4];
    } PowerData;

    // Using ID = GROUND_STATION - Should be spoofed by UMSATS tool.
    struct
    {
      unsigned char command;
      union
      {
        struct
        {
          unsigned char wellNumber;
          unsigned char secondsUntilCommandLSB[4]; 
        } payLoadCommand;
      } dataBytes;
    } GroundStationData;

    // Using ID = PAYLOAD_ID - Should only be transmitted by the payload module.
    struct
    {
      unsigned char wellNumber;
      unsigned char reading[4];
      unsigned char reserved[3];
    } PayloadData;

    // Using ID = CDH_ID
    struct
    {
      unsigned char command;
      unsigned char wellNumber;
    } CdhMessage;
    
    unsigned char bytes[8];
} BytesData;

typedef struct 
{
    unsigned int id;
    int extended;
    unsigned char length;
    BytesData data;
} CAN_Message;

#endif // UMSATS_COMMON_H
