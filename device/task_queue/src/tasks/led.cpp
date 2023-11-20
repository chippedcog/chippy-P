#include <Arduino.h>

#include "../pins.h"

// GLOBALS
// --- task data
struct TaskDataLED
{
  bool isRunning = false;
  int createdLastMs;
};
// --- instance
TaskDataLED taskDataLED;

//========================================

// TASK (executes)
void taskLED(void *param)
{
  Serial.println("TASK => taskLED: start");
  // --- flip the LED!
  if (digitalRead(PIN_LED_BUILTIN) == HIGH)
  {
    digitalWrite(PIN_LED_BUILTIN, LOW);
  }
  else
  {
    digitalWrite(PIN_LED_BUILTIN, HIGH);
  }
  // --- "done"
  taskDataLED.isRunning = false;
  // --- delete this task
  Serial.println("TASK => taskLED: delete");
  vTaskDelete(NULL);
}

// TASK CREATOR (checks if running before adding)
void taskLEDCreator()
{
  int currentMillis = millis();
  // Serial.printf("[%d] [taskLEDCreator]\n", currentMillis);
  if (!taskDataLED.isRunning)
  {
    taskDataLED.isRunning = true;
    taskDataLED.createdLastMs = currentMillis;
    // xTaskCreatePinnedToCore(taskLED, "taskLED", 1000, NULL, 1, NULL, 1);
    xTaskCreate(taskLED, "taskLED", 1000, NULL, 1, NULL);
  }
}