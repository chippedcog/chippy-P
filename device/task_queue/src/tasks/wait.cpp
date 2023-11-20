#include <Arduino.h>

#include "led.h"

// GLOBALS
// --- task data
struct TaskDataWait
{
  bool isRunning = false;
  int timerMs = 10000;
  int createdLastMs;
};
// --- instance
TaskDataWait taskDataWait;

//========================================

// TASK (executes)
void taskWait(void *param)
{
  Serial.println("TASK => taskWait: start");
  // --- simulate work being done
  vTaskDelay(pdMS_TO_TICKS(8000));
  taskLEDCreator(); // spawn another task from this task before it closes
  // LEARNING TIME
  // If task pinned to core 1, the LED task will start/end before this wait task ends
  // If task not pinned, this will end while the other core picks up the LED task
  // So pinning to core, if that task keeps creating tasks recursively, could lead to an overflow? I think.
  // But in any case, probably best not to pin so that both cores can take on work. Seems more efficient w/ less blocking.
  // --- "done"
  taskDataWait.isRunning = false;
  // --- delete this task
  Serial.println("TASK => taskWait: delete");
  vTaskDelete(NULL);
}

// TASK CREATOR (checks if running before adding)
void taskWaitCreator(TimerHandle_t xTimer)
{
  int currentMillis = millis();
  // Serial.printf("[%d] [taskWaitCreator]\n", currentMillis);
  if (!taskDataWait.isRunning)
  {
    taskDataWait.isRunning = true;
    taskDataWait.createdLastMs = currentMillis;
    // xTaskCreatePinnedToCore(taskWait, "taskWait", 1000, NULL, 1, NULL, 1);
    xTaskCreate(taskWait, "taskWait", 1000, NULL, 1, NULL);
  }
}