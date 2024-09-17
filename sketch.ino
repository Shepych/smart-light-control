#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "wifi_name";
const char* password = "wifi_pass";

WiFiClientSecure client;

const int relayPin = D1; // Пин, к которому подключено реле

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
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) { 
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();   
    HTTPClient http; 

    http.begin(*client, "https://api.bigsports.ru/arduino/index.php");
    int httpCode = http.GET();

    if (httpCode > 0) { // Проверка наличия ответа от сервера
      if (httpCode == HTTP_CODE_OK) {     
        String payload = http.getString();
        Serial.println(payload);
        if(payload == "1") {
          digitalWrite(relayPin, HIGH);
        } else if (payload == "0") {
          digitalWrite(relayPin, LOW);
        }
      } else {
        Serial.printf("HTTP request failed with code: %d\n", httpCode);
      }
    } else {
      Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(1000); // Задержка 1 секунда
}