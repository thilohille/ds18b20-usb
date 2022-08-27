# ds18b20-usb
A simple ds18b20 to usb tool using an esp32 (should work with arduino as well).
Connect one or multiple temperature sensors to pin D15.
The arduino-sketch will discover the bus and dump all readings to the serial port in json-format when asked to do so.
The programm in the pythonfolder will read the data from the usb-serial and and write it to the stdout in the outformat of digitemp.

# why?
We have a temperature monitoring system based on ds1820 sensors connected to an rs232 port. Some tools arround it rely on the output-format of digitemp (https://www.digitemp.com/software.shtml). Now we have to add sensors and digitemp is quiet old and rs232 is not very common to have on computers.

# mounting
I soldered the sensors and cables to a small 3x3 pcb-strip-board.
There is a model for the housing of the sensor in the stl-folder. Just shove it in and secure it with zipties.    
