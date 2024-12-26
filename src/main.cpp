#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <LiquidCrystal_I2C.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "WifiConfigManager.h" 

const char* ssid = "ESP32-SoftAP";
const char* password = "12345678"; // Minimum 8 characters


WifiConfigManager* wifiConfigManager;


void taskRunWifiConfigManager(void *pvParameters) {
  /** 
   * Launches the AP and starts the web server. 
   * **/
  wifiConfigManager = new WifiConfigManager(ssid, password);
  wifiConfigManager->startAP();
  wifiConfigManager->startConfigWebServer();
  
  while (true) {
    wifiConfigManager->handleClient();
    delay(1);
  }
}

void startWifi() {
  int count = 0;
  bool connected = false;
  Preferences preferences;
  preferences.begin("wifi", true);
  String wifi_ssid = preferences.getString("wifi_ssid", "");
  String wifi_password = preferences.getString("wifi_password", "");
  preferences.end();

  if (wifi_ssid.length() > 0 && wifi_password.length() > 0) {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    while (WiFi.status() != WL_CONNECTED || count < 3) {
      Serial.println("Connecting to WiFi...");
      delay(1000);
      count += 1;
      if (count == 2) {
        Serial.println("Failed to connect to WiFi");
        break;
      }
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi");
        connected = true;
        break;
      }
    }
    if (connected) {
      Serial.println("Connected to WiFi");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());

    } else {
      Serial.println("Failed to connect to WiFi");
      //Stop the WiFi connection
      WiFi.disconnect();
    } 
  }  
  else{
    /** If there is no saved credentials then we should launch the 
     * WifiConfigManager to allow the user to enter the credentials
     * This is done by creating a new task that will run the WifiConfigManager
     */

    Serial.println("No WiFi credentials found. Going to AP mode");
    
    xTaskCreatePinnedToCore(
      taskRunWifiConfigManager, /* Function to implement the task */
      "taskRunWifiConfigManager", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      1,  /* Priority of the task */
      NULL,  /* Task handle. */
      0); /* Core where the task should run */
  }
} 

void setup() {
  String wifi_ssid;
  String wifi_password;

  Preferences preferences;
  preferences.begin("wifi", false);
  wifi_ssid = preferences.getString("wifi_ssid", "");
  wifi_password = preferences.getString("wifi_password", "");
  preferences.end();

  Serial.begin(115200);
  startWifi();
  Serial.print("WiFi SSID: ");
  Serial.println(wifi_ssid);
  Serial.print("WiFi Password: ");
  Serial.println(wifi_password);
  Serial.println("Wifi IP Address: "+ WiFi.localIP().toString());
  Serial.println("Wifi MAC Address: "+ WiFi.macAddress());
}

void loop() {
}