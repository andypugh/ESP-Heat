ESP32 Heating Controller
==============

A multi-zone central heating controller based on the ESP32.

It can be configured for an almost unlimited number of zones, pumps and separate boilers.
Boiler run-on is supported, and configurable. 
Temperature sensing assumes DS18B20 sensors. 
One zone per valve, and it is assumed that zone valves trip microswitches when open. 
 
Branches: 
no_master: Uses GPIO to read from the DS18B20 sensors

master: using a DS2482 1-wire master adaptor for longer cable runs. Using WebServer
to handle http requests. 

WiFiServer: Initial version using wiFi server and parsing incoming URLs byte-by-byte
 
The DS2482-100 is a I2C->1-Wire bridge (http://www.maximintegrated.com/datasheet/index.mvp/id/4382),
contains functionality that means it is better at driving long and complex networks. 

I used a DS2482-800 which additionally expands the bus to 8 separate channels, which allows
for longer single-ended networks.
For simplicity, using this break-out board. 
https://uk.rs-online.com/web/p/communication-wireless-development-tools/1360731/

Here is one of the programming screens

![Alt text](Screenshot.png?raw=true "Programming screen for one zone")

And here is one iteration of the hardware installed in a consumer unit case with
Neutrik connectors for the pumps and valves and CB connectors for the temperature sensors:

![Alt text](IMG_5434.jpg?raw=true "Hardware")
