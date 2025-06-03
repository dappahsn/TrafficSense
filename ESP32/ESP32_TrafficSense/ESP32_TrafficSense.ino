#include "WiFi.h"
#include "HTTPClient.h"

// Konfigurasi WiFi
    const char* ssid = "ruryaw";
const char* password = "anakhusen";

// Konfigurasi Host
const char* host = "trafficsense.kesug.com";

// Pin sensor suara analog
const int soundSensorPin = 34;

// Array penampung data suara
const int sampleCount = 10; // 10 kali x 0.5 detik = 5 detik
int soundSamples[sampleCount];
int sampleIndex = 0;

void setup() {
  Serial.begin(115200);
  pinMode(soundSensorPin, INPUT);

  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Terhubung!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nGagal terhubung ke WiFi.");
  }
}

void loop() {
  // Baca suara setiap 0.5 detik
  int soundValue = analogRead(soundSensorPin);
  Serial.print("Pembacaan ke-");
  Serial.print(sampleIndex + 1);
  Serial.print(": ");
  Serial.println(soundValue);

  soundSamples[sampleIndex] = soundValue;
  sampleIndex++;

  // Jika sudah 10 sampel (5 detik)
  if (sampleIndex >= sampleCount) {
    // Hitung rata-rata
    int sum = 0;
    for (int i = 0; i < sampleCount; i++) {
      sum += soundSamples[i];
    }
    int avgSound = sum / sampleCount;

    Serial.print("Rata-rata suara selama 5 detik: ");
    Serial.println(avgSound);

    // Kirim ke server jika WiFi terhubung
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String link = "http://" + String(host) + "/kirim-data.php?suara=" + String(avgSound);

      http.begin(link);
      int httpCode = http.GET();

      if (httpCode > 0) {
        String respon = http.getString();
        Serial.println("Respon dari server:");
        Serial.println(respon);
      } else {
        Serial.println("Gagal mengirim data ke server.");
      }

      http.end();
    } else {
      Serial.println("WiFi tidak terhubung, tidak mengirim data.");
    }

    // Reset index untuk siklus berikutnya
    sampleIndex = 0;
  }

  delay(500); // Tunggu 0.5 detik sebelum pembacaan berikutnya
}
