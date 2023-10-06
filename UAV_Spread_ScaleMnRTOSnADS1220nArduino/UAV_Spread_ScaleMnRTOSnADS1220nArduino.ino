#include "src/define.hpp"

/*
Project have 6 tasks:                Piority                 Stacksize
1. Modbus                               2                       100   
2. SerDebug                             3                       100
3. LedStatus                            4                       50
4. CalculateValue                       1                       100
5. CalibrateZero                        1                       100
6. CalibrateTare                        1                       100
7. CalibrateStandardWeight              1                       100    
*/

//define task handles
TaskHandle_t TaskModbus_Handler;
TaskHandle_t TaskSerDebug_Handler;
TaskHandle_t TaskLedStatus_Handler;
TaskHandle_t TaskCalculateValue_Handler;
TaskHandle_t TaskCalibrateZero_Handler;
TaskHandle_t TaskCalibrateTare_Handler;
TaskHandle_t TaskCalibrateStandardWeight_Handler;

// define task
static void TaskModbus( void *pvParameters );
static void TaskSerDebug(void* pvParameters);
static void TaskLedStatus(void *pvParameters);
static void TaskCalculateValue( void *pvParameters );
static void TaskCalibrateZero(void* pvParameters);
static void TaskCalibrateTare(void *pvParameters);
static void TaskCalibrateStandardWeight( void *pvParameters );

// define Queue
QueueHandle_t qStatus;

// setup
void setup()
{
    /*------------------------ Init System ----------------------*/
    // Modbus port UART2
    P_Modbus.begin(P_MODBUS_BAUDRATE);
    pinMode(P_MODBUS_PINDIR, OUTPUT);
    digitalWrite(P_MODBUS_PINDIR, LOW);

    // Scale frame define
    EX_FRAME_CONFIG(&frame_Checkconnect, P_MODBUS_SLAVEID, SCALE_MODBUS_FUNCTION_CHECK_CONNECT);
    EX_FRAME_CONFIG(&frame_StatusTask, P_MODBUS_SLAVEID, SCALE_MODBUS_FUNCTION_STATUS_TASK);

    // Serial Debug UART3
    P_SerDebug.begin(P_SerDebug_BAUDRATE); 
    while(!P_SerDebug) {}
    P_SerDebug.println(INIT_BOOT_STRING);

    // init ADS1220
    if (ADS1220_Setup(&ads1))   { P_SerDebug.println("ads1 setup done!");}
    else                        { P_SerDebug.println("ads1 setup failed!");}
    if (ADS1220_Setup(&ads2))   { P_SerDebug.println("ads2 setup done!");}
    else                        { P_SerDebug.println("ads2 setup failed!");}
    if (ADS1220_Setup(&ads3))   { P_SerDebug.println("ads3 setup done!");}
    else                        { P_SerDebug.println("ads3 setup failed!");}
    if (ADS1220_Setup(&ads4))   { P_SerDebug.println("ads4 setup done!");}
    else                        { P_SerDebug.println("ads4 setup failed!");}

    // get offset from eeproom
    coef_m    = EEPROM_getDouble(ADDR_HESO_M);
    coef_n    = EEPROM_getDouble(ADDR_HESO_N);
    offset[0] = EEPROM_getDouble(ADDR_OFFSET0);
    offset[1] = EEPROM_getDouble(ADDR_OFFSET1);
    offset[2] = EEPROM_getDouble(ADDR_OFFSET2);
    offset[3] = EEPROM_getDouble(ADDR_OFFSET3);

    /*-- Create queue --*/
    qStatus = xQueueCreate(1, sizeof(struct system));
    /*-- Create Task --*/

    xTaskCreate(    TaskModbus,
                    "Modbus",
                    MODBUS_STACKSIZE,
                    nullptr,
                    PRIO2,
                    &TaskModbus_Handler);

    xTaskCreate(    TaskSerDebug,
                    "SerDebug",
                    SERDEBUG_STACKSIZE,
                    nullptr,
                    PRIO1,
                    nullptr);

    xTaskCreate(    TaskLedStatus,
                    "LedStatus",
                    LEDSTATUS_STACKSIZE,
                    nullptr,
                    PRIO3,
                    &TaskLedStatus_Handler);

    xTaskCreate(    TaskCalculateValue,
                    "CalculateValue",
                    CACULATEVALUE_STACKSIZE,
                    nullptr,
                    PRIO4,
                    &TaskCalculateValue_Handler);

    xTaskCreate(    TaskCalibrateZero,
                    "CalibrateZero",
                    CALIBRATEZERO_STACKSIZE,
                    nullptr,
                    PRIO4,
                    &TaskCalibrateZero_Handler);

    xTaskCreate(    TaskCalibrateTare,
                    "CalibrateTare",
                    CALIBRATETARE_STACKSIZE,
                    nullptr,
                    PRIO4,
                    &TaskCalibrateTare_Handler);

    xTaskCreate(    TaskCalibrateStandardWeight,
                    "CalibrateStandardWeight",
                    CALIBATESTANDARDWEIGHT_STACKSIZE,
                    nullptr,
                    PRIO4,
                    &TaskCalibrateStandardWeight_Handler);

     /*-- Start FreeRTOS --*/
    vTaskSuspend(TaskCalibrateZero_Handler); 
    vTaskSuspend(TaskCalibrateTare_Handler); 
    vTaskSuspend(TaskCalibrateStandardWeight_Handler); 

    // debug comment for modbus
    //vTaskSuspend(TaskCalculateValue_Handler);
    //vTaskSuspend(TaskSerDebug_Handler);

    vTaskStartScheduler();

    // should never return
    P_SerDebug.println(F("Die")); 
    while(1);
}

