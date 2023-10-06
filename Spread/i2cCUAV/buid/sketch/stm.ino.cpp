#include <Arduino.h>
#line 1 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
#include "src/define.hpp"

#line 3 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
void receiveEvent(int howMany);
#line 24 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
void requestEvent();
#line 70 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
void writeWord(uint16_t _data);
#line 80 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
void writeByte(uint8_t _data);
#line 86 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
void setup();
#line 130 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
void loop();
#line 214 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
uint8_t flowRateToHopperPercent(double flowRate);
#line 238 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
double coefA(double x1, double x2, double y1, double y2);
#line 243 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
double coefB(double x1, double y1, double a);
#line 248 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
void senDataFrameSpread485(uint8_t cmd[]);
#line 255 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
void recDataFrameSpread485();
#line 403 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
void debugWriteByte(uint8_t m);
#line 411 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
static uint16_t MODBUS_CRC16_v3( const unsigned char *buf, unsigned int len );
#line 3 "d:\\QNX\\QNX_DroneV1\\Module\\Spread\\i2cCUAV\\stm.ino"
void receiveEvent(int howMany)
{
	id = Wire.read();
    if (id == TTSPREAD_ID_SET_SPINNER)
    {
        dataRec = (uint16_t)Wire.read() << 8 | (uint16_t)Wire.read();
    }
    if (id == TTSPREAD_ID_SET_HOPPER)
    {
        dataRec = Wire.read();
    }
    if (id == TTSPREAD_ID_SET_LEVEL)
    {
        dataRec = Wire.read();
    }
    if (id == TTSPREAD_ID_SET_SCALE)
    {
        dataRec = Wire.read();
    }    
}

void requestEvent()
{
    switch (id)
    {
    case TTSPREAD_ID_CHECKCONNECT:
        writeByte(TTSPREAD_ID_CHECKCONNECT);
        break;
    case TTSPREAD_ID_CHECKCONNECT_PART:
        writeByte(TTSPREAD_ID_CHECKCONNECT_PART);
        checkconnect =  true;
        break;
    case TTSPREAD_ID_READ_SPINNER: // read
        writeWord(i2cCube.spinnerRead);
        break;   
    case TTSPREAD_ID_READ_HOPPER:
        writeByte(i2cCube.hopperRead);
        break;
    case TTSPREAD_ID_READ_LEVEL:
        writeByte(i2cCube.levelRead);
        break;  
    case TTSPREAD_ID_READ_SCALE:
        writeWord(i2cCube.scaleRead);
        break;                       
    case TTSPREAD_ID_SET_SPINNER: // set
        i2cCube.spinnerSet = dataRec;
        writeByte(TTSPREAD_ID_SET_SPINNER);
        dataRec = 0;
        break;          
    case TTSPREAD_ID_SET_HOPPER:
        i2cCube.hopperSet = dataRec;
        writeWord(TTSPREAD_ID_SET_HOPPER);
        dataRec = 0;
        break; 
    case TTSPREAD_ID_SET_LEVEL:
        i2cCube.levelSet = dataRec;
        writeByte(TTSPREAD_ID_SET_LEVEL);
        dataRec = 0;
        break; 
    case TTSPREAD_ID_SET_SCALE:
        i2cCube.scaleSet = dataRec;
        writeByte(TTSPREAD_ID_SET_SCALE);
        dataRec = 0;
        break;                           
    }
}

void writeWord(uint16_t _data)
{
    uint8_t first_byte = _data & FIRST_BYTE_MASK;
    uint8_t second_byte = (_data & SECOND_BYTE_MASK) >> 8;
    Wire.write(first_byte);
    Wire.flush(); 
    Wire.write(second_byte);
    Wire.flush(); 
}

void writeByte(uint8_t _data)
{
    Wire.write(_data);
    Wire.flush();  
}

