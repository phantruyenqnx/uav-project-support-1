#include "define.hpp"

void functiontest();

void setup()
{
    Serial.begin(115200);

    pinMode(11,OUTPUT); // chua biet
    scales.set_gain(64);
    // PARAM EEPROM
    // eeprom null -> calib tare -> ghi eeprom - ngắt điện -> eeprom không null 
    // -> lay gia tri ra (không cần tare)
    // offset[0] , offset[1], offset[2], offset[3] : 4 he so
    // 2 he so double
    // offset[0] = (200-203) , offset[1] = (204-207), offset[2] = (208-2011), offset[3] = (212-215)
    // he_so_1_A = (216-219) , he_so_2_A = (220-223);
    // last_offset = (224, 227);
    // kiem tra xem eeprom co luu hay chua
    while(!scales.is_ready());
    // PARAM EEPROM
    uint8_t tempEepromCount = 0;
    for (uint8_t i = 200; i < 224; i++)
    {
        if (EEPROM.read(i) == 255)
        {
            tempEepromCount ++ ;
            //Serial.println(tempEepromCount);
        }
    }
    if (tempEepromCount > 10)
    {
        //Serial.println("eeprom null -> calib tare ");
        tare();
    }
    else
    {
        offset[0]   = getLongIntFromEeprom(ADDR_OFFSET0);
        offset[1]   = getLongIntFromEeprom(ADDR_OFFSET1);
        offset[2]   = getLongIntFromEeprom(ADDR_OFFSET2);
        offset[3]   = getLongIntFromEeprom(ADDR_OFFSET3);
        he_so_1_A   = (double)getLongIntFromEeprom(ADDR_HESO1)/1000000.0;
        he_so_2_A   = (double)getLongIntFromEeprom(ADDR_HESO2)/1000000.0;
        last_offset = (double)getLongIntFromEeprom(ADDR_LAST_OFFSET)/1000000.0;
    }
    // Serial.println(offset[0]);
    // Serial.println(offset[1]);
    // Serial.println(offset[2]);
    // Serial.println(offset[3]);
    // Serial.println(he_so_1_A, 6);
    // Serial.println(he_so_2_A, 6);
    // Serial.println(last_offset);
    timeSend = millis();

}
void loop()
{
    //Find_max();
    //Find_min();
    receive();
    handleRecFrame();
    readScaleValue();
}

/// ------------------------- CALCULATE --------------------------- ///
void Find_max()
{
    if(check == 1 && scaleValue > 0.50)
    {
        temp_max = scaleValue;
        check = 0;
    }
    if(scaleValue; scaleValue>temp_max)
    {
        scaleValue = temp_max;
    }
}

void Find_min()
{
    double a;
    double temp_min = scaleValue;
    double temp_min_first;
    double temp_min_last;
    readScaleValue();
    if (scaleValue; scaleValue<temp_min)
    {
        temp_min_first = abs(scaleValue);
        readScaleValue();
        readScaleValue();
        temp_min_last = abs(scaleValue);
        if (temp_min_last < temp_min_first)
        {
            readScaleValue();
            temp_min_last = scaleValue;  
            check = 1;
        }
        if (temp_min_last>=temp_min_first)
        {
            temp_min_last = scaleValue;
            check = 0;
        }
    }
}

void heso()
{
    double y_fixed[3] = {0,10.18};
    double x_measured[3] = {first_ket_qua,last_ket_qua};
    double coef[2];
    int sss = fitCurve(1, 3, x_measured, y_fixed, 2, coef);
    if (sss == 0)
    {
        he_so_1_A = coef[0];
        he_so_2_A = coef[1];
        long int he_so_1_A_Eep = (long int)(he_so_1_A * 1000000);
        setLongIntToEeprom(he_so_1_A_Eep, ADDR_HESO1);
        long int he_so_2_A_Eep = (long int)(he_so_2_A * 1000000);
        setLongIntToEeprom(he_so_2_A_Eep, ADDR_HESO2);
        // Serial.print("calib hs1 ");
        // Serial.print(he_so_1_A, 6);
        // Serial.println();        
        // Serial.print("calib hs2 ");
        // Serial.print(he_so_2_A, 6);
        // Serial.println();        
    }  
}

