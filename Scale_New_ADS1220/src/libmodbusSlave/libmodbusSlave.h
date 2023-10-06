#ifndef LIB_MODBUS_SLAVE_H
#define LIB_MODBUS_SLAVE_H

#include "Arduino.h"

// function definitions
unsigned int modbus_update();
void modbus_update_comms(long baud, unsigned char byteFormat, unsigned char _slaveID);
void modbus_configure(HardwareSerial *SerialPort,
											long baud,
											unsigned char byteFormat,
											unsigned char _slaveID, 
                      unsigned char _TxEnablePin, 
                      unsigned int _holdingRegsSize,
                      unsigned int* _regs);

#endif
