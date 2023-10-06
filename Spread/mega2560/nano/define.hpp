#include <curveFitting.h>
#include "HX711-multi.h" 
#include <SimpleKalmanFilter.h>
#include <EEPROM.h>

SimpleKalmanFilter kalmanFilter(2, 2, 0.001);

#define CLK                         13 // clock pin to the load cell amp 
#define DOUT1                       A0 // data pin to the first lca 
#define DOUT2                       A1 // data pin to the second lca 
#define DOUT3                       A2 // data pin to the third lca 
#define DOUT4                       A3 // data pin to the fourth lca 

#define ADDR_LAST_OFFSET            224
#define ADDR_OFFSET0                200
#define ADDR_OFFSET1                204
#define ADDR_OFFSET2                208
#define ADDR_OFFSET3                212
#define ADDR_HESO1                  216
#define ADDR_HESO2                  220

#define BOOT_MESSAGE                "CALIBRATING OFFSET ....." 
#define TARE_TIMEOUT_SECONDS        5

// define delegate class HX711-multi 
byte DOUTS[4] = {DOUT1, DOUT2, DOUT3, DOUT4}; 
#define CHANNEL_COUNT               sizeof(DOUTS)/sizeof(byte) 
long int  results[CHANNEL_COUNT];
HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK);

// define variable use in main code
#define TIMES_CAL                   30 
#define TIMES_READ                  2  
#define DEBUG_SERIAL                true

long  int offset[4]; // = {1324503, 242134, 123451234, 23123};
unsigned long combine = 0;
byte split[4];
double first_ket_qua = 0.00;
double last_ket_qua = 411.78;
double he_so_1_A = 0.00;
double he_so_2_A = 0.00;
double scaleValue = 0.00;
int set = 0;
int check = 0;
double temp_max = 10.18;
double counter_1;
int num_first = 0;
int num_last = 0;
double sum_last;
double sum_exact[TIMES_CAL];
double last_offset;

uint8_t count = 0;

struct setupStruct
{
    bool checkEeprom = false;
}setupStruct;


///  communication variable
enum FUNCTION
{
    FUNCTION_VALUE = 0x15,
    FUNCTION_CALIB_TARE  = 0x20,
    FUNCTION_CALIB_EMPTY_TANK = 0x30,
    FUNCTION_CALIB_20KG = 0x50,
};

uint8_t frValue[8] =            {0x11, 0x15, 0x30, 0x00, 0x00, 0x00, 0x00, 0x92};
uint8_t frCalibTare[8] =        {0x11, 0x20, 0x30, 0x00, 0x13, 0xCC, 0x11, 0x92};
uint8_t frCalibEmptyTank[8] =   {0x11, 0x30, 0x30, 0x00, 0xAA, 0xDD, 0x07, 0x92};
uint8_t frCalib20Kg[8] =        {0x11, 0x50, 0x30, 0x00, 0x32, 0x11, 0x22, 0x92};
uint8_t frRec[8] = {0};
uint8_t selectFunc = FUNCTION_VALUE ;
uint8_t stepRec = 0;
bool checkCRC = false;
unsigned long timeRec = 0;
unsigned long timeSend = 0;
///  communication variable