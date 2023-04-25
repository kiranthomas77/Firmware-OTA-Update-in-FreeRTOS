#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>

// Network configuration variables
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
const char* host = "http://3.110.114.64/“;
const int port = 443;

// Update file variables
const char* updateUrl = "http://13.233.137.149/updated.bin”;
const char* updateFilename = "updated.bin";

// Task handle for Communication module task
TaskHandle_t CommunicationTaskHandle = NULL;

// Communication module task
void CommunicationTask(void *pvParameters) {
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  // Establish secure connection to update portal
  WiFiClientSecure client;
  if (!client.connect(host, port)) {
    Serial.println("Failed to connect to update portal");
    vTaskDelete(CommunicationTaskHandle);
    return;
  }

  // Request update file
  HTTPClient http;
  http.begin(client, updateUrl);
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.println("Failed to download update file");
    vTaskDelete(CommunicationTaskHandle);
    return;
  }

  // Save update file to SPIFFS
  File updateFile = SPIFFS.open(updateFilename, FILE_WRITE);
  if (!updateFile) {
    Serial.println("Failed to open update file");
    vTaskDelete(CommunicationTaskHandle);
    return;
  }
  http.writeToStream(&updateFile);
  updateFile.close();

void setup()
 {
  Serial.begin(115200);
  SPIFFS.begin();

  // Create Communication module task
  xTaskCreatePinnedToCore(CommunicationTask, "Communication Task", 4096, NULL, 1, &CommunicationTaskHandle, 0);
}

void loop() {
  // do nothing
}
