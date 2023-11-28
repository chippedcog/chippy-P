#ifndef __wav_reader_h__
#define __wav_reader_h__

#include "SampleSource.h"

class WAVReader : public SampleSource
{
public:
  WAVReader(const uint8_t *buffer, size_t bufferSize);
  bool getFrames(Frame_t *frames, int number_frames);
  int sampleRate() { return m_sample_rate; }

private:
  const uint8_t *m_buffer;
  size_t m_bufferSize;
  size_t m_currentPos;
  int m_num_channels;
  int m_sample_rate;
};

#endif