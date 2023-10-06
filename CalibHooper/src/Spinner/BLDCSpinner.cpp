#include "BLDCSpinner.hpp"

Servo Spinner; 

BLDCSpinnerClass::BLDCSpinnerClass(uint32_t pin, uint32_t value)
{
    this->pin = pin;
    this->startPWM = value;
}

void BLDCSpinnerClass::init()
{
    Spinner.attach(PINSPINNER);
    Spinner.writeMicroseconds(startPWM);
    delay(1000);
}

void BLDCSpinnerClass::setPWMstr(uint16_t pwm, String *str, unsigned long *_time)
{
    if(pwm<1000) pwm = 1000;
    if(pwm>2000) pwm = 2000;
    
    uint16_t pwm_old = Spinner.readMicroseconds();

    if (((unsigned long)(HAL_GetTick() - *_time)> 10) && (pwm_old != pwm))
    {
        if (pwm > pwm_old)
        {
            pwm_old += 10;
        }
        else if (pwm < pwm_old)
        {
            pwm_old -= 10;
        }
        else
        {
            pwm_old = pwm;
        }
        Spinner.writeMicroseconds(pwm_old); 
        *_time = HAL_GetTick();
        *str = (String)Spinner.readMicroseconds();
    }
}

void BLDCSpinnerClass::setRPMstr(uint16_t rpm, String *str, unsigned long *_time)
{
    uint16_t pwm = map(rpm, 300, 1300, 1100, 1950);
    setPWMstr(pwm, str, _time);
}

void BLDCSpinnerClass::setRPM(uint16_t rpm, unsigned long *_time)
{

    uint16_t pwm = map(rpm, 250, 1350, 1000, 2000);

    if(pwm<1000) pwm = 1000;
    if(pwm>2000) pwm = 2000;
    
    uint16_t pwm_old = Spinner.readMicroseconds();

    if (((unsigned long)(HAL_GetTick() - *_time)> 10) && (pwm_old != pwm))
    {
        if (pwm > pwm_old)
        {
            pwm_old += 10;
        }
        else if (pwm < pwm_old)
        {
            pwm_old -= 10;
        }
        else
        {
            pwm_old = pwm;
        }
        Spinner.writeMicroseconds(pwm_old); 
        *_time = HAL_GetTick();
    } 
}

uint16_t BLDCSpinnerClass::readPWM()
{
    return Spinner.readMicroseconds();
}

uint16_t BLDCSpinnerClass::readRPM()
{
    return map(Spinner.readMicroseconds(), 1000, 2000, 250, 1350);
}