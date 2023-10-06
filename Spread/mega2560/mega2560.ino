uint8_t checkConnect[8] = {0x1A, 0x01, 0x02, 0x75, 0x1F, 0x1A, 0x30, 0x44};
uint8_t setupDone[8]    = {0x1A, 0x02, 0x02, 0x3F, 0x11, 0x12, 0x30, 0x14};
uint8_t calHop[8]       = {0x1A, 0x10, 0x02, 0x76, 0x25, 0x1E, 0x30, 0x40};
uint8_t calPer[8]       = {0x1A, 0x11, 0x01, 0x00, 0x01, 0x00, 0x00, 0x4F};
uint8_t calSuccess[8]   = {0x1A, 0x13, 0x26, 0x75, 0x60, 0x12, 0x34, 0x22};

struct servoHopperUse
{
    uint8_t stepRec = 0;
    uint8_t func = 0 ;
    uint8_t recHanlde = 0;
    uint8_t dataFrame[20] = {};
    unsigned long timeDelayRec = 0;

}sHp;
enum FUNC_SPREAD
{
    // Setup
    FUNC_SPREAD_SETUP_CHECKCONNECT = 0x01,
    FUNC_SPREAD_SETUP_DONE         = 0x02,
    // Loop
    // Calib hopper
    FUNC_SPREAD_LOOP_CALIBHOPPER   = 0x10,
    FUNC_SPREAD_LOOP_CALIBPERCENT  = 0x11,
    FUNC_SPREAD_LOOP_CALIBERROR    = 0x12,
    FUNC_SPREAD_LOOP_CALIBSUCCESS  = 0x13,
};
#define ELE(x) (sizeof(x)/sizeof(x[0]))
unsigned long time1 = 0;
bool per = false;
void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200);
}
void loop()
{
    // if (Serial1.available())
    // {
    //     Serial.print((char)Serial1.read());
    // }
    
    if (Serial.available())
    {
        char c = (char)Serial.read();
        switch (c)
        {
        case 'i':
            sHp.func = FUNC_SPREAD_LOOP_CALIBHOPPER;
            break;
        case 's':
            sHp.func = FUNC_SPREAD_SETUP_CHECKCONNECT;
            break;
        // case 'd':
        //     sHp.func = FUNC_SPREAD_SETUP_DONE;
        //    break;
        case 'p':
            sHp.func = FUNC_SPREAD_LOOP_CALIBPERCENT;
            break;
        }       
    }
    if (per)
    {
        if ((unsigned long)(millis() - time1) > 300)
        {
            sHp.func = FUNC_SPREAD_LOOP_CALIBPERCENT;
            time1 = millis();
        }
    }
    
    recDataFrameSpread485_2();
    switch (sHp.func)
    {
    case FUNC_SPREAD_LOOP_CALIBHOPPER:
        //Serial1.write(0x00);
        //Serial1.write(0x00);
        senDataFrameSpread485(calHop, 8);
        Serial.println("alo");
        sHp.recHanlde = sHp.func;
        sHp.func = 0;
        break;
    case FUNC_SPREAD_LOOP_CALIBPERCENT:
        senDataFrameSpread485(calPer, 8);
        sHp.recHanlde = sHp.func;
        sHp.func = 0;
        break;    
    case FUNC_SPREAD_SETUP_CHECKCONNECT:
        // Serial1.write(0x00);
        // Serial1.write(0x00);
        senDataFrameSpread485(checkConnect, 8);
        sHp.recHanlde = sHp.func;
        sHp.func = 0;
        break;
    case FUNC_SPREAD_SETUP_DONE:
        if ((unsigned long)(millis() - time1) > 1000)
        {
            senDataFrameSpread485(setupDone, 8);
            sHp.recHanlde = sHp.func;
            sHp.func = 0;
        }
        break;   
    case FUNC_SPREAD_LOOP_CALIBSUCCESS:
        senDataFrameSpread485(calSuccess, 8);
        sHp.recHanlde = sHp.func;
        sHp.func = 0;
        break;       
    }
}

