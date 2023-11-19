#include <Arduino.h>
#include "driver/i2s.h"
#include <math.h>

#include "SampleSource.h"
#include "I2SOutput.h"

// number of frames to try and send at once (a frame is a left and right sample)
#define NUM_FRAMES_TO_SEND 512

void i2sWriterTask(void *param)
{
    I2SOutput *output = (I2SOutput *)param;
    int availableBytes = 0;
    int buffer_position = 0;
    Frame_t *frames = (Frame_t *)malloc(sizeof(Frame_t) * NUM_FRAMES_TO_SEND);

    // not allowing this task to return until we hit output->stop() and delete it
    while (true)
    {
        // wait for some data to be requested
        i2s_event_t evt;
        if (xQueueReceive(output->m_i2sQueue, &evt, portMAX_DELAY) == pdPASS)
        {
            if (evt.type == I2S_EVENT_TX_DONE)
            {
                size_t bytesWritten = 0;
                // otherwise continue to process
                do
                {
                    if (availableBytes == 0)
                    {
                        if (!output->m_sample_generator->getFrames(frames, NUM_FRAMES_TO_SEND))
                        {
                            // No more frames were filled, end of playback by deleting this task
                            Serial.println("[i2sWriterTask] stopping");
                            output->stop();
                            Serial.println("[i2sWriterTask] stopped. YOU SHOULD NOT BE SEEING THIS MESSAGE");
                            return;
                        }
                        // how many bytes do we now have to send
                        availableBytes = NUM_FRAMES_TO_SEND * sizeof(uint32_t);
                        // reset the buffer position back to the start
                        buffer_position = 0;
                    }
                    // do we have something to write?
                    if (availableBytes > 0)
                    {
                        // write data to the i2s peripheral
                        i2s_write(output->m_i2sPort, buffer_position + (uint8_t *)frames,
                                  availableBytes, &bytesWritten, portMAX_DELAY);
                        availableBytes -= bytesWritten;
                        buffer_position += bytesWritten;
                    }
                } while (bytesWritten > 0);
            }
        }
    }
}

void I2SOutput::start(i2s_port_t i2sPort, i2s_pin_config_t &i2sPins, SampleSource *sample_generator)
{
    m_sample_generator = sample_generator;
    // i2s config for writing both channels of I2S
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = m_sample_generator->sampleRate(),
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024};

    m_i2sPort = i2sPort;
    // install and start i2s driver
    i2s_driver_install(m_i2sPort, &i2sConfig, 4, &m_i2sQueue);
    // set up the i2s pins
    i2s_set_pin(m_i2sPort, &i2sPins);
    // clear the DMA buffers
    i2s_zero_dma_buffer(m_i2sPort);
    // start a task to write samples to the i2s peripheral
    TaskHandle_t writerTaskHandle;
    Serial.println("[I2SOutput::start] creating task: 'i2s Writer Task'");
    xTaskCreate(i2sWriterTask, "i2s Writer Task", 4096, this, 1, &writerTaskHandle);
}

// Implented because we want to end the task, unlike the example which plays continuously
void I2SOutput::stop()
{
    // Stop and delete the i2s driver
    Serial.println("[I2SOutput::stop] i2s_driver_uninstall'ing");
    i2s_driver_uninstall(m_i2sPort);
    // Stop the writer task
    Serial.println("[I2SOutput::stop] vTaskDelete'ing");
    vTaskDelete(m_i2sWriterTaskHandle);
    // code here didn't seemingly run bc we're ending this task? this also means in the above writing task, we never actually return
    Serial.println("[I2SOutput::stop] YOU SHOULD NOT BE SEEING THIS MESSAGE");
}
