/*********
  Andy Pugh 2020
  
  Based on work by Rui Saontos: https://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include "credentials.h"
#include <WiFi.h>
#include "time.h"
#include "EEPROM.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define __P(f_, ...) snprintf (buffer, 200, (f_), ##__VA_ARGS__) ; client.println(buffer) ;
//#define __P(f_, ...) snprintf (buffer, 150, (f_), ##__VA_ARGS__) ; Serial.println(buffer)

// **************************CONFIGURATION************************************************

const char* hostname = "heating";
const char* ntpServer = "pool.ntp.org";
const char *TZstr = "GMT0BST,M3.3.0/1,M10.5.0";
// maximum num_zones is EEPROM.length() / 8
#define num_zones 5
#define num_pumps 3
#define num_boilers 1
float hyst = 0.5; // temperature hysteresis
int units = 0; // Set to 1 for Fahrenheit
// GPIO Pins
int boiler_out[num_boilers] = {33};
int zone_out[num_zones] = {25, 26, 27, 14, 19};
int zone_in[num_zones] = {36, 39, 34, 35, 32};
int pump_out[num_pumps] = {22, 1, 21};
const char* zone_names[num_zones]={"Far End", "Solar", "Hall", "Downstairs", "Upstairs"};
int zone_pos[num_zones][4] = {{350, 60,190,190},
                              {350,250,190,140}, 
                              {350,390,190,200},
                              {350,590,270,180},
                              { 50,690,270,180}};
OneWire oneWire(23); // OneWire GPIO pin
DeviceAddress ds18b20[num_zones] = {{0x28, 0x23, 0x58, 0xC0, 0x32, 0x20, 0x01, 0x6D},
                                    {0x28, 0x7B, 0xB7, 0xE4, 0x32, 0x20, 0x01, 0x29},
                                    {0x28, 0x58, 0xBB, 0x12, 0x33, 0x20, 0x01, 0x34},
                                    {0x28, 0xA0, 0x02, 0x8A, 0x32, 0x20, 0x01, 0x0D},
                                    {0x28, 0x4E, 0x22, 0x41, 0x33, 0x20, 0x01, 0xC9}};
//For each pump the pump mask is AND-ed with the active zone bitfield. Non-zero turns on the pump
int pump_mask[num_pumps] = {0x01, 0x06, 0x18};
//For each pump the pump mask is AND-ed with the boiler run-on bitfield. Non-zero turns on the pump
int run_on_mask[num_pumps] = {0x0, 0x0, 0x1};
//For each boiler the pump mask is AND-ed with the active zone bitfield. Non-zero turns on the pump
int boiler_mask[num_boilers] = {0x1F};
int run_on_time = 120;
const char* off_colour = "#aaffaa";
const char* on_colour = "#ffaaaa";

// **************************/CONFIGURATION***********************************************

const char* valve_states[7] ={"closed", "opening", "open", "closing", "stuck open FAULT", "stuck closed FAULT", "temp sensor FAULT"};
const char* boiler_states[3] = {"Off", "On", "Run-On"};
int valve_timeout[num_zones];
int valve[num_zones] = {0}; // valve status
int boiler[num_boilers] = {0}; // boiler state
long zone[num_zones] = {0}; // bit field. 0 = 00:00 to 01:00 ... 23 = 23:00 to 00. 24 = manual-on, 25 = manual-off
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
DallasTemperature sensors(&oneWire);

