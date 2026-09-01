#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "WSU_EZ_Connect";
const char* endpoint = "http://typicode.com";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, NULL);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  SPI.begin();          // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522 card reader
  Serial.println("Scan an RFID tag...");
}

void post(String user) {
  String jsonPayload = "{\"authenticated-user\":\"" + user + "\"}";
  
  HTTPClient http;
  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonPayload)
  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  delay(10000);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi Disconnected");
      return; 
  }

  // Reset the loop if no new card is present
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Show UID on serial monitor
  Serial.print("UID tag #: ");
  String content= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();

  post(content); 
  
  mfrc522.PICC_HaltA();  // Halt communication
}
