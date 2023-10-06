#include "Scale.hpp"

HardwareSerial SerialScale (USART6);

ModuleScaleClass::ModuleScaleClass(uint32_t baudrate) {
    this->baudrate = baudrate;
    init();
}

void ModuleScaleClass::init() {
    SerialScale.begin(baudrate);
    pinMode(PB8, OUTPUT);
    //pinMode(PA4, OUTPUT);
    digitalWriteFast(digitalPinToPinName(PB8),LOW);
    //SerialScale.setTimeout(150);
}

void ModuleScaleClass::writeByte(uint8_t _cmd)
{
    digitalWriteFast(digitalPinToPinName(PB8),HIGH);
    SerialScale.write(_cmd);
    SerialScale.flush();
    digitalWriteFast(digitalPinToPinName(PB8),LOW);

}
void ModuleScaleClass::checkConnect(char *_str)
{
    if (SerialScale.available() > 0)
    {//*_str = SerialScale.read();
        if (SerialScale.peek() == 0x11)
        {
            *_str = SerialScale.read();
        }        
        else
        {
            byteRecTemp = SerialScale.read();
        }
    }
}
void ModuleScaleClass::readScale()
{
    switch (stepRead) // s20.32*HL
    {
    case 0:
        if (SerialScale.available() > 0)
        {
            if (SerialScale.peek() == 0x11)
            {
                timeRead = HAL_GetTick();
                stepRead = 1;
            }
            else
            {
                byteRecTemp = SerialScale.read();
            }
        }
        break;
    case 1:
        if ((unsigned long)(HAL_GetTick() - timeRead) > 10)
        {
            SerialScale.readBytes(dtFrame, 8);
            uint16_t crcCheck = MODBUS_CRC16(dtFrame , 8);
            uint16_t crcRec = (uint16_t)(SerialScale.read() << 8) 
                            | (uint16_t) SerialScale.read();
            checkFr = (crcCheck == crcRec) ? true : false ; 
            stepRead = 0;
        }
        break;
    }
}
void ModuleScaleClass::readScaleValue(String *_str)
{
    if (checkFr && dtFrame[1] == FUNCTION_VALUE)
    {
        *_str = (String)dtFrame[3] + (String)dtFrame[4] + (String)dtFrame[5] + (String)dtFrame[6];
        checkFr = false;
    }
}

void ModuleScaleClass::readCalibTare(String *_str)
{
    if (checkFr && dtFrame[1] == FUNCTION_CALIB_TARE)
    {
        *_str = (String)dtFrame[3];
        checkFr = false;
    }
}

void ModuleScaleClass::readCalibEmptyTank(String *_str)
{
    if (checkFr && dtFrame[1] == FUNCTION_CALIB_EMPTY_TANK)
    {
        *_str = (String)dtFrame[3];
        checkFr = false;
    }
}

void ModuleScaleClass::readCalib20Kg(String *_str)
{
    if (checkFr && dtFrame[1] == FUNCTION_CALIB_20KG)
    {
        *_str = (String)dtFrame[3];
        checkFr = false;
    }
}

void ModuleScaleClass::setCalibTare(uint8_t cmd)
{
    frCalibTare[1] = cmd;
    for (uint8_t w = 0; w < 8; w++)
    {
        writeByte(frCalibTare[w]);
    }
    writeByte((uint8_t)((MODBUS_CRC16(frCalibTare, 8) & 0xFF00) >> 8)); 
    writeByte((uint8_t)((MODBUS_CRC16(frCalibTare, 8) & 0x00FF) ));
}

void ModuleScaleClass::setCalibEmptyTank(uint8_t cmd)
{
    frCalibEmptyTank[1] = cmd;
    for (uint8_t w = 0; w < 8; w++)
    {
        writeByte(frCalibEmptyTank[w]);
    }
    writeByte((uint8_t)((MODBUS_CRC16(frCalibEmptyTank, 8) & 0xFF00) >> 8)); 
    writeByte((uint8_t)((MODBUS_CRC16(frCalibEmptyTank, 8) & 0x00FF) ));
}

void ModuleScaleClass::setCalib20Kg(uint8_t cmd)
{
    frCalib20Kg[1] = cmd;
    for (uint8_t w = 0; w < 8; w++)
    {
        writeByte(frCalib20Kg[w]);
    }
    writeByte((uint8_t)((MODBUS_CRC16(frCalib20Kg, 8) & 0xFF00) >> 8)); 
    writeByte((uint8_t)((MODBUS_CRC16(frCalib20Kg, 8) & 0x00FF) ));
}

uint16_t ModuleScaleClass::MODBUS_CRC16( const unsigned char *buf, unsigned int len )
{
	uint16_t crc = 0xFFFF;
	char i = 0;

	while(len--)
	{
		crc ^= (*buf++);

		for(i = 0; i < 8; i++)
		{
			if( crc & 1 )
			{
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
			{
				crc >>= 1;
			}
		}
	}

	return crc;
}

