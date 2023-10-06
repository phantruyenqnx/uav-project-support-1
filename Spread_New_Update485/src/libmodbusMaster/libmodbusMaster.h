#ifndef LIB_MODBUS_MASTER_H
#define LIB_MODBUS_MASTER_H

#include "Arduino.h"

#define COIL_OFF 0x0000 // Function 5 OFF request is 0x0000
#define COIL_ON 0xFF00 // Function 5 ON request is 0xFF00
#define READ_COIL_STATUS 1 // Reads the ON/OFF status of discrete outputs (0X references, coils) in the slave.
#define READ_INPUT_STATUS 2 // Reads the ON/OFF status of discrete inputs (1X references) in the slave.
#define READ_HOLDING_REGISTERS 3 // Reads the binary contents of holding registers (4X references) in the slave.
#define READ_INPUT_REGISTERS 4 // Reads the binary contents of input registers (3X references) in the slave. Not writable.
#define FORCE_SINGLE_COIL 5 // Forces a single coil (0X reference) to either ON (0xFF00) or OFF (0x0000).
#define PRESET_SINGLE_REGISTER 6 // Presets a value into a single holding register (4X reference).
#define FORCE_MULTIPLE_COILS 15 // Forces each coil (0X reference) in a sequence of coils to either ON or OFF.
#define PRESET_MULTIPLE_REGISTERS 16 // Presets values into a sequence of holding registers (4X references).

typedef struct
{
  // specific packet info
  unsigned char id;
  unsigned char function;
  unsigned int address;
	
	// For functions 1 & 2 data is the number of points
	// For function 5 data is either ON (oxFF00) or OFF (0x0000)
	// For function 6 data is exactly that, one register's data
  // For functions 3, 4 & 16 data is the number of registers
  // For function 15 data is the number of coils
  unsigned int data; 
	
	unsigned int local_start_address;
  
  // modbus information counters
  unsigned int requests;
  unsigned int successful_requests;
	unsigned int failed_requests;
	unsigned int exception_errors;
  unsigned int retries;
  	
  // connection status of packet
  unsigned char connection; 
  
}Packet;

// function definitions
void modbus_update();

void modbus_construct(Packet *_packet, 
											unsigned char id, 
											unsigned char function, 
											unsigned int address, 
											unsigned int data,
											unsigned _local_start_address);
											
void modbus_configure(HardwareSerial* SerialPort,
											long baud, 
											unsigned char byteFormat,
											long _timeout, 
											long _polling, 
											unsigned char _retry_count, 
											unsigned char _TxEnablePin,
											Packet* _packets, 
											unsigned int _total_no_of_packets,
											unsigned int* _register_array);

#endif