void setup()
{
    dbug.begin(115200);
    spread485.begin(115200); 

    dbug.println(F_CPU); // sua lai chan cho mạch mới

    pinMode(PC13, OUTPUT);   
    pinMode(PA4,OUTPUT);
    digitalWriteFast(digitalPinToPinName(PA4),LOW);

    bool check_operation = false;
    unsigned long time1 = 0;
    time1 = HAL_GetTick();
    while (!check_operation)
    {       
        digitalWrite(PC13, HIGH);
        unsigned long time2 = HAL_GetTick();
        bool _led = false;
        while ((unsigned long)(HAL_GetTick() - time1) < 15000)
        {
            if ((unsigned long)(HAL_GetTick() - time2) > 500)
            {
                _led = !_led;
                digitalWrite(PC13, _led);      
                time2 = HAL_GetTick();          
            }
        }
        if (spread485.available()) dbug.print((char)spread485.read());
        digitalWrite(PC13, LOW);
        check_operation = true;
        break;    
    }
    Wire.setSDA(PB9);
    Wire.setSCL(PB8);
    Wire.begin(ADDRESS_I2CCUBE);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    while (!checkconnect);
    sHp.func = FUNC_SPREAD_SETUP_CHECKCONNECT; // viet lai logic checkconnect

    dbug.println(F_CPU);
}
void loop()
{
    recDataFrameSpread485();

    if (spreadVal.setupDone)
    {   
        if ((unsigned long)(HAL_GetTick() - timeI2C) > 70){
            if(stepI2C == 4) stepI2C = 0;
            switch (stepI2C)
            {
            case 0:
                if(i2cCube.hopperRead != i2cCube.hopperSet){
                    sHp.setHopper[3] = i2cCube.hopperSet;
                    senDataFrameSpread485(sHp.setHopper);
                    dbug.println(i2cCube.hopperSet);
                }
                break;
            case 1:
                if(i2cCube.levelRead != i2cCube.levelSet){
                    spreadVal.levelSetFr[3] = i2cCube.levelSet;
                    senDataFrameSpread485(spreadVal.levelSetFr);
                    dbug.println(i2cCube.levelRead);
                }
                break;
            case 2:
                if (i2cCube.spinnerRead < (i2cCube.spinnerSet - 10))
                {
                    dbug.println(i2cCube.spinnerRead);
                    if ((i2cCube.spinnerSet > 250) && (i2cCube.spinnerSet <= 1300))
                    {
                        spreadVal.spinSetFr[3] = (uint8_t)((i2cCube.spinnerSet & 0xFF00) >> 8);
                        spreadVal.spinSetFr[4] = (uint8_t)(i2cCube.spinnerSet & 0x00FF);
                        senDataFrameSpread485(spreadVal.spinSetFr);
                        dbug.println(i2cCube.spinnerSet);
                    }
                    else
                    {
                        spreadVal.spinSetFr[3] = (uint8_t)((250 & 0xFF00) >> 8);
                        spreadVal.spinSetFr[4] = (uint8_t)(250 & 0x00FF);
                        senDataFrameSpread485(spreadVal.spinSetFr);
                    }   
                }
                else if (i2cCube.spinnerRead > (i2cCube.spinnerSet + 10))
                {
                    dbug.println(i2cCube.spinnerRead);
                    if ((i2cCube.spinnerSet > 250) && (i2cCube.spinnerSet <= 1300))
                    {
                        spreadVal.spinSetFr[3] = (uint8_t)((i2cCube.spinnerSet & 0xFF00) >> 8);
                        spreadVal.spinSetFr[4] = (uint8_t)(i2cCube.spinnerSet & 0x00FF);
                        senDataFrameSpread485(spreadVal.spinSetFr);
                        dbug.println(i2cCube.spinnerSet);
                    }
                    else
                    {
                        spreadVal.spinSetFr[3] = (uint8_t)((250 & 0xFF00) >> 8);
                        spreadVal.spinSetFr[4] = (uint8_t)(250 & 0x00FF);
                        senDataFrameSpread485(spreadVal.spinSetFr);
                    } 
                }
                break;
            case 3:
                dbug.println("scaleSet");
                break;    
            }
            stepI2C++;       
            timeI2C = HAL_GetTick();
        }  
    }

    switch (sHp.func)
    {
    /// -------------------------- SETUP ------------------------ ///
    case FUNC_SPREAD_SETUP_CHECKCONNECT:
        senDataFrameSpread485(sHp.checkConnect);
        sHp.miniFunc = FUNC_SPREAD_SETUP_CHECKCONNECT ; 
        sHp.func = 0;
        break;
    case FUNC_SPREAD_SETUP_DONE:
        senDataFrameSpread485(sHp.setupDone);
        sHp.func = 0;
        break; 
    }
}

