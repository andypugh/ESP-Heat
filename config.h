// **************************CONFIGURATION************************************************

const char* hostname = "heating";
const char* ntpServer = "pool.ntp.org";
const char *TZstr = "GMT0BST,M3.3.0/1,M10.5.0";
// maximum num_zones is EEPROM.length() / 8
#define max_zones 8
#define max_pumps 8
#define max_boilers 4
#define max_sensors 32

#define USE_DS2482 

#define EEPROM_BASE 256

byte BLINK_LED = 23;
float hyst = 0.5; // temperature hysteresis

const char* boiler_states[3] = {"Off", "On", "Run-On"};
const char* valve_states[7] = {"closed", "opening", "open", "closing", "stuck open FAULT", "stuck closed FAULT", "temp sensor FAULT"};

int zone_pos[max_zones][4] =        {{350, 60, 190, 190},
                                    {350, 250, 190, 140},
                                    {350, 390, 190, 200},
                                    {350, 590, 270, 180},
                                    { 50, 690, 270, 180}};

unsigned long  run_on_time = 120000; //milliseconds
const char* off_colour = "#aaffaa";
const char* on_colour = "#ffaaaa";

// **************************/CONFIGURATION***********************************************
