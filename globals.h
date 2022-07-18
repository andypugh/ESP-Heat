// Defaut Pin States. 0 out o=low, 1 out high, i input, x ignore
//                         111111111122222222222333333333
//               0123456789012345678901234567890123456789
char pins[41] = "0xix00xxxxxx0i000000xxxxx000xxxxi0iiixxi";

struct DS18B20 {
  char channel = -1; // DS482 channel number -1 indicates temp over IP.
  DeviceAddress address; // DS18B20 ROM address
  char str_address[17];
  char dot_address[25];
};

struct zone {
  int state;
  long hours; // bit field. 0 = 00:00 to 01:00 ... 23 = 23:00 to 00:00. 24 = manual-on, 25 = manual-off
  byte on_temp;
  byte off_temp;
  float temp;
  byte in_pin = -1;
  byte out_pin;
  bool default_state;
  DS18B20 sensor;
  char name[20];
  char shape[50];
  time_t timeout;
};

struct pump {
  int state;
  int mask;
  int out_pin;
  int run_on_mask;
};

struct boiler {
  int state;
  int mask;
  int out_pin;
  unsigned long run_on_timer;
  DS18B20 f_sensor;
  DS18B20 r_sensor;
  float f_temp;
  float r_temp;
};

zone zones[max_zones];
pump pumps[max_pumps];
boiler boilers[max_boilers];

DS18B20 all_sensors[max_sensors]; // array of found sensors

// Some initial defaults
int num_zones = 5;
int num_pumps = 3;
int num_boilers = 1;
int num_sensors = 0; // sensors are discovered
int units = 0; // Set to 1 for Fahrenheit
int ds2482_reset = 15; 

// GPIO Pins
//int boiler_out[max_boilers] = {33};
//int zone_out[max_zones] = {25, 26, 27, 14, 19}; // and 16
//int zone_in[max_zones] = {36, 39, 34, 35, 32}; // and 13
//int pump_out[max_pumps] = {4, 5, 12}; // and 17, 18

//bool default_state[max_zones] = {0, 0, 0, 1, 0};
//For each pump the pump mask is AND-ed with the active zone bitfield. Non-zero turns on the pump
//int pump_mask[max_pumps] = {0x01, 0x06, 0x18};
//For each pump the pump mask is AND-ed with the boiler run-on bitfield. Non-zero turns on the pump
//int run_on_mask[max_pumps] = {0x0, 0x0, 0x1};
//For each boiler the pump mask is AND-ed with the active zone bitfield. Non-zero turns on the pump
//int boiler_mask[max_boilers] = {0x1F};

char* ext_ip[16];
unsigned long valve_timeout[max_zones];
//int valve[max_zones] = {0}; // valve status
//long zone[max_zones] = {0}; // bit field. 0 = 00:00 to 01:00 ... 23 = 23:00 to 00:00. 24 = manual-on, 25 = manual-off
int zone_on; // bitfield
int pump_on; // bitfield
int boiler_on; // bit field
int run_on; // bit-field
//byte on_temp[max_zones]; // temperature * 2
//byte off_temp[max_zones];
//float temp[max_zones];
//int run_on_timer[max_boilers];
byte counter[4] = {0}; // up/down arrow accelleration
struct tm timeinfo;
char buffer[200];
