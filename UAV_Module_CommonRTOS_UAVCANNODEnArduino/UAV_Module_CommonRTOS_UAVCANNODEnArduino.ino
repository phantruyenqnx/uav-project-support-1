#include "src/define.hpp"

/*
Project have 4 tasks:                Piority                 Stacksize
1. UAVCAN communicate                      4                       256
2. UAVCAN middleware                       3                       100
3. Modbus communicate                      3                       100
4. Led Status                              2                       50
*/

//define task handles
TaskHandle_t TaskUAVCANCommunicate_Handler;
TaskHandle_t TaskUAVCANMiddleware_Handler;
TaskHandle_t TaskModbus_Handler;
TaskHandle_t TaskLedStatus_Handler;
// define task
static void TaskUAVCANCommunicate( void *pvParameters );
static void TaskUAVCANMiddleware( void *pvParameters );
static void TaskModbus( void *pvParameters );
static void TaskLedStatus( void *pvParameters );
// define Queue
QueueHandle_t qStatus;


// uavcan function
void showRcpwmonUart(void);
void sendCanard(void);
void receiveCanard(void);
void spinCanard(void);
void publishCanard(void);
void showRcpwmonUart(void);
void rawcmdHandleCanard(CanardRxTransfer* transfer);
void getsetHandleCanard(CanardRxTransfer* transfer);
void getNodeInfoHandleCanard(CanardRxTransfer* transfer);
uint16_t makeNodeInfoMessage(uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE]);

