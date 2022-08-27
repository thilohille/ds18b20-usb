#!/usr/bin/python3

import signal
import sys
from time import sleep
import serial
import logging

#Raise exception on exit
def signal_handler(sig, frame):
        logging.info('shutting down')
        raise SystemExit('shutdown')
signal.signal(signal.SIGINT, signal_handler)        


class TemperatureReader:
    def __init__(self, cfg):
        self.ser = serial.Serial(cfg["port"], cfg["baudrate"], timeout=1)  # open serial port
    def read(self):
        logging.debug("read start")
        line = self.ser.readline().decode("utf-8").rstrip()
        line = line.rstrip('\0')
        if (line[:22] == "Guru Meditation Error:"):
            raise TemperatureRead(line[23:])
        if len(line) > 0:
            logging.debug("read: "+line.rstrip('\0'))
        logging.debug("read done")
        return line;
 
    def startmeasurement(self):
        logging.debug("reader start")
        self.ser.write(b'\0')
        return

    def close(self):
        logging.debug("close start")
         # serial close port
        self.ser.close()
        logging.debug("close done")

class TemperatureReaderCrash(Exception):
    pass
