#include <Arduino.h>
#include "src/define.hpp"
#include "src/rs485/rs458.hpp"



HardwareSerial Serial2(PA3, PA2);

RS485 rs485(&Serial1, BAUDRATE, EN_pin);

struct system_variable
{    
    #ifdef MASTER
        uint8_t deviceID = 0x11; 
        bool slave_fault = 0;
        uint8_t step_func = 0;
        uint8_t last_step_func = 0;
        bool slave_sended = true;
        uint16_t write_dongco1 = 1000;
        uint16_t last_write_dongco1 = 1000;
        bool write_led = 0;
        bool last_write_led = 0;
    #endif

    #ifdef SLAVE
        uint8_t deviceID = 0x72; 
        double cambien1; // tach ra gui 2 byte vi du 23.72 -> byte 1 = 23 và byte 2 = 72 
        double cambien2; // tach ra gui 2 byte vi du 23.72 -> byte 1 = 23 và byte 2 = 72
        double cambien3; // tach ra gui 2 byte vi du 23.72 -> byte 1 = 23 và byte 2 = 72
        uint16_t dongco1 = 1000; // pwm 1000-2000
        bool led = 0;
    #endif
}_sys;


void setup()
{
    Serial2.begin(115200);
    
    #ifdef MASTER
        Serial2.println("Master device");
        if(rs485.init(_sys.deviceID)) 
        {
            Serial2.println("COMMUNICATION: Success");
            _sys.slave_fault = 0;
        }
        else
        {
            Serial2.println("COMMUNICATION: Error");
            _sys.slave_fault = 1;
        }
    #endif
    
    #ifdef SLAVE
            pinMode(PC13, OUTPUT);
            Serial2.println("Slave device");
            if(rs485.init(_sys.deviceID)) 
            {
                Serial2.println("COMMUNICATION: Success");
            }
            else
            {
                Serial2.println("COMMUNICATION: Error");
            }
    #endif
    

 

}