// setup
void setup()
{
    /*------------------------ Init System ----------------------*/

    // can init
    CAN_HW_Init();
    uavcanInit();

    // Modbus init
    P_Modbus.begin(P_MODBUS_BAUDRATE); 
    while(!P_Modbus) {}
    pinMode(P_MODBUS_PINDIR, OUTPUT);
    digitalWrite(P_MODBUS_PINDIR, LOW);
    // Modbus frame define
    EX_FRAME_CONFIG(&frame_Checkconnect, P_MODBUS_SLAVEID, MODBUS_FUNCTION_CHECK_CONNECT);
    EX_FRAME_CONFIG(&frame_StatusTask, P_MODBUS_SLAVEID, MODBUS_FUNCTION_STATUS_TASK);

    // Serial Debug UART3
    P_SerDebug.begin(P_SerDebug_BAUDRATE); 
    while(!P_SerDebug) {}
    P_SerDebug.print(INIT_BOOT_STRING);
    
    // led status
    pinMode(LED_BUILTIN, OUTPUT);

    /*-- Create queue --*/
    qStatus = xQueueCreate(1, sizeof(struct system));
    /*-- Create Task --*/

    xTaskCreate(    TaskUAVCANCommunicate,
                    "UAVCANCommunicate",
                    UAVCAN_COM_STACKSIZE,
                    nullptr,
                    PRIO4,
                    &TaskUAVCANCommunicate_Handler);
    xTaskCreate(    TaskUAVCANMiddleware,
                    "UAVCANMiddleware",
                    UAVCAN_MIDDLE_STACKSIZE,
                    nullptr,
                    PRIO2,
                    &TaskUAVCANMiddleware_Handler);
    xTaskCreate(    TaskModbus,
                    "Modbus",
                    UAVCAN_MODBUS_STACKSIZE,
                    nullptr,
                    PRIO3,
                    &TaskModbus_Handler);   
    xTaskCreate(    TaskLedStatus,
                    "LedStatus",
                    LEDSTATUS_STACKSIZE,
                    nullptr,
                    PRIO1,
                    &TaskLedStatus_Handler);   
     /*-- Start FreeRTOS --*/
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

static void TaskUAVCANCommunicate(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    unsigned long previousMillis = HAL_GetTick(); 
    for (;;) 
    {
        sendCanard();
        receiveCanard();
        spinCanard();
        publishCanard();
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskUAVCANMiddleware(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    uint8_t _countPrint = 0;
    char _c ;
    uint16_t _spinner = 0;
    uint8_t _last_setTask = 0;
    int16_t _last_pwm2 = 0;
    int16_t _last_pwm3 = 0;
    for (;;) 
    {
        hopper_setStatus = (abs(rc_pwm[0]) > 100) ? 100 : abs(rc_pwm[0]);
        spinner_setStatus = (abs(rc_pwm[1]) > 1300) ? 1300 : abs(rc_pwm[1]);
        // frameHandle_setStatus.data2 = (uint8_t)(_spinner >> 8);
        // frameHandle_setStatus.data3 = (uint8_t)(_spinner & 0xFF);
        if (_last_pwm2 != abs(rc_pwm[2]))
        {
          if (abs(rc_pwm[2]) == 0)
          {
            levelSensor_setStatus = 0;
          }
          else if (abs(rc_pwm[2]) == 81)
          {
            levelSensor_setStatus = 5;
            P_SerDebug.println("Level V");
          }
          else if (abs(rc_pwm[2]) == 163)
          {
            levelSensor_setStatus = 4;
          }
          _last_pwm2 = abs(rc_pwm[2]);
        }

        //levelSensor_setStatus = (abs(rc_pwm[2]) > 0) ? 5 : 4;
        if (_last_pwm3 != abs(rc_pwm[3]))
        {
          if (abs(rc_pwm[3]) == 81)
          {
            //frameHandle_setStatus.data5 = 6;
            scale_setTask = STEP6;
            P_SerDebug.println("Scale Tare");
          }
          else if (abs(rc_pwm[3]) == 163)
          {
            //frameHandle_setStatus.data5 = 5;
            scale_setTask = STEP5;
            P_SerDebug.println("Scale Zero");
          }
          else if (abs(rc_pwm[3]) == 245)
          {
            // frameHandle_setStatus.data5 = 7; frameHandle_setStatus.data6 = 10;
            scale_setTask = STEP7;
            scale_setValue = 10;
            P_SerDebug.println("Scale Weight");
          }
          // else
          // {
          //   frameHandle_setStatus.data5 = 0;
          //   frameHandle_setStatus.data6 = 0;
          // }
          _last_pwm3 = abs(rc_pwm[3]) ;
        }
        

        if (_countPrint == 20)
        {
          P_SerDebug.print("H: ");
          P_SerDebug.print(frameHandle_getStatus.data1);
          P_SerDebug.print(" S: ");
          P_SerDebug.print(((frameHandle_getStatus.data2  << 8) | frameHandle_getStatus.data3));
          P_SerDebug.print(" L: ");
          P_SerDebug.print(frameHandle_getStatus.data4);
          P_SerDebug.print(" S1: ");
          P_SerDebug.print(frameHandle_getStatus.data5);
          P_SerDebug.print(" S2: ");
          if (frameHandle_getStatus.data5 != STEP4)
          {
            P_SerDebug.println(frameHandle_getStatus.data8);
          }
          else
          {
            P_SerDebug.println((double)((int16_t)((frameHandle_getStatus.data6 << 8) | frameHandle_getStatus.data7)) + (double) frameHandle_getStatus.data8 / 100.0);
          }
          P_SerDebug.flush();
          _countPrint = 0;
        }
        _countPrint ++ ;
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

static void TaskModbus(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    uint8_t frameRec[P_MODBUS_FRAME_LENGTH] = {0};
    uint8_t count1 = 0;
    uint8_t count2 = 0;
    uint16_t period = 0;
    for (;;) 
    {
        // // handle response SLAVE
        if(P_Modbus.available() > 0)
        {
            if (P_Modbus.peek() == P_MODBUS_SLAVEID)
            {
                P_Modbus.readBytes(frameRec, P_MODBUS_FRAME_LENGTH);
                frame_Receive.id    = frameRec[0];
                frame_Receive.func  = frameRec[1];
                frame_Receive.data1 = frameRec[2];
                frame_Receive.data2 = frameRec[3];
                frame_Receive.data3 = frameRec[4];
                frame_Receive.data4 = frameRec[5];
                frame_Receive.data5 = frameRec[6];
                frame_Receive.data6 = frameRec[7];
                frame_Receive.data7 = frameRec[8];
                frame_Receive.data8 = frameRec[9];
                frame_Receive.crcH  = frameRec[10];
                frame_Receive.crcL  = frameRec[11];
                bool _check_frame = true;
                // CRC check
                uint16_t crc = ((frame_Receive.crcH  << 8) | frame_Receive.crcL);
                if (crc != EX_FRAME_CALC_CRC(frame_Receive)) _check_frame = false;
                if (_check_frame) // handle SLAVE value
                {
                    switch (frame_Receive.func)
                    {
                    case MODBUS_FUNCTION_CHECK_CONNECT:
                        if(frame_Receive.data1 == MODBUS_FUNCTION_CHECK_CONNECT)
                        {
                          modbus_checkConnect = true;
                          frameHandle_getStatus.id = P_MODBUS_SLAVEID;
                        }   
                        break;
                    case MODBUS_FUNCTION_STATUS_TASK:
                        if (modbus_checkConnect)
                        {
                          memcpy(&frameHandle_getStatus, &frame_Receive, sizeof(frame_Receive));
                          if (frame_Receive.data5 != STEP4)
                          {
                            scale_setTask = STEP4;
                          }
                          levelSensor_setStatus = STEP0;
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
            else{ P_Modbus.read();}
        }
        // handle request SLAVE
        if(period == P_MODBUS_PERIOD_GET_VALUE)
        {
            // Handle frame 1. check connect
            frame_Checkconnect.crcH = EX_FRAME_CALC_CRC(frame_Checkconnect) >> 8;
            frame_Checkconnect.crcL = EX_FRAME_CALC_CRC(frame_Checkconnect) & 0xFF;
            MODBUS_sendFrame(frame_Checkconnect, &P_Modbus, P_MODBUS_PINDIR);
            modbus_checkConnect = false;
            vTaskDelay(P_MODBUS_BAUDRATE * 2/P_MODBUS_BAUDRATE);

            // Handle frame 2. status task
            // frame_StatusTask.data1 = frameHandle_setStatus.data1;
            // frame_StatusTask.data2 = frameHandle_setStatus.data2;
            // frame_StatusTask.data3 = frameHandle_setStatus.data3;
            // frame_StatusTask.data4 = frameHandle_setStatus.data4;
            // frame_StatusTask.data5 = frameHandle_setStatus.data5;
            // frame_StatusTask.data6 = frameHandle_setStatus.data6;
            // frame_StatusTask.data7 = frameHandle_setStatus.data7;
            // frame_StatusTask.data8 = frameHandle_setStatus.data8;
            frame_StatusTask.data1 = hopper_setStatus;
            frame_StatusTask.data2 = (uint8_t)(spinner_setStatus >> 8);
            frame_StatusTask.data3 = (uint8_t)(spinner_setStatus & 0xFF);
            frame_StatusTask.data4 = levelSensor_setStatus;
            frame_StatusTask.data5 = scale_setTask;
            frame_StatusTask.data6 = scale_setValue;
            frame_StatusTask.crcH = EX_FRAME_CALC_CRC(frame_StatusTask) >> 8;
            frame_StatusTask.crcL = EX_FRAME_CALC_CRC(frame_StatusTask) & 0xFF;
            MODBUS_sendFrame(frame_StatusTask, &P_Modbus, P_MODBUS_PINDIR);
            
            period = 0;
        }
        period++;
        count1++;
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

uint16_t EX_FRAME_CALC_CRC(frame_t _frame) {
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


/*======================================== Function Modbus ====================================*/

void MODBUS_sendFrame(frame_t _frame, HardwareSerial *_port , uint8_t _pinDir)
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

/*======================================== Function 1 ====================================*/
// can init

void CAN_HW_Init(void) {

  GPIO_InitTypeDef GPIO_InitStruct;

  // GPIO Ports Clock Enable
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // CAN1 clock enable
  __HAL_RCC_CAN1_CLK_ENABLE();
  
  // CAN GPIO Configuration    
  // PA11     ------> CAN_RX
  // PA12     ------> CAN_TX 

#if defined (STM32F103x6) || defined (STM32F103xB)
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#elif defined (STM32F303x8) || defined (STM32F303xC) || defined (STM32F303xE)
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_CAN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#else
#error "Warning untested processor variant"
#endif

}

// 
bool shouldAcceptTransfer(const CanardInstance* ins,
                          uint64_t* out_data_type_signature,
                          uint16_t data_type_id,
                          CanardTransferType transfer_type,
                          uint8_t source_node_id)
{

  if ((transfer_type == CanardTransferTypeRequest) &&(data_type_id == UAVCAN_GET_NODE_INFO_DATA_TYPE_ID)) {
    *out_data_type_signature = UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE;
    return true;
  }
  if (data_type_id == UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID) {
    *out_data_type_signature = UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE;
    return true;
  }
  if (data_type_id == UAVCAN_PROTOCOL_PARAM_GETSET_ID) {
    *out_data_type_signature = UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE;
    return true;
  }
  return false;

}

//////////////////////////////////////////////////////////////////////////////////////

void onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer)
{

  if ((transfer->transfer_type == CanardTransferTypeRequest) && (transfer->data_type_id == UAVCAN_GET_NODE_INFO_DATA_TYPE_ID)) {
    getNodeInfoHandleCanard(transfer);
  } 

  if (transfer->data_type_id == UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID) {
    rawcmdHandleCanard(transfer);
  }

  if (transfer->data_type_id == UAVCAN_PROTOCOL_PARAM_GETSET_ID) {
    getsetHandleCanard(transfer);
  }
    
}


void getNodeInfoHandleCanard(CanardRxTransfer* transfer)
{

  uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE];
  memset(buffer,0,UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE);
  uint16_t len = makeNodeInfoMessage(buffer);
  int result = canardRequestOrRespond(&g_canard,
                                      transfer->source_node_id,
                                      UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE,
                                      UAVCAN_GET_NODE_INFO_DATA_TYPE_ID,
                                      &transfer->transfer_id,
                                      transfer->priority,
                                      CanardResponse,
                                      &buffer[0],
                                      (uint16_t)len);

}


void uavcanInit(void)
{

  CanardSTM32CANTimings timings;
  int result = canardSTM32ComputeCANTimings(HAL_RCC_GetPCLK1Freq(), 1000000, &timings);
  if (result) {
    __ASM volatile("BKPT #01");
  }
  result = canardSTM32Init(&timings, CanardSTM32IfaceModeNormal);
  if (result) {
    __ASM volatile("BKPT #01");
  }
 
  canardInit(&g_canard,                         // Uninitialized library instance
             g_canard_memory_pool,              // Raw memory chunk used for dynamic allocation
             sizeof(g_canard_memory_pool),      // Size of the above, in bytes
             onTransferReceived,                // Callback, see CanardOnTransferReception
             shouldAcceptTransfer,              // Callback, see CanardShouldAcceptTransfer
             nullptr);
 
  canardSetLocalNodeID(&g_canard, CANARD_LOCAL_NODE_ID);

}


void sendCanard(void)
{

  const CanardCANFrame* txf = canardPeekTxQueue(&g_canard); 

  while(txf) {
    const int tx_res = canardSTM32Transmit(txf);
    if (tx_res < 0) {                 // Failure - drop the frame and report
      __ASM volatile("BKPT #01");     // TODO: handle the error properly
    }
    if(tx_res > 0) {
      canardPopTxQueue(&g_canard);
    }
    txf = canardPeekTxQueue(&g_canard); 
  }

}


void receiveCanard(void)
{

    CanardCANFrame rx_frame;
    int res = canardSTM32Receive(&rx_frame);
    if(res) {
        canardHandleRxFrame(&g_canard, &rx_frame, HAL_GetTick() * 1000);
    }    

}

void spinCanard(void)
{  

    static uint32_t spin_time = 0;

    if(HAL_GetTick() < spin_time + CANARD_SPIN_PERIOD) return;    // rate limiting
    
    spin_time = HAL_GetTick();
    //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        
    uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE];    
    static uint8_t transfer_id = 0;                               // This variable MUST BE STATIC; refer to the libcanard documentation for the background
    makeNodeStatusMessage(buffer);  
    canardBroadcast(&g_canard, 
                    UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE,
                    UAVCAN_NODE_STATUS_DATA_TYPE_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer, 
                    UAVCAN_NODE_STATUS_MESSAGE_SIZE);             //some indication
    
}


void publishCanard(void)
{  

  static uint32_t publish_time = 0;
  static int step = 0;
  if(HAL_GetTick() < publish_time + PUBLISHER_PERIOD_mS) {return;} // rate limiting
  publish_time = HAL_GetTick();

  uint8_t buffer[UAVCAN_PROTOCOL_DEBUG_KEYVALUE_MESSAGE_SIZE];
  UC_sendFrame("H", (float)frameHandle_getStatus.data1, buffer);
  UC_sendFrame("S", (float)((frameHandle_getStatus.data2  << 8) | frameHandle_getStatus.data3), buffer);
  UC_sendFrame("L", (float)frameHandle_getStatus.data4, buffer);
  UC_sendFrame("S1", (float)frameHandle_getStatus.data5, buffer);
  if (frameHandle_getStatus.data5 != STEP4){ UC_sendFrame("S2", (float)frameHandle_getStatus.data8, buffer);}
  else{UC_sendFrame("S2", (float)((int16_t)((frameHandle_getStatus.data6 << 8) | frameHandle_getStatus.data7)) + (double) frameHandle_getStatus.data8 / 100.0, buffer);}
}

void UC_sendFrame(const char *key, float _value, uint8_t _buffer[62] )
{
  memset(_buffer,0x00,UAVCAN_PROTOCOL_DEBUG_KEYVALUE_MESSAGE_SIZE);
  static uint8_t transfer_id = 0;
  canardEncodeScalar(_buffer, 0, 32, &_value);
  memcpy(&_buffer[4], key, 3);  
  canardBroadcast(&g_canard, 
                  UAVCAN_PROTOCOL_DEBUG_KEYVALUE_SIGNATURE,
                  UAVCAN_PROTOCOL_DEBUG_KEYVALUE_ID,
                  &transfer_id,
                  CANARD_TRANSFER_PRIORITY_LOW,
                  &_buffer[0], 
                  7);
}

void makeNodeStatusMessage(uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE])
{

    uint8_t node_health = UAVCAN_NODE_HEALTH_OK;
    uint8_t node_mode   = UAVCAN_NODE_MODE_OPERATIONAL;
    memset(buffer, 0, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
    uint32_t uptime_sec = (HAL_GetTick() / 1000);
    canardEncodeScalar(buffer,  0, 32, &uptime_sec);
    canardEncodeScalar(buffer, 32,  2, &node_health);
    canardEncodeScalar(buffer, 34,  3, &node_mode);

}


uint16_t makeNodeInfoMessage(uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE])
{

  memset(buffer, 0, UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE);
  makeNodeStatusMessage(buffer);
   
  buffer[7] = APP_VERSION_MAJOR;
  buffer[8] = APP_VERSION_MINOR;
  buffer[9] = 1;                            // Optional field flags, VCS commit is set
  uint32_t u32 = GIT_HASH;
  canardEncodeScalar(buffer, 80, 32, &u32); 
    
  readUniqueID(&buffer[24]);
  const size_t name_len = strlen(APP_NODE_NAME);
  memcpy(&buffer[41], APP_NODE_NAME, name_len);
  return 41 + name_len ;

}


void readUniqueID(uint8_t* out_uid)
{

  for (uint8_t i = 0; i < UNIQUE_ID_LENGTH_BYTES; i++) {
    out_uid[i] = i;
  }

}


void rawcmdHandleCanard(CanardRxTransfer* transfer)
{
    
  int offset = 0;
  for (int i = 0; i<6; i++) {
    if (canardDecodeScalar(transfer, offset, 14, true, &rc_pwm[i])<14) { break; }
    offset += 14;
  }
  // rcpwmUpdate(ar);

}


void showRcpwmonUart()
{

    //char str[5];
    //itoa(rc_pwm[0],str,10);
    //HAL_UART_Transmit(&huart1,str,5,0xffff);
    //HAL_UART_Transmit(&huart1,"\n",2,0xffff);
    printArray(rc_pwm);

}


void printArray( int16_t arr[]) {
    char str[20];
    itoa(arr[0], str, 10);
    P_SerDebug.print("ESC Array: ");
    P_SerDebug.print(str);
    for (int i = 1; i < 4; i++) {
    itoa(arr[i], str, 10);
    P_SerDebug.print(", ");
    P_SerDebug.print(str);
    }
    P_SerDebug.println();
}


inline param_t * getParamByIndex(uint16_t index)
{

  if(index >= ARRAY_SIZE(parameters))  {
    return nullptr;
  }
  return &parameters[index];

}


inline param_t * getParamByName(uint8_t * name)
{

  for(uint16_t i = 0; i < ARRAY_SIZE(parameters); i++)
  {
    if(strncmp((char const*)name, (char const*)parameters[i].name,strlen((char const*)parameters[i].name)) == 0) {
      return &parameters[i];
    }
  }      
  return nullptr;

}


uint16_t encodeParamCanard(param_t * p, uint8_t * buffer)
{

  uint8_t n     = 0;
  int offset    = 0;
  uint8_t tag   = 1;

  if(p==nullptr) {   
    tag = 0;
    canardEncodeScalar(buffer, offset, 5, &n);
    offset += 5;
    canardEncodeScalar(buffer, offset,3, &tag);
    offset += 3;
        
    canardEncodeScalar(buffer, offset, 6, &n);
    offset += 6;
    canardEncodeScalar(buffer, offset,2, &tag);
    offset += 2;

    canardEncodeScalar(buffer, offset, 6, &n);
    offset += 6;
    canardEncodeScalar(buffer, offset, 2, &tag);
    offset += 2;
    buffer[offset / 8] = 0;
    return ( offset / 8 + 1 );

  }

  canardEncodeScalar(buffer, offset, 5,&n);
  offset += 5;
  canardEncodeScalar(buffer, offset, 3, &tag);
  offset += 3;
  canardEncodeScalar(buffer, offset, 64, &p->val);
  offset += 64;

  canardEncodeScalar(buffer, offset, 5, &n);
  offset += 5;
  canardEncodeScalar(buffer, offset, 3, &tag);
  offset += 3;
  canardEncodeScalar(buffer, offset, 64, &p->defval);
  offset += 64;
    
  canardEncodeScalar(buffer, offset, 6, &n);
  offset += 6;
  canardEncodeScalar(buffer, offset, 2, &tag);
  offset += 2;
  canardEncodeScalar(buffer, offset, 64, &p->max);
  offset += 64;
    
  canardEncodeScalar(buffer, offset, 6, &n);
  offset += 6;
  canardEncodeScalar(buffer, offset,2,&tag);
  offset += 2;
  canardEncodeScalar(buffer, offset,64,&p->min);
  offset += 64;
    
  memcpy(&buffer[offset / 8], p->name, strlen((char const*)p->name));
  return  (offset/8 + strlen((char const*)p->name)); 

}


void getsetHandleCanard(CanardRxTransfer* transfer)
{

  uint16_t index = 0xFFFF;
  uint8_t tag    = 0;
  int offset     = 0;
  int64_t val    = 0;

  canardDecodeScalar(transfer, offset,  13, false, &index);
  offset += 13;
  canardDecodeScalar(transfer, offset, 3, false, &tag);
  offset += 3;

  if(tag == 1) {
    canardDecodeScalar(transfer, offset, 64, false, &val);
    offset += 64;
  }

  uint16_t n = transfer->payload_len - offset / 8 ;
  uint8_t name[16]      = "";
  for(int i = 0; i < n; i++) {
    canardDecodeScalar(transfer, offset, 8, false, &name[i]);
    offset += 8;
  }

  param_t * p = nullptr;

  if(strlen((char const*)name)) {
    p = getParamByName(name);
  } else {
    p = getParamByIndex(index);
  }

  if((p)&&(tag == 1)) {
    p->val = val;
  }

  uint8_t  buffer[64] = "";
  uint16_t len = encodeParamCanard(p, buffer);
  int result = canardRequestOrRespond(&g_canard,
                                      transfer->source_node_id,
                                      UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE,
                                      UAVCAN_PROTOCOL_PARAM_GETSET_ID,
                                      &transfer->transfer_id,
                                      transfer->priority,
                                      CanardResponse,
                                      &buffer[0],
                                      (uint16_t)len);

}