#ifndef SERVO_HOPPER_H
#define SERVO_HOPPER_H
#include <Arduino.h>

#define PWM_MIN         1050
#define PWM_MIDDLE      1500
#define PWM_MAX         1950
#define PINHOPPER       PB7

class SERVOHopperClass 
{  
  private:
    uint32_t pin = 0;
    uint32_t startPWM = 0;
  public:
    SERVOHopperClass(uint32_t pin, uint32_t value);
    void init();
    void calib();
    void setOpenPercent(uint8_t percent);
    uint16_t readPWM();
    uint16_t readPercent();
};
#endif