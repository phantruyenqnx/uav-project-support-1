#include <Arduino.h>
#include "rs458.hpp"
#include "src/define.hpp"

RS485::RS485(HardwareSerial *serialPort, uint32_t baurate, uint32_t en)
{
    this->_serial = serialPort;
    this->_en = en; 
    this->_baudrate = baurate;
}

bool RS485::init(uint8_t deviceID)
{
    _id = deviceID;

    (*_serial).begin(_baudrate);
    pinMode(_en, OUTPUT);
    while (!(*_serial));digitalWrite(EN_pin, LOW);
    if (_baudrate > 19200)
    {
        _T1_5 = 750; 
        _T3_5 = 1750; 
    }
    else 
    {
        _T1_5 = 15000000/_baudrate; 
        _T3_5 = 35000000/_baudrate; 
    }

    #ifdef MASTER
        if(master_setup() == 1) 
        {
            return 1;
        }
        else if(master_setup() != 1) 
        {
            return 0;
        }
    #endif

    #ifdef SLAVE 
        if(slave_setup(_id))
        {
            return 1;
        }
        else 
        {
            return 0;
        }
    #endif
}

void RS485::updateFrame(uint8_t id, uint8_t function, uint16_t mode, 
                        uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5)
{
    _fr[0] = id;
    _fr[1] = function;
    _fr[2] = mode;
    _fr[3] = byte1;
    _fr[4] = byte2;
    _fr[5] = byte3;
    _fr[6] = byte4;
    _fr[7] = byte5;
    uint16_t CRC_val = CRC16(_fr);
    _fr[8] = CRC_val >> 8;
    _fr[9] = CRC_val & 0xFF;
    sendData(_fr);
    delayMicroseconds(_T3_5);
}

void RS485::sendData(uint8_t frame[])
{
    digitalWrite(EN_pin, 1);
    for(int i = 0; i < ELE_FRAME; i++)
    {
      (*_serial).write(frame[i]);  
      delayMicroseconds(_T1_5);
    }
    (*_serial).flush();    
    digitalWrite(EN_pin, 0);
}

void RS485::getData()
{
    digitalWrite(EN_pin, LOW);
    for(int i = 0; i < ELE_FRAME; i++)
    {
        delayMicroseconds(_T1_5);
        receiveData[i] = (*_serial).read();  
    }
    digitalWrite(EN_pin, HIGH);
}

bool RS485::master_setup()
{
    updateFrame(SLAVE_ID
                , FR0
                , READ
                , 0x00
                , 0x00
                , 0x00
                , 0x00
                , 0x00);

    while((*_serial).available() <= 0)
    {
        if(HAL_GetTick() - _prev >= 500)
        {
            _prev = HAL_GetTick();
            sendData(_fr);
        }
        if(HAL_GetTick() >= 5000) 
            {
                break;
            }
    }
    getData();
    uint16_t CRC_val = (receiveData[8] << 8) | receiveData[9];
    if(CRC16(receiveData) == CRC_val)
        {
            _slave_response = 1;
        }
    else 
        {
            _slave_response = 0;
        }
    
    if(_slave_response == 0) 
    {
        return 0;
    }
    else return 1;
}

   
bool RS485::slave_setup(uint8_t deviceID)
{
    while((*_serial).available() <= 0); 
    getData();
    uint16_t CRC_val = (receiveData[8] << 8) | receiveData[9];
    if(CRC16(receiveData) == CRC_val)
        {
            if(receiveData[0] == deviceID)
                {
                    delayMicroseconds(_T3_5);
                    updateFrame(_id
                                , FR0
                                , READ
                                , 0x00
                                , 0x00
                                , 0x00
                                , 0x00
                                , 0x00);
                    delayMicroseconds(_T3_5);
                } 
            return 1;  
        }
        else 
        {
            return 0;
        }
}

uint8_t RS485::receive()
{
    if((*_serial).available())
    {
        while((*_serial).available() < 9);

        for(int i = 0; i < ELE_FRAME; i++)
        {
            delayMicroseconds(_T1_5);
            receiveData[i] = (*_serial).read();  
        }
        uint16_t crc_val = (receiveData[8] << 8) | receiveData[9];
        //#ifdef SLAVE
            if(crc_val == CRC16(receiveData))
            {
                // func => xử lý => cảm biến gửi lại.
                return receiveData[1];
            }
            else
            {
                return 0;
            }

    }
    else
    {
        return 0;
    }  
}

uint16_t RS485::CRC16(uint8_t message[])
{
    uint16_t crc = 0xffff;
    uint8_t lsb; 
    for(int i = 0; i < (sizeof(message)/sizeof(message[0]) - 2); i++)
    {
        crc ^= message[i];
        for(int j = 0; j < sizeof(message[i]); j++)
        {
            lsb = crc & 0x01; 
            crc >>= 1; 
            if(lsb == 1) 
            {
                crc ^= 0xA001;
            }
        }
    }
    return crc; 
}