uint8_t flowRateToHopperPercent(double flowRate)
{
    double a = 0.0;
    double b = 0.0;
    if (flowRate < sHp.valHopperCalib[1])
    {
        a =  coefA(  sHp.valHopperCalib[0],
                            sHp.valHopperCalib[1],
                            sHp.perHopperCalib[0],
                            sHp.perHopperCalib[1]);
        b =  coefB(sHp.valHopperCalib[0], sHp.perHopperCalib[0], a);       
    }
    else
    {
        a =  coefA(  sHp.valHopperCalib[1],
                            sHp.valHopperCalib[2],
                            sHp.perHopperCalib[1],
                            sHp.perHopperCalib[2]);
        b =  coefB(sHp.valHopperCalib[1], sHp.perHopperCalib[1], a);
       
    }
    return a * flowRate + b ; 
}

double coefA(double x1, double x2, double y1, double y2)
{   
    return (y1 - y2) / (x1 - x2) ;
}

double coefB(double x1, double  y1, double a)
{
    return y1 - (x1 * a) ;
}

void senDataFrameSpread485(uint8_t cmd[])
{
    for (uint8_t i = 0; i < 8; i++)debugWriteByte(cmd[i]);
    debugWriteByte((uint8_t)((MODBUS_CRC16_v3(cmd,8) & 0xFF00) >> 8));
    debugWriteByte((uint8_t)(MODBUS_CRC16_v3(cmd,8) & 0x00FF)); 
}

