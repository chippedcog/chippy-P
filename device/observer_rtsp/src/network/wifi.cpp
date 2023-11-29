#include "wifi.h"

WiFiManager::WiFiManager(const char *ssid, const char *password)
{
  this->ssid = ssid;
  this->password = password;
}

void WiFiManager::connect()
{
  Serial.print("[WiFiManager::connect] Connecting to WiFi...");
  WiFi.begin(ssid, password);
  // --- attempt (<15 times)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  // --- mention failure if hit 15
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[WiFiManager::connect] Failed to connect to WiFi. Please check credentials and signal.");
    return; // or handle the error differently
  }
  // --- mention connection!
  Serial.println("[WiFiManager::connect] WiFi connected");
}
