/**
 * IoT Weather Station Platform - ESP8266 / Arduino Firmware
 * 
 * Hardware:
 *  - ESP8266 NodeMCU / Wemos D1 Mini
 *  - Sensor DHT11 / DHT22 (Suhu & Kelembapan)
 *  - Sensor Curah Hujan Analog (FC-37 / Raindrop Sensor pada Pin A0)
 * 
 * Fitur:
 *  - Mengukur suhu, kelembapan, dan persentase curah hujan
 *  - Mengirim data berkala via HTTP POST JSON ke Python Backend API
 * 
 * Author: mprmdya
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// ========== KONFIGURASI WIFI & SERVER ==========
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASS     = "YOUR_WIFI_PASSWORD";
const char* SERVER_URL    = "http://192.168.1.100:5000/api/weather/telemetry"; // Sesuaikan IP backend Python
const char* STATION_ID    = "STATION_ALPHA_01";

// ========== PINOUT DEFINITIONS ==========
#define PIN_DHT           D4   // GPIO 2 pada NodeMCU
#define DHTTYPE           DHT11
#define PIN_RAIN_ANALOG   A0   // ADC Pin sensor hujan (0 - 1023)

DHT dht(PIN_DHT, DHTTYPE);

unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 10000; // Kirim data setiap 10 detik

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== IoT WEATHER STATION PLATFORM ===");

  dht.begin();
  pinMode(PIN_RAIN_ANALOG, INPUT);

  // Sambungkan WiFi
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Terhubung!");
  Serial.print("IP Address NodeMCU: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  unsigned long now = millis();
  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;
    sendWeatherData();
  }
}

void sendWeatherData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi terputus. Mencoba reconnect...");
    WiFi.reconnect();
    return;
  }

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  int rainRaw = analogRead(PIN_RAIN_ANALOG);

  if (isnan(temp) || isnan(hum)) {
    Serial.println("[Warning] Gagal membaca DHT sensor!");
    return;
  }

  // Kalibrasi Sensor Hujan (Analog: 1023 = Sangat Kering, 0-300 = Hujan Deras)
  // Konversi menjadi persentase intensitas (0% kering -> 100% hujan sangat deras)
  int rainIntensity = map(constrain(rainRaw, 200, 1023), 1023, 200, 0, 100);
  String rainStatus = "Kering";
  if (rainIntensity > 60) {
    rainStatus = "Hujan Deras";
  } else if (rainIntensity > 20) {
    rainStatus = "Gerimis / Hujan Ringan";
  }

  // Buat Payload JSON
  StaticJsonDocument<256> doc;
  doc["stationId"]     = STATION_ID;
  doc["temperature"]   = round(temp * 10) / 10.0;
  doc["humidity"]      = round(hum * 10) / 10.0;
  doc["rainIntensity"] = rainIntensity;
  doc["rainStatus"]    = rainStatus;
  doc["rawRain"]       = rainRaw;

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  // Kirim HTTP POST
  WiFiClient client;
  HTTPClient http;
  
  http.begin(client, SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  Serial.printf("[HTTP POST] Mengirim ke %s: %s\n", SERVER_URL, jsonPayload.c_str());
  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("[HTTP Response] Code: %d | Body: %s\n", httpResponseCode, response.c_str());
  } else {
    Serial.printf("[HTTP Error] Gagal kirim, error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
}
