#ifndef __i2s_output_h__
#define __i2s_output_h__

#include <Arduino.h>
#include "driver/i2s.h"

class SampleSource;

/**
 * Base Class for both the ADC and I2S sampler
 **/
class I2SOutput
{
private:
  i2s_port_t m_i2sOutputPort;
  TaskHandle_t m_taskI2SOutputHandle;       // I2S write task
  QueueHandle_t m_i2sOutputQueue;           // i2s writer queue
  SampleSource *m_i2sOutputSampleGenerator; // src of samples for us to play

public:
  void start(i2s_port_t i2sPort, i2s_pin_config_t &i2sPins, SampleSource *sample_generator);
  void stop();

  friend void taskI2SOutput(void *param);
};

#endif