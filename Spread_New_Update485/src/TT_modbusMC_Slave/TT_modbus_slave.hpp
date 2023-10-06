#ifndef TT_MODBUS_SLAVE_H
#define TT_MODBUS_SLAVE_H
#include <Arduino.h>

#define ELE_FRAME 7

class TT_MODBUS_SLAVE
{
    public:
        TT_MODBUS_SLAVE(HardwareSerial *serialPort, uint32_t baudrate, uint16_t txEnablePin, uint8_t slaveID);

        uint8_t recFr[ELE_FRAME];
        
        void init();   
        void getData();   
        
        void updateFrame(uint8_t function, uint16_t mode, uint8_t byte1, uint8_t byte2); 

        bool check_connect_master(uint8_t function, uint8_t mode, uint8_t check);

        uint8_t receive(); 

        uint32_t error;

    private:
        HardwareSerial *_serialPort;
        uint32_t _baudrate;
        uint8_t  _slaveID;
        uint16_t _txEnablePin;
        uint32_t _delayT1; // delay time between two element of frame 
        uint32_t _delayT2; // delay time between two frame

        uint32_t _errorCount;

        uint8_t _fr[ELE_FRAME]; //id, function, mode, byte1, byte2, CRCH, CRCL;
        
        uint16_t CRC16( const unsigned char *buf, unsigned int len );
        void sendData(uint8_t frame[]);     
};
#endif