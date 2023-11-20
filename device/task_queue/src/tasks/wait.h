#ifndef TASK_WAIT_H
#define TASK_WAIT_H

#include <Arduino.h>

// Structure to hold shared task data
struct TaskDataWait
{
  bool isRunning = false;
  int timerMs = 500;
  int createdLastMs;
};

extern TaskDataWait taskDataWait;

// Functions
void taskWait(void *param);
void taskWaitCreator(TimerHandle_t xTimer);

#endif // TASK_WAIT_H