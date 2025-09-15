#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WebSocketsClient_Generic.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "config.h"

const char* server_host = "number-frame-server.noshado.ws";
const int websocket_port = 80;

Adafruit_7segment matrix = Adafruit_7segment();
WebSocketsClient webSocket;

void fetchCurrentQuestion() {
  WiFiClient client;
  HTTPClient http;
  
  http.begin(client, "http://" + String(server_host) + "/get-current-question");
  
  if (http.GET() > 0) {
    DynamicJsonDocument doc(1024);
    if (deserializeJson(doc, http.getString()) == DeserializationError::Ok) {
      if (doc["success"] == true && doc.containsKey("number")) {
        int number = doc["number"];
        if (number >= 0 && number <= 9999) {
          matrix.clear();
          matrix.print(number);
          matrix.writeDisplay();
        }
      }
    }
  }
  
  http.end();
}

void setup() {
  Wire.begin(4, 5);
  matrix.begin(0x70);
  matrix.setBrightness(5);
  matrix.clear();
  matrix.print(8888);
  matrix.writeDisplay();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  fetchCurrentQuestion();
  
  webSocket.onEvent(webSocketEvent);
  webSocket.begin(server_host, websocket_port, "/");
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
  delay(10);
}

void webSocketEvent(const WStype_t& type, uint8_t * payload, const size_t&) {
  if (type == WStype_TEXT) handleMessage((char*)payload);
}

void handleMessage(const char* message) {
  DynamicJsonDocument doc(512);
  if (deserializeJson(doc, message) == DeserializationError::Ok && doc.containsKey("number")) {
    int number = doc["number"];
    if (number >= 0 && number <= 9999) {
      matrix.clear();
      matrix.print(number);
      matrix.writeDisplay();
    }
  }
}
