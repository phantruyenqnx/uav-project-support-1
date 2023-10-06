#include <src/SensLevel/SensLevel.hpp>
#include <BLDCSpinner.hpp>
#include <curveFitting.h>
UARTSensLevelClass sensLevel(2000000);

HardwareSerial RS485DebugCubeCom (USART1);
#define BAUDRATE_DEBUG 115200

unsigned long time1 = 0;
String _haveVolume = "";

void setup()
{
    RS485DebugCubeCom.begin(BAUDRATE_DEBUG);
    pinMode(PA8,OUTPUT);
    digitalWriteFast(digitalPinToPinName(PA8),HIGH);
    RS485DebugCubeCom.print(F_CPU);
    RS485DebugCubeCom.flush();
    digitalWriteFast(digitalPinToPinName(PA8),LOW);
    
    sensLevel.startMotor();
    digitalWriteFast(digitalPinToPinName(PA8),HIGH);
    RS485DebugCubeCom.print("F_CPU");
    RS485DebugCubeCom.flush();
    digitalWriteFast(digitalPinToPinName(PA8),LOW);
    time1 = HAL_GetTick();
}
   
void loop()
{
    sensLevel.readVolume(&_haveVolume);
    if ((unsigned long)(HAL_GetTick() - time1) > 1000)
    {
        if (_haveVolume == "1")
        {
            digitalWriteFast(digitalPinToPinName(PA8),HIGH);
            RS485DebugCubeCom.println("het giong");
            RS485DebugCubeCom.flush();
            digitalWriteFast(digitalPinToPinName(PA8),LOW);
        }
        else if(_haveVolume == "0")
        {
            digitalWriteFast(digitalPinToPinName(PA8),HIGH);
            RS485DebugCubeCom.println("con giong");
            RS485DebugCubeCom.flush();
            digitalWriteFast(digitalPinToPinName(PA8),LOW);
        }
        time1 = HAL_GetTick();
    }
    
    if (RS485DebugCubeCom.available() > 0)
    {
        digitalWriteFast(digitalPinToPinName(PA8),HIGH);
        uint8_t c = RS485DebugCubeCom.readDelay();
        RS485DebugCubeCom.write(c);
        RS485DebugCubeCom.flush();
        digitalWriteFast(digitalPinToPinName(PA8),LOW);
    }
}

/*
void function()
{
	switch (selectFunction)
	{
	case CALIB_HOPPER:
		hopperCalibFunc(); // proccessing ---- 
		break;
	case CALIB_SPINNER:
		spinnerCalibFunc(); // proccessing ---- 
		break;
	case TEST:
		testFunc(test.flowRateSet); // proccessing ---- 
		break;
	case MISSION:
		missionFunc(); // proccessing ---- 
		break;
	}
} 

*/