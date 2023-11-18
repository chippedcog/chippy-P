#include "WAVReader.h"

#pragma pack(push, 1)
typedef struct
{
  // RIFF Header
  char riff_header[4]; // Contains "RIFF"
  int wav_size;        // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
  char wave_header[4]; // Contains "WAVE"

  // Format Header
  char fmt_header[4]; // Contains "fmt " (includes trailing space)
  int fmt_chunk_size; // Should be 16 for PCM
  short audio_format; // Should be 1 for PCM. 3 for IEEE Float
  short num_channels;
  int sample_rate;
  int byte_rate;          // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
  short sample_alignment; // num_channels * Bytes Per Sample
  short bit_depth;        // Number of bits per sample

  // Data
  char data_header[4]; // Contains "data"
  int data_bytes;      // Number of bytes in data. Number of samples * num_channels * sample byte size
                       // uint8_t bytes[]; // Remainder of wave file is bytes
} wav_header_t;
#pragma pack(pop)

WAVReader::WAVReader(const uint8_t *buffer, size_t bufferSize)
    : m_buffer(buffer), m_bufferSize(bufferSize), m_currentPos(0)
{

  // Read the WAV header
  wav_header_t wav_header;
  memcpy(&wav_header, m_buffer, sizeof(wav_header_t));
  m_currentPos += sizeof(wav_header_t);

  // Sanity check the bit depth
  if (wav_header.bit_depth != 16)
  {
    Serial.printf("ERROR: bit depth %d is not supported\n", wav_header.bit_depth);
  }

  Serial.printf("fmt_chunk_size=%d, audio_format=%d, num_channels=%d, sample_rate=%d, sample_alignment=%d, bit_depth=%d, data_bytes=%d\n",
                wav_header.fmt_chunk_size, wav_header.audio_format, wav_header.num_channels, wav_header.sample_rate, wav_header.sample_alignment, wav_header.bit_depth, wav_header.data_bytes);

  m_num_channels = wav_header.num_channels;
  m_sample_rate = wav_header.sample_rate;
}

WAVReader::~WAVReader()
{
  // noop, used to be file.close()
}

void WAVReader::getFrames(Frame_t *frames, int number_frames)
{
  // Fill the buffer with data from the memory buffer
  for (int i = 0; i < number_frames; i++)
  {
    // If we've reached the end of the buffer, wrap around (if looping is desired)
    if (m_currentPos >= m_bufferSize)
    {
      m_currentPos = sizeof(wav_header_t); // Reset to after header
    }

    // Read the next sample for the left channel
    memcpy(&frames[i].left, m_buffer + m_currentPos, sizeof(int16_t));
    m_currentPos += sizeof(int16_t);

    // Duplicate or read the right channel sample
    if (m_num_channels == 1)
    {
      frames[i].right = frames[i].left;
    }
    else
    {
      memcpy(&frames[i].right, m_buffer + m_currentPos, sizeof(int16_t));
      m_currentPos += sizeof(int16_t);
    }
  }
}