#ifndef UART_SENS_LEVEL_H
#define UART_SENS_LEVEL_H
#include <Arduino.h>
class UARTSensLevelClass 
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
    UARTSensLevelClass(uint32_t baudrate);
    void init();
    uint8_t startMotor(bool stop, uint8_t *percent);
    void readVolume(String *str);
    void startMotor2(bool stop);
};
#endif