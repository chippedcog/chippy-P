#include <Arduino.h>
#include <ArduinoJson.h> // installed via Arduino IDE
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "env.h"
#include "network.h"
#include "pins.h"

NETWORK_H WiFiManager myWiFiManager(ENV_WIFI_SSID, ENV_WIFI_PASSWORD);

//========================================

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  // --- wifi
  myWiFiManager.connect();
  // --- led
  pinMode(PIN_LED, OUTPUT);
}

//========================================

unsigned long lastTime = 0;
unsigned long timerDelay = 3000; // measured in milliseconds = 3 sec

void loop()
{
  // LOOP: HTTP Request (Xs)
  if ((millis() - lastTime) > timerDelay)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient http;
      String apiHost = std::string(ENV_API_URL).c_str();
      String apiPath = "/device/sketch_led_state_polling/state";
      String apiUrl = apiHost + apiPath;
      http.begin(apiUrl.c_str());
      // --- request (-1 code means error)
      int httpResponseCode = http.GET();
      if (httpResponseCode > 0)
      {
        String payload = http.getString();
        // --- parse json (ex: { "on": 1 })
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload);
        int on = doc["on"];
        // --- set led state
        Serial.print("on: ");
        Serial.println(on);
        if (on == 1)
        {
          digitalWrite(PIN_LED, HIGH);
        }
        else
        {
          digitalWrite(PIN_LED, LOW);
        }
      }
      else
      {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        Serial.print(" ");
        Serial.print(http.errorToString(httpResponseCode));
      }
      // --- close
      http.end();
    }
    else
    {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}