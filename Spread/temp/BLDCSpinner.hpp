#ifndef BLDC_SPINNER_H
#define BLDC_SPINNER_H
#include <Arduino.h>
#include <Servo.h>

class BLDCSpinnerClass 
{  
  private:
    uint32_t baudrate;
    uint8_t byteSend = 0;
    bool _detect = false;
    uint8_t count = 0;
    uint8_t rec_temp = 0;
    uint8_t rec[24] = {};

    void sendCMD(uint8_t cmd[], uint8_t length);
  public:
    BLDCSpinnerClass(uint32_t baudrate);
    void init();
    void startMotor();
    void readVolume(String *str);
};
#endif