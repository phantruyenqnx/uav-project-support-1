#ifndef MODULE_SCALE_H
#define MODULE_SCALE_H
#include <Arduino.h>
class ModuleScaleClass 
{  
  private:
    uint32_t baudrate;
    uint8_t stepRead = 0;
    uint8_t dtFrame[8] = {0};
	uint8_t byteRecTemp = 0;
	bool checkFr = false;
    unsigned long timeRead = 0;

	enum FUNCTION
	{
		FUNCTION_VALUE = 0x15,
		FUNCTION_CALIB_TARE  = 0x20,
		FUNCTION_CALIB_EMPTY_TANK = 0x30,
		FUNCTION_CALIB_20KG = 0x50,
	};

    uint16_t MODBUS_CRC16( const unsigned char *buf, unsigned int len );
    void writeByte(uint8_t _cmd);
  public:
	uint8_t frCalibTare[8] =        {0x11, 0x20, 0x30, 0x00, 0x13, 0xCC, 0x11, 0x92};
	uint8_t frCalibEmptyTank[8] =   {0x11, 0x30, 0x30, 0x00, 0xAA, 0xDD, 0x07, 0x92};
	uint8_t frCalib20Kg[8] =        {0x11, 0x50, 0x30, 0x00, 0x32, 0x11, 0x22, 0x92};

    ModuleScaleClass(uint32_t baudrate);
    void init();
	void readScale();
    void readScaleValue(String *_str);
    
	void readCalibTare(String *_str);
	void readCalibEmptyTank(String *_str);
	void readCalib20Kg(String *_str);

	void setCalibTare(uint8_t cmd);
	void setCalibEmptyTank(uint8_t cmd);
	void setCalib20Kg(uint8_t cmd);

	void checkConnect(char *_str);
};
#endif