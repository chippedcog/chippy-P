#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>

class WiFiManager
{
public:
  WiFiManager(const char *ssid, const char *password); // Constructor
  void connect();                                      // Method to connect to WiFi
private:
  const char *ssid;
  const char *password;
};

#endif