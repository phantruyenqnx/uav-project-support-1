#include "C:\\Users\\Admin\\AppData\\Local\\Arduino15\\packages\\STMicroelectronics\\hardware\\stm32\\2.3.0\\cores\\arduino\\wiring_digital.c"
//#include "src/Scale/Scale.hpp"
#include "src/libmodbusMaster/libmodbusMaster.h"

#include "src/TT_modbusMC_Slave/TT_modbus_slave.hpp"

#define LED PB12

  #define FIRST_BYTE_MASK   		0b00000000000000000000000011111111
  #define SECOND_BYTE_MASK  		0b00000000000000001111111100000000
  #define THIRD_BYTE_MASK   		0b00000000111111110000000000000000
  #define FOURTH_BYTE_MASK        0b11111111000000000000000000000000


// ==================== VARIABLE SYSTEM  ===========//

          HardwareSerial SerialMain (USART1);
          #define SerialMain_Pincontrol485 PA8
          #define baud 1000000
          #define SLAVEID 0x51 
          #define ELE(x) (sizeof(x)/sizeof(x[0]))

          #define FR0 0x15 // check_connet with module Spread
          #define FR1 0x25 // scale
          #define FR1_CALIB_TARE  0x26
          #define FR1_CALIB_ZERO  0x36
          #define FR2 0x35 // spinner
          #define FR3 0x45 // hopper
          #define FR4 0x57 // sensor level
          #define FR4_RUN 0x75
          #define FR4_STOP 0x77
          #define FR5 0x65 // led
          #define READ 0x17
          #define WRITE 0x20

          struct system_variable
          {
                uint8_t deviceID = 0x72; 
                double cambien1; // tach ra gui 2 byte vi du 23.72 -> byte 1 = 23 và byte 2 = 72 
                double cambien2; // tach ra gui 2 byte vi du 23.72 -> byte 1 = 23 và byte 2 = 72
                double cambien3; // tach ra gui 2 byte vi du 23.72 -> byte 1 = 23 và byte 2 = 72
                uint16_t dongco1 = 1000; // pwm 1000-2000
                bool led = 0;
          }_sys;

// ==================== FUNCTION -> SCALE  ===========//
          HardwareSerial SerialScale (USART6);
          #define timeout 100
          #define polling 50 // the scan rate : toc do gui frame
          #define retry_count 50
          // used to toggle the receive/transmit pin on the driver
          #define scale_TxEnablePin PB8 

          enum
          {
            SCALEVALUE,
            CALIB,
            TOTAL_NO_OF_PACKETS // leave this last entry
          };

          enum
          {
            BYTE1,
            BYTE2,
            FUNC,
            TOTAL_NO_OF_REGISTERS // leave this last entry
          };

          enum FUNC_SPREAD
          {
            // // Setup
            // FUNC_SPREAD_SETUP_CHECKCONNECT  = 0x01,
            // FUNC_SPREAD_SETUP_DONE          = 0x02,
            // // Loop
            // // Calib hopper
            // FUNC_SPREAD_LOOP_CALIBHOPPER    = 0x10,
            // FUNC_SPREAD_LOOP_CALIBPERCENT   = 0x11,
            // FUNC_SPREAD_LOOP_CALIBERROR     = 0x12,
            // FUNC_SPREAD_LOOP_CALIBSUCCESS   = 0x13,
            // // Hopper set
            // FUNC_SPREAD_HOPPER_SET          = 0x14,
            // // Spinner set
            // FUNC_SPREAD_SPINNER_SET         = 0x15,
            // // Level set
            // FUNC_SPREAD_LEVEL_SET           = 0x16,
            // Scale
            FUNCTION_VALUE                  = 0x15,
            FUNCTION_SCALE_CALIB_TARE       = 0x20,
            FUNCTION_SCALE_CALIB_TARE_DONE  = 0x25,
            FUNCTION_SCALE_CALIB_EMPTY_TANK = 0x30,
            FUNCTION_SCALE_CALIB_EMPTY_DONE = 0x35,
            FUNCTION_SCALE_CALIB_20KG       = 0x50,
            FUNCTION_SCALE_CALIB_20KG_DONE  = 0x60,        
          };

          // Create an array of Packets to be configured
          Packet scale_packets[TOTAL_NO_OF_PACKETS];

          // Masters register array
          unsigned int scale_regs[TOTAL_NO_OF_REGISTERS];
          uint8_t scale_valueH;
          uint8_t scale_valueL;

// ==================== FUNCTION -> LEVEL SENSOR  ===========//

          struct sensLeveluse
          {
              String str = "SLv: ";
              String _haveVolume = "";
              bool last_control = true;
              bool control = true;
              uint8_t temp = 0;
          }sLv;
// ==================== FUNCTION -> SPINNER  ===========//

          struct spinner
          {
              unsigned long time1 = 0;
              uint16_t val = 250;
              uint16_t last_val = 0;
          }sPi;
// ==================== FUNCTION -> HOPPER  ===========//

          // struct hopper
          // {
          //     unsigned long time1 = 0;
          //     uint16_t val = 1000;
          //     uint16_t last_val = 0;
          // }hOp;