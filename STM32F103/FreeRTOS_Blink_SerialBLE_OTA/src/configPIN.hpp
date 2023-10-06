#ifndef CONFIGPIN_H
#define CONFIGPIN_H
#endif
//RS485
#define CTR485_PIN  PA8
//I2C
#define SCL_PIN     PB8
#define SDA_PIN     PB9

//DIGITAL READ STATUS ALARM
#define READALARM1_PIN  PA4
#define READALARM2_PIN  PA5

//SERVO
#define CW1         PB12    //pin control rotate
#define CCW1        PB13    //pin control rotate
#define PWM1        PA0     //pin control PWM
#define CW2         PB14    //pin control rotate
#define CCW2        PB15    //pin control rotate
#define PWM2        PA1     //pin control PWM

//SPINNER CONTROL
#define SP1_PIN     PA6
#define SP1_PIN     PA7

//MOSET CIRCUIT
#define CMOS1_PIN   PB0
#define CMOS2_PIN   PB1

//COMMUNICATION


#define RS485   USART2
#define dbugBLE USART3

/*COMMAND*/
/*
1. UART DEBUG   USART3
    TX - PB10
    RX - PB9
2. PWM OUTOUT
    PWM1 - PA0
    PWM2 - PA1
*/

