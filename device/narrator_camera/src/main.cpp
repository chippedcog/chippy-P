#include <Arduino.h>
#include <ArduinoJson.h> // installed via Arduino IDE
#include "esp_camera.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "env.h"
#include "pins.h"
// fyi, arduino IDE requires extra files be placed in a src directory: https://github.com/microsoft/vscode-arduino/issues/763
// refactored audio example removing SD card file loading: https://github.com/atomic14/esp32_audio/blob/4a39101ea0083aa12dcd3d838c3e51613ecdf3e3/i2s_output/src/main.cpp
#include "audio/I2SOutput.h"
#include "audio/WAVReader.h"
#include "camera/camera_config_init.h" // references header files in that dir that we don't need to import here
#include "network/network.h"

// Audio
i2s_pin_config_t i2sPins = {
    .bck_io_num = PIN_AMP_BCLK,
    .ws_io_num = PIN_AMP_LRC,
    .data_out_num = PIN_AMP_DOUT,
    .data_in_num = -1};
I2SOutput *output;
SampleSource *sampleSource;
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
  // --- camera
  camera_config_init();
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK)
  {
    Serial.printf("[setup] Camera init failed with error 0x%x \n", err);
    return;
  }
  sensor_t *s = esp_camera_sensor_get();
  // https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/
  s->set_brightness(s, 1); // tweak brightness up
  s->set_saturation(s, 1);
  // --- leds (indicator of wip)
  pinMode(PIN_LED_BUILTIN, OUTPUT);
}

//========================================

int lastButtonState = LOW; // LOW is the default/non-pressed button state
bool isSubmittingRequest = false;

void loop()
{
  // LOOP: BUTTON STATE
  // ... if state goes to HIGH, it's being pressed. I think we need to listen for changes so we continuously trigger
  if (digitalRead(PIN_BUTTON) != lastButtonState)
  {
    isSubmittingRequest = digitalRead(PIN_BUTTON) == LOW; // we don't want to submit if button lifted and is now LOW
    lastButtonState = digitalRead(PIN_BUTTON);
  }

  // LOOP: CAPTURE FRAME -> HTTP REQUEST -> PLAY BACK AUDIO (TODO)
  // ...
  if (isSubmittingRequest == true)
  {
    // ... immediately say reqest is not submitting so it doesn't get called again
    isSubmittingRequest = false;
    digitalWrite(PIN_LED_BUILTIN, HIGH); // turn on LED to indicate we're working
    // ... check if connected to wifi
    if (WiFi.status() == WL_CONNECTED)
    {
      // SETUP
      // --- vars to carry between http requests. c++ scopes vars in if blocks, so hoisted to here
      String apiHost = std::string(ENV_API_URL).c_str();
      String captionText;

      // 1. CAPTURE FRAME
      // HACK: capture and throw away first frame. common issue with camera initialization/startup. the esp_camera_fb_return'ed a black frame or prior frame in loop below (did in setup, but after a crash doesn't seem to work)
      // https://github.com/espressif/esp32-camera/issues/545#issuecomment-1600335819
      esp_camera_fb_return(esp_camera_fb_get());
      // --- capture frame for captioning/narration
      camera_fb_t *fb = esp_camera_fb_get();
      if (!fb)
      {
        Serial.println("[loop] Camera capture failed");
        return;
      }
      else
      {
        Serial.println("[loop] Camera capture success");
      }

      // 2. CAPTION HTTP REQUEST
      // --- http
      HTTPClient httpCaption;
      String api1Path = "/device/narrator_camera/caption";
      httpCaption.begin((apiHost + api1Path).c_str());
      httpCaption.addHeader("Content-Type", "image/jpeg");
      // --- post (TODO: graceful err handling)
      int httpCaptionResponseCode = httpCaption.POST(fb->buf, fb->len);
      Serial.printf("[loop] httpCaptionResponseCode = %d\n", httpCaptionResponseCode);
      if (httpCaptionResponseCode < 0)
      {
        Serial.printf("[loop] Error code: %d %s\n", httpCaptionResponseCode, httpCaption.errorToString(httpCaptionResponseCode));
      }
      String jsonCaptionSerialized = httpCaption.getString();
      Serial.printf("[loop] jsonCaptionSerialized = ");
      Serial.println(jsonCaptionSerialized);
      // --- parse response
      StaticJsonDocument<1024> jsonCaption; // 1024 is num bytes allocated, if too low I think it cleaves data. ex: had at 200, and long strings became null
      DeserializationError error = deserializeJson(jsonCaption, jsonCaptionSerialized);
      captionText = jsonCaption["caption"].as<String>(); // had to do this casting
      Serial.printf("[loop] captionText = ");
      Serial.println(captionText);
      // --- close
      httpCaption.end();

      // 3. RELEASE FRAME (to free up memory)
      esp_camera_fb_return(fb);

      // ... ensure we got a caption before continuing (we cast to string which is why we're doing 'null' instead of NULL)
      if (captionText != "null" && captionText.length() > 4)
      {
        // 4. NARRATION AUDIO HTTP REQUEST
        // --- http
        HTTPClient httpNarrate;
        String api2Path = "/device/narrator_camera/narrate";
        httpNarrate.begin((apiHost + api2Path).c_str());
        // --- post (TODO: graceful err handling)
        StaticJsonDocument<1024> docPostBody; // could maybe be using DynamicJsonDocument https://arduinojson.org/v6/api/dynamicjsondocument
        docPostBody["text"] = captionText;
        String jsonPostBody;
        serializeJson(docPostBody, jsonPostBody);
        int httpNarrateResponseCode = httpNarrate.POST(jsonPostBody);
        Serial.printf("[loop] httpNarrateResponseCode = %d\n", httpNarrateResponseCode);
        if (httpNarrateResponseCode < 0)
        {
          Serial.printf("[loop] Error code: %d %s\n", httpNarrateResponseCode, httpNarrate.errorToString(httpNarrateResponseCode));
        }
        // --- parse response
        // Get the size of the payload
        size_t sizeOfPayloadBuffer = httpNarrate.getSize();
        std::unique_ptr<uint8_t[]> audioBuffer(new uint8_t[sizeOfPayloadBuffer]); // Create a buffer to hold the audio data
        // Read the data into the buffer
        httpNarrate.getStream().readBytes(audioBuffer.get(), sizeOfPayloadBuffer);
        // --- close
        httpNarrate.end();

        // 5. PLAY BACK AUDIO
        Serial.println("[loop] Narrating on core 1 via task/queue...");
        sampleSource = new WAVReader(audioBuffer.get(), sizeOfPayloadBuffer); // we need get() to convert from unique_ptr to uint8_t
        output = new I2SOutput();
        output->start(I2S_NUM_1, i2sPins, sampleSource);
      }
    }
    else
    {
      Serial.println("[loop] WiFi Disconnected");
    }
    // turn oof LED to indicate we're done (whether there was err or not)
    digitalWrite(PIN_LED_BUILTIN, LOW);
  }
}
