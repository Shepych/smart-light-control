#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <RCSwitch.h> // Библиотека для работы с 433 МГц
#include "config.h"   // Подключаем файл конфигурации

WiFiClientSecure client;
RCSwitch mySwitch = RCSwitch(); // Создаём объект для работы с приёмником

const int relayPin = D1; // Пин, к которому подключено реле
const int receiverPin = D2; // Пин, к которому подключён приёмник 433 МГц

const int enterCode = 7203346;
const int exitCode = 2991634;

bool openRoom = false;

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  Serial.begin(9600);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  mySwitch.enableReceive(receiverPin); // Включаем приёмник на указанном пине
}

void loop() {
  if (mySwitch.available()) {
    unsigned long receivedValue = mySwitch.getReceivedValue(); // Получаем значение


    // Получили сигнал
    if (receivedValue != 0) {
      if(receivedValue == enterCode) {
        openRoom = true;
        digitalWrite(relayPin, HIGH);
        if (WiFi.status() == WL_CONNECTED) {
          std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
          client->setInsecure();
          HTTPClient http;

          http.begin(*client, "https://jsonb.ru/meetingRoom/status_room_api.php?key=1");
          int httpCode = http.GET();
          if (httpCode > 0) {
              // Проверяем код ответа
              if (httpCode == HTTP_CODE_OK) {
                  String payload = http.getString();  // Получаем ответ от сервера
                  Serial.println("Ответ сервера: " + payload);
              } else {
                  Serial.printf("Ошибка HTTP: %s\n", http.errorToString(httpCode).c_str());
              }
          } else {
              Serial.println("Ошибка подключения");
          }
        }
      } else if(receivedValue == exitCode) {
        openRoom = false;
        digitalWrite(relayPin, LOW);
        if (WiFi.status() == WL_CONNECTED) {
          std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
          client->setInsecure();
          HTTPClient http;

          http.begin(*client, "https://jsonb.ru/meetingRoom/status_room_api.php?key=0");
          int httpCode = http.GET();
          if (httpCode > 0) {
              // Проверяем код ответа
              if (httpCode == HTTP_CODE_OK) {
                  String payload = http.getString();  // Получаем ответ от сервера
                  Serial.println("Ответ сервера: " + payload);
              } else {
                  Serial.printf("Ошибка HTTP: %s\n", http.errorToString(httpCode).c_str());
              }
          } else {
              Serial.println("Ошибка подключения");
          }
        }
      }
      Serial.print("\nReceived 433 MHz signal: ");
      Serial.println(receivedValue);
      mySwitch.resetAvailable(); // Сбрасываем состояние приемника
    }
  }
}