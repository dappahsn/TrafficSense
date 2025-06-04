#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"

// Ganti dengan WiFi kamu
const char* ssid = "ruryaw";
const char* password = "anakhusen";

// Firestore
const String projectId = "trafficsense1";
const String collection = "sensor_suara";

// Sensor suara
const int soundSensorPin = 34;

// NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

void setup() {
  Serial.begin(115200);
  pinMode(soundSensorPin, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung!");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Sinkronisasi waktu...");
}

void loop() {
  int jumlah = 0;
  int pembacaan = 10; // karena 5 detik / 0.5 detik = 10 kali

  for (int i = 0; i < pembacaan; i++) {
    int suara = analogRead(soundSensorPin);
    jumlah += suara;
    Serial.printf("Pembacaan %d: %d\n", i + 1, suara);
    delay(500); // Delay 0.5 detik
  }

  int rataRataSuara = jumlah / pembacaan;

  // Ambil waktu saat ini
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Gagal mendapatkan waktu");
    return;
  }

  char timestamp[30];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

  Serial.printf("Rata-rata Suara: %d | Waktu: %s\n", rataRataSuara, timestamp);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://firestore.googleapis.com/v1/projects/" + projectId + "/databases/(default)/documents/" + collection;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    // JSON ke Firestore
    String json = "{ \"fields\": {";
    json += "\"nilai_suara\": { \"integerValue\": \"" + String(rataRataSuara) + "\" },";
    json += "\"timestamp\": { \"stringValue\": \"" + String(timestamp) + "\" }";
    json += "} }";

    int httpResponseCode = http.POST(json);
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    Serial.println("Response: " + http.getString());

    http.end();
  } else {
    Serial.println("WiFi tidak terhubung!");
  }

  // Langsung ulangi loop
}
