#include <Arduino.h>
#include <ArduinoJson.h> // installed via Arduino IDE
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "env.h"
#include "pins.h"
#include "audio/I2SInput.h"
// #include "audio/I2SOutput.h"
// #include "audio/WAVReader.h"
#include "network/network.h"

// APi
String apiHost = std::string(ENV_API_URL).c_str();
// Audio
// --- input
I2SInput i2sInput(PIN_MIC_BCLK, PIN_MIC_LRCLK, PIN_MIC_DATA); // 2nd arg is I2S_WS_PIN, what is that
// --- output
// i2s_pin_config_t i2sPins = {
//     .bck_io_num = PIN_AMP_BCLK,
//     .ws_io_num = PIN_AMP_LRC,
//     .data_out_num = PIN_AMP_DOUT,
//     .data_in_num = -1};
// I2SOutput *output;
// SampleSource *sampleSource;
// Wifi
NETWORK_H WiFiManager myWiFiManager(ENV_WIFI_SSID, ENV_WIFI_PASSWORD);

//========================================

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  // --- wifi
  myWiFiManager.connect();
  // --- button
  pinMode(PIN_BUTTON, INPUT);
  // --- leds (indicator of wip)
  pinMode(PIN_LED_BUILTIN, OUTPUT);
}

//========================================

void loop()
{
  // RECORDING
  // --- check button state, if pressed down we're recording
  bool shouldRecord = digitalRead(PIN_BUTTON) == LOW;
  // --- this will be blocking if shouldRecord is true
  std::vector<uint8_t> recordedData = i2sInput.record(shouldRecord);

  // RECORDED, DO SOMETHING
  if (!recordedData.empty())
  {
    Serial.println("[loop] posting recording: " + String(recordedData.size()) + " bytes");

    // SERVER: Send for speech-to-text & text-to-speech back
    // --- http
    HTTPClient http;
    String api1Path = "/device/converse_action_button/say";
    http.begin((apiHost + api1Path).c_str());
    http.addHeader("Content-Type", "application/octet-stream");
    // --- post (TODO: graceful err handling)
    int httpResponseCode = http.POST(recordedData.data(), recordedData.size());
    // --- clear the buffer now that we're done processing
    i2sInput.clear();
    // --- await response
    Serial.printf("[loop] httpResponseCode = %d\n", httpResponseCode);
    if (httpResponseCode < 0)
    {
      Serial.printf("[loop] Error code: %d %s\n", httpResponseCode, http.errorToString(httpResponseCode));
    }
    // --- parse response
    // --- close
    http.end();

    // Do something
  }
}
