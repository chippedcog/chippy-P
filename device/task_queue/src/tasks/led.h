#ifndef TASK_WAIT_H
#define TASK_WAIT_H

#include <Arduino.h>

// Structure to hold shared task data
struct TaskDataLED
{
  bool isRunning = false;
  int createdLastMs;
};

extern TaskDataLED taskDataLED;

// Functions
void taskLED(void *param);
void taskLEDCreator();

#endif // TASK_WAIT_H