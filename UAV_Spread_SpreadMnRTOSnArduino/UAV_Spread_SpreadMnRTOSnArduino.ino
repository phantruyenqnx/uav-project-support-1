#include "src/define.hpp"

/*
Project have 7 tasks:                Piority                 Stacksize
1. Modbus Module Common UART1             3                       100  
2. Modbus Scale RS485 UART6               3                       100          
3. Uart Level sensor UART2                3                       100
4. Control hopper                         3                       100
5. Control spinner                        3                       100
6. Calib spread system                    1                       100
7. Led status                             2                       50
*/

//define task handles
TaskHandle_t TaskCommon_Handler;
TaskHandle_t TaskScale_Handler;
TaskHandle_t TaskLevelSensor_Handler;
TaskHandle_t TaskHopper_Handler;
TaskHandle_t TaskSpinner_Handler;
TaskHandle_t TaskCalibSystem_Handler;
TaskHandle_t TaskLedStatus_Handler;

// define task
static void TaskCommon( void *pvParameters );
static void TaskScale( void *pvParameters );
static void TaskLevelSensor( void *pvParameters );
static void TaskHopper( void *pvParameters );
static void TaskSpinner( void *pvParameters );
static void TaskCalibSystem( void *pvParameters );
static void TaskLedStatus( void *pvParameters );

// define Queue
QueueHandle_t qStatus;

// setup
void setup()
{
    /*------------------------ Init System ----------------------*/
    // Serial Common
    P_SerCommon.begin(P_SERSCOMMON_BAUDRATE); 
    while(!P_SerCommon) {}
    pinMode(P_SERSCOMMON_PINDIR, OUTPUT);
    digitalWrite(P_SERSCOMMON_PINDIR, LOW);
    COMMON_sendString(INIT_BOOT_STRING, &P_SerCommon, P_SERSCOMMON_PINDIR);
    EX_FRAME12_CONFIG(&frame12_Checkconnect, P_SERSCOMMON_SLAVEID, MODBUS_FUNCTION_CHECK_CONNECT);
    EX_FRAME12_CONFIG(&frame12_StatusTask, P_SERSCOMMON_SLAVEID, MODBUS_FUNCTION_STATUS_TASK);

    // Serial scale
    P_SerScale.begin(P_SERSCALE_BAUDRATE); 
    while(!P_SerScale) {}
    pinMode(P_SERSCALE_PINDIR, OUTPUT);
    digitalWrite(P_SERSCALE_PINDIR, LOW);
    // Scale frame define
    EX_FRAME_CONFIG(&frame_Checkconnect, P_SERSCALE_SLAVEID, SCALE_MODBUS_FUNCTION_CHECK_CONNECT);
    EX_FRAME_CONFIG(&frame_StatusTask, P_SERSCALE_SLAVEID, SCALE_MODBUS_FUNCTION_STATUS_TASK);

    // Level sensor init
    P_LevelSensor.init();
    P_LevelSensor.startMotor2(true); // true chạy. false dừng

    // Hopper init
    P_Hopper.attach(P_HOPPER_PIN);
    P_Hopper.writeMicroseconds(P_HOPPER_MIN_VALUE);

    // Spinner init
    P_Spinner.attach(P_SPINNER_PIN);
    P_Spinner.writeMicroseconds(P_SPINNER_MIN_VALUE);

    // led status
    pinMode(LED_BUILTIN, OUTPUT);
    delay(10000);

    /*-- Create queue --*/
    qStatus = xQueueCreate(10, sizeof(struct system));
    /*-- Create Task --*/
    xTaskCreate(    TaskCommon,
                    "Common",
                    COMMON_STACKSIZE,
                    nullptr,
                    PRIO3,
                    &TaskCommon_Handler);

    xTaskCreate(    TaskScale,
                    "Scale",
                    SCALE_STACKSIZE,
                    nullptr,
                    PRIO3,
                    &TaskScale_Handler);

    xTaskCreate(    TaskLevelSensor,
                    "LevelSensor",
                    LEVELSENSOR_STACKSIZE,
                    nullptr,
                    PRIO3,
                    &TaskLevelSensor_Handler);

    xTaskCreate(    TaskHopper,
                    "Hopper",
                    HOPPER_STACKSIZE,
                    nullptr,
                    PRIO3,
                    &TaskHopper_Handler);

    xTaskCreate(    TaskSpinner,
                    "Spinner",
                    SPINNER_STACKSIZE,
                    nullptr,
                    PRIO3,
                    &TaskSpinner_Handler);

    xTaskCreate(    TaskCalibSystem,
                    "CalibSystem",
                    CALIBATESYSTEM_STACKSIZE,
                    nullptr,
                    PRIO1,
                    &TaskCalibSystem_Handler);

    xTaskCreate(    TaskLedStatus,
                    "LedStatus",
                    LEDSTATUS_STACKSIZE,
                    nullptr,
                    PRIO2,
                    &TaskLedStatus_Handler);                                                            

     /*-- Start FreeRTOS --*/
    vTaskStartScheduler();

    // should never return
    while(1);
}

