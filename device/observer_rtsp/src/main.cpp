#include "OV2640.h"
#include <Arduino.h>
#include "esp_camera.h"
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "CRtspSession.h"
#include "OV2640Streamer.h"

#include "env.h"
#include "pins.h"
#include "camera/cam.h"
#include "network/wifi.h"

// Camera
OV2640 cam;            // needed to isolate for rtsp stream fns
CamManager camManager; // probably should just be funcs rather than class
// RTSP
WiFiClient rtspClient;
WiFiServer rtspServer(8554);
CStreamer *streamer = nullptr;
CRtspSession *session = nullptr;
// Wifi
WiFiManager myWiFiManager(ENV_WIFI_SSID, ENV_WIFI_PASSWORD);

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  // --- stats
  Serial.printf("[setup] CPU Freq: %d Mhz, %d core(s)\n", getCpuFrequencyMhz(), ESP.getChipCores());
  Serial.printf("[setup] Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("[setup] Free PSRAM: %d bytes\n", ESP.getFreePsram());
  Serial.printf("[setup] SDK version: %s\n", ESP.getSdkVersion());
  Serial.printf("[setup] ESP Device ID: %d\n", ESP.getEfuseMac());
  // --- wifi
  myWiFiManager.connect();
  // --- camera
  esp_err_t camera_init_result = camManager.initialize();
  if (camera_init_result == ESP_OK)
    camManager.updateSettings();
  // --- rtsp
  rtspServer.begin();
  Serial.println("[setup] done");
}

void loop()
{
  // RTSP
  uint32_t msecPerFrame = 500;
  static uint32_t lastimageAt = millis();
  // --- if connection
  if (session)
  {
    session->handleRequests(0);
    // instead of timeout (which would be blocking), send when enough milliseconds have passed
    uint32_t now = millis();
    if (now > lastimageAt + msecPerFrame || now < lastimageAt)
    {
      // Serial.printf("session->m_streaming: %d\n", session->m_streaming);
      session->broadcastCurrentFrame(now);
      lastimageAt = now;
      // check if we are overrunning our max frame rate
      now = millis();
      if (now > lastimageAt + msecPerFrame)
        Serial.printf("[loop] warning exceeding max frame rate of %d ms\n", now - lastimageAt);
    }
    if (session->m_stopped)
    {
      delete session;
      delete streamer;
      session = NULL;
      streamer = NULL;
    }
  }
  else
  {
    rtspClient = rtspServer.accept();
    if (rtspClient)
    {
      Serial.println("[loop] new streamer + rtspClient");
      streamer = new OV2640Streamer(&rtspClient, cam);   // our streamer for UDP/TCP based RTP transport
      session = new CRtspSession(&rtspClient, streamer); // our threads RTSP session and state
    }
  }
}
