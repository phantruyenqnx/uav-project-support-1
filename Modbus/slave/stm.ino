#include "src/define.hpp"

#define  LED 9  

// Using the enum instruction allows for an easy method for adding and 
// removing registers. Doing it this way saves you #defining the size 
// of your slaves register array each time you want to add more registers
// and at a glimpse informs you of your slaves register layout.

//////////////// registers of your slave ///////////////////
enum 
{     
  // just add or remove registers and your good to go...
  // The first register starts at address 0
  BYTE1,     
  BYTE2,       
  HOLDING_REGS_SIZE // leave this one
  // total number of registers for function 3 and 16 share the same register array
  // i.e. the same address space
};

unsigned int holdingRegs[HOLDING_REGS_SIZE]; // function 3 and 16 register array
////////////////////////////////////////////////////////////

HardwareSerial serScale (USART1);

HardwareSerial serDebug (USART2);

void setup()
{
    /* parameters(HardwareSerial* SerialPort,
                    long baudrate, 
            unsigned char byteFormat,
                    unsigned char ID, 
                    unsigned char transmit enable pin, 
                    unsigned int holding registers size,
                    unsigned int* holding register array)
    */
    
    /* Valid modbus byte formats are:
        SERIAL_8N2: 1 start bit, 8 data bits, 2 stop bits
        SERIAL_8E1: 1 start bit, 8 data bits, 1 Even parity bit, 1 stop bit
        SERIAL_8O1: 1 start bit, 8 data bits, 1 Odd parity bit, 1 stop bit
        
        You can obviously use SERIAL_8N1 but this does not adhere to the
        Modbus specifications. That said, I have tested the SERIAL_8N1 option 
        on various commercial masters and slaves that were suppose to adhere
        to this specification and was always able to communicate... Go figure.
        
        These byte formats are already defined in the Arduino global name space. 
    */
    serDebug.begin(115200);
    serDebug.println(F_CPU);
    modbus_configure(&serScale, 115200, SERIAL_8N1, 1, PA8, HOLDING_REGS_SIZE, holdingRegs);

    // modbus_update_comms(baud, byteFormat, id) is not needed but allows for easy update of the
    // port variables and slave id dynamically in any function.
    modbus_update_comms(115200, SERIAL_8N1, 1);
  

}

void loop()
{
    // modbus_update() is the only method used in loop(). It returns the total error
    // count since the slave started. You don't have to use it but it's useful
    // for fault finding by the modbus master.
    String c = "";
    modbus_update(&c);
    if(c.length() > 0) serDebug.println(c);
    holdingRegs[BYTE1] = 0x32;
    holdingRegs[BYTE2] = 0x39;

}