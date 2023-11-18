#include <Arduino.h>
#include <ArduinoJson.h> // installed via Arduino IDE
#include "esp_camera.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "pins.h"
#include "env.h"
// fyi, arduino IDE requires extra files be placed in a src directory: https://github.com/microsoft/vscode-arduino/issues/763
// refactored audio example removing SD card file loading: https://github.com/atomic14/esp32_audio/blob/4a39101ea0083aa12dcd3d838c3e51613ecdf3e3/i2s_output/src/main.cpp
#include "src/audio/I2SOutput.h"
#include "src/audio/WAVReader.h"
#include "src/camera/camera.h"
#include "src/camera/pins_camera.h"
#include "src/network/network.h"

// Audio
i2s_pin_config_t i2sPins = {
    .bck_io_num = PIN_AMP_BCLK,
    .ws_io_num = PIN_AMP_LRC,
    .data_out_num = PIN_AMP_DOUT,
    .data_in_num = -1};
I2SOutput *output;
SampleSource *sampleSource;

// Camera
camera_config_t config;
void camera_config_init();

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
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  sensor_t *s = esp_camera_sensor_get();
  // https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/
  s->set_brightness(s, 1); // tweak brightness up
  s->set_saturation(s, 1);
  s->set_contrast(s, 1);
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
    // ... check if connected to wifi
    if (WiFi.status() == WL_CONNECTED)
    {
      // SETUP
      // --- vars to carry between http requests. c++ scopes vars in if blocks, so hoisted to here
      String apiHost = std::string(ENV_API_URL).c_str();
      String captionText;

      // 1. CAPTURE FRAME
      camera_fb_t *fb = esp_camera_fb_get();
      if (!fb)
      {
        Serial.println("Camera capture failed");
        return;
      }
      else
      {
        Serial.println("Camera capture success");
      }

      // 2. CAPTION HTTP REQUEST
      // --- http
      HTTPClient httpCaption;
      String api1Path = "/sketch/sketch_narrator_camera/caption";
      String api1Url = apiHost + api1Path;
      httpCaption.begin(api1Url.c_str());
      httpCaption.addHeader("Content-Type", "image/jpeg");
      // --- post
      int httpCaptionResponseCode = httpCaption.POST(fb->buf, fb->len);
      if (httpCaptionResponseCode < 0)
      {
        Serial.print("Error code: ");
        Serial.println(httpCaptionResponseCode);
        Serial.println(" ");
        Serial.println(httpCaption.errorToString(httpCaptionResponseCode));
      }
      String jsonCaptionSerialized = httpCaption.getString();
      Serial.println(jsonCaptionSerialized);
      // --- parse response
      StaticJsonDocument<1024> jsonCaption; // 1024 is num bytes allocated, if too low I think it cleaves data. ex: had at 200, and long strings became null
      DeserializationError error = deserializeJson(jsonCaption, jsonCaptionSerialized);
      captionText = jsonCaption["caption"].as<String>(); // had to do this casting
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
        String api2Path = "/sketch/sketch_narrator_camera/narrate";
        String api2Url = apiHost + api2Path;
        httpNarrate.begin(api2Url.c_str());
        // --- post
        StaticJsonDocument<1024> docPostBody; // could maybe be using DynamicJsonDocument https://arduinojson.org/v6/api/dynamicjsondocument
        docPostBody["text"] = captionText;
        String jsonPostBody;
        serializeJson(docPostBody, jsonPostBody);
        int httpNarrateResponseCode = httpNarrate.POST(jsonPostBody);
        if (httpNarrateResponseCode < 0)
        {
          Serial.print("Error code: ");
          Serial.println(httpNarrateResponseCode);
          Serial.println(" ");
          Serial.println(httpNarrate.errorToString(httpNarrateResponseCode));
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
        Serial.println("Narrating...");
        sampleSource = new WAVReader(audioBuffer.get(), sizeOfPayloadBuffer);
        output = new I2SOutput();
        output->start(I2S_NUM_1, i2sPins, sampleSource);
        Serial.println("Narrating done!");
      }
    }
    else
    {
      Serial.println("WiFi Disconnected");
    }
  }
}

// TODO: how should I refactor this out
void camera_config_init()
{
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  // config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
}
