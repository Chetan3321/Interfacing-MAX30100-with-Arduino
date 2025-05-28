#include <ESP8266WiFi.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 1000

// WiFi credentials
const char* ssid = "maxwell";
const char* password = "CSkhadse3321";

PulseOximeter pox;
uint32_t tsLastReport = 0;

// Variables to hold latest data
float bpm = 0;
float spo2 = 0;

// Callback when a beat is detected
void onBeatDetected() {
  Serial.println("Beat!");
}

// Web server on port 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();

  Serial.print("Initializing pulse oximeter.. ");
  if (!pox.begin()) {
    Serial.println("FAILED");
    while (1);
  } else {
    Serial.println("SUCCESS");
  }

  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
  pox.update();

  // Update data every second
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    bpm = pox.getHeartRate();
    spo2 = pox.getSpO2();
    Serial.printf("BPM: %.2f | SpO2: %.2f\n", bpm, spo2);
    tsLastReport = millis();
  }

  // Check for web client
  WiFiClient client = server.available();
  if (client) {
    // Wait until client sends request
    while (!client.available()) {
      delay(1);
    }

    // Read request (not used here)
    while (client.available()) {
      client.read();
    }

    // Send HTML response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html><html><head><meta http-equiv='refresh' content='2'>");
    client.println("<title>Pulse Oximeter</title></head><body>");
    client.println("<h1>Pulse Oximeter Data</h1>");
    client.printf("<p><strong>Heart Rate:</strong> %.2f BPM</p>", bpm);
    client.printf("<p><strong>SpO2:</strong> %.2f%%</p>", spo2);
    client.println("</body></html>");

    delay(10);  // Allow client to receive
    client.stop();
  }
}
