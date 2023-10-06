#include "./src/Servo_Hopper/SERVOHopper.hpp"
#include "./src/scale/Scale.hpp"
#include "./src/Spinner/BLDCSpinner.hpp"
#include "./src/SensLevel/SensLevel.hpp"
#include <curveFitting.h>
#include <EEPROM.h>
HardwareSerial rs485(USART1);
HardwareSerial dbug(USART2);
#define RS485_Pin   PA8
//System
struct System
{
    unsigned long timer=0;
    uint8_t stepCalib=10;
    double lastWeight = 0.0;
    double coefCalib[2];
    unsigned long timePrint = 0;
    bool checkStatus = true;
    bool checkScale = true;
    unsigned long timeDelay=0;
}_sys;
//RS485
struct Receive_485
{
    String Recei_Data="";
    uint8_t dataRecei[8]={0};
    uint8_t stepRecei = 0;
}_Re485;
//servo calib
struct Servo_Hopper{
    // double pwmCalib[19] = {10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 
    //                         65, 70, 75, 80, 85, 90, 95, 100};
    double pwmCalib[19] = {10, 20, 30, 40, 50, 60, 
                             70, 80, 90, 100};
    uint64_t servo_Mode=0;
    uint8_t numCalib=0;
}_servoHop;

//calib
struct Calib_Hopper
{
    double weightCalib[19];
}_calibHop;
//eeprom
struct EEPROM
{
    uint8_t numValue=0;
    uint8_t eepAddress=100;
}_eeprom;

//sender 485
struct send485
{
    uint8_t senderValue=0;
}_send485;


//scale
struct Scale
{
    double scaleValue=0.0;
    //double scaleRecei = 20;
    String scaleVal="";

    //calib tare
    String calibTareVal = "";
    //calib empty
    String calibEmptyVal = "";
    //calib 20kg
    String calib20kgVal = "";
    //select mode calib
    uint8_t modeCalib=0;
    //time read
    unsigned long timeRead = 0;
    


}_scl;

//send data
enum FUNCTION{
    //scale
    FUNCTION_VALUE = 0x15,
    FUNCTION_CALIBTARE = 0x20,
    FUNCTION_CALIBEMPTY = 0x30,
    FUNCTION_CALIB20KG = 0x50,
    

};

//rs485
struct MASTER485
{
    String dataRecei="";
    uint8_t checkData=0;
}_recei485;

//time read

struct FRAMEDATA
{
    unsigned long timeRead = 0;
    bool chekcFrame = false;
}_frameData;

//Spinner

struct SPINNER
{
    unsigned long timeSet = 0;
    unsigned long timSetup = 0;
    uint16_t spinSetVal = 0;
    uint16_t timeSetupLimit = 12000;
    uint8_t stepSetup = 0;

}_spin;

//sens Level
struct SENLEVEL
{
    String volumeStatus = "";
    uint8_t sLvpercent = 0;
    bool setupMotor = true;
}_sLv;


//test spinner
struct TERSTSPINNER
{
    uint8_t stepSetup =0;
    uint8_t count =0;
}_print;







