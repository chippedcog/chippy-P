#include <Arduino.h>

// GLOBALS
// --- task data (shareable state between timer, task creator, and task itself) maybe abstract
struct TaskDataPrint
{
  bool isRunning = false;
  int timerMs = 500;
  int createdLastMs;
};
// --- instance
TaskDataPrint taskDataPrint;

//========================================

// TASK (executes)
void taskPrint(void *param)
{
  // Serial.printing GOTCHAS
  // 1. DONT CALL MILLIS DIRECTLY HERE. This causes heap errors. This is why I save globally in the creator
  // 2. Template strings are often causing different weird errors at random times!
  // 3. Apparently, you can cause crashes if two things are trying to access Serial.print() at the same time
  Serial.println("TASK => taskPrint");
  // --- simulate work being done
  vTaskDelay(pdMS_TO_TICKS(2000));
  // --- "done" => our task is trying to be created every 500ms (timerMs above), but we're only unblocking when we set the global bool back to false)
  taskDataPrint.isRunning = false;
  // --- delete this task
  vTaskDelete(NULL);
}

// TASK CREATOR (checks if running before adding)
void taskPrintCreator(TimerHandle_t xTimer)
{
  int currentMillis = millis();
  // given our loop is printing every 1s, expect two of these messages per blank space (using timerMs above to indicate how often this is called)
  // Serial.printf("[%d] [taskPrintCreator]\n", currentMillis);
  if (!taskDataPrint.isRunning)
  {
    taskDataPrint.isRunning = true;
    taskDataPrint.createdLastMs = currentMillis;
    // xTaskCreatePinnedToCore(taskPrint, "taskPrint", 1000, NULL, 1, NULL, 1);
    xTaskCreate(taskPrint, "taskPrint", 1000, NULL, 1, NULL);
  }
}