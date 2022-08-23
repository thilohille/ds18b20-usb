#!/usr/bin/python3
import signal
import sys, getopt, copy
import time, logging
from io import StringIO
import json
from temperaturereader import TemperatureReader,TemperatureReaderCrash
DEBUG = True
MAXERRORS = 10
SERIALPORT =  { 
        "port": "/dev/ttyUSB0",
        "baudrate": 115200
}

 

def main(argv):
    global DEBUG
    if DEBUG:
        logging.basicConfig(format='%(asctime)s %(levelname)-2s %(message)s', 
        level=logging.DEBUG,
        datefmt='%Y-%m-%d %H:%M:%S')    
    else:
        logging.basicConfig(format='%(asctime)s %(levelname)-2s %(message)s', 
        level=logging.INFO,
        datefmt='%Y-%m-%d %H:%M:%S')  

    errcounter = 0
    Reader = TemperatureReader(SERIALPORT)
    while errcounter<MAXERRORS:      
        line = Reader.read()
        if len(line) != 0:
            time.sleep(0.2)
            #continue
            #logging.info(line)
            try:
                obj = json.loads(line)
            except json.JSONDecodeError:
                logging.debug("json decode error: " + line)
                errcounter += 1
                time.sleep(0.3)
                continue
            valueerr = False
            for address in obj:
                if obj[address][0] == "85.00":
                    logging.debug("read value error C = " + obj[address][0])
                    valueerr = True
                    break
            if valueerr:
                time.sleep(0.3)
                errcounter += 1
                continue
            print(obj);
            break
    Reader.close()
    #scan(scanner, cfg, data)
    
if __name__ == "__main__":
   main(sys.argv[1:])