// loop not use
void loop(){}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

static void TaskModbus(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    uint8_t frameRec[P_MODBUS_FRAME_LENGTH] = {0};
    uint8_t count1 = 0;
    uint8_t count2 = 0;
    for (;;) 
    {
        if(P_Modbus.available() > 0)
        {
            if (P_Modbus.peek() == P_MODBUS_SLAVEID)
            {
                P_Modbus.readBytes(frameRec, P_MODBUS_FRAME_LENGTH);
                frame_Receive.id    = frameRec[0]; //P_SerDebug.print(frameRec[0], HEX); P_SerDebug.print(" ");
                frame_Receive.func  = frameRec[1];// P_SerDebug.print(frameRec[1], HEX); P_SerDebug.print(" ");
                frame_Receive.data1 = frameRec[2]; //P_SerDebug.print(frameRec[3], HEX); P_SerDebug.print(" ");
                frame_Receive.data2 = frameRec[3]; //P_SerDebug.print(frameRec[4], HEX); P_SerDebug.print(" ");
                frame_Receive.data3 = frameRec[4]; //P_SerDebug.print(frameRec[5], HEX); P_SerDebug.print(" ");
                frame_Receive.data4 = frameRec[5]; //P_SerDebug.print(frameRec[6], HEX); P_SerDebug.print(" ");
                frame_Receive.crcH  = frameRec[6];// P_SerDebug.print(frameRec[8], HEX); P_SerDebug.print(" ");
                frame_Receive.crcL  = frameRec[7];//P_SerDebug.println(frameRec[9], HEX);
                bool _check_frame = true; 
                // CRC check
                uint16_t crc = ((frame_Receive.crcH  << 8) | frame_Receive.crcL);
                if (crc != EX_FRAME_CALC_CRC(frame_Receive)) _check_frame = false;
                
                if (_check_frame) // reply to master
                {
                    switch (frame_Receive.func)
                    {
                    case SCALE_MODBUS_FUNCTION_CHECK_CONNECT:
                        frame_Checkconnect.data1 = SCALE_MODBUS_FUNCTION_CHECK_CONNECT;
                        frame_Checkconnect.crcH = EX_FRAME_CALC_CRC(frame_Checkconnect) >> 8;
                        frame_Checkconnect.crcL = EX_FRAME_CALC_CRC(frame_Checkconnect) & 0xFF;
                        MODBUS_sendFrame(frame_Checkconnect, &P_Modbus, P_MODBUS_PINDIR);
                        //P_SerDebug.println(frame_Receive.func);
                        break;
                    case SCALE_MODBUS_FUNCTION_STATUS_TASK:
                        // handle set task
                        if (frame_Receive.data1 == STEP5)
                        {
                            vTaskSuspend(TaskCalculateValue_Handler); // stop task value
                            vTaskResume(TaskCalibrateZero_Handler);   // resume task zero
                        }
                        else if (frame_Receive.data1 == STEP6)
                        {
                            vTaskSuspend(TaskCalculateValue_Handler); // stop task value
                            vTaskResume(TaskCalibrateTare_Handler);   // resume task tare
                        }
                        else if (frame_Receive.data1 == STEP7)
                        {
                            coef_kg_ref = frame_Receive.data2;
                            vTaskSuspend(TaskCalculateValue_Handler); // stop task value
                            vTaskResume(TaskCalibrateStandardWeight_Handler);   // resume task StandardWeight
                        }

                        frame_StatusTask.data1 = task_curr;
                        if (task_curr == STEP4)
                        {
                            frame_StatusTask.data2 = (((long long)value_curr) >> 8 ) & 0xFF;
                            frame_StatusTask.data3 = (((long long)value_curr)) & 0xFF;
                            frame_StatusTask.data4 = (uint8_t)(abs(value_curr - (int32_t)value_curr) * 100.0);
                        }
                        else
                        {
                            frame_StatusTask.data2 = 0;
                            frame_StatusTask.data3 = 0;
                            frame_StatusTask.data4 = (uint8_t)value_curr; 
                        }
                        frame_StatusTask.crcH = EX_FRAME_CALC_CRC(frame_StatusTask) >> 8;
                        frame_StatusTask.crcL = EX_FRAME_CALC_CRC(frame_StatusTask) & 0xFF;
                        MODBUS_sendFrame(frame_StatusTask, &P_Modbus, P_MODBUS_PINDIR);
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    count2++;
                }
            }
            else
            {
                P_Modbus.read();
            }
        }
        count1++;
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskSerDebug(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    for (;;) 
    {
        struct system _sys;
        while(P_SerDebug.available()>0){
            switch(P_SerDebug.read()){
                case 's':
                    vTaskSuspend(TaskLedStatus_Handler); 
                    P_SerDebug.println("Suspend!");
                    setTask = STEP4;
                    break;
                case 'r':
                    vTaskResume(TaskLedStatus_Handler);
                    P_SerDebug.println("Resume!");
                    setTask = STEP5;
                    break;
                case P_SerDebug_CALIBTARE: 
                    vTaskSuspend(TaskCalculateValue_Handler); // stop task value
                    vTaskResume(TaskCalibrateTare_Handler);   // resume task tare
                    P_SerDebug.println("TARE!");
                    break;
                case P_SerDebug_CALIBZERO:
                    vTaskSuspend(TaskCalculateValue_Handler); // stop task value
                    vTaskResume(TaskCalibrateZero_Handler);   // resume task zero
                    P_SerDebug.println("ZERO!");
                    break;
                case P_SerDebug_CALIBWEIGHT:
                    coef_kg_ref = P_SerDebug_WEIGHTREF;
                    vTaskSuspend(TaskCalculateValue_Handler); // stop task value
                    vTaskResume(TaskCalibrateStandardWeight_Handler);   // resume task StandardWeight
                    P_SerDebug.println("STANDARD WEIGHT!");
                    break;
            }
        }
        if (xQueueReceive(qStatus, &_sys, portMAX_DELAY) == pdPASS) {
            task_curr  = _sys.task;
            value_curr = _sys.value;
            P_SerDebug.print("T: ");
            P_SerDebug.print(_sys.task);
            P_SerDebug.print(" V: ");
            P_SerDebug.print(_sys.value);
            P_SerDebug.print(" A: ");
            P_SerDebug.print(coef_a);
            P_SerDebug.print(" B: ");
            P_SerDebug.print(coef_b);
            P_SerDebug.print(" M: ");
            P_SerDebug.print(coef_m);
            P_SerDebug.print(" N: ");
            P_SerDebug.print(coef_n);
            P_SerDebug.print(" 1: ");
            P_SerDebug.print(offset[0]);
            P_SerDebug.print(" 2: ");
            P_SerDebug.print(offset[1]);
            P_SerDebug.print(" 3: ");
            P_SerDebug.print(offset[2]);
            P_SerDebug.print(" 4: ");
            P_SerDebug.println(offset[3]);
        }
        vTaskDelay((20L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskLedStatus(void *pvParameters) 
{
/*
    Explain Task:

*/
    (void) pvParameters;
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) 
    {
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        vTaskDelay( 300 / portTICK_PERIOD_MS ); // wait for one second
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        vTaskDelay( 300 / portTICK_PERIOD_MS ); // wait for one second
    }
}

static void TaskCalculateValue(void* pvParameters){
/*
    Explain Task:

*/
    for (;;) 
    {
        struct system _sys;
        _sys.task = STEP4;
        _sys.value = kalmanFilter.updateEstimate(ADS1220_mainValue(offset)/1000.0)*coef_m + coef_n;
        xQueueSend(qStatus,  &_sys, portMAX_DELAY);
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskCalibrateZero(void* pvParameters){
/*
    Explain Task:

*/
    for (;;) 
    {
        struct system _sys;
        double _sum = 0.0;
        for (uint16_t i = 0; i < ADS_CALIB_NUM; i++)
        {
            _sum += kalmanFilter.updateEstimate(ADS1220_mainValue(offset)/1000.0)/ADS_CALIB_NUM;
            _sys.task = STEP5;
            _sys.value = i*100.0/ADS_CALIB_NUM;
            xQueueSend(qStatus,  &_sys, ADS_CALIB_portMAX_DELAY);
        }
        coef_a = _sum;
        _sys.value = 100.0; // done 100%
        xQueueSend(qStatus,  &_sys, ADS_CALIB_portMAX_DELAY);
        vTaskResume(TaskCalculateValue_Handler);
        vTaskSuspend(TaskCalibrateZero_Handler); 
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskCalibrateTare(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    for (;;) 
    {
        struct system _sys;
        _sys.task = STEP6;
        _sys.value = 0.0;
        xQueueSend(qStatus,  &_sys, ADS_CALIB_portMAX_DELAY);

        bool _tareSuccessful = false;  
        unsigned long _tareStartTime = HAL_GetTick(); 
        while (!_tareSuccessful && HAL_GetTick()<(_tareStartTime+ADS_CALIB_TARE_DURATION)) 
        {
            uint16_t i,j;
            long values[ADS_COUNT];
        
            long minValues[ADS_COUNT];
            long maxValues[ADS_COUNT];

            for (i=0; i<ADS_COUNT; ++i) {
                minValues[i]=0x7FFFFFFF;
                maxValues[i]=0x80000000;
            }

            _sys.value = 5.0;
            xQueueSend(qStatus,  &_sys, ADS_CALIB_portMAX_DELAY);

            for (i=0; i<ADS_CALIB_TARE_TIMES; ++i) {
                ADS1220_Raw(values);
                for (j=0; j<ADS_COUNT; ++j) {
                    if (values[j]<minValues[j]) {minValues[j]=values[j];}	
                    if (values[j]>maxValues[j]) {maxValues[j]=values[j];} 
                }
                _sys.value = 5.0 + i*80/ADS_CALIB_TARE_TIMES;
                xQueueSend(qStatus,  &_sys, ADS_CALIB_portMAX_DELAY);		
            }

            uint8_t _temp = 0;
            for (i=0; i<ADS_COUNT; ++i) {
                if (abs(maxValues[i]-minValues[i])>ADS_CALIB_TARE_TOLERANCE) {
                    _temp++;
                }
            }
            if (_temp > 0)
            {
                _tareSuccessful = false;
            }
            else
            {
                //set the offsets
                for (i=0; i<ADS_COUNT; ++i) {
                    offset[i] = values[i];
                }
                _tareSuccessful = true;
            }   
        }
        if (_tareSuccessful)
        {
            EEPROM_saveDouble(ADDR_OFFSET0, offset[0]);
            EEPROM_saveDouble(ADDR_OFFSET1, offset[1]);
            EEPROM_saveDouble(ADDR_OFFSET2, offset[2]);
            EEPROM_saveDouble(ADDR_OFFSET3, offset[3]);
            _sys.value = 100.0; // done
        
        }
        else
        {
            _sys.value = 0.0; // fail
        }
        xQueueSend(qStatus,  &_sys, ADS_CALIB_portMAX_DELAY);
        vTaskResume(TaskCalculateValue_Handler);
        vTaskSuspend(TaskCalibrateTare_Handler); 
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskCalibrateStandardWeight(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    for (;;) 
    {
        struct system _sys;
        double _sum = 0.00;
        for (uint16_t i = 0; i < ADS_CALIB_NUM ; i++)
        {
            _sum += kalmanFilter.updateEstimate(ADS1220_mainValue(offset)/1000.0) / ADS_CALIB_NUM ; 
            _sys.task = STEP7;
            _sys.value = i*100.0/ADS_CALIB_NUM;
            xQueueSend(qStatus,  &_sys, ADS_CALIB_portMAX_DELAY);
        }

        coef_b = _sum;
        coef_m = coef_kg_ref / (coef_b - coef_a);
        coef_n = - coef_m * coef_a ;
        EEPROM_saveDouble(ADDR_HESO_M, coef_m);
        EEPROM_saveDouble(ADDR_HESO_N, coef_n);

        _sys.value = 100.0; // done 100%
        xQueueSend(qStatus,  &_sys, ADS_CALIB_portMAX_DELAY);
        vTaskResume(TaskCalculateValue_Handler);
        vTaskSuspend(TaskCalibrateStandardWeight_Handler);
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

/*--------------------------------------------------*/
/*---------------- Function Support -----------------*/
/*--------------------------------------------------*/
void EX_FRAME_CONFIG( frame_t *_frame, uint8_t _slaveID, uint8_t _function)
{
    _frame->id      = _slaveID;
    _frame->func    = _function;
}

uint16_t EX_FRAME_CALC_CRC(frame_t _frame) {
	uint8_t CRCHi = 0xFF, CRCLo = 0x0FF, Index;
    Index = CRCHi ^ _frame.id; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.func; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data1; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data2; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data3; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data4; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    return (CRCHi << 8) | CRCLo;
}
/*======================================== ADS 1220 ====================================*/
bool ADS1220_Setup(ADS1220_WE *_ads)
{
    if(!_ads->init()){ return false;}
    _ads->start();
    _ads->reset();
    _ads->powerDown();
    _ads->setSPIClockSpeed(ADS_SPI_CLOCKSPEED);
    _ads->setCompareChannels(ADS1220_MUX_1_2);
    _ads->setGain(ADS1220_GAIN_128);
    _ads->setDataRate(ADS1220_DR_LVL_6);
    _ads->setOperatingMode(ADS1220_TURBO_MODE);
    _ads->setVRefSource(ADS1220_VREF_REFP1_REFN1);
    _ads->setVRefValue_V(ADS_VRefValue_V);
    _ads->setLowSidePowerSwitch(ADS1220_SWITCH);
    return true;
}

void ADS1220_Raw(long *_val) // cần cải thiện
{
    _val[0] = -ads1.getRawData() + ADS_RAW_OFFSET; 
    _val[1] = -ads2.getRawData() + ADS_RAW_OFFSET; 
    _val[2] =  ads3.getRawData() + ADS_RAW_OFFSET; 
    _val[3] = -ads4.getRawData() + ADS_RAW_OFFSET; 
}

void ADS1220_readAfterTare(long *_val, volatile long *_offset) // đọc raw - offset
{
    ADS1220_Raw(_val);
    for (int j = 0; j < ADS_COUNT; ++j) {
		    _val[j] = _val[j] -  _offset[j];  
    }    
}

double ADS1220_mainValue(volatile long *_offset) // trung bình cộng 4 giá trị ads => giá trị main value
{
    long _r[ADS_COUNT] = {0};
    ADS1220_readAfterTare(_r, _offset);
    return ((double)_r[0] + (double)_r[1] + (double)_r[2] + (double)_r[3])/ (double)ADS_COUNT ;
}

/*================================ EPPROM ======================================*/

void EEPROM_saveDouble(int address, double value) {
  uint8_t* p = (uint8_t*)(void*)&value; // Convert the double pointer to a byte pointer
  for (int i = 0; i < sizeof(value); i++) {
    EEPROM.write(address++, *p++); // Write each byte of the double to EEPROM
  }
}

double EEPROM_getDouble(int address) {
  double value = 0;
  uint8_t* p = (uint8_t*)(void*)&value; // Convert the double pointer to a byte pointer
  for (int i = 0; i < sizeof(value); i++) {
    *p++ = EEPROM.read(address++); // Read each byte of the double from EEPROM
  }
  return value;
}

/*================================ RS485 ======================================*/
// V1.0 chưa dùng frame dùng thuần tín hiệu UART
void MODBUS_sendFrame(frame_t _frame, HardwareSerial *_port , uint8_t _pinDir)
{
    digitalWrite(_pinDir, HIGH);
    (*_port).write(_frame.id);
    (*_port).write(_frame.func);
    (*_port).write(_frame.data1);
    (*_port).write(_frame.data2);
    (*_port).write(_frame.data3);
    (*_port).write(_frame.data4);
    (*_port).write(_frame.crcH);
    (*_port).write(_frame.crcL);
    (*_port).flush();
    digitalWrite(_pinDir, LOW);
}
