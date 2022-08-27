# ds18b20-usb
A simple ds18b20 to usb tool using an esp32 (should work with arduino as well).
Connect one or multiple temperature sensors to pin D15.
The arduino-sketch will discover the bus and dump all readings to the serial port in json-format when asked to do so.
The program in the pythonfolder will read the data from the usb-serial and and write it to the stdout in the outformat of digitemp.

# why?
We have a quiet old temperature monitoring system based on ds1820 sensors connected to an rs232 port. Some tools arround it rely on the output-format of digitemp (https://www.digitemp.com/software.shtml). Now we have to add sensors and digitemp is quiet old and rs232 is not very common to have on computers.

# mounting
I soldered the sensors and cables to a small interconnect 3x3 pcb-strip-board. 
There is a model for a sensorhousing  in the stl-folder. Print it shove it in and secure it with zip-ties. mount with zipties or ducttape.
![Sensor in housing](img/sensorhousing.jpg?raw=true "Sensor in housing")

# LED blinking pattern
If powered up the controller scans the onewire bus and slowly blinks he onboard LED for each detected sensor.
3 sensors connected (and detected) = 3 x blinky.
The led pattern is repeated faster for each measurement cycle.
Tested on an ESP32 DevKit. It should work with other boards with leds if the constant LED_BUILTIN is defined.
