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
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

char buffer[200];
#define __P(f_, ...) snprintf (buffer, 200, (f_), ##__VA_ARGS__) ; content += buffer ;
//#define __P(f_, ...) snprintf (buffer, 150, (f_), ##__VA_ARGS__) ; Serial.println(buffer)

const char* valve_states[7] = {"closed", "opening", "open", "closing", "stuck open FAULT", "stuck closed FAULT", "temp sensor FAULT"};
char* ext_ip[16];
const char* boiler_states[3] = {"Off", "On", "Run-On"};
unsigned long valve_timeout[num_zones];
int valve[num_zones] = {0}; // valve status
byte ds2482_chan[num_zones];
int boiler[num_boilers] = {0}; // boiler state
long zone[num_zones] = {0}; // bit field. 0 = 00:00 to 01:00 ... 23 = 23:00 to 00:00. 24 = manual-on, 25 = manual-off
int zone_on; // bitfield
int pump_on; // bitfield
int boiler_on; // bit field
int run_on; // bit-field
byte on_temp[num_zones]; // temperature * 2
byte off_temp[num_zones];
float temp[num_zones];
int run_on_timer[num_boilers];
byte counter[4] = {0}; // up/down arrow accelleration
struct tm timeinfo;
OneWire oneWire(0); //I2C device address 0
DallasTemperature sensors(&oneWire);

// Set web server port number to 80
WebServer server(80);

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

int error_flag = 2; // 1 flash is just a heartbeat. use 2 to send reboot email

void setup() {
  Serial.begin(115200);
  sensors.begin();
  EEPROM.begin(512);
  byte connection_count = EEPROM.read(511);
  // Connect to Wi-Fi network with SSID and password
  // count failed connection attempts. Eventually give up
  Serial.print("Connection attempt ");
  Serial.print(connection_count);
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
  while (WiFi.waitForConnectResult() != WL_CONNECTED && connection_count < 60) {
    digitalWrite(BLINK_LED, HIGH);
    delay(250);
    WiFi.begin(ssid, password);
    Serial.print(WiFi.status());
    digitalWrite(BLINK_LED, LOW);
    delay(250);
    connection_count++;
  }
  EEPROM.write(511, 0); EEPROM.commit();

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
  server.on("/reset", handle_Reset); // external reset
  server.onNotFound([]() {
    server.send(404, "text/plain", "FileNotFound");
  });
 
  configTime(0, 3600, ntpServer);
  setenv ("TZ", TZstr, 1);
  tzset ();   // save the TZ variable

  // Get programming from eeprom)
  for (int i = 0; i < num_zones; i++) {
    zone[i] = EEPROM.read(i * 8 + 0) * 0x1000000L + EEPROM.read(i * 8 + 1) * 0x10000L + EEPROM.read(i * 8 + 2) * 0x100L + EEPROM.read(i * 8 + 3) * 0x1L;
    on_temp[i] =  EEPROM.read(i * 8 + 4); // 0 to 128 resolution 0.5 to allow for Farenheit
    if (on_temp[i] > 250) on_temp[i] = (units) ? 140 : 40; // in case if unintialised eeprom
    off_temp[i] = EEPROM.read(i * 8 + 5); // 0 to 128 resolution 0.5 to allow for Farenheit
    if (off_temp[i] > 250) off_temp[i] = (units) ? 80 : 10;
  }
  for (int i = 0; i < num_boilers; i++) {
    pinMode(boiler_out[i], OUTPUT);
    digitalWrite(boiler_out[i], HIGH);
  }
  for (int i = 0; i < num_zones; i++) {
    pinMode(zone_out[i], OUTPUT);
    digitalWrite(zone_out[i], HIGH);
  }
  for (int i = 0; i < num_zones; i++) pinMode(zone_in[i], INPUT_PULLUP);
  for (int i = 0; i < num_pumps; i++) {
    pinMode(pump_out[i], OUTPUT);
    digitalWrite(pump_out[i], HIGH);
  }

  // Find which DS2482 channel each DS18B20 sensor can be found on
  for (int i = 0; i < num_zones; i++) {
    ds2482_chan[i] = 255;
    temp[i] = -127;
  }
  for (byte c = 0; c < 8; c++) {
    byte addr[8];
    oneWire.setChannel(c);
    sensors.begin();
    Serial.printf("Checking channel %d\n", c);
    int j = sensors.getDS18Count() - 1;
    for (; j >= 0; j--){
      sensors.getAddress(addr, j);
      Serial.printf("    Found device addr 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
                     addr[0], addr[1],  addr[2], addr[3],  addr[4], addr[5],  addr[6], addr[7]);
    }
    for (int i = 0; i < num_zones; i++) {
      if (sensors.isConnected(ds18b20[i])) {
        Serial.printf("    Sensor for zone %i found\n", i);
        ds2482_chan[i] = c;
      }
    }
  }

  server.begin();
}

