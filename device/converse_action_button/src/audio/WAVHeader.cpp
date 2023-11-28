#include <vector>
#include <cstdint>

const int WAV_HEADER_SIZE = 44;

std::vector<uint8_t> createWavHeader(uint32_t numSamples, int sampleRate, int numChannels, int bitsPerSample)
{
    int byteRate = sampleRate * numChannels * bitsPerSample / 8;
    int blockAlign = numChannels * bitsPerSample / 8;

    uint32_t dataSize = numSamples * numChannels * bitsPerSample / 8;
    uint32_t fileSize = WAV_HEADER_SIZE + dataSize;

    std::vector<uint8_t> header(WAV_HEADER_SIZE, 0);

    // RIFF header
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    header[4] = fileSize & 0xff;
    header[5] = (fileSize >> 8) & 0xff;
    header[6] = (fileSize >> 16) & 0xff;
    header[7] = (fileSize >> 24) & 0xff;
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';

    // fmt subchunk
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';
    header[16] = 16; // Subchunk1Size (16 for PCM)
    header[20] = 1;  // AudioFormat (PCM = 1)
    header[22] = numChannels;
    header[24] = sampleRate & 0xff;
    header[25] = (sampleRate >> 8) & 0xff;
    header[26] = (sampleRate >> 16) & 0xff;
    header[27] = (sampleRate >> 24) & 0xff;
    header[28] = byteRate & 0xff;
    header[29] = (byteRate >> 8) & 0xff;
    header[30] = (byteRate >> 16) & 0xff;
    header[31] = (byteRate >> 24) & 0xff;
    header[32] = blockAlign;
    header[34] = bitsPerSample;

    // data subchunk
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';
    header[40] = dataSize & 0xff;
    header[41] = (dataSize >> 8) & 0xff;
    header[42] = (dataSize >> 16) & 0xff;
    header[43] = (dataSize >> 24) & 0xff;

    return header;
}