#include <Arduino.h>

#include "env.h"
#include "pins.h"
#include "tasks/print.h"
#include "tasks/wait.h"

//========================================

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  // --- led
  pinMode(PIN_LED_BUILTIN, OUTPUT);
  // --- task (background): print
  TimerHandle_t tp = xTimerCreate("taskPrintCreator", pdMS_TO_TICKS(taskDataPrint.timerMs), pdTRUE, NULL, taskPrintCreator);
  xTimerStart(tp, 0); // FYI 0 = no block FOR TIMER handler. Irrelevant for task/queue adding. Need global bool for that as you'll see in files
  // --- task (background) (spawns a LED toggle task w/o timer/recurrance)
  TimerHandle_t tw = xTimerCreate("taskWaitCreator", pdMS_TO_TICKS(taskDataWait.timerMs), pdTRUE, NULL, taskWaitCreator);
  xTimerStart(tw, 0);
}

//========================================

unsigned int previousMillis = 0;
int interval = 1000; // 1 second

void loop()
{
  // demoing background actions and blocking/non-blocking, but printing every 1s for tracking cadence
  int currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    Serial.printf("[%d]\n", currentMillis);
  }
}
