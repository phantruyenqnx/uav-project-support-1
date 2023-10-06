#include <Wire.h>

#define I2C_ADDR  0x20

HardwareSerial dbugBLE(USART3);

uint8_t x = 0;

void setup()
{
  Wire.setSDA(PB9);
  Wire.setSCL(PB8);
  Wire.begin();        // join i2c bus (address optional for master)
  dbugBLE.begin(115200);  // start serial for output
  dbugBLE.println("Hello i'm master");
}

void loop()
{
  Wire.requestFrom(I2C_ADDR, 6);  // request 6 bytes from slave device

  while(Wire.available())         // slave may send less than requested
  {
    char c = Wire.read();         // receive a byte as character
    dbugBLE.print(c);              // print the character
  }

  delay(10);

  Wire.beginTransmission(I2C_ADDR); // transmit to device
  //Wire.write("x is ");              // sends five bytes
  //Wire.write(x);                    // sends one byte
  Wire.write("a");
  Wire.write("a");
  Wire.write("a");
  Wire.write("a");       
  Wire.endTransmission();           // stop transmitting
  //x++;

  delay(500);
}


/* Example pinmap for Bluepill I2Cs (by Testato)

 I2C-1 standard pins: PB7(sda) PB6(scl)
 Use it by "Wire" without pin declaration
  Wire.begin();

 I2C-1 alternative pins: PB9(sda) PB8(scl)
 Remap the first I2C before call begin()
  Wire.setSDA(PB9);
  Wire.setSCL(PB8);
  Wire.begin();

 I2C-2: PB11(sda) PB10(scl)
 Remap the second I2C before call begin()
  Wire.setSDA(PB11);
  Wire.setSCL(PB10);
  Wire.begin();

 If you want to use the two I2Cs simultaneously, create a new instance for the second I2C
  TwoWire Wire2(PB11,PB10);
  Wire2.begin();
 
*/


// #include <Wire.h>

// HardwareSerial dbugBLE(USART3);

// void setup() {

//   dbugBLE.begin(115200);
//   Wire.begin();
//   dbugBLE.println("\nI2C Scanner");
// }


// void loop() {
//   uint8_t error, address;
//   int nDevices;

//   dbugBLE.println("Scanning...");

//   nDevices = 0;
//   for(address = 1; address < 127; address++) {
//     // The i2c_scanner uses the return value of
//     // the Write.endTransmisstion to see if
//     // a device did acknowledge to the address.

//     Wire.beginTransmission(address);
//     error = Wire.endTransmission();
    
//     if (error == 0) {
//       dbugBLE.print("I2C device found at address 0x");
//       if (address < 16) 
//         dbugBLE.print("0");
//       dbugBLE.println(address, HEX);

//       nDevices++;
//     }
//     else if (error == 4) {
//       dbugBLE.print("Unknown error at address 0x");
//       if (address < 16) 
//         dbugBLE.print("0");
//       dbugBLE.println(address, HEX);
//     }    
//   }
//   if (nDevices == 0)
//     dbugBLE.println("No I2C devices found");
//   else
//     dbugBLE.println("done");

//   delay(5000);           // wait 5 seconds for next scan
// }