bool temp_valid(int z) {
  if (! units) {
    if (temp[z] > 80 || temp[z] < -100) return 0;
  } else {
    if (temp[z] > 180 || temp[z] < -100) return 0;
  }
  return 1;
}

// This function handles blink-codes and watches for a new web interface connection
// returns a true if there is a connection to handle, NULL otherwise
void do_status() {
  static int old_error;
  int i;
  static int c_counter;
  if (WiFi.status() != WL_CONNECTED){
    error_flag = 2;
    c_counter +=1;
    if (c_counter > 30){ // wait 5 mins between connection attempts
      Serial.println("Reconnection attempt");
      WiFi.begin(ssid, password);
      c_counter = 0;
    }
  }
  for (i = 0; i < 10; i++) {
    if (WiFi.status() == WL_CONNECTED) server.handleClient();
    if (i < error_flag) {
      digitalWrite(BLINK_LED, HIGH);
      delay(200);
      digitalWrite(BLINK_LED, LOW);
      delay(200);
    } else {
      digitalWrite(BLINK_LED, LOW);
      delay(400);
    }
  }

  // send an email message about the error
    
  if (old_error == error_flag || error_flag <= 1) return; // don't send same email twice
  if (WiFi.status() != WL_CONNECTED) return; // don't attempt to send an email with no connection
  
  if (error_email != "\0") {
    SMTPData smtpData;
    smtpData.setLogin(smtp_server, smtp_port, error_email,error_password);
    smtpData.setSender("Heating Controller", error_email);
    smtpData.addRecipient(error_recipient);
    smtpData.setSubject("Heating fault code detected " + WiFi.localIP().toString());
    String content = "Current status of controller:\n\n";
    
    HTTPClient http;
    http.begin("https://api.ipify.org/?format=text");
    int httpCode = http.GET();
    __P("External IP = http://");
    content += http.getString();
    http.end();

    __P("\n\nError code %i\n\n", error_flag);
    for (int i = 0; i < num_boilers; i++) { __P("Boiler %d %s\n", i, boiler_states[boiler[i]]);}
    __P("\n");
    for (int i = 0; i < num_zones; i++) { __P("Valve %d %s\n", i, valve_states[valve[i]]);}
    __P("\n");
    for (int i = 0; i < num_pumps; i++) { __P("Pump %d %s\n", i, (zone_on & pump_mask[i])?"On":"Off");}
    smtpData.setMessage(content, false);
    MailClient.sendMail(smtpData);
  }
  if (error_flag == 2) error_flag = 1; // reconnected
  old_error = error_flag;
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
    if (ds2482_chan[z] < 8){
      oneWire.setChannel(ds2482_chan[z]);
      sensors.requestTemperatures();
      if (units) {
        temp[z] = sensors.getTempF(ds18b20[z]);
      } else {
        temp[z] = sensors.getTempC(ds18b20[z]);
      }
    }
    if ( ! temp_valid(z)) {
      valve[z] = 6;
    }
    if ((bitRead(zone[z], timeinfo.tm_hour) || bitRead(zone[z], 24)) && ! bitRead(zone[z], 25)) {
      demand_temp = on_temp[z] / 2.0;
    } else {
      demand_temp = off_temp[z] / 2.0;
      off_flag = true; // Used to reset valve-stuck-closed
    }
    /*************************************
          Valve control state machine
     *************************************/
    switch (valve[z]) {
      case 0: // closed
        if (temp[z] < demand_temp - hyst) {
          digitalWrite(zone_out[z], LOW);
          valve[z] = 1;
          valve_timeout[z] = millis();
          Serial.printf("Turning On zone %d\n", z);
        }
        break;
      case 1: // valve opening
        if (! digitalRead(zone_in[z])) { // valve has opened
          bitSet(zone_on, z);
          valve[z] = 2;
          break;
        }
        if (millis() - valve_timeout[z] > 60000) {
          error_flag = 5;
          valve[z] = 5;
        }
        break;
      case 2: // Valve opened
        if (temp[z] > demand_temp + hyst) {
          digitalWrite(zone_out[z], HIGH);
          valve[z] = 3;
          valve_timeout[z] = millis();
          bitClear(zone_on, z);
          Serial.printf("Turning Off zone %d\n", z);
        }
        break;
      case 3: // valve closing
        if (digitalRead(zone_in[z])) { // valve has closed
          valve[z] = 0;
          break;
        }
        if (millis() - valve_timeout[z] > 60000) { // timeout closing
          valve[z] = 4;
          error_flag = 4;
        }
      case 4: // Stuck Open fault: We can still control temp, and it might close
        if (digitalRead(zone_in[z])) { // valve has finally closed
          bitSet(zone_on, z);
          valve[z] = 0;
          error_flag = 1;
          break;
        }
        if (temp[z] > demand_temp + hyst) {
          bitClear(zone_on, z);
        } else if (temp[z] < demand_temp - hyst) {
          bitSet(zone_on, z);
        }
        break;
      case 5: // stuck closed fault
        digitalWrite(zone_out[z], HIGH); // avoid overheating motor
        bitClear(zone_on, z); // turn off the pump
        if (off_flag) {
          valve[z] = 0;
          error_flag = 1;
        }
        break;
      case 6: // Temperature sensor fault
        // allow it to fix itself
        if (temp_valid(z)) {
          digitalWrite(zone_out[z], HIGH);
          valve[z] = 0;
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
    switch (boiler[i]) {
      case 0: // boiler off
        if (zone_on && boiler_mask[i]) {
          bitSet(boiler_on, i);
          digitalWrite(boiler_out[i], LOW);
          boiler[i] = 1;
        }
        break;
      case 1: // boiler on
        if (! zone_on && boiler_mask[i]) {
          run_on_timer[i] = millis();
          bitClear(boiler_on, i);
          bitSet(run_on, i);
          boiler[i] = 2;
          digitalWrite(boiler_out[i], HIGH);
        }
        break;
      case 2: // run-on timer
        // check for timeout _or_ re-activation
        if (((millis() - run_on_timer[i]) > run_on_time) || (zone_on && boiler_mask[i])) {
          bitClear(run_on, i);
          boiler[i] = 0;
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
    if ((pump_mask[i] & zone_on) || (run_on_mask[i] & run_on)) {
      digitalWrite(pump_out[i], LOW);
    } else {
      digitalWrite(pump_out[i], HIGH);
    }
  }
}


void handle_OnConnect() {
  // Display the status web page
  char timeStr[50];
  strftime(timeStr, sizeof(timeStr), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  Serial.println("New Connection");
  String content = "<!DOCTYPE html>";
  __P("<head><meta http-equiv=\"refresh\" content=\"60\"></head>");
  __P("<html>");
  __P("<link rel=\"icon\" href=\"data:,\">");
  __P("<body>");
  __P("<svg svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" width=\"297mm\" height=\"210mm\">");
  __P("<text y=\"70\" x=\"60\" font-size=\"40\" fill=\"#ao5a2c\" font-family=\"Times\" > %s </text>", timeStr);
  for (int i = 0; i < num_boilers; i++) {
    __P("<text y=\"%d\" x=\"80\" font-size=\"30\" dominant-baseline=\"middle\" text-anchor=\"left\" fill=\"0\" font-family=\"Times\" >Boiler %d %s</text>",
        35 * i + 110, i, boiler_states[boiler[i]]);
  }
  for (int i = 0; i < num_zones; i++) {
    __P("<text y=\"%d\" x=\"80\" font-size=\"30\" dominant-baseline=\"middle\" text-anchor=\"left\" fill=\"0\" font-family=\"Times\" >Valve  %d %s</text>",
        35 * i + 130 + 30 * num_boilers, i, valve_states[valve[i]]);
  }
  for (int i = 0; i < num_pumps; i++) {
    __P("<text y=\"%d\" x=\"475\" font-size=\"30\" dominant-baseline=\"middle\" text-anchor=\"left\" fill=\"0\" font-family=\"Times\" >Pump  %d %s</text>",
        35 * i + 130 + 30 * num_boilers, i, digitalRead(pump_out[i])?"Off":"On");
  }
  for (int i = 0; i < num_zones; i++) {
    __P("<a xlink:href=\"set?zone=%d\"><rect y=\"%d\" x=\"%d\" height=\"%d\" width=\"%d\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/></a>",
        i, zone_pos[i][0], zone_pos[i][1], zone_pos[i][2], zone_pos[i][3], bitRead(zone_on, i) ? on_colour : off_colour);
    __P("<text y=\"%d\" x=\"%d\" font-size=\"20\" dominant-baseline=\"middle\" text-anchor=\"middle\" fill=\"0\" font-family=\"Times\" >%s</text>",
        zone_pos[i][0] + zone_pos[i][2] - 20,
        zone_pos[i][1] + zone_pos[i][3] / 2,
        zone_names[i]);
    __P("<text y=\"%d\" x=\"%d\" font-size=\"40\" dominant-baseline=\"middle\" text-anchor=\"middle\" fill=\"0\" font-family=\"Times\" >%d.%1d%s</text>",
        zone_pos[i][0] + 60,
        zone_pos[i][1] + zone_pos[i][3] / 2,
        (int)temp[i], (int)(temp[i] * 10) % 10,
        units ? "&#8457" : "&#8451");
  }
  __P("</svg>");
  __P("</body></html>");
  server.send(200, "text/html", content);
}

void handle_OnSet() {
  int z = server.arg("zone").toInt();
  Serial.printf("Setting Screen: setting zone %d\n", z);
  program(z);
}
// Programming Screen
void program(int z) {
  if (z >= 0 && z < num_zones) {
    String content = "<!DOCTYPE html>";
    __P("<head> </head>");
    __P("<html>");
    __P("<link rel=\"icon\" href=\"data:,\">");
    __P("<body>");
    __P("<svg svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" width=\"297mm\" height=\"210mm\">");
    // Header
    __P("<text y=\"100\" x=\"60\" font-size=\"30\" fill=\"0\" font-family=\"Times\" >%s</text>", zone_names[z]);
    // Setting clock faces
    for (int h = 0; h < 24; h++) {
      int r = 100;
      __P("<a xlink:href=\"hour?zone=%02d&h=%02d\"><path d=\"M 0 0 L %d %d A %d %d 0 0 1 %d %d L 0 0\" style=\"fill:%s;stroke:#000000;stroke-opacity:1\" transform=\"translate(%d,300)\" /></a>",
          z, h,
          (int)(r * sin(0.5236 * h)), (int)(-r * cos(0.5236 * h)),
          r, r, (int)(r * sin(0.5236 * (h + 1))), (int)(-r * cos(0.5236 * (h + 1))),
          (zone[z] & (1 << 24 | 1 << h) && !(zone[z] & 1 << 25)) ? on_colour : off_colour,
          (h < 12) ? 200 : 500);
      __P("<text x=\"%d\" y=\"%d\" font-size=\"15\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"middle\" font-family=\"Times\" transform=\"translate(%d,300)\"> %02d </text>",
          (int)((r + 20) * sin(0.5236 * h)), (int)(-(r + 20) * cos(0.5236 * h)),
          (h < 12) ? 200 : 500,
          h);
    }
    // Set / display on temperature
    __P("<rect y=\"480\" x=\"130\" height=\"80\" width=\"80\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/>", on_colour);
    __P("<text y=\"520\" x=\"170\" font-size=\"30\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"middle\" font-family=\"Times\"> %d.%1d </text>",
        (int)(on_temp[z] / 2), (int)(on_temp[z] * 5) % 10);
    __P("<a xlink:href=\"on_step?zone=%02d&d=10\"> <path d=\"M 129 475 l 54 0 l -27 -25 z\" /></a>", z);
    __P("<a xlink:href=\"on_step?zone=%02d&d=-10\"><path d=\"M 129 565 l 54 0 l -27  25 z\" /></a>", z);
    __P("<a xlink:href=\"on_step?zone=%02d&d=1\">  <path d=\"M 184 475 l 27 0 l -14 -13 z\" /></a>", z);
    __P("<a xlink:href=\"on_step?zone=%02d&d=-1\"> <path d=\"M 184 565 l 27 0 l -14  13 z\" /></a>", z);
    // Set / display off temperature
    __P("<rect y=\"480\" x=\"290\" height=\"80\" width=\"80\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/>", off_colour);
    __P("<text y=\"520\" x=\"330\" font-size=\"30\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"middle\" font-family=\"Times\"> %d.%1d </text>",
        (int)(off_temp[z] / 2), (int)(off_temp[z] * 5) % 10);
    __P("<a xlink:href=\"off_step?zone=%02d&d=10\"> <path d=\"M 289 475 l 54 0 l -27 -25 z\" /></a>", z);
    __P("<a xlink:href=\"off_step?zone=%02d&d=-10\"><path d=\"M 289 565 l 54 0 l -27  25 z\" /></a>", z);
    __P("<a xlink:href=\"off_step?zone=%02d&d=1\">  <path d=\"M 344 475 l 27 0 l -14 -13 z\" /></a>", z);
    __P("<a xlink:href=\"off_step?zone=%02d&d=-1\"> <path d=\"M 344 565 l 27 0 l -14  13 z\" /></a>", z);
    // AUTO Button
    __P("<a xlink:href=\"auto?zone=%d&a=0\"><rect y=\"460\" x=\"460\" height=\"25\" width=\"25\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/></a>", z, (zone[z] & 0x3000000) ? "#FFFFFF" : "#FF0000");
    __P("<text y=\"472\" x=\"500\" font-size=\"30\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"left\" font-family=\"Times\"> AUTO </text>");
    // Manual ON button
    __P("<a xlink:href=\"auto?zone=%d&a=1\"><rect y=\"500\" x=\"460\" height=\"25\" width=\"25\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/></a>", z,  (zone[z] & 0x1000000) ? "#FF0000" : "#FFFFFF");
    __P("<text y=\"512\" x=\"500\" font-size=\"30\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"left\" font-family=\"Times\"> Manual ON </text>");
    // Manual OFF button
    __P("<a xlink:href=\"auto?zone=%d&a=2\"><rect y=\"540\" x=\"460\" height=\"25\" width=\"25\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/></a>", z,  (zone[z] & 0x2000000) ? "#FF0000" : "#FFFFFF");
    __P("<text y=\"562\" x=\"500\" font-size=\"30\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"left\" font-family=\"Times\"> Manual OFF </text>");
    // Back link
    __P("<a xlink:href=\"../\"><text y=\"650\" x=\"60\" font-size=\"20\" fill=\"#0000FF\" font-family=\"Times\" >Back to Main Screen</text></a>");
    __P("</svg>");
    __P("</body></html>");
    server.send(200, "text/html", content);
  }
}

void handle_OnHour() {
  int z = server.arg("zone").toInt();
  int h = server.arg("h").toInt();
  if (z < 0 || z > num_zones - 1 || h < 0 || h > 23) return;
  zone[z] &= 0x00FFFFFF; // set to auto mode if user tries to program
  zone[z] ^= (1L << h);
  EEPROM.write(z * 8 + 1, (zone[z] & 0x00FF0000) >> 16);
  EEPROM.write(z * 8 + 2, (zone[z] & 0x0000FF00) >>  8);
  EEPROM.write(z * 8 + 3, (zone[z] & 0x000000FF));
  EEPROM.commit();
  program(z);
}
void handle_OnAuto() {
  int z = server.arg("zone").toInt();
  int a = server.arg("a").toInt();
  if (z < 0 || z > num_zones - 1 || a < 0 || a > 2) return;
  zone[z] &= 0x00FFFFFF;
  zone[z] += (long)a * 0x1000000;
  EEPROM.write(z * 8 + 0, (zone[z] & 0xFF000000) >> 24);
  EEPROM.commit();
  program(z);
}
void handle_OnStep() {
  int z = server.arg("zone").toInt();
  int d = server.arg("d").toInt();
  if (z < 0 || z > num_zones - 1) return;
  on_temp[z] += d;
  EEPROM.write(z * 8 + 4, on_temp[z]);
  EEPROM.commit();
  program(z);
}
void handle_OffStep() {
  int z = server.arg("zone").toInt();
  int d = server.arg("d").toInt();
  if (z < 0 || z > num_zones - 1) return;
  counter[0] = counter[1] = counter[3] = 0;
  off_temp[z] += d;
  EEPROM.write(z * 8 + 5, off_temp[z]);
  EEPROM.commit();
  program(z);
}

void handle_UpdateTemp() {
  int z = server.arg("zone").toInt();
  int t = server.arg("temp").toFloat();
  if (z < 0 || z > num_zones - 1) return;
  temp[z] = t;
  String content = "<!DOCTYPE html>";
  __P("Zone %d set to temperature %d\n", z, t);
  server.send(200, "text/html", content);
}

void handle_Reset(){
  ESP.restart();
}