void calib20Kg()
{ 
    double sum_exact[TIMES_CAL] = {0.0}; 
    double sum_last = 0.00;
    for (int j = 0; j < TIMES_CAL ; j++)
    {
        scales.readCustom(results,offset); 
        for (int i=0; i<scales.get_count(); ++i) 
        {
            double _temp = kalmanFilter.updateEstimate(results[i]);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            sum_exact[j] += (double)(abs(_temp)) / 1000.0;       
        }
        sum_last += sum_exact[j] / TIMES_CAL ; 
    }
    last_ket_qua = sum_last;
    heso();
}

void readScaleValue()
{ 
    scales.readCustom(results, offset);
    if (count < TIMES_READ)
    {
        for (int i=0; i<scales.get_count(); ++i) 
        {
            double _temp = kalmanFilter.updateEstimate(results[i]);
            //_temp = kalmanFilter.updateEstimate(_temp);
            //_temp = kalmanFilter.updateEstimate(_temp);
            sum_exact[count] += (double)(abs(_temp)) / 1000.0;       
        }
        sum_last += sum_exact[count] / TIMES_READ ;
        count ++ ;
    }
    if (count == TIMES_READ)
    {
        scaleValue = sum_last *  he_so_1_A + he_so_2_A - last_offset; 
        sendScaleValue(scaleValue);
        sum_last = 0;
        for (uint8_t i = 0; i < TIMES_READ; i++) sum_exact[i] = 0;  
        count = 0;
    } 
}

void calibOffset()
{ 
    double sum_exact[TIMES_CAL] = {0.0}; 
    double sum_last = 0.00;
    for (int j = 0; j < TIMES_CAL ; j++)
    {
        scales.readCustom(results, offset); 
        for (int i=0; i<scales.get_count(); ++i) 
        {
            double _temp = kalmanFilter.updateEstimate(results[i]);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            sum_exact[j] += (double)(abs(_temp)) / 1000.0;         
        }
        sum_last += sum_exact[j] / TIMES_CAL ; 
    }
    first_ket_qua = sum_last;
    heso();
}

void calibBi()
{ 
    double sum_exact[TIMES_CAL] = {0.0}; 
    double sum_last = 0.00;
    for (int j = 0; j < TIMES_CAL ; j++)
    {
        scales.readCustom(results,offset); 
        for (int i=0; i<scales.get_count(); ++i) 
        {
            double _temp = kalmanFilter.updateEstimate(results[i]);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            _temp = kalmanFilter.updateEstimate(_temp);
            sum_exact[j] += (double)(abs(_temp)) / 1000.0;
        }
        sum_last += sum_exact[j] / TIMES_CAL ; 
    }
    last_offset = sum_last*he_so_1_A + he_so_2_A; // luu eerpom
    long int last_offset_eeprom = (long int)(last_offset * 1000000);
    setLongIntToEeprom(last_offset_eeprom, ADDR_LAST_OFFSET);
    // Serial.print("calib bi ");
    // Serial.print(last_offset);
    // Serial.println();
}

long int getLongIntFromEeprom(uint8_t addr)
{
    return ((long int)EEPROM.read(addr)  <<24) |((long int)EEPROM.read(addr+1)<<16)|
           ((long int)EEPROM.read(addr+2)<<8)  | (long int)EEPROM.read(addr+3);
}

void setLongIntToEeprom(long int value, uint8_t addr)
{
    EEPROM.write(addr, (value & 0xFF000000 )>> 24 );
    EEPROM.write(addr+1, (value & 0x00FF0000) >> 16);
    EEPROM.write(addr+2, (value & 0x0000FF00) >> 8);
    EEPROM.write(addr+3, (uint8_t)(value& 0x000000FF));
}

void tare() 
{ 
    bool tareSuccessful = false; 
    unsigned long tareStartTime = millis(); 
    while (!tareSuccessful && millis()<(tareStartTime+TARE_TIMEOUT_SECONDS*1000)) 
    { tareSuccessful = scales.tareCustom(100,5000, offset, 4);} 
    setLongIntToEeprom(offset[0], 200);
    setLongIntToEeprom(offset[1], 204);
    setLongIntToEeprom(offset[2], 208);
    setLongIntToEeprom(offset[3], 212);
    // Serial.print("tare");
    // Serial.println(offset[0]);
    // Serial.println(offset[1]);
    // Serial.println(offset[2]);
    // Serial.println(offset[3]);
}   