// Set web server port number to 80
WiFiServer server(80);
// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  byte connection_count = EEPROM.read(511);
  // Connect to Wi-Fi network with SSID and password
   // count failed connection attempts. Eventually give up
  Serial.print("Connection attempt ");
  Serial.print(connection_count);
  Serial.print(" Connecting to ");
  Serial.println(ssid);
  WiFi.disconnect();
  WiFi.setHostname(hostname);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && connection_count < 6) {
    int i;
    
    delay(500);
    Serial.print(".");
    if (i++ > 30){
      Serial.println("Restarting");
      EEPROM.write(511, connection_count + 1);
      EEPROM.commit();
      ESP.restart();
    }
  }
  EEPROM.write(511, 0); EEPROM.commit();
  
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  configTime(0, 3600, ntpServer);
  setenv ("TZ", TZstr, 1);
  tzset ();   // save the TZ variable
  
  // Get programming from eeprom)
  for (int i = 0; i < num_zones; i++){
    zone[i]=EEPROM.read(i * 8 + 0) * 0x1000000L + EEPROM.read(i * 8 + 1) * 0x10000L + EEPROM.read(i * 8 + 2) * 0x100L + EEPROM.read(i * 8 + 3) * 0x1L;
    on_temp[i] =  EEPROM.read(i * 8 + 4); // 0 to 128 resolution 0.5 to allow for Farenheit
    if (on_temp[i] > 250) on_temp[i] = (units)?140:40; // in case if unintialised eeprom
    off_temp[i] = EEPROM.read(i * 8 + 5); // 0 to 128 resolution 0.5 to allow for Farenheit
    if (off_temp[i] > 250) off_temp[i] = (units)?80:10;
  }
  for (int i = 0; i < num_boilers; i++) {
    pinMode(boiler_out[i], OUTPUT);
    digitalWrite(boiler_out[i], HIGH);
  }
  for (int i = 0; i < num_zones; i++) {
    pinMode(zone_out[i], OUTPUT);
    digitalWrite(zone_out[i], HIGH);
  }
  for (int i = 0; i < num_zones; i++) pinMode(zone_in[i], INPUT);
  for (int i = 0; i < num_pumps; i++) {
    pinMode(pump_out[i], OUTPUT);
    digitalWrite(pump_out[i], HIGH);
  }
}

bool temp_valid(int z) {
  if (! units) {
    if (temp[z] > 80 || temp[z] < -100) return 0;
  } else {
    if (temp[z] > 180 || temp[z] < -100) return 0;
  }
  return 1;
}

