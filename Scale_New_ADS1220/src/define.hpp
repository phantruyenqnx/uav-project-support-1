#include "C:\\Users\\Admin\\AppData\\Local\\Arduino15\\packages\\STMicroelectronics\\hardware\\stm32\\2.3.0\\cores\\arduino\\wiring_digital.c"
#include <ADS1220_WE.h>
#include <SPI.h>
#include <SimpleKalmanFilter.h>
#include <EEPROM.h>
#include "libmodbusSlave/libmodbusSlave.h"

#define ELE(x) (sizeof(x)/sizeof(x[0]))

#define ADDR_OFFSET0                170
#define ADDR_OFFSET1                180
#define ADDR_OFFSET2                190
#define ADDR_OFFSET3                200
#define ADDR_HESO_M                 210
#define ADDR_HESO_N                 160
#define ADDR_KG_REF                 150
// chieu tu tren xuong theo huong lap cua mạch cân
// ------------------------------------------------
/*
mạch
        stm32
ads2            ads3
        rs485
ads1            ads4

cơ khí
cell 2          cell 3

cell 1          cell 4
*/
#define ADS_CS_PIN_1    PA1
#define ADS_DRDY_PIN_1  PB5
ADS1220_WE ads1 = ADS1220_WE(ADS_CS_PIN_1, ADS_DRDY_PIN_1);

#define ADS_CS_PIN_2    PA4
#define ADS_DRDY_PIN_2  PB6
ADS1220_WE ads2 = ADS1220_WE(ADS_CS_PIN_2, ADS_DRDY_PIN_2);

#define ADS_CS_PIN_3    PB1
#define ADS_DRDY_PIN_3  PB14
ADS1220_WE ads3 = ADS1220_WE(ADS_CS_PIN_3, ADS_DRDY_PIN_3);

#define ADS_CS_PIN_4    PB0
#define ADS_DRDY_PIN_4  PB15
ADS1220_WE ads4 = ADS1220_WE(ADS_CS_PIN_4, ADS_DRDY_PIN_4);

/* Create your ADS1220 object */

/* Alternatively you can also pass the SPI object as reference */
// ADS1220_WE ads = ADS1220_WE(&SPI, ADS1220_CS_PIN, ADS1220_DRDY_PIN);


HardwareSerial serDebug (USART2);
HardwareSerial serScale (USART1);

SimpleKalmanFilter kalmanFilter(2, 2, 0.15);

#define COUNT 4

struct tareVariable
{
    bool debugEnabled = false;
    long offset[4];
}tareVar;

struct eeproomVariable
{
    double a; // value at 0kg
    double b; // value at 20kg
    double m; // y = mx + n 
    double n;
    double kg_ref;
}eeproomVar;


//////////////// registers of your slave ///////////////////
enum 
{     
    // just add or remove registers and your good to go...
    // The first register starts at address 0
    BYTE1,     
    BYTE2,
    CALIB,      
    HOLDING_REGS_SIZE // leave this one
    // total number of registers for function 3 and 16 share the same register array
    // i.e. the same address space
};

unsigned int holdingRegs[HOLDING_REGS_SIZE]; // function 3 and 16 register array
////////////////////////////////////////////////////////////

enum FUNC_SPREAD
{
    FUNCTION_SCALE_CALIB_TARE       = 0x20,
    FUNCTION_SCALE_CALIB_TARE_DONE  = 0x25,
    FUNCTION_SCALE_CALIB_EMPTY_TANK = 0x30,
    FUNCTION_SCALE_CALIB_EMPTY_DONE = 0x35,
    FUNCTION_SCALE_CALIB_20KG       = 0x50,
    FUNCTION_SCALE_CALIB_20KG_DONE  = 0x60,        
};
uint8_t _calibtare = 0;
uint8_t calibzero = 0;
uint8_t _calib20kg = 0;
uint16_t count = 0;
double sum_last = 0.00;
double _temp = 0.00;
bool _tareSuccessful = false; 
unsigned long _tareStartTime  = 0;