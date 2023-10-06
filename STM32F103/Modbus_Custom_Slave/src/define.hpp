#include "C:\\Users\\Admin\\AppData\\Local\\Arduino15\\packages\\STMicroelectronics\\hardware\\stm32\\2.3.0\\cores\\arduino\\wiring_digital.c"
#include <STM32FreeRTOS.h>

HardwareSerial P_SerDebug(USART3);
#define P_MODBUS_PINDIR     PA8
#define P_MODBUS_BAUDRATE   115200
#define P_MODBUS_SLAVEID    10  // ID SCALE MODULE
HardwareSerial P_Modbus(USART1);

/****************************  CONFIG  ****************************/
// UART3 Tx PB10 Rx PB11
#define P_SerDebug_BAUDRATE 115200
// config main

// common define
/*
Project have ... tasks:                Piority                 Stacksize

*/
#define DEFAULT_STACKSIZE                            1

#define STEP1                       1
#define STEP2                       2
#define STEP3                       3
#define STEP4                       4
#define STEP5                       5
#define STEP6                       6
#define STEP7                       7
#define STEP8                       8
#define STEP9                       9
#define STEP0                       0

#define PRIO1                       1 // chỉ số càng thấp ưu tiên càng thấp
#define PRIO2                       2
#define PRIO3                       3
#define PRIO4                       4
#define PRIO5                       5
#define PRIO6                       6
#define PRIO7                       7
#define PRIO8                       8
#define PRIO9                       9
#define PRIO0                       0

#define INIT_BOOT_STRING            "Drone Module v1.0"

struct system
{
    uint8_t task;
    double value;
};