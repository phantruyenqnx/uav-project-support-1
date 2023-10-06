#ifndef BLDC_SPINNER_H
#define BLDC_SPINNER_H
#include <Arduino.h>
#include <Servo.h>

#define PWM_MIN         1000
#define PWM_MAX         2000
#define PINSPINNER      PA6

class BLDCSpinnerClass 
{  
  private:
    uint32_t pin = 0;
    uint32_t startPWM = 0;
    unsigned long timeHal = 0;
  public:
    BLDCSpinnerClass(uint32_t pin, uint32_t value);
    void init();
    void setPWMstr(uint16_t pwm, String *str, unsigned long *_time);
    void setRPMstr(uint16_t rpm, String *str, unsigned long *_time);
    void setRPM(uint16_t rpm, unsigned long *_time);
    uint16_t readPWM();
    uint16_t readRPM();
};
#endif