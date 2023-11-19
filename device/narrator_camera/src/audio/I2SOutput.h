#ifndef __sampler_base_h__
#define __sampler_base_h__

#include <Arduino.h>
#include "driver/i2s.h"

class SampleSource;

/**
 * Base Class for both the ADC and I2S sampler
 **/
class I2SOutput
{
private:
  i2s_port_t m_i2sPort;
  TaskHandle_t m_i2sWriterTaskHandle; // I2S write task
  QueueHandle_t m_i2sQueue;           // i2s writer queue
  SampleSource *m_sample_generator;   // src of samples for us to play

public:
  void start(i2s_port_t i2sPort, i2s_pin_config_t &i2sPins, SampleSource *sample_generator);
  void stop();

  friend void i2sWriterTask(void *param);
};

#endif