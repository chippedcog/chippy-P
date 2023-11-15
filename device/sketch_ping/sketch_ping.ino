#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "env.h"

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

//========================================

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  // --- wifi
  Serial.print("Configuring wifi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15)
  { // 20 attempts, for example
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Failed to connect to WiFi. Please check credentials and signal.");
    return; // or handle the error differently
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

//========================================

void loop()
{
  // Send an HTTP GET request every "timerDelay" min/sec
  if ((millis() - lastTime) > timerDelay)
  {
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
      // HTTP REQUEST
      // https://randomnerdtutorials.com/esp32-http-get-post-arduino/
      // https://randomnerdtutorials.com/esp32-https-requests
      // --- client
      WiFiClientSecure *client = new WiFiClientSecure;
      client->setInsecure();
      // client->setCACert(rootCACertificate); // TODO: for calling a https API, we need to pass in a root ca cert

      HTTPClient https;

      // --- init
      String apiHost = std::string(API_URL).c_str(); // #arduino define to c-compatible string for concat
      String apiPath = "/blueprint_sketch_ping/ping";
      String apiUrl = apiHost + apiPath; // do I need to do clean up with vars in c++?
      https.begin(apiUrl.c_str());
      // --- headers
      https.addHeader("ngrok-skip-browser-warning", ""); // temporary, i'm ngrok'ing to closer simulate outbound http to my python server

      // --- GET
      int httpResponseCode = https.GET();

      // --- response
      if (httpResponseCode > 0)
      {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = https.getString();
        Serial.println(payload);
      }
      else
      {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        Serial.print(" ");
        Serial.print(https.errorToString(httpResponseCode));
      }
      // --- close
      https.end();
    }
    else
    {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}