void loop(){
  int z = -1; // zone index
  int h = -1; // hour index
  int error_flag;
  struct tm timeinfo;
  char buffer[200];
  char header[21] = {0}; int r = 0; // header buffer and index, ignore all but the first 20 chars
  WiFiClient client = server.available();   // Listen for incoming clients
  if (! client){ // do heating control
    delay (1000);
    getLocalTime(&timeinfo);
    sensors.requestTemperatures();
    for (z = 0; z < num_zones; z++){
      float demand_temp;
      if (units) {
        temp[z] = sensors.getTempF(ds18b20[z]);
      } else {
        temp[z] = sensors.getTempC(ds18b20[z]);
      }
      if ( ! temp_valid(z)) {
        valve[z] = 6;
      }
      if ((bitRead(zone[z], timeinfo.tm_hour) || bitRead(zone[z], 24)) && ! bitRead(zone[z], 25)) {
        demand_temp = on_temp[z] / 2.0;
      } else { 
        demand_temp = off_temp[z] / 2.0;
      }
      /*************************************
       *    Valve control state machine    *
       *************************************/
      switch (valve[z]) {
        case 0: // closed
          if (temp[z] < demand_temp - hyst){
            digitalWrite(zone_out[z], LOW);
            Serial.print("Turning on zone ");
            Serial.println(z);
            valve[z] = 1;
            valve_timeout[z] = 120;
          }
          break;
        case 1: // valve opening
          if (digitalRead(zone_in[z])) { // valve has opened
            bitSet(zone_on, z);
            valve[z] = 2;
            break;
          }

          // TESTING!!!!!!!!
          if (--valve_timeout[z] <=100) { // timeout opening
            bitSet(zone_on, z);
            valve[z] = 2;
          }
          // TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          break;
        case 2: // Valve opened
          if (temp[z] > demand_temp + hyst) {
            digitalWrite(zone_out[z], HIGH);
            valve[z] = 3;
            valve_timeout[z] = 120;
            bitClear(zone_on, z);
            Serial.print("Turning Off zone ");
            Serial.println(z);
          }
          break;
        case 3: // valve closing
          if (! digitalRead(zone_in[z])) { // valve has closed
            valve[z] = 0;
            break;
          }
          if (--valve_timeout[z] <=1) { // timeout closing
            valve[z] = 4;
            error_flag = 4;
          }
        case 4: // Stuck Open fault: We can still control temp, and it might close
          if (! digitalRead(zone_in[z])) { // valve has finally closed
            bitSet(zone_on, z);
            valve[z] = 0;
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
          // Just stick here? 
          break;
        case 6: // Temperature sensor fault
          // allow it to fix itself
          if (temp_valid(z)) {
            digitalWrite(zone_out[z], HIGH);
            valve[z] = 0;
          }
          
        default:
          error_flag = 10;
      }
    }
        
    /*********************************
     *     Boiler State Machine      *
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
            run_on_timer[i] = run_on_time;
            bitClear(boiler_on, i);
            bitSet(run_on, i);
            boiler[i] = 2;
            digitalWrite(boiler_out[i], HIGH);
          }
          break;
        case 2: // run-on timer
          if (--run_on_timer[i] <= 1){
            bitClear(run_on, i);
            boiler[i] = 2;
            break;
          }
        default:
          error_flag = 10;
      }
    }
    /****************************
     *        Pump Control      *
     ****************************/
    for (int i = 0; i < num_pumps; i++) {
      if ((pump_mask[i] & zone_on) || (run_on_mask[i] & run_on)) {
        digitalWrite(pump_out[i], LOW);
      } else {
        digitalWrite(pump_out[i], HIGH);
      }
    }
  } else {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (r == 1 && c == '\n') {
          // if there are two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
          // and a content-type so the client knows what's coming, then a blank line:
          __P("HTTP/1.1 200 OK");
          __P("Content-type:text/html");
          __P("Connection: close");
          __P("");
          // Programming Screen
          if (z >= 0){
              __P("<!DOCTYPE html>");
              __P("<head> </head>");
              __P("<html>");
              __P("<link rel=\"icon\" href=\"data:,\">");
              __P("<body>");
              __P("<svg svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" width=\"297mm\" height=\"210mm\">");
              // Header
              __P("<text y=\"100\" x=\"60\" font-size=\"30\" fill=\"0\" font-family=\"Times\" >%s</text>", zone_names[z]);
              // Setting clock faces
              for (int h = 0; h < 24; h++){
                int r = 100;
                __P("<a xlink:href=\"hour_%02d_%02d\"><path d=\"M 0 0 L %d %d A %d %d 0 0 1 %d %d L 0 0\" style=\"fill:%s;stroke:#000000;stroke-opacity:1\" transform=\"translate(%d,300)\" /></a>",
                     z, h,
                     (int)(r * sin(0.5236 * h)), (int)(-r * cos(0.5236 * h)), 
                     r, r, (int)(r * sin(0.5236 * (h + 1))), (int)(-r * cos(0.5236 * (h + 1))),
                     (zone[z] & (1 << 24 | 1 << h) && !(zone[z] & 1 << 25))?on_colour:off_colour,
                     (h < 12)?200:500);
                __P("<text x=\"%d\" y=\"%d\" font-size=\"15\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"middle\" font-family=\"Times\" transform=\"translate(%d,300)\"> %02d </text>",
                     (int)((r + 20) * sin(0.5236 * h)), (int)(-(r + 20) * cos(0.5236 * h)),
                     (h < 12)?200:500,
                     h);
              }
              // Set / display on temperature
              __P("<rect y=\"480\" x=\"130\" height=\"80\" width=\"80\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/>", on_colour);
              __P("<text y=\"520\" x=\"170\" font-size=\"30\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"middle\" font-family=\"Times\"> %d.%1d </text>",
                    (int)(on_temp[z]/2), (int)(on_temp[z]*5)%10);
              __P("<a xlink:href=\"on_plus_%02\"><path d=\"M 130 475 l 80 0 l -40 -30 z\" /></a>", z);
              __P("<a xlink:href=\"on_minus_%02\"><path d=\"M 130 565 l 80 0 l -40  30 z\" /></a>", z);
              // Set / display off temperature
              __P("<rect y=\"480\" x=\"290\" height=\"80\" width=\"80\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/>", off_colour);
              __P("<text y=\"520\" x=\"330\" font-size=\"30\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"middle\" font-family=\"Times\"> %d.%1d </text>",
                    (int)(off_temp[z]/2), (int)(off_temp[z]*5)%10);
              __P("<a xlink:href=\"off_plus_%02d\"><path d=\"M 290 475 l 80 0 l -40 -30 z\" /></a>", z);
              __P("<a xlink:href=\"off_minus_%02d\"><path d=\"M 290 565 l 80 0 l -40  30 z\" /></a>", z);
              // AUTO Button
              __P("<a xlink:href=\"auto_%d_0\"><rect y=\"460\" x=\"460\" height=\"25\" width=\"25\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/></a>",z, (zone[z]&0x3000000)?"#FFFFFF":"#FF0000");
              __P("<text y=\"472\" x=\"500\" font-size=\"30\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"left\" font-family=\"Times\"> AUTO </text>");
              // Manual ON button
              __P("<a xlink:href=\"auto_%d_1\"><rect y=\"500\" x=\"460\" height=\"25\" width=\"25\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/></a>",z,  (zone[z]&0x1000000)?"#FF0000":"#FFFFFF");
              __P("<text y=\"512\" x=\"500\" font-size=\"30\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"left\" font-family=\"Times\"> Manual ON </text>");
              // Manual OFF button
              __P("<a xlink:href=\"auto_%d_2\"><rect y=\"540\" x=\"460\" height=\"25\" width=\"25\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/></a>",z,  (zone[z]&0x2000000)?"#FF0000":"#FFFFFF");
              __P("<text y=\"562\" x=\"500\" font-size=\"30\" fill=\"#000000\" dominant-baseline=\"middle\" text-anchor=\"left\" font-family=\"Times\"> Manual OFF </text>");
              // Back link
              __P("<a xlink:href=\"../\"><text y=\"650\" x=\"60\" font-size=\"20\" fill=\"#0000FF\" font-family=\"Times\" >Back to Main Screen</text></a>");
              __P("</svg>");
              __P("</body></html>");
          } else {
            // Display the status web page
            __P("<!DOCTYPE html>");
            __P("<head><meta http-equiv=\"refresh\" content=\"60\"></head>");
            __P("<html>");
            __P("<link rel=\"icon\" href=\"data:,\">");
            __P("<body>");
            __P("<svg svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" width=\"297mm\" height=\"210mm\">");
            client.print("<text y=\"70\" x=\"60\" font-size=\"40\" fill=\"#ao5a2c\" font-family=\"Times\" >");
            client.print(&timeinfo, "%A, %B %d %Y %H:%M");
            client.println("</text>");
            for (int i = 0; i < num_boilers; i++){
              __P("<text y=\"%d\" x=\"80\" font-size=\"30\" dominant-baseline=\"middle\" text-anchor=\"left\" fill=\"0\" font-family=\"Times\" >Boiler %d %s</text>", 
                   35 * i + 110, i, boiler_states[boiler[i]]);
            }
            for (int i = 0; i < num_zones; i++){
              __P("<text y=\"%d\" x=\"80\" font-size=\"30\" dominant-baseline=\"middle\" text-anchor=\"left\" fill=\"0\" font-family=\"Times\" >Valve  %d %s</text>", 
                   35 * i + 130 + 30 * num_boilers, i, valve_states[valve[i]]);
            }
            for (int i = 0; i < num_zones; i++){
              __P("<a xlink:href=\"set_%d\"><rect y=\"%d\" x=\"%d\" height=\"%d\" width=\"%d\" style=\"fill:%s;stroke:#000000;stroke-width:3\"/></a>",
              i, zone_pos[i][0], zone_pos[i][1], zone_pos[i][2], zone_pos[i][3], bitRead(zone_on, i)?on_colour:off_colour);
              __P("<text y=\"%d\" x=\"%d\" font-size=\"20\" dominant-baseline=\"middle\" text-anchor=\"middle\" fill=\"0\" font-family=\"Times\" >%s</text>", 
              zone_pos[i][0] + zone_pos[i][2] - 20,
              zone_pos[i][1] + zone_pos[i][3] / 2,
              zone_names[i]);
              __P("<text y=\"%d\" x=\"%d\" font-size=\"40\" dominant-baseline=\"middle\" text-anchor=\"middle\" fill=\"0\" font-family=\"Times\" >%d.%1d%s</text>", 
              zone_pos[i][0] + 60,
              zone_pos[i][1] + zone_pos[i][3] / 2,
              (int)temp[i], (int)(temp[i]*10)%10,
              units?"&#8457":"&#8451");
            }
            __P("</svg>");
            __P("</body></html>");
          }
          // The HTTP response ends with another blank line
          client.println();
          z = -1; // reset z flag
          // Break out of the while loop
          break;
        } else { // not end of request, check the headers
          header[r] = c;
          r = min(20, r + 1);
          if (c == '\n'){
            r = 0;
            // interrogate the URL
            if (sscanf(header, "GET /set_%d", &z)){
              Serial.println("Programming Command");
            } else if (sscanf(header, "GET /hour_%d_%d", &z, &h)){
              if (z < 0 || z > num_zones -1 || h < 0 || h > 23) break;
              zone[z] &= 0x00FFFFFF; // set to auto mode if user tries to program
              zone[z] ^= (1L << h);
              EEPROM.write(z * 8 + 1, (zone[z] & 0x00FF0000) >> 16);
              EEPROM.write(z * 8 + 2, (zone[z] & 0x0000FF00) >>  8);
              EEPROM.write(z * 8 + 3, (zone[z] & 0x000000FF));
              EEPROM.commit();
              Serial.println(zone[z],BIN);
             } else if (sscanf(header, "GET /auto_%d_%d", &z, &h)){
              if (z < 0 || z > num_zones -1 || h < 0 || h > 2) break;
              zone[z] &= 0x00FFFFFF;
              zone[z] += (long)h * 0x1000000;
              EEPROM.write(z * 8 + 0, (zone[z] & 0xFF000000) >> 24);
              EEPROM.commit();
              Serial.println(h, DEC);
              Serial.println((long)h * 0x1000000, BIN);
              Serial.println(zone[z],BIN);
            } else if (sscanf(header, "GET /on_plus_%2d", &z)){
              if (z < 0 || z > num_zones -1) break;
              counter[1] = counter[2] = counter[3] = 0;
              on_temp[z] += (counter[0]++ > 10) ? 10 : 1;
              EEPROM.write(z * 8 + 4, on_temp[z]);
              EEPROM.commit();
              Serial.print("plus on ");
              Serial.println(counter[0]);
            } else if (sscanf(header, "GET /on_minus_%2d", &z)){
              if (z < 0 || z > num_zones -1) break;
              counter[0] = counter[2] = counter[3] = 0;
              on_temp[z] -= (counter[1]++ > 10) ? 10 : 1;
              EEPROM.write(z * 8 + 4, on_temp[z]);
              EEPROM.commit();
                            Serial.print("minus on ");
              Serial.println(counter[1]);
            } else if (sscanf(header, "GET /off_plus_%2d", &z)){
              if (z < 0 || z > num_zones -1) break;
              counter[0] = counter[1] = counter[3] = 0;
              off_temp[z] += (counter[2]++ > 10) ? 10 : 1;
              EEPROM.write(z * 8 + 5, off_temp[z]);
              EEPROM.commit();
                            Serial.print("plus off ");
              Serial.println(counter[2]);
            } else if (sscanf(header, "GET /off_minus_%2d", &z)){
              if (z < 0 || z > num_zones -1) break;
              counter[0] = counter[1] = counter[2] = 0;
              off_temp[z] -= (counter[3]++ > 10) ? 10 : 1;
              EEPROM.write(z * 8 + 5, off_temp[z]);
              EEPROM.commit();
              Serial.print("minus off ");
              Serial.println(counter[3]);
            }
          }
        }
      }
    }
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
