#ifndef _RS485_H
#define _RS485_H
#include <Arduino.h>
// #include "src/define.hpp"

#define EN_pin PA4 
#define ELE_FRAME 10

class RS485
{
    public:
        RS485(HardwareSerial *serialPort, uint32_t  baurate, uint32_t en);

        uint8_t receiveData[ELE_FRAME];

        bool init(uint8_t deviceID);   
        
          
        void updateFrame(uint8_t id, uint8_t function, uint16_t mode, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5); 
        bool slave_setup(uint8_t deviceID);
        bool master_setup();
        void control_slave();
        uint8_t receive(); 
   
    private:
        HardwareSerial *_serial;
        uint32_t _en;
        uint32_t _baudrate;
        uint8_t _id;

        uint32_t _T1_5; 
        uint32_t _T3_5;

        int _prev;
        bool _slave_response = 0;
        uint8_t _fr[ELE_FRAME]; //id, function, mode, byte1, byte2, byte3, byte4, byte5, CRCH, CRCL;
        uint16_t CRC16(uint8_t frame[]);
        void sendData(uint8_t frame[]); 
        void getData(); 
};
#endif

/*
update thanh class
public
var2;
init()
update()
private
 _var1;
*/