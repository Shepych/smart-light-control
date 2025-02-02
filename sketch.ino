#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <RCSwitch.h>
#include "config.h"

WiFiClientSecure client;
RCSwitch mySwitch = RCSwitch();

const int relayPin = D1;
const int receiverPin = D2;
const int enterCode = 7203346;
const int exitCode = 2991634;

bool openRoom = false;
unsigned long lastWifiCheck = 0;
const unsigned long wifiCheckInterval = 30000;
bool wifiConnecting = false;

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  mySwitch.enableReceive(receiverPin);
}

void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED && !wifiConnecting) {
    Serial.println("WiFi disconnected. Attempting to reconnect...");
    wifiConnecting = true;
    WiFi.disconnect();
    WiFi.begin(ssid, password);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi reconnected!");
    wifiConnecting = false;
  }
}

void sendHttpRequest(const char* url) {
  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient http;

    http.begin(*client, url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        Serial.println("Ответ сервера: " + http.getString());
      } else {
        Serial.printf("Ошибка HTTP: %s\n", http.errorToString(httpCode).c_str());
      }
    } else {
      Serial.println("Ошибка подключения");
    }
    http.end();
  } else {
    Serial.println("WiFi не подключен. HTTP-запрос не отправлен.");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastWifiCheck >= wifiCheckInterval) {
    lastWifiCheck = currentMillis;
    reconnectWiFi();
  }

  if (mySwitch.available()) {
    unsigned long receivedValue = mySwitch.getReceivedValue();
    if (receivedValue != 0) {
      if (receivedValue == enterCode) {
        openRoom = true;
        digitalWrite(relayPin, HIGH);
        sendHttpRequest("https://jsonb.ru/meetingRoom/status_room_api.php?key=1");
      } else if (receivedValue == exitCode) {
        openRoom = false;
        digitalWrite(relayPin, LOW);
        sendHttpRequest("https://jsonb.ru/meetingRoom/status_room_api.php?key=0");
      }
      Serial.print("\nReceived 433 MHz signal: ");
      Serial.println(receivedValue);
      mySwitch.resetAvailable();
    }
  }
}
