#ifndef WIFICONFIGMANAGER_H
#define WIFICONFIGMANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <LiquidCrystal_I2C.h>

class WifiConfigManager {
  private:
    const char* ssid;
    const char* password;
    WebServer* server; // Use a pointer for dynamic allocation
    LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
    Preferences preferences;

  public:
    WifiConfigManager(const char* ssid, const char* password);
    ~WifiConfigManager();

    void startAP();
    void startConfigWebServer();
    void handleRoot();
    void handleSubmit();
    void displayOnLCD(String message);
    void handleClient();
};

#endif // WIFICONFIGMANAGER_H