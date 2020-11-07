A multi-zone central heating controller based on the ESP32.

It can be configured for an almost unlimited number of zones, pumps and separate boilers.
Boiler run-on is supported, and configurable. 
Temperature sensing assumes DS18B20 sensors. 
One zone per valve, and it is assumed that zone valves trip microswitches when open. 

Branches: 
no_master: Uses GPIO to read from the DS18B20 sensors
master: using a DS2482 1-wire master adaptor for longer cable runs 

![Alt text](Screenshot.png?raw=true "Programming screen for one zone")

