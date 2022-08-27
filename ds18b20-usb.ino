// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 15 //onewire gpio pin
#define TEMPERATURE_PRECISION 9
#define MAXSENSORS 10 //max device to search fpr on the bus.
#define MEASUREMENTS 3 //measurements per cycle
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress Thermometers[MAXSENSORS];
int detectedsensors = 0;

void notify_num_sensors(int blink_times, int blink_delay) {
  for (int i=0; i < blink_times; i++){
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(blink_delay);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(blink_delay);                       // wait for a second
  }
}

void setup(void)
{
  // start serial port
  Serial.begin(115200);
  //Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the library
  sensors.begin();

  // how many sensors are on the bus?. 
  
  for (int i=0; i<MAXSENSORS; i++){
    if (!sensors.getAddress(Thermometers[i], i)) break;
    detectedsensors = i + 1;
  }

  // blink the led if supported
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  notify_num_sensors(detectedsensors, 400);
#endif

  // Must be called before search()
  oneWire.reset_search();

  // set the resolution for all sensors
  for (int i=0; i<detectedsensors; i++){
    sensors.setResolution(Thermometers[i], TEMPERATURE_PRECISION);
  }
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}


// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print("\"");
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.print("\": [\"");
  float tempC = sensors.getTempC(deviceAddress);
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    Serial.print("Error: Could not read temperature data");
    return;
  }
  Serial.print(tempC);
  Serial.print("\",\"");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
  Serial.print("\"]");
  //Serial.print("0f8989898": ["26.6","156"],)
}

/*
   Main function, calls the temperatures in a loop, if something has been received on the serialport.
*/
void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  //Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  //Serial.println("DONE");

  // print the device information
  delay(50);
  if (!Serial.available()){
    return;
  }
  //"consume" the data
  while (Serial.available()){
    Serial.read();
  }
  //do MEASUREMENTS measurements and send it to the serialport
  for (int m=0; m<MEASUREMENTS; m++){
    Serial.print("{");  
    for (int i=0; i<detectedsensors; i++){
      printData(Thermometers[i]);
      delay(30);
      if (i < detectedsensors-1){
        Serial.print(",");  
      }
    }
    Serial.println("}");  
  }
#ifdef LED_BUILTIN
  notify_num_sensors(detectedsensors, 100);
#endif
}
