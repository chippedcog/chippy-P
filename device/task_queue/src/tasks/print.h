#ifndef TASK_PRINT_H
#define TASK_PRINT_H

#include <Arduino.h>

// Structure to hold shared task data
struct TaskDataPrint
{
  bool isRunning = false;
  int timerMs = 500;
  int createdLastMs;
};

extern TaskDataPrint taskDataPrint;

// Functions
void taskPrint(void *param);
void taskPrintCreator(TimerHandle_t xTimer);

#endif // TASK_PRINT_H