// loop not use
void loop(){}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

static void TaskCommon(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    //char _c ; 
    uint8_t frame12Rec[P_SERSCOMMON_FRAME_LENGTH] = {0};
    uint8_t count1 = 0;
    uint8_t count2 = 0;
    for (;;) 
    {
        // if (P_SerCommon.available()  > 0)
        // {
        //     _c  = P_SerCommon.read();
        //     switch (_c)
        //     {
        //     case '1':
        //         levelSensor_setStatus = LEVELSENSOR_STATUS_SET_HAVE_VOLUME;
        //         break;
        //     case '2':
        //         levelSensor_setStatus = LEVELSENSOR_STATUS_SET_NO_VOLUME;
        //         break;
        //     case '3':
        //         hopper_setStatus = P_HOPPER_MAX_PERCENT;
        //         break;
        //     case '4':
        //         hopper_setStatus = P_HOPPER_MIN_PERCENT;
        //         break;
        //     case '5':
        //         spinner_setStatus = P_SPINNER_MAX_RPM;
        //         break;
        //     case '6':
        //         spinner_setStatus = P_SPINNER_MAX_RPM/2;
        //         break;  
        //     case '7':
        //         spinner_setStatus = P_SPINNER_MIN_RPM;
        //         break;   
        //     case '8':
        //         scale_setTask = STEP5;
        //         break;
        //     case '9':
        //         scale_setTask = STEP6;
        //         break;  
        //     case '0':
        //         scale_setTask = STEP7;
        //         frame_StatusTask.data2 = 10;
        //         break;  
        //     default:
        //         break;
        //     }
        // }
        
        // digitalWrite(P_SERSCOMMON_PINDIR, HIGH);
        // P_SerCommon.print("H: ");
        // P_SerCommon.print(hopper_getStatus);
        // P_SerCommon.print(" S: ");
        // P_SerCommon.print(spinner_getStatus);
        // P_SerCommon.print(" L: ");
        // P_SerCommon.print(levelSensor_getStatus);
        // P_SerCommon.print(" S1: ");
        // P_SerCommon.print(scale_task_curr);
        // P_SerCommon.print(" S2: ");
        // P_SerCommon.println(scale_value_curr);
        // P_SerCommon.flush();
        // digitalWrite(P_SERSCOMMON_PINDIR, LOW);
        // vTaskDelay((20L * configTICK_RATE_HZ) / 1000L);
        if(P_SerCommon.available() > 0)
        {
            if (P_SerCommon.peek() == P_SERSCOMMON_SLAVEID)
            {
                P_SerCommon.readBytes(frame12Rec, P_SERSCOMMON_FRAME_LENGTH);
                frame12_Receive.id    = frame12Rec[0];
                frame12_Receive.func  = frame12Rec[1];
                frame12_Receive.data1 = frame12Rec[2];
                frame12_Receive.data2 = frame12Rec[3];
                frame12_Receive.data3 = frame12Rec[4];
                frame12_Receive.data4 = frame12Rec[5];
                frame12_Receive.data5 = frame12Rec[6];
                frame12_Receive.data6 = frame12Rec[7];
                frame12_Receive.data7 = frame12Rec[8];
                frame12_Receive.data8 = frame12Rec[9];
                frame12_Receive.crcH  = frame12Rec[10];
                frame12_Receive.crcL  = frame12Rec[11];
                bool _check_frame = true;
                // CRC check
                uint16_t crc = ((frame12_Receive.crcH  << 8) | frame12_Receive.crcL);
                if (crc != EX_FRAME12_CALC_CRC(frame12_Receive)) _check_frame = false;
                
                if (_check_frame) // reply to master
                {
                    switch (frame12_Receive.func)
                    {
                    case MODBUS_FUNCTION_CHECK_CONNECT:
                        frame12_Checkconnect.data1 = MODBUS_FUNCTION_CHECK_CONNECT;
                        frame12_Checkconnect.crcH = EX_FRAME12_CALC_CRC(frame12_Checkconnect) >> 8;
                        frame12_Checkconnect.crcL = EX_FRAME12_CALC_CRC(frame12_Checkconnect) & 0xFF;
                        MODBUS_sendFrame(frame12_Checkconnect, &P_SerCommon, P_SERSCOMMON_PINDIR);
                        break;
                    case MODBUS_FUNCTION_STATUS_TASK:
                        hopper_setStatus = frame12_Receive.data1;
                        spinner_setStatus = (uint16_t)((frame12_Receive.data2  << 8) | frame12_Receive.data3);
                        levelSensor_setStatus = frame12_Receive.data4;
                        scale_setTask = frame12_Receive.data5;
                        scale_setValue = frame12_Receive.data6;
                        frame12_StatusTask.data1 = hopper_getStatus;
                        frame12_StatusTask.data2 = (uint8_t) (spinner_getStatus >> 8);
                        frame12_StatusTask.data3 = (uint8_t) (spinner_getStatus & 0xFF);
                        frame12_StatusTask.data4 = levelSensor_getStatus;
                        frame12_StatusTask.data5 = scale_task_curr;
                        frame12_StatusTask.data6 = scale_value_curr_byte1;
                        frame12_StatusTask.data7 = scale_value_curr_byte2;
                        frame12_StatusTask.data8 = scale_value_curr_byte3;
                        frame12_StatusTask.crcH = EX_FRAME12_CALC_CRC(frame12_StatusTask) >> 8;
                        frame12_StatusTask.crcL = EX_FRAME12_CALC_CRC(frame12_StatusTask) & 0xFF;
                        MODBUS_sendFrame(frame12_StatusTask, &P_SerCommon, P_SERSCOMMON_PINDIR);
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
                P_SerCommon.read();
            }
        }
        count1++;
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskScale(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    uint16_t period = 0;
    uint8_t frameRec[P_SERSCALE_FRAME_LENGTH] = {0};
    uint8_t count1 = 0;
    uint8_t count2 = 0;
    for (;;) 
    {
        // // handle response SLAVE
        if(P_SerScale.available() > 0)
        {
            if (P_SerScale.peek() == P_SERSCALE_SLAVEID)
            {
                P_SerScale.readBytes(frameRec, P_SERSCALE_FRAME_LENGTH);
                frame_Receive.id    = frameRec[0];
                frame_Receive.func  = frameRec[1];
                frame_Receive.data1 = frameRec[2];
                frame_Receive.data2 = frameRec[3];
                frame_Receive.data3 = frameRec[4];
                frame_Receive.data4 = frameRec[5];
                frame_Receive.crcH  = frameRec[6];
                frame_Receive.crcL  = frameRec[7];
                bool _check_frame = true;
                // CRC check
                uint16_t crc = ((frame_Receive.crcH  << 8) | frame_Receive.crcL);
                if (crc != EX_FRAME_CALC_CRC(frame_Receive)) _check_frame = false;
                if (_check_frame) // handle SLAVE value
                {
                    switch (frame_Receive.func)
                    {
                    case SCALE_MODBUS_FUNCTION_CHECK_CONNECT:
                        if(frame_Receive.data1 == SCALE_MODBUS_FUNCTION_CHECK_CONNECT)
                        {
                            //COMMON_sendString("scaOK\n", &P_SerCommon, P_SERSCOMMON_PINDIR);
                            scale_checkConnect = true;
                        }   
                        break;
                    case SCALE_MODBUS_FUNCTION_STATUS_TASK:
                        if (scale_checkConnect)
                        {
                            scale_task_curr = frame_Receive.data1;
                            scale_value_curr_byte1 = frame_Receive.data2;
                            scale_value_curr_byte2 = frame_Receive.data3;
                            scale_value_curr_byte3 = frame_Receive.data4;
                            if (scale_task_curr == STEP4)
                            {
                                scale_value_curr = (double)((int16_t)((frame_Receive.data2 << 8) | frame_Receive.data3)) + (double) frame_Receive.data4 / 100.0;
                            }
                            else
                            {
                                scale_value_curr = (double)frame_Receive.data4;
                                scale_setTask = STEP4;
                            }
                        }
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
            else{ P_SerScale.read();}
        }
        // handle request SLAVE
        if(period == P_SERSCALE_PERIOD_GET_VALUE)
        {
            // Handle frame 1. check connect
            frame_Checkconnect.crcH = EX_FRAME_CALC_CRC(frame_Checkconnect) >> 8;
            frame_Checkconnect.crcL = EX_FRAME_CALC_CRC(frame_Checkconnect) & 0xFF;
            SCALE_sendFrame(frame_Checkconnect, &P_SerScale, P_SERSCALE_PINDIR);
            scale_checkConnect = false;
            vTaskDelay(P_SERSCALE_BAUDRATE * 20/P_SERSCALE_BAUDRATE);

            // Handle frame 2. status task
            frame_StatusTask.data1 = scale_setTask;
            frame_StatusTask.data2 = scale_setValue;
            frame_StatusTask.crcH = EX_FRAME_CALC_CRC(frame_StatusTask) >> 8;
            frame_StatusTask.crcL = EX_FRAME_CALC_CRC(frame_StatusTask) & 0xFF;
            SCALE_sendFrame(frame_StatusTask, &P_SerScale, P_SERSCALE_PINDIR);
            
            period = 0;
        }
        period++;
        count1++;
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskLevelSensor(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    String _haveVolume;
    for (;;) 
    {
        // get status
        P_LevelSensor.readVolume2(&_haveVolume);
        if(_haveVolume == "1")
        {
            levelSensor_getStatus = LEVELSENSOR_STATUS_NO_VOLUME;
        }
        else if(_haveVolume == "0")
        {
            levelSensor_getStatus = LEVELSENSOR_STATUS_HAVE_VOLUME;
        }
        else if(_haveVolume == "3")
        {
            levelSensor_getStatus = LEVELSENSOR_STATUS_NO_CONNECT;
        }
        // set status
        if(levelSensor_setStatus == LEVELSENSOR_STATUS_SET_NO_VOLUME)
        {
            vTaskSuspendAll();
            P_LevelSensor.startMotor2(true);  
            levelSensor_setStatus = LEVELSENSOR_STATUS_INIT;
            xTaskResumeAll();
        }
        else if (levelSensor_setStatus == LEVELSENSOR_STATUS_SET_HAVE_VOLUME)
        {
            vTaskSuspendAll();
            P_LevelSensor.startMotor2(false);
            levelSensor_setStatus = LEVELSENSOR_STATUS_INIT;
            xTaskResumeAll();
        }
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskHopper(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    uint8_t _lastStatus;
    for (;;) 
    {
        if (_lastStatus != hopper_setStatus)
        {

            P_Hopper.writeMicroseconds(map( hopper_setStatus, 
                                            P_HOPPER_MIN_PERCENT, 
                                            P_HOPPER_MAX_PERCENT, 
                                            P_HOPPER_MIN_VALUE, 
                                            P_HOPPER_MAX_VALUE));
            _lastStatus = hopper_setStatus;
        }
        hopper_getStatus = map( P_Hopper.readMicroseconds(), 
                                P_HOPPER_MIN_VALUE, 
                                P_HOPPER_MAX_VALUE, 
                                P_HOPPER_MIN_PERCENT, 
                                P_HOPPER_MAX_PERCENT);

        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskSpinner(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    unsigned long _time = HAL_GetTick();
    uint16_t _lastStatus = P_SPINNER_MIN_VALUE;
    uint16_t _setStatus = P_SPINNER_MIN_VALUE;
    for (;;) 
    {
        _setStatus = map(   spinner_setStatus, 
                            P_SPINNER_MIN_RPM, 
                            P_SPINNER_MAX_RPM, 
                            P_SPINNER_MIN_VALUE, 
                            P_SPINNER_MAX_VALUE);
        if (((unsigned long)(HAL_GetTick() - _time)> P_SPINNER_TIME_ACCEL) && (_lastStatus != _setStatus))
        {
            if (_setStatus > _lastStatus)
            {
                _lastStatus += P_SPINNER_ACCEL;
            }
            else if (_setStatus < _lastStatus)
            {
                _lastStatus -= P_SPINNER_ACCEL;
            }
            else
            {
                _lastStatus = _setStatus;
            }
            P_Spinner.writeMicroseconds(_lastStatus); 
        }
        spinner_getStatus = map(P_Spinner.readMicroseconds(), 
                                P_SPINNER_MIN_VALUE, 
                                P_SPINNER_MAX_VALUE, 
                                P_SPINNER_MIN_RPM, 
                                P_SPINNER_MAX_RPM);
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskCalibSystem(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    for (;;) 
    {
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskLedStatus(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    for (;;) 
    {
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        vTaskDelay( 300 / portTICK_PERIOD_MS ); // wait for one second
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        vTaskDelay( 300 / portTICK_PERIOD_MS ); // wait for one second
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

void EX_FRAME12_CONFIG( frame12_t *_frame, uint8_t _slaveID, uint8_t _function)
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

uint16_t EX_FRAME12_CALC_CRC(frame12_t _frame) {
	uint8_t CRCHi = 0xFF, CRCLo = 0x0FF, Index;
    Index = CRCHi ^ _frame.id; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.func; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data1; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data2; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data3; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data4; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data5; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data6; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data7; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    Index = CRCHi ^ _frame.data8; CRCHi = CRCLo ^ _auchCRCHi[Index]; CRCLo = _auchCRCLo[Index];
    return (CRCHi << 8) | CRCLo;
}
/*======================================== Function Scale ====================================*/

void SCALE_sendFrame(frame_t _frame, HardwareSerial *_port , uint8_t _pinDir)
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

void MODBUS_sendFrame(frame12_t _frame, HardwareSerial *_port , uint8_t _pinDir)
{
    digitalWrite(_pinDir, HIGH);
    (*_port).write(_frame.id);
    (*_port).write(_frame.func);
    (*_port).write(_frame.data1);
    (*_port).write(_frame.data2);
    (*_port).write(_frame.data3);
    (*_port).write(_frame.data4);
    (*_port).write(_frame.data5);
    (*_port).write(_frame.data6);
    (*_port).write(_frame.data7);
    (*_port).write(_frame.data8);
    (*_port).write(_frame.crcH);
    (*_port).write(_frame.crcL);
    (*_port).flush();
    digitalWrite(_pinDir, LOW);
}

/*======================================== Function Common ====================================*/

void COMMON_sendString(char *_str, HardwareSerial *_port , uint8_t _pinDir)
{
    digitalWrite(_pinDir, HIGH);
    (*_port).println(_str);
    (*_port).flush();
    digitalWrite(_pinDir, LOW);
}