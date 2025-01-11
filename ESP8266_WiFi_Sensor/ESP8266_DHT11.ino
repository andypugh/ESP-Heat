#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>

#define WIFI_SSID "slaithwaitehall"
#define WIFI_PASSWORD "harriet123456"
#define DHTPIN D4
#define DHTTYPE DHT11  // DHT 11
#define LED 2
#define DHTTYPE    DHT11     // DHT 11

DHT dht(DHTPIN, DHTTYPE);
char zone;
void setup() {
  EEPROM.begin(1);
  EEPROM.get(0, zone);
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  Serial.printf("Zone is set to %i\n", zone);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    digitalWrite(LED, LOW);  // turn the LED on.
    delay(100);
    digitalWrite(LED, HIGH);
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  char serverPath[50];
  int ret;
  float temperature;

    
  if (Serial.available()){
    char buf[4];
    for(int i = 0; Serial.available() && i < 4; buf[i++] = Serial.read());
    zone = atoi(buf);
    Serial.printf("Setting the zone for this device to %i\n", zone);
    EEPROM.write(0, zone);
    EEPROM.commit();
  }

  delay(100);             // wait for 1 second.
  digitalWrite(LED, HIGH);  // turn the LED on.
  delay(10000);
  digitalWrite(LED, LOW);

  temperature = dht.readTemperature();

  if(WiFi.status()== WL_CONNECTED && ! isnan(temperature)){
      WiFiClient client;
      HTTPClient http;

      int ret = sprintf(serverPath, "http://heating.local/update_temp?zone=%i&temp=%02.2f", zone, temperature);
      Serial.println(serverPath);
      http.begin(client, serverPath);
  
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }

}
