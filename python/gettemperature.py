#!/usr/bin/python3
import signal
import sys, getopt, copy
import time, logging
from io import StringIO
import json
import yaml
from temperaturereader import TemperatureReader,TemperatureReaderCrash
DEBUG = False
MAXERRORS = 10

def writeSensorData(address, SensorData, Config):
	datetime_local = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
	print("{} {} C: {} F: {}".format(datetime_local, Config.sensors.get(address,address), SensorData[0], SensorData[1]))
	
 

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

    with open('config.yml', 'r') as file:
        Config = yaml.safe_load(file)
    print(Config)
    errcounter = 0
    Reader = TemperatureReader(Config['serial'])
    Reader.startmeasurement()
    while errcounter<MAXERRORS:      
        line = Reader.read()
        if line and len(line) != 0:
            #time.sleep(0.2)
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
                logging.debug("Valueerror = ")
                time.sleep(0.3)
                errcounter += 1
                continue
            #print(obj);
            for i in obj:
                writeSensorData(i, obj[i], Config)
            break
    Reader.close()
    
if __name__ == "__main__":
   main(sys.argv[1:])
