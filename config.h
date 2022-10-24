// **************************CONFIGURATION************************************************

// maximum num_zones is EEPROM.BASE() / 8

#define max_zones 8
#define max_pumps 8
#define max_boilers 4
#define max_sensors 32
#define TIMEOUT 1800

#define USE_DS2482 

#define EEPROM_SIZE 1536
#define EEPROM_BASE 256
#define CREDENTIALS_BASE 1024

byte BLINK_LED = 23;
float hyst = 0.5; // temperature hysteresis

const char* boiler_states[3] = {"Off", "On", "Run-On"};
const char* valve_states[8] = {"closed", "opening", "open", "closing", "stuck open FAULT", "stuck closed FAULT", "sensor FAULT", ""};

unsigned long  run_on_time = 120; //seconds
const char* off_colour = "#aaffaa";
const char* on_colour = "#ffaaaa";

const unsigned long timeoutTime = 40;

// **************************/CONFIGURATION***********************************************
