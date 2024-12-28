#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <LiquidCrystal_I2C.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "WifiConfigManager.h"

WifiConfigManager::WifiConfigManager(const char* ssid, const char* password) {
  this->ssid = ssid;
  this->password = password;
  this->server = nullptr; // Initialize the pointer to nullptr
  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Turn on the backlight
}

WifiConfigManager::~WifiConfigManager() {
  Serial.println("Destroying WifiConfigManager");
  if (server != nullptr) {
    server->stop();
    delete server; // Clean up the dynamically allocated WebServer instance
  }
}

void WifiConfigManager::startAP() {
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void WifiConfigManager::startConfigWebServer() {
  server = new WebServer(80); // Dynamically allocate the WebServer instance

  server->on("/", HTTP_GET, std::bind(&WifiConfigManager::handleRoot, this));
  server->on("/submit", HTTP_POST, std::bind(&WifiConfigManager::handleSubmit, this));
  server->begin();
  Serial.println("HTTP server started");
}

void WifiConfigManager::handleRoot() {
  Serial.println("Wifi Config page requested");
  String html = "<html><body>"
                "<h1>WiFi Configuration</h1>"
                "<form action=\"/submit\" method=\"POST\">"
                "WiFi SSID: <input type=\"text\" name=\"wifi_ssid\"><br>"
                "WiFi Password: <input type=\"password\" name=\"wifi_password\"><br>"
                "<input type=\"submit\" value=\"Submit\">"
                "</form>"
                "</body></html>";
  server->send(200, "text/html", html);
}

void WifiConfigManager::handleSubmit() {
  if (server->hasArg("wifi_ssid") && server->hasArg("wifi_password")) {
    String wifi_ssid = server->arg("wifi_ssid");
    String wifi_password = server->arg("wifi_password");

    // Save to preferences
    preferences.begin("wifi", false);
    preferences.putString("wifi_ssid", wifi_ssid);
    preferences.putString("wifi_password", wifi_password);
    preferences.end();

    server->send(200, "text/html", "Credentials saved. You can now connect to the WiFi network.");

    // Update the LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Credentials Saved");
  } else {
    server->send(400, "text/html", "Invalid request");
  }
}

void WifiConfigManager::displayOnLCD(String message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void WifiConfigManager::handleClient() {
  if (server != nullptr) {
    server->handleClient();
  }
}

char ssid[] = "ESPLab-Access-Point";
char password[] = "1234567890";
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

