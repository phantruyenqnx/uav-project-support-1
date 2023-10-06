#include "src/define.hpp"

HardwareSerial SerialBLE(dbugBLE);

void setup()
{
    SerialBLE.begin(115200);
    pinMode(PC13, OUTPUT);
}

void loop()
{
    if (SerialBLE.available() > 0)
    {
        SerialBLE.println((char)SerialBLE.read());
    }
    
    digitalWrite(PC13, HIGH);
    delay(200);
    digitalWrite(PC13, LOW);
    delay(200);
    SerialBLE.println("test");
}