/// ------------------------- COMMUNICATION --------------------------- ///
void sendScaleValue(double value)
{
    if ((value < 100) && (value > - 0.1))
    {
        uint16_t _value = (uint16_t)(roundf(value*100));
        frValue[3] = _value / 1000;
        frValue[4] = (_value % 1000) / 100 ;
        frValue[5] = ((_value % 1000) % 100) / 10;
        frValue[6] = ((_value % 1000) % 100) % 10;
        // Serial.print(frValue[3]);
        // Serial.print(frValue[4]);
        // Serial.print(frValue[5]);
        // Serial.print(frValue[6]);
        // Serial.println();
        sendFr(frValue, 8);
    }
}

void sendFr(uint8_t fr[], uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        Serial.write(fr[i]);
        Serial.flush();
    }
    Serial.write((uint8_t)((MODBUS_CRC16_v3(fr, len) & 0xFF00) >> 8)); Serial.flush(); 
    Serial.write((uint8_t)((MODBUS_CRC16_v3(fr, len) & 0x00FF) )); Serial.flush();
}

void serialEvent()
{
    if ((Serial.peek() == 0x11) && (stepRec == 0))
    {
        timeRec = millis();
        stepRec = 1;
    } 
    if ((Serial.peek() != 0x11) && (stepRec == 0))
    {
        uint8_t bytetemp = Serial.read();
    }
    // char c = Serial.read();     
    // if (c == 't')
    // {
    //     tare();
    // }
    // if (c == 'e')
    // {
    //     calibOffset();
    //     calibBi();
    // }
    // if (c == '2')
    // {
    //     calib20Kg();
    // }
}

void handleRecFrame()
{
    if (((unsigned long)(millis() - timeRec) > 10) && (stepRec == 1))
    {
        Serial.readBytes(frRec, 8);
        uint16_t crcCheck = MODBUS_CRC16_v3(frRec , 8);
        uint16_t crcRec = (uint16_t)(Serial.read() << 8) 
                        | (uint16_t) Serial.read();
        checkCRC = (crcCheck == crcRec ) ? true : false;
        stepRec = 0;
    } 
    if (checkCRC)
    {
        selectFunc = frRec[1];
        checkCRC = false;
    }
    
}

void receive()
{
    switch (selectFunc)
    {
    case FUNCTION_CALIB_TARE : // calib tare
        //Serial.println("calib tare");
        frCalibTare[3] = 0x00;
        sendFr(frCalibTare, 8);
        tare();
        frCalibTare[3] = 0x01;
        sendFr(frCalibTare, 8);
        //Serial.println("done!");
        timeSend = millis();
        selectFunc = FUNCTION_VALUE;
        break;    
    case FUNCTION_CALIB_EMPTY_TANK:   // calib binh rong
        //Serial.println("calib empty tank");
        frCalibEmptyTank[3] = 0x00;
        sendFr(frCalibEmptyTank, 8);
        calibOffset();
        calibBi();
        frCalibEmptyTank[3] = 0x01;
        sendFr(frCalibEmptyTank, 8);
        //Serial.println("done!");
        timeSend = millis();
        selectFunc = FUNCTION_VALUE;
        break;
    case FUNCTION_CALIB_20KG :   // calib 20 kg
        //Serial.println("calib 20kg");
        frCalib20Kg[3] = 0x00;
        sendFr(frCalib20Kg, 8);
        calib20Kg();
        frCalib20Kg[3] = 0x01;
        sendFr(frCalib20Kg, 8);   
        //Serial.println("done!");
        timeSend = millis();
        selectFunc = FUNCTION_VALUE;
        break;
    }
    // if ((unsigned long)(millis() - timeSend) > 50 )
    // {
        
    //     timeSend = millis();
    // }    
}

static uint16_t MODBUS_CRC16_v3( const unsigned char *buf, unsigned int len )
{
	uint16_t crc = 0xFFFF;
	char i = 0;

	while(len--)
	{
		crc ^= (*buf++);

		for(i = 0; i < 8; i++)
		{
			if( crc & 1 )
			{
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
			{
				crc >>= 1;
			}
		}
	}

	return crc;
}
