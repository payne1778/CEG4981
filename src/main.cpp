#include "Arduino.h"
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <MFRC522.h>

const char* ssid = "WSU_EZ_Connect";
const char* endpoint = "https://seniorprojectsite.onrender.com/obi/"; // https://seniorprojectsite.onrender.com/

//! Used for ESP32-S3
// constexpr uint8_t SPI_SCK  = 12;
// constexpr uint8_t SPI_MISO = 13;
// constexpr uint8_t SPI_MOSI = 11;
// constexpr uint8_t SS_PIN   = 10; 
// constexpr uint8_t RST_PIN  = 9;

constexpr uint8_t SPI_SCK  = 18;
constexpr uint8_t SPI_MISO = 19;
constexpr uint8_t SPI_MOSI = 23;
constexpr uint8_t SS_PIN   = 21; 
constexpr uint8_t RST_PIN  = 22;

MFRC522 rfidModule(SS_PIN, RST_PIN);

void post(String rfidUID) {
  HTTPClient http;
  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("accept", "text/html");
  
  String username = rfidUID == "72 E6 3F 5C" ? "obi wan kenobi" : "other rebel";
  Serial.println("User was: " + username);
  String jsonPayload = "{\"user\":\"" + username + "\"}";
  int httpResponseCode = http.GET(); 

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("HTTP Response: ");
    Serial.println(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  delay(10000);
}

const char* wl_status_to_string(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
    case WL_STOPPED: return "WL_STOPPED";
  }
  return "INVALID_STATUS";
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.enableSTA(true);
  WiFi.setMinSecurity(WIFI_AUTH_OPEN);
  WiFi.begin(ssid);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 40000) {
    Serial.print(". ");
    delay(1000); // Gives background tasks time to run
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Connection failed!");

    Serial.println("--- WiFi Diagnostic Data ---");
    Serial.print("WiFi Status Code: ");
    Serial.println(wl_status_to_string(WiFi.status()));
    Serial.print("ESP32 MAC Address: ");
    Serial.println(WiFi.macAddress());
    WiFi.printDiag(Serial);
    Serial.println("--------------------------");

    WiFi.disconnect();
  }
}

void setup() {
  delay(2000);
  Serial.begin(115200);
  while (!Serial && millis() < 5000); 
  delay(1000);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SS_PIN); 

  connectToWiFi();
  
  rfidModule.PCD_Init();        
  rfidModule.PCD_DumpVersionToSerial();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi is not connected.");
    connectToWiFi();
    return;
  }
  
  Serial.println("Scan an RFID tag...");
  
  // Reset the loop if no new card is present
  if (!rfidModule.PICC_IsNewCardPresent() || !rfidModule.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("UID tag #: ");
  String uid = "";
  for (byte i = 0; i < rfidModule.uid.size; i++) {
     uid.concat(String(rfidModule.uid.uidByte[i] < 0x10 ? " 0" : " "));
     uid.concat(String(rfidModule.uid.uidByte[i], HEX));
  }
  Serial.println(uid);

  post(uid); 
  
  rfidModule.PICC_HaltA();  // Halt communication
}






