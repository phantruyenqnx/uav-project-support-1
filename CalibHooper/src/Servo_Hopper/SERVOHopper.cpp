#include "SERVOHopper.hpp"
#include <Servo.h>
Servo Hopper;

SERVOHopperClass::SERVOHopperClass(uint32_t pin, uint32_t value)
{
    this->pin = pin;
    this->startPWM = value;
}

void SERVOHopperClass::init()
{
    Hopper.attach(PINHOPPER);
    Hopper.writeMicroseconds(startPWM);
    delay(1000);
}


void SERVOHopperClass::calib()
{

}
// servo 750 - 1500 - 2250
void SERVOHopperClass::setOpenPercent(uint8_t percent)
{
    uint16_t pwm = map(percent, 0, 100, 750, 2250);
    if(pwm < 750) pwm = 750;
    if(pwm > 2250) pwm = 2250;
    Hopper.writeMicroseconds(pwm);
}

uint16_t SERVOHopperClass::readPWM()
{
    return Hopper.readMicroseconds();
}

uint16_t SERVOHopperClass::readPercent()
{
    uint16_t percent = map(Hopper.readMicroseconds(), 750, 2250, 0, 100);
    return percent;
}