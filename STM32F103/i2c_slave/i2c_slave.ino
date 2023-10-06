#include <Wire.h>

#define I2C_ADDR  0x20

HardwareSerial dbugBLE(USART3);

void setup()
{
    Wire.setSDA(PB9);
    Wire.setSCL(PB8);
  Wire.begin(I2C_ADDR);         // join i2c bus with address #4
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent); // register event
  dbugBLE.begin(115200);           // start serial for output
  dbugBLE.println("Hello i'm slave");
}

void loop()
{
  //empty loop
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read();     // receive byte as a character
    dbugBLE.print(c);          // print the character
  }
  int x = Wire.read();        // receive byte as an integer
  dbugBLE.println(x);          // print the integer
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent()
{
  Wire.write("hello\n");  // respond with message of 6 bytes
                          // as expected by master
}
