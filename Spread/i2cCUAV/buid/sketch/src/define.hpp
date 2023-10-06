#line 1 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\src\\define.hpp"
#include <Wire.h>
#include <C:\\Users\\Admin\\AppData\\Local\\Arduino15\\packages\\STMicroelectronics\\hardware\\stm32\\2.3.0\\cores\\arduino\\wiring_digital.c>
#define ELE(x) (sizeof(x)/sizeof(x[0]))

HardwareSerial spread485 (USART1);
HardwareSerial dbug(USART2);

#define BAUDRATE_SPREAD 115200
#define ADDRESS_I2CCUBE 0x75

#define FIRST_BYTE_MASK   		0b00000000000000000000000011111111
#define SECOND_BYTE_MASK  		0b00000000000000001111111100000000
#define THIRD_BYTE_MASK   		0b00000000111111110000000000000000
#define FOURTH_BYTE_MASK        0b11111111000000000000000000000000

enum TTSPREAD_ID {
    TTSPREAD_ID_CHECKCONNECT = 0x71,
    TTSPREAD_ID_CHECKCONNECT_PART = 0x72,
    TTSPREAD_ID_READ_SPINNER = 0x73,
    TTSPREAD_ID_READ_HOPPER = 0x74,
    TTSPREAD_ID_READ_LEVEL = 0x75,
    TTSPREAD_ID_READ_SCALE = 0x76,

    TTSPREAD_ID_SET_SPINNER = 0x77,
    TTSPREAD_ID_SET_HOPPER  = 0x78,
    TTSPREAD_ID_SET_LEVEL = 0x79,
    TTSPREAD_ID_SET_SCALE = 0x80,
};

enum FUNC_SPREAD
{
    // Setup
    FUNC_SPREAD_SETUP_CHECKCONNECT = 0x01,
    FUNC_SPREAD_SETUP_DONE         = 0x02,
    // Loop
    // Calib hopper
    FUNC_SPREAD_LOOP_CALIBHOPPER   = 0x10,
    FUNC_SPREAD_LOOP_CALIBPERCENT  = 0x11,
    FUNC_SPREAD_LOOP_CALIBERROR    = 0x12,
    FUNC_SPREAD_LOOP_CALIBSUCCESS  = 0x13,
    // Hopper set
    FUNC_SPREAD_HOPPER_SET         = 0x14,
    // Spinner set
    FUNC_SPREAD_SPINNER_SET        = 0x15,
    // Level set
    FUNC_SPREAD_LEVEL_SET          = 0x16,   
    // Scale
    FUNCTION_VALUE                  = 0x15,
    FUNCTION_SCALE_CALIB_TARE       = 0x20,
    FUNCTION_SCALE_CALIB_TARE_DONE  = 0x25,
    FUNCTION_SCALE_CALIB_EMPTY_TANK = 0x30,
    FUNCTION_SCALE_CALIB_EMPTY_DONE = 0x35,
    FUNCTION_SCALE_CALIB_20KG       = 0x50,
    FUNCTION_SCALE_CALIB_20KG_DONE  = 0x60,        
};



struct i2cCube
{
    // Spinner
    uint16_t spinnerRead;
    uint16_t spinnerSet;
    // Hopper
    uint8_t  hopperRead;
    uint8_t  hopperSet;
    // Level 
    uint8_t  levelRead;
    uint8_t  levelSet;
    // Scale
    uint16_t scaleRead;
    uint8_t  scaleSet;
}i2cCube;

struct servoHopperUse
{
    uint8_t valueSet = 0;
    uint8_t value = 0;
    unsigned long timeValue = 0;
    bool handleRec = false;
    bool hanldeCalibPercent = false;
    uint8_t calperval= 0;
    uint8_t miniFunc = 0;
    uint8_t stepRec = 0;
    uint8_t func = 0 ;
    uint8_t dataFrame[20] = {};
    uint8_t FrameCalibSuccess[42] = {0};
    unsigned long timeDelayRec = 0;
    unsigned long timeTemp = 0;
    uint8_t checkConnect[8] = {0x1A, 0x01, 0x02, 0x75, 0x1F, 0x1A, 0x30, 0x44};
    uint8_t setupDone[8] = {0x1A, 0x02, 0x02, 0x3F, 0x11, 0x12, 0x30, 0x14};
    uint8_t calHop[8] = {0x1A, 0x10, 0x02, 0x76, 0x25, 0x1E, 0x30, 0x40};
    uint8_t calPer[8] = {0x1A, 0x11, 0x01, 0x00, 0x01, 0x00, 0x00, 0x4F};
    uint8_t calSuccess[8]   = {0x1A, 0x13, 0x26, 0x75, 0x60, 0x12, 0x34, 0x22};
    uint8_t frValue[8] = {0x1A, 0x20, 0x26, 0x75, 0x60, 0x12, 0x34, 0x22}; 
    uint8_t setHopper[8] = {0x1A, 0x14, 0x02, 0x00, 0x10, 0xF1, 0xA6, 0xC7};

    uint8_t perHopperCalib[3] = {25, 40, 90};
    double valHopperCalib[3] = {0.0246, 0.0836, 0.456};
}sHp;

struct Spread485
{
    bool levelSetTest = false;
    bool setupDone = false;
    String readRaw = "" ;
    uint8_t hopperRaw = 0;
    uint16_t spinnerRaw = 0;
    bool  levelRaw = false;
    uint16_t scaleRaw = 0; 
    uint16_t spinSetVal = 0;
    uint8_t spinSetFr[8]  = {0x1A, 0x15, 0x02, 0x00, 0x00, 0x41, 0xA6, 0xB7};
    uint8_t levelSetFr[8] = {0x1A, 0x16, 0x02, 0x00, 0x11, 0x35, 0x26, 0x37};
}spreadVal;

struct RS485ScaleUse
{
    uint8_t frValue[8] =            {0x1A, 0x15, 0x30, 0x00, 0x00, 0x00, 0x00, 0x92};
    uint8_t frCalibTare[8] =        {0x1A, 0x20, 0x30, 0x00, 0x13, 0xCC, 0x11, 0x92};
    uint8_t frCalibEmptyTank[8] =   {0x1A, 0x30, 0x30, 0x00, 0xAA, 0xDD, 0x07, 0x92};
    uint8_t frCalib20Kg[8] =        {0x1A, 0x50, 0x30, 0x00, 0x32, 0x11, 0x22, 0x92};    
}scaleVar;

uint8_t recFrameRaw[8] = {0};
unsigned long timeI2C = 0;
uint16_t spinnerSet1 = 0;

uint8_t id = 0;
bool checkconnect = false;
uint16_t dataRec = 0;
uint8_t stepI2C = 0;