void recDataFrameSpread485()
{
    switch (sHp.stepRec)
    {
    case 0:
        if (spread485.available())
        {
            if (spread485.peek() == 0x1A){sHp.stepRec = 1;  sHp.timeDelayRec = HAL_GetTick();}
            else
            {
                i2cCube.spinnerRead = spreadVal.spinnerRaw;
                i2cCube.hopperRead  = spreadVal.hopperRaw;
                i2cCube.levelRead   = spreadVal.levelRaw;
                i2cCube.scaleRead   = spreadVal.scaleRaw;

                uint8_t byte_temp = 0;
                if (spread485.peek() == 'h')
                {
                    //spreadVal.hopperRaw = spreadVal.readRaw.toInt();//dbug.print(spreadVal.readRaw);
                    spreadVal.readRaw = "";
                    byte_temp = spread485.read();
                }
                else if (spread485.peek() == 'p')
                {
                    spreadVal.hopperRaw = spreadVal.readRaw.toInt();//dbug.println(spreadVal.readRaw);
                    spreadVal.readRaw = "";
                    byte_temp = spread485.read();
                }
                else if (spread485.peek() == 'l')
                {
                    spreadVal.spinnerRaw = spreadVal.readRaw.toInt();//dbug.println(spreadVal.readRaw);
                    spreadVal.readRaw = "";
                    byte_temp = spread485.read();
                }
                else if (spread485.peek() == 's')
                {
                    spreadVal.levelRaw = spreadVal.readRaw.toInt();//dbug.println(spreadVal.readRaw);
                    spreadVal.readRaw = "";
                    byte_temp = spread485.read();
                }
                else if (spread485.peek() == '.')
                {
                    spreadVal.scaleRaw = spreadVal.readRaw.toInt();//dbug.println(spreadVal.readRaw);
                    spreadVal.readRaw = "";
                    byte_temp = spread485.read();
                }
                else
                {
                    spreadVal.readRaw += (char)spread485.read();
                }
            }
        }
        break;
    case 1:
        if ((unsigned long)(HAL_GetTick() - sHp.timeDelayRec) > 10)
        {
            uint16_t crcCheck = 0 ;
            uint16_t crcRec =  0;
            if (sHp.miniFunc != FUNC_SPREAD_LOOP_CALIBSUCCESS)
            {
                spread485.readBytes(recFrameRaw, 8);
                crcCheck = MODBUS_CRC16_v3(recFrameRaw, 8);
                if (sHp.miniFunc == FUNC_SPREAD_LOOP_CALIBPERCENT) sHp.calperval = recFrameRaw[3];
            }
            else
            {
                spread485.readBytes(sHp.FrameCalibSuccess, 42);
                for (uint8_t x = 0; x < 42; x++)
                {
                    dbug.print(sHp.FrameCalibSuccess[x], HEX);
                    dbug.print(" ");
                }
                dbug.println();
                crcCheck = MODBUS_CRC16_v3(sHp.FrameCalibSuccess, 42);
                sHp.miniFunc = 0;
            }
            crcRec = (uint16_t)(spread485.read() << 8) | (uint16_t)spread485.read();
            sHp.handleRec = (crcCheck == crcRec) ? true : false;
            sHp.timeTemp = HAL_GetTick();
            sHp.stepRec = 0;
        }
        break;
    }
    if (sHp.handleRec)
    {
        switch (sHp.miniFunc)
        {
        case FUNC_SPREAD_SETUP_CHECKCONNECT:
            if((unsigned long)(HAL_GetTick() - sHp.timeTemp) > 500) 
            {
                sHp.func = FUNC_SPREAD_SETUP_DONE;
                sHp.miniFunc = FUNC_SPREAD_SETUP_DONE;
                dbug.println("Spread setup ....");
                sHp.handleRec = false;
            }
            break;
        case FUNC_SPREAD_SETUP_DONE:
            sHp.miniFunc = 0;
            spreadVal.setupDone = true;
            dbug.println("Spread setup done");
            sHp.timeValue = HAL_GetTick();
            sHp.handleRec = false;
            timeI2C = HAL_GetTick();
            break;    
        case FUNC_SPREAD_LOOP_CALIBHOPPER:
            sHp.hanldeCalibPercent = true;
            sHp.timeTemp = HAL_GetTick();
            sHp.miniFunc = 0;
            sHp.handleRec = false;
            break;               
        }        
    
        if (recFrameRaw[1] == FUNCTION_SCALE_CALIB_TARE)
        {
            dbug.println("Calib Tare ....");
            sHp.func = FUNCTION_SCALE_CALIB_TARE_DONE;
            sHp.handleRec = false;
        }
        if (recFrameRaw[1] == FUNCTION_SCALE_CALIB_TARE_DONE)
        {
            dbug.println("Calib Tare Done!");
            sHp.handleRec = false;
        }      
        if (recFrameRaw[1] == FUNCTION_SCALE_CALIB_EMPTY_TANK)
        {
            dbug.println("Calib Empty Tank ....");
            sHp.func = FUNCTION_SCALE_CALIB_EMPTY_DONE;
            sHp.handleRec = false;
        }
        if (recFrameRaw[1] == FUNCTION_SCALE_CALIB_EMPTY_DONE)
        {
            dbug.println("Calib Empty Tank Done!");
            sHp.handleRec = false;
        }     
        if (recFrameRaw[1] == FUNCTION_SCALE_CALIB_20KG)
        {
            dbug.println("Calib 20kg ....");
            sHp.func = FUNCTION_SCALE_CALIB_20KG_DONE;
            sHp.handleRec = false;
        }
        if (recFrameRaw[1] == FUNCTION_SCALE_CALIB_20KG_DONE)
        {
            dbug.println("Calib 20kg Done!");
            sHp.handleRec = false;
        }               
    }
}

void debugWriteByte(uint8_t m)
{
    digitalWriteFast(digitalPinToPinName(PA4),HIGH);
    spread485.write(m);
    spread485.flush();
    digitalWriteFast(digitalPinToPinName(PA4),LOW);
}

static uint16_t MODBUS_CRC16_v3( const unsigned char *buf, unsigned int len )
{
    static const uint16_t table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 };

    uint8_t xor1 = 0;
    uint16_t crc = 0xFFFF;

    while( len-- )
    {
        xor1 = (*buf++) ^ crc;
        crc >>= 8;
        crc ^= table[xor1];
    }

    return crc;
}
