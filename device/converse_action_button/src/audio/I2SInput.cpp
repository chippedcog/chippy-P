#include <driver/i2s.h>
#include "soc/i2s_reg.h"

#include "../pins.h" // TODO: this should be configured on instantiation
#include "I2SInput.h"
#include "WAVHeader.h"

#define I2S_PORT I2S_NUM_0

// https://www.youtube.com/watch?v=3g7l5bm7fZ8
i2s_config_t i2s_input_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // I2S_CHANNEL_FMT_RIGHT_LEFT, // I2S_CHANNEL_FMT_ONLY_RIGHT, I2S_CHANNEL_FMT_RIGHT_LEFT
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level
    .dma_buf_count = 4,                       // Number of DMA buffers. Adjust as needed, but keep within 2-128
    .dma_buf_len = 1024,                      // Size of each DMA buffer in bytes
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

I2SInput::I2SInput(int bckPin, int lrclkPin, int dataPin) : m_isRecording(false)
{
  m_pin_config.bck_io_num = bckPin;
  m_pin_config.ws_io_num = lrclkPin;
  m_pin_config.data_out_num = I2S_PIN_NO_CHANGE;
  m_pin_config.data_in_num = dataPin;
}

std::vector<uint8_t> I2SInput::record(bool shouldRecord)
{
  // if shouldRecord is true, append audio data
  if (shouldRecord)
  {
    if (m_isRecording == false)
    {
      // --- install drivers if we aren't already recording install the i2s drivers (we uninstall to avoid conflcit with output, need to verify is an issue tho)
      Serial.println("[I2SInput::record] i2s install");
      i2s_driver_install(I2S_PORT, &i2s_input_config, 0, NULL);
      i2s_set_pin(I2S_PORT, &m_pin_config);
      // --- timing issue fix for SPH0645 mic breakout (https://youtu.be/3g7l5bm7fZ8?si=ySMNzM_kTW4FgDBl&t=268)
      // REG_SET_BIT(I2S_TX_TIMING_REG(I2S_PORT), BIT(9));
      // REG_SET_BIT(I2S_TX_CONF_REG(I2S_PORT), I2S_RX_MSB_SHIFT);
    }
    m_isRecording = true;
    // --- read bytes
    uint8_t i2s_read_buff[512];
    size_t bytes_read;
    esp_err_t result = i2s_read(I2S_PORT, &i2s_read_buff, sizeof(i2s_read_buff), &bytes_read, portMAX_DELAY);
    if (result != ESP_OK)
    {
      Serial.printf("[I2SInput::record] Recording failed with error 0x%x \n", result);
    }
    else if (bytes_read == 0)
    {

      Serial.println("[I2SInput::record] No data read from I2S");
    }
    // --- append bytes to internal data vector
    m_audioData.insert(m_audioData.end(), i2s_read_buff, i2s_read_buff + bytes_read);
  }
  // if not signalling recording, but we have audio data, means we're done! let's return it
  else if (!shouldRecord && !m_audioData.empty())
  {
    m_isRecording = false;
    Serial.println("[I2SInput::record] adding WAV header");
    // --- create WAV header
    auto wavHeader = createWavHeader(m_audioData.size() / (i2s_input_config.bits_per_sample / 8), i2s_input_config.sample_rate, 2, i2s_input_config.bits_per_sample);
    // --- prepend WAV header
    std::vector<uint8_t> wavData;
    wavData.reserve(wavHeader.size() + m_audioData.size());
    wavData.insert(wavData.end(), wavHeader.begin(), wavHeader.end());
    wavData.insert(wavData.end(), m_audioData.begin(), m_audioData.end());
    Serial.println("[I2SInput::record] returning audio data");
    // --- check data
    Serial.printf("[I2SInput::record] recording (%d)\n", m_audioData.size());
    for (size_t i = 0; i < 16000; i++)
    {
      Serial.print(wavData[i], HEX); // Print each byte in hexadecimal format
    }
    Serial.println();
    // --- return
    return wavData;
  }
  return std::vector<uint8_t>(); // Return an empty vector if not recording
}

void I2SInput::clear()
{
  // --- i think i need to uninstall the buffer?
  Serial.println("[I2SInput::clear] i2s uninstall");
  i2s_driver_uninstall(I2S_PORT);
  // --- clear the vector
  Serial.println("[I2SInput::clear] clear");
  m_audioData.clear();
}