#include "src/define.hpp"

/*
Project have ... tasks:                Piority                 Stacksize   
*/

//define task handles
TaskHandle_t TaskDefault_Handler;

// define task
static void TaskDefault( void *pvParameters );

// define Queue
QueueHandle_t qStatus;

// setup
void setup()
{
    /*------------------------ Init System ----------------------*/

    // Serial Debug UART3
    P_SerDebug.begin(P_SerDebug_BAUDRATE); 
    while(!P_SerDebug) {}
    P_SerDebug.println(INIT_BOOT_STRING);

    /*-- Create queue --*/
    qStatus = xQueueCreate(1, sizeof(struct system));
    /*-- Create Task --*/

    xTaskCreate(    TaskDefault,
                    "Default",
                    DEFAULT_STACKSIZE,
                    nullptr,
                    PRIO0,
                    &TaskDefault_Handler);

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

static void TaskDefault(void* pvParameters){
/*
    Explain Task:

*/
    (void) pvParameters;
    for (;;) 
    {
        vTaskDelay((1L * configTICK_RATE_HZ) / 1000L);
    }
}

/*--------------------------------------------------*/
/*---------------- Function Support -----------------*/
/*--------------------------------------------------*/

/*======================================== Function 1 ====================================*/
