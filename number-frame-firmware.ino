#include <ESP8266WiFi.h>
#include <WebSocketsClient_Generic.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "config.h"

const char* websocket_host = "number-frame-server.noshado.ws";
const int websocket_port = 80;

Adafruit_7segment matrix = Adafruit_7segment();
WebSocketsClient webSocket;

void setup() {
  Serial.begin(115200);
  
  Wire.begin(4, 5);
  matrix.begin(0x70);
  matrix.setBrightness(10);
  matrix.clear();
  matrix.print(0);
  matrix.writeDisplay();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  webSocket.onEvent(webSocketEvent);
  webSocket.begin(websocket_host, websocket_port, "/");
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
  delay(10);
}

void webSocketEvent(const WStype_t& type, uint8_t * payload, const size_t& length) {
  switch(type) {
    case WStype_CONNECTED:
      webSocket.sendTXT("Number frame connected");
      break;
    case WStype_TEXT:
      handleMessage((char*)payload);
      break;
  }
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