void loop()
{
    #ifdef MASTER
    if(_sys.slave_fault == 0)
    {
        Master_serial2_CMD(); // dk dong co + led;

            if ((_sys.step_func < TOTAL_FUNC) && (_sys.slave_sended == true))
            {
                switch (_sys.step_func)
                {
                case FUNC1:
                    rs485.updateFrame(  _sys.deviceID,
                                    FR1,
                                    READ,
                                    0x00,
                                    0x00,
                                    0x00,
                                    0x00,
                                    0x00);
                    break;
                case FUNC2:
                    rs485.updateFrame(  _sys.deviceID,
                                    FR2,
                                    READ,
                                    0x00,
                                    0x00,
                                    0x00,
                                    0x00,
                                    0x00);
                    break; 
                case FUNC3:
                    rs485.updateFrame(  _sys.deviceID,
                                    FR3,
                                    READ,
                                    0x00,
                                    0x00,
                                    0x00,
                                    0x00,
                                    0x00);
                    break;
                case FUNC4:
                    if (_sys.last_write_dongco1 != _sys.write_dongco1)
                    {
                        rs485.updateFrame(  _sys.deviceID,
                                            FR4,
                                            WRITE,
                                            convert_double_2_byte(_sys.write_dongco1,0),
                                            convert_double_2_byte(_sys.write_dongco1,1),
                                            0x00,
                                            0x00,
                                            0x00);
                        _sys.last_write_dongco1 = _sys.write_dongco1;
                    }
                    else
                    {
                        rs485.updateFrame(  _sys.deviceID,
                                            FR4,
                                            READ,
                                            0x00,
                                            0x00,
                                            0x00,
                                            0x00,
                                            0x00);
                    }
                    
                    break; 
                case FUNC5:
                    Serial2.println(_sys.write_led);
                    if (_sys.last_write_led != _sys.write_led)
                    {
                        
                        rs485.updateFrame(  _sys.deviceID,
                                            FR5,
                                            WRITE,
                                            _sys.write_led,
                                            0x00,
                                            0x00,
                                            0x00,
                                            0x00);
                        _sys.last_write_led = _sys.write_led ;                  
                    }
                    else
                    {
                        rs485.updateFrame(  _sys.deviceID,
                                            FR5,
                                            READ,
                                            0x00,
                                            0x00,
                                            0x00,
                                            0x00,
                                            0x00);
                    }
                    break;  
                                              
                default:
                    break;
                }
                _sys.step_func ++;            
            }
            else if ((_sys.step_func == TOTAL_FUNC) && (_sys.slave_sended == true)) {_sys.step_func = FUNC1;} // dong co 2 mode read + write.
        //}
        switch (rs485.receive())
        {
        case FR1:
            // Serial2.print("ADC1H: ");
            // Serial2.println(rs485.receiveData[3]);
            // Serial2.print("ADC1L: ");
            // Serial2.println(rs485.receiveData[4]);
            _sys.slave_sended = true;
            break;
        case FR2:
            // Serial2.print("ADC2H: ");
            // Serial2.println(rs485.receiveData[3]);
            // Serial2.print("ADC2L: ");
            // Serial2.println(rs485.receiveData[4]);
            _sys.slave_sended = true;
            break;
        case FR3:
            // Serial2.print("ADC3H: ");
            // Serial2.println(rs485.receiveData[3]);
            // Serial2.print("ADC3L: ");
            // Serial2.println(rs485.receiveData[4]);
            _sys.slave_sended = true;
            break;
        case FR4:
            Serial2.print("DCH: ");
            Serial2.println(rs485.receiveData[3]);
            Serial2.print("DCL: ");
            Serial2.println(rs485.receiveData[4]);
            _sys.slave_sended = true;
            break;
        case FR5:
            Serial2.print("LED1: ");
            Serial2.println(rs485.receiveData[3]);
            _sys.slave_sended = true;
            break;
        default:
            //Serial2.println(_sys.step_func);
            _sys.slave_sended = (_sys.step_func != TOTAL_FUNC) ? false : true;
            break;
        }
    }
    else 
    {
        Serial2.println("REBOOT needed!"); 
        delay(1000);
    }
    #endif

    #ifdef SLAVE
        Slave_cal_control_sys();
        switch (rs485.receive())
        {
        case FR1:
            rs485.updateFrame(  _sys.deviceID,
                            FR1,
                            READ,
                            convert_double_2_byte(_sys.cambien1, 0),
                            convert_double_2_byte(_sys.cambien1, 1),
                            0x00,
                            0x00,
                            0x00);
            break;
        case FR2:
            rs485.updateFrame(  _sys.deviceID,
                            FR2,
                            READ,
                            convert_double_2_byte(_sys.cambien2, 0),
                            convert_double_2_byte(_sys.cambien2, 1),
                            0x00,
                            0x00,
                            0x00);
            break; 
        case FR3:
            rs485.updateFrame(  _sys.deviceID,
                            FR3,
                            READ,
                            convert_double_2_byte(_sys.cambien3, 0),
                            convert_double_2_byte(_sys.cambien3, 1),
                            0x00,
                            0x00,
                            0x00);
            break;    
        case FR4:
            if (rs485.receiveData[2] == WRITE)
            {
                // set bien dong co
                _sys.dongco1 = rs485.receiveData[3];
                Serial2.println(_sys.dongco1);
            }
            
            rs485.updateFrame(  _sys.deviceID,
                            FR4,
                            rs485.receiveData[2],
                            convert_double_2_byte(_sys.dongco1, 0),
                            convert_double_2_byte(_sys.dongco1, 1),
                            0x00,
                            0x00,
                            0x00);
            break; 
        case FR5:
            if (rs485.receiveData[2] == WRITE)
            {
                // set bien led
                _sys.led = rs485.receiveData[3];
                Serial2.println(_sys.led);
            }
            rs485.updateFrame(  _sys.deviceID,
                            FR5,
                            READ,
                            convert_double_2_byte(_sys.led, 0),
                            0x00,
                            0x00,
                            0x00,
                            0x00);
            break;                        
        default:
            break;
        }
    #endif
}

void Master_serial2_CMD()
{
    #ifdef MASTER
    if (Serial2.available())
    {
        char cmd = Serial2.read();
        switch (cmd)
        {
        case '1':
            _sys.write_led = !_sys.write_led;
            break;
        case '2':
            _sys.write_dongco1++;
            break;
        default:
            break;
        }
    }
    #endif
}
// SLAVE CAL CONTROL
uint8_t AdcValueGenerate()
{   
    return random(0,255); 
}

void Slave_cal_control_sys()
{
    #ifdef SLAVE
        _sys.cambien1 = AdcValueGenerate() + (AdcValueGenerate()/100);
        for(int i = 0; i < 200; i++)
        {
            if(i == 0)
            {
                _sys.cambien2 = AdcValueGenerate() + (AdcValueGenerate()/100);
            } 
            else
            {
                _sys.cambien2 = (_sys.cambien2 + (AdcValueGenerate() + (AdcValueGenerate()/100)))/2;
            }       
        }
        
        for(int i = 0; i < 100; i++)
        {
            if(i == 0)
            {
                _sys.cambien3 = AdcValueGenerate() + (AdcValueGenerate()/100);
            } 
            else
            {
                _sys.cambien3 = (_sys.cambien3 + (AdcValueGenerate() + (AdcValueGenerate()/100)))/2;
            }       
        }

        // nhan gia tri dong co va dieuu khien + LED
        digitalWrite(PC13, _sys.led);

    #endif
}

uint8_t convert_double_2_byte(double value, bool type)
{
    uint8_t byteH = (uint8_t)value; // 243.12 -> 
    uint8_t byteL = (uint8_t)((value - byteH) * 100) ;
    return (type == false) ? byteH : byteL;
}