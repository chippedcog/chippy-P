#ifndef __wav_header_h__
#define __wav_header_h__

#include <vector>
#include <cstdint>

const int WAV_HEADER_SIZE = 44;

// Function to create a WAV file header
std::vector<uint8_t> createWavHeader(uint32_t numSamples, int sampleRate, int numChannels, int bitsPerSample);

#endif