// Simple demo of three threads
// LED blink thread, print thread, and idle loop
#include <STM32FreeRTOS.h>
#include "src/configPIN.hpp"

const uint8_t LED_PIN = LED_BUILTIN;

volatile uint32_t count = 0;

HardwareSerial SerialBLE(dbugBLE);

// handle for blink task
TaskHandle_t blink;

//------------------------------------------------------------------------------
// high priority for blinking LED
static void vLEDFlashTask(void *pvParameters) {
  UNUSED(pvParameters);
  pinMode(LED_PIN, OUTPUT);

  // Flash led every 200 ms.
  for (;;) {
    // Turn LED on.
    digitalWrite(LED_PIN, HIGH);

    // Sleep for 50 milliseconds.
    vTaskDelay((50L * configTICK_RATE_HZ) / 1000L);

    // Turn LED off.
    digitalWrite(LED_PIN, LOW);

    // Sleep for 150 milliseconds.
    vTaskDelay((150L * configTICK_RATE_HZ) / 1000L);
  }
}
//------------------------------------------------------------------------------
static void vPrintTask(void *pvParameters) {
  UNUSED(pvParameters);
  while (1) {
    // Sleep for one second.
    vTaskDelay(configTICK_RATE_HZ);

    // Print count for previous second.
    SerialBLE.print(F("Count: "));
    SerialBLE.print(count);

    // Print unused stack for threads.
    SerialBLE.print(F(", Unused Stack: "));
    SerialBLE.print(uxTaskGetStackHighWaterMark(blink));
    SerialBLE.print(' ');
    SerialBLE.print(uxTaskGetStackHighWaterMark(0));
    SerialBLE.print(' ');
    SerialBLE.println(uxTaskGetStackHighWaterMark(xTaskGetIdleTaskHandle()));

    // Zero count.
    count = 0;
  }
}
//------------------------------------------------------------------------------
void setup() {
  SerialBLE.begin(115200);
  // wait for Leonardo
  while(!SerialBLE) {}

  // create blink task
  xTaskCreate(vLEDFlashTask,
    "Task1",
    configMINIMAL_STACK_SIZE + 50,
    NULL,
    tskIDLE_PRIORITY + 2,
    &blink);

  // create print task
  xTaskCreate(vPrintTask,
    "Task2",
    configMINIMAL_STACK_SIZE + 100,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL);

  // start FreeRTOS
  vTaskStartScheduler();

  // should never return
  SerialBLE.println(F("Die"));
  while(1);
}
//------------------------------------------------------------------------------
// WARNING idle loop has a very small stack (configMINIMAL_STACK_SIZE)
// loop must never block
void loop() {
  while(1) {
    // must insure increment is atomic
    // in case of context switch for print
    noInterrupts();
    count++;
    interrupts();
  }
}