void senDataFrameSpread485(uint8_t cmd[], uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        debugWriteByte(cmd[i]);
        //Serial.print(cmd[i]);
        //Serial.print(" ");
    }
    debugWriteByte((uint8_t)((MODBUS_CRC16_v3(cmd,length) & 0xFF00) >> 8));
    debugWriteByte((uint8_t)(MODBUS_CRC16_v3(cmd,length) & 0x00FF)); 
    //Serial.print((uint8_t)((MODBUS_CRC16_v3(cmd,length) & 0xFF00) >> 8));
    //Serial.print(" ");
    //Serial.print((uint8_t)(MODBUS_CRC16_v3(cmd,length) & 0x00FF));
}

uint8_t recFrame[8] = {0};
uint8_t recFrameCRC[6] = {0};
uint16_t crcCheckHandle = 0;
uint16_t crcCheckRec = 0;

void recDataFrameSpread485_2()
{
    if ((Serial1.available()) && (sHp.stepRec == 0))
    {
        if (Serial1.peek() == 0x1A)
        {
            sHp.stepRec = 1;  
            sHp.timeDelayRec = millis();
        }
        else
        {
            Serial.print((char)Serial1.read());
        }
    }
    if (sHp.stepRec == 1)
    {
        if ((unsigned long)(millis() - sHp.timeDelayRec) > 20)
        {
            switch (sHp.recHanlde)
            {
            case FUNC_SPREAD_SETUP_CHECKCONNECT:
                Serial1.readBytes(recFrame, 8);
                for (uint8_t i = 0; i < 6; i++) recFrameCRC[i] = recFrame[i];
                crcCheckHandle = MODBUS_CRC16_v3(recFrameCRC, 6);
                crcCheckRec = (uint16_t)(recFrame[6] << 8) | (uint16_t)recFrame[7] ;
                if (crcCheckHandle == crcCheckRec)
                {
                    Serial.println("OK");
                }
                else
                {
                    Serial.println("!O");
                }
                sHp.recHanlde = 0;
                sHp.stepRec = 0 ;
                break;
            }
        }
    }
    
}

void recDataFrameSpread485()
{
    switch (sHp.stepRec)
    {
    case 0:
        if (Serial1.available())
        {
            uint8_t id = Serial1.peek();
            if (id == 0x1A)
            {
                sHp.dataFrame[0] = Serial1.read();
                sHp.stepRec = 1;  
                sHp.timeDelayRec = millis();
            }
            else
            {
                Serial.print((char)Serial1.read());
            }
        }
        break;
    case 1:
        if ((unsigned long)(millis() - sHp.timeDelayRec) > 10)
        {
            uint8_t rec_available = Serial1.available();
            uint8_t *rec_data = new uint8_t[rec_available-1];
            for (uint8_t i = 0; i < rec_available ; i++)
            {
                sHp.dataFrame[i+1] = Serial1.read();
                Serial.print(sHp.dataFrame[i+1], HEX);
                Serial.print(" ");
            }
            //Serial.println();
            for (uint8_t i = 0; i < rec_available - 1 ; i++)
                rec_data[i] = sHp.dataFrame[i];
            
            uint16_t crcCheck = MODBUS_CRC16_v3(rec_data, rec_available-1);
            uint16_t crcRec = (uint16_t)(sHp.dataFrame[rec_available-1] << 8) 
                            | (uint16_t)sHp.dataFrame[rec_available];

            if (crcCheck == crcRec)
            {
                if (sHp.dataFrame[1] == FUNC_SPREAD_SETUP_CHECKCONNECT) { sHp.func = FUNC_SPREAD_SETUP_DONE; time1 = millis();}
                if (sHp.dataFrame[1] == FUNC_SPREAD_LOOP_CALIBPERCENT)
                {
                    if (sHp.dataFrame[3] == 100)
                    {
                        per = false;   
                        sHp.func = FUNC_SPREAD_LOOP_CALIBSUCCESS; 
                    }
                }
                if (sHp.dataFrame[1] == FUNC_SPREAD_LOOP_CALIBHOPPER)
                {
                    per = true;
                    time1 = millis();
                }
                
                Serial.println("OK");
            }
            else
            {
                Serial.println("!O");
                
            } 
            delete[] rec_data;
            sHp.stepRec = 0;
        }
        
        break;
    }
}

void debugWriteByte(uint8_t m)
{
    Serial1.write(m);
    Serial1.flush();
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
