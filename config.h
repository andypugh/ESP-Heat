// **************************CONFIGURATION************************************************

const char* hostname = "heating";
const char* ntpServer = "pool.ntp.org";
const char *TZstr = "GMT0BST,M3.3.0/1,M10.5.0";
// maximum num_zones is EEPROM.length() / 8
#define num_zones 5
#define num_pumps 3
#define num_boilers 1
byte BLINK_LED = 2;
float hyst = 0.5; // temperature hysteresis
int units = 0; // Set to 1 for Fahrenheit
// GPIO Pins
int boiler_out[num_boilers] = {33};
int zone_out[num_zones] = {25, 26, 27, 14, 19};
int zone_in[num_zones] = {36, 39, 34, 35, 32};
int pump_out[num_pumps] = {4, 5, 23};
// spare_out[num_spare_outs] = {18, 12};
// spare_in {2, 13, 16};
const char* zone_names[num_zones] = {"Far End", "Solar", "Hall", "Downstairs", "Upstairs"};
int zone_pos[num_zones][4] =        {{350, 60, 190, 190},
                                    {350, 250, 190, 140},
                                    {350, 390, 190, 200},
                                    {350, 590, 270, 180},
                                    { 50, 690, 270, 180}};

DeviceAddress ds18b20[num_zones] = {{0x28, 0x23, 0x58, 0xC0, 0x32, 0x20, 0x01, 0x6D},
                                    {0x28, 0x7B, 0xB7, 0xE4, 0x32, 0x20, 0x01, 0x29},
                                    {0x28, 0x58, 0xBB, 0x12, 0x33, 0x20, 0x01, 0x34},
                                    {0x28, 0xA0, 0x02, 0x8A, 0x32, 0x20, 0x01, 0x0D},
                                    {0x28, 0x4E, 0x22, 0x41, 0x33, 0x20, 0x01, 0xC9}};
bool default_state[num_zones] = {0, 0, 0, 1, 0};
//For each pump the pump mask is AND-ed with the active zone bitfield. Non-zero turns on the pump
int pump_mask[num_pumps] = {0x01, 0x06, 0x18};
//For each pump the pump mask is AND-ed with the boiler run-on bitfield. Non-zero turns on the pump
int run_on_mask[num_pumps] = {0x0, 0x0, 0x1};
//For each boiler the pump mask is AND-ed with the active zone bitfield. Non-zero turns on the pump
int boiler_mask[num_boilers] = {0x1F};
unsigned long  run_on_time = 120000; //milliseconds
const char* off_colour = "#aaffaa";
const char* on_colour = "#ffaaaa";

// **************************/CONFIGURATION***********************************************
