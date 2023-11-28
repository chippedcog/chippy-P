#ifndef __i2s_input_h__
#define __i2s_input_h__

#include <Arduino.h>
#include <driver/i2s.h>
#include <vector>

class I2SInput
{
public:
  I2SInput(int bckPin, int lrclkPin, int dataPin);
  std::vector<uint8_t> record(bool shouldRecord);
  void clear();

private:
  i2s_pin_config_t m_pin_config;
  bool m_isRecording;
  std::vector<uint8_t> m_audioData;
};

#endif // __i2s_input_h__