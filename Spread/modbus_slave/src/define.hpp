#define ELE_FRAME 7

#define SLAVEID 0x51

#define BAUDRATE_MODBUS 1000000
#define MODBUS_PIN PA4

uint8_t recFr[ELE_FRAME] = {0};  //Receive incoming request buffer
uint8_t _fr[ELE_FRAME] = {0}; //Send response buffer


HardwareSerial SerialSlave (USART1);
HardwareSerial SerailDebug (USART2);

uint32_t _T1_5; 
uint32_t _T3_5;

#define FR0 0x15 // check_connet with module Spread
#define FR1 0x25 // scale
#define FR2 0x35 // spinner
#define FR3 0x45 // hopper
#define FR4 0x57 // sensor level
#define FR5 0x65 // led
#define READ 0x17
#define WRITE 0x20

enum
{
    FUNC1, // read sens 1
    FUNC2, // read sens 2 
    FUNC3, // read sens 3
    FUNC4, // read dong co 1
    FUNC5, // read led 1
    TOTAL_FUNC,
};

uint32_t error;