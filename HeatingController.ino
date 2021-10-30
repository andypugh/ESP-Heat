/*********
  Andy Pugh 2020

  Based on work by Rui Saontos: https://randomnerdtutorials.com
*********/

// Load Wi-Fi library
#include "credentials.h"
#include <WiFi.h>
#include <WebServer.h>
#include "ESP32_MailClient.h"
#include "time.h"
#include "EEPROM.h"
// https://github.com/cybergibbons/DS2482_OneWire _NOT_ the standard library
// If using the DS2482. Otherwise use the standard OneWire lib
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"
#include "globals.h"

#define __P(f_, ...) snprintf (buffer, 200, (f_), ##__VA_ARGS__) ; content += buffer ;
//#define __P(f_, ...) snprintf (buffer, 200, (f_), ##__VA_ARGS__) ; Serial.println(buffer);

#ifdef USE_DS2482
OneWire oneWire(0); //I2C device address 0
#else
OneWire oneWire(23); //Bitbanged device, pin 23
#endif

DallasTemperature sensors(&oneWire);

// Set web server port number to 80
WebServer server(80);

int error_flag = 2; // 1 flash is just a heartbeat. use 2 to send reboot email

void setup() {
  int connection_count;
  Serial.begin(115200);
  sensors.begin();
  EEPROM.begin(1024);

  // Get setup from eeprom
  read_EEPROM(EEPROM_BASE);
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print(" Connecting to ");
  Serial.println(ssid);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname);
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  pinMode(BLINK_LED, OUTPUT);
  do {
    digitalWrite(BLINK_LED, HIGH);
    delay(250);
    WiFi.begin(ssid, password);
    Serial.print(WiFi.status());
    digitalWrite(BLINK_LED, LOW);
    delay(250);
    connection_count++;
  } while (WiFi.waitForConnectResult() != WL_CONNECTED && connection_count < 60);

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.printf("esp_idf_version %s\n", esp_get_idf_version());
  server.on("/", handle_OnConnect);
  server.on("/set", handle_OnSet);
  server.on("/hour", handle_OnHour);
  server.on("/on_step", handle_OnStep);
  server.on("/off_step", handle_OffStep);
  server.on("/auto", handle_OnAuto);
  server.on("/update-temp", handle_UpdateTemp); // take temperature data from remote sensors via URL eg update-temp?zone=3&temp=20
  server.on("/settings", handle_Settings);
  server.on("/newsettings", handle_newsettings);
  server.on("/reset", handle_Reset); // external reset
  server.onNotFound([]() {
     server.send(404, "text/plain", "FileNotFound");
  });
 
  configTime(0, 3600, ntpServer);
  setenv ("TZ", TZstr, 1);
  tzset ();   // save the TZ variable

  // Get programming from eeprom)
  for (int i = 0; i < num_zones; i++) {
    zones[i].hours= EEPROM.read(i * 8 + 0) * 0x1000000L + EEPROM.read(i * 8 + 1) * 0x10000L + EEPROM.read(i * 8 + 2) * 0x100L + EEPROM.read(i * 8 + 3) * 0x1L;
    zones[i].on_temp =  EEPROM.read(i * 8 + 4); // 0 to 128 resolution 0.5 to allow for Farenheit
    if (zones[i].on_temp > 250) zones[i].on_temp = (units) ? 140 : 40; // in case if unintialised eeprom
    zones[i].off_temp = EEPROM.read(i * 8 + 5); // 0 to 128 resolution 0.5 to allow for Farenheit
    if (zones[i].off_temp > 250) zones[i].off_temp = (units) ? 80 : 10;
  }
  for (int i = 0; i < num_boilers; i++) {
    pinMode(boilers[i].out_pin, OUTPUT);
    digitalWrite(boilers[i].out_pin, HIGH);
  }
  for (int i = 0; i < num_zones; i++) {
    pinMode(zones[i].out_pin, OUTPUT);
    digitalWrite(zones[i].out_pin, HIGH);
  }
  for (int i = 0; i < num_zones; i++) pinMode(zones[i].in_pin, INPUT_PULLUP);
  for (int i = 0; i < num_pumps; i++) {
    pinMode(pumps[i].out_pin, OUTPUT);
    digitalWrite(pumps[i].out_pin, HIGH);
  }

  // Find which DS2482 channel each DS18B20 sensor can be found on

  num_sensors = 0;
  
