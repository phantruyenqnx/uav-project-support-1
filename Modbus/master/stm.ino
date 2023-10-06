#include "src/define.hpp"


#define baud 115200
#define timeout 1000
#define polling 200 // the scan rate
#define retry_count 10

// used to toggle the receive/transmit pin on the driver
#define TxEnablePin PB8 

// The total amount of available memory on the master to store data
#define TOTAL_NO_OF_REGISTERS 3

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum
{
  PACKET1,
  PACKET2,
  TOTAL_NO_OF_PACKETS // leave this last entry
};

// Create an array of Packets to be configured
Packet packets[TOTAL_NO_OF_PACKETS];

// Masters register array
unsigned int regs[TOTAL_NO_OF_REGISTERS];

HardwareSerial SerialScale (USART6);
HardwareSerial serDebug   (USART1);
void setup()
{
    serDebug.begin(baud);
    // Initialize each packet
    modbus_construct(&packets[PACKET1], 1, READ_HOLDING_REGISTERS, 0, 2, 0);
    //modbus_construct(&packets[PACKET2], 1, READ_HOLDING_REGISTERS, 1, 1, 0);
    //modbus_construct(&packets[PACKET2], 1, PRESET_MULTIPLE_REGISTERS, 4, 4, 0);
    
    // Initialize the Modbus Finite State Machine
    modbus_configure(&SerialScale, baud, SERIAL_8N1, timeout, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS, regs);
    SerialScale.println(F_CPU);

}

void loop()
{
    modbus_update();

    int16_t _v = regs[0] * 100 ;
    int16_t _b = (_v > 0) ? regs[1] : - regs[1] ;

    serDebug.println(_v + _b);
}