#ifdef USE_DS2482

  Serial.println("Searching for DS18B20 Sensors on DS2482");
  for (byte c = 0; c < 8; c++) {
    oneWire.setChannel(c);
    sensors.begin();
    Serial.printf("Checking channel %d\n", c);
    for (int j = sensors.getDS18Count() - 1; j >= 0; j--){
      sensors.getAddress(all_sensors[num_sensors].address, j);
      byte *a = all_sensors[num_sensors].address;
      Serial.printf("Found device addr %02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x\n",
                     a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
      all_sensors[num_sensors].channel = c;
      num_sensors = min(max_sensors, ++num_sensors);
    }
    for (int z = 0; z < num_zones; z++) {
      zones[z].temp=-127;
      if (sensors.isConnected(zones[z].sensor.address) && zones[z].sensor.channel >= 0) {
        Serial.printf("    Sensor for zone %i found\n", z);
        zones[z].sensor.channel = c;
      }
    }
  }
  
#else

  Serial.println("Searching for DS18B20 Sensors");
  DeviceAddress addr;
  while (! oneWire.search(addr)){
    oneWire.reset();
    delay(250);
  }
  oneWire.reset_search();
  while (oneWire.search(all_sensors[num_sensors].address)){
    byte *a = all_sensors[num_sensors].address;
    Serial.printf("Found device addr %02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x\n",
                   a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
    num_sensors = min(max_sensors, ++num_sensors);
  }
  for (int z = 0; z < num_zones; z++) {
    zones[z].temp=-127;
    if (sensors.isConnected(zones[z].sensor.address) && zones[z].sensor.channel >= 0) {
      Serial.printf("    Sensor for zone %i found\n", z);
    }
  }

#endif
  Serial.printf("Found a total of %i sensors\n", num_sensors);

  server.begin();
}

bool temp_valid(int z) {
  if (! units) {
    if (zones[z].temp > 80 || zones[z].temp < -100) return 0;
  } else {
    if (zones[z].temp > 180 || zones[z].temp < -100) return 0;
  }
  return 1;
}

void loop() {
  int z = -1; // zone index
  int h = -1; // hour index
  char header[21] = {0}; int r = 0; // header buffer and index, ignore all but the first 20 chars
  do_status(); // blink-codes and connection housekeeping
  getLocalTime(&timeinfo);
  for (z = 0; z < num_zones; z++) {
    float demand_temp;
    bool off_flag = 0;

#ifdef USE_DS2482
    if (zones[z].sensor.channel <= 7 && zones[z].sensor.channel >=0 ){
      oneWire.setChannel(zones[z].sensor.channel);
    }
#endif

    sensors.requestTemperatures();
    if (units) {
      zones[z].temp = sensors.getTempF(zones[z].sensor.address);
    } else {
      zones[z].temp = sensors.getTempC(zones[z].sensor.address);
    }
    if ( ! temp_valid(z)) {
      zones[z].state = 6;
    }
    if ((bitRead(zones[z].hours, timeinfo.tm_hour) || bitRead(zones[z].hours, 24)) && ! bitRead(zones[z].hours, 25)) {
      demand_temp = zones[z].on_temp / 2.0;
    } else {
      demand_temp = zones[z].off_temp / 2.0;
      off_flag = true; // Used to reset valve-stuck-closed
    }
    /*************************************
          Valve control state machine
     *************************************/
    switch (zones[z].state) {
      case 0: // closed
        if (zones[z].temp < demand_temp - hyst) {
          digitalWrite(zones[z].out_pin, LOW);
          zones[z].state = 1;
          zones[z].timeout = millis();
          Serial.printf("Turning On zone %d\n", z);
        }
        break;
      case 1: // valve opening
        if (! digitalRead(zones[z].in_pin)) { // valve has opened
          bitSet(zone_on, z);
          zones[z].state = 2;
          break;
        }
        if (millis() - zones[z].timeout > 60000) {
          error_flag = 5;
          zones[z].state = 5;
        }
        break;
      case 2: // Valve opened
        if (zones[z].temp > demand_temp + hyst) {
          digitalWrite(zones[z].out_pin, HIGH);
          zones[z].state = 3;
          zones[z].timeout = millis();
          bitClear(zone_on, z);
          Serial.printf("Turning Off zone %d\n", z);
        }
        break;
      case 3: // valve closing
        if (digitalRead(zones[z].in_pin)) { // valve has closed
          zones[z].state = 0;
          break;
        }
        if (millis() - zones[z].timeout > 60000) { // timeout closing
          zones[z].state = 4;
          error_flag = 4;
        }
      case 4: // Stuck Open fault: We can still control temp, and it might close
        if (digitalRead(zones[z].in_pin)) { // valve has finally closed
          bitSet(zone_on, z);
          zones[z].state = 0;
          error_flag = 1;
          break;
        }
        if (zones[z].temp > demand_temp + hyst) {
          bitClear(zone_on, z);
        } else if (zones[z].temp < demand_temp - hyst) {
          bitSet(zone_on, z);
        }
        break;
      case 5: // stuck closed fault
        digitalWrite(zones[z].out_pin, HIGH); // avoid overheating motor
        bitClear(zone_on, z); // turn off the pump
        if (off_flag) {
          zones[z].state = 0;
          error_flag = 1;
        }
        break;
      case 6: // Temperature sensor fault
        // allow it to fix itself
        if (temp_valid(z)) {
          digitalWrite(zones[z].out_pin, HIGH);
          zones[z].state = 0;
          error_flag = 1;
        } else {
          error_flag = 6;
        }
        break;
      default:
        error_flag = 10;
    }
  }

  /*********************************
         Boiler State Machine
   *********************************/
  for (int i = 0; i < num_boilers; i++) {
    switch (boilers[i].state) {
      case 0: // boiler off
        if (zone_on && boilers[i].mask) {
          bitSet(boiler_on, i);
          digitalWrite(boilers[i].out_pin, LOW);
          boilers[i].state = 1;
        }
        break;
      case 1: // boiler on
        if (! zone_on && boilers[i].mask) {
          boilers[i].run_on_timer = millis();
          bitClear(boiler_on, i);
          bitSet(run_on, i);
          boilers[i].state = 2;
          digitalWrite(boilers[i].out_pin, HIGH);
        }
        break;
      case 2: // run-on timer
        // check for timeout _or_ re-activation
        if (((millis() - boilers[i].run_on_timer) > run_on_time) || (zone_on && boilers[i].mask)) {
          bitClear(run_on, i);
          boilers[i].state = 0;
          break;
        }
        break;
      default:
        error_flag = 10;
    }
  }
  /****************************
            Pump Control
   ****************************/
  for (int i = 0; i < num_pumps; i++) {
    if ((pumps[i].mask & zone_on) || (pumps[i].run_on_mask & run_on)) {
      digitalWrite(pumps[i].out_pin, LOW);
    } else {
      digitalWrite(pumps[i].out_pin, HIGH);
    }
  }
}
