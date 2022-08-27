// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 15
#define TEMPERATURE_PRECISION 9
#define MAXSENSORS 10
#define MEASUREMENTS 3
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress Thermometers[10];
int detectedsensors = 0;
// Assign address manually. The addresses below will need to be changed
// to valid device addresses on your bus. Device address can be retrieved
// by using either oneWire.search(deviceAddress) or individually via
// sensors.getAddress(deviceAddress, index)
// DeviceAddress Thermometer1 = { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 };
// DeviceAddress Thermometer2   = { 0x28, 0x3F, 0x1C, 0x31, 0x2, 0x0, 0x0, 0x2 };

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

  // locate devices on the bus
  //Serial.print("Locating devices...");
  //Serial.print("Found ");
  //Serial.print(sensors.getDeviceCount(), DEC);
  //Serial.println(" devices.");

  // report parasite power requirements
  //Serial.print("Parasite power is: ");
  //if (sensors.isParasitePowerMode()) Serial.println("ON");
  //else Serial.println("OFF");

  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).
  //
  // method 1: by index
  
  for (int i=0; i<MAXSENSORS; i++){
    if (!sensors.getAddress(Thermometers[i], i)) break;
    detectedsensors = i + 1;
  }
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  notify_num_sensors(detectedsensors, 400);
#endif

  // Must be called before search()
  oneWire.reset_search();
  // assigns the first address found to Thermometer1
  // if (!oneWire.search(Thermometer1)) Serial.println("Unable to find address for Thermometer1");
  // assigns the seconds address found to Thermometer2
  //if (!oneWire.search(Thermometer2)) Serial.println("Unable to find address for Thermometer2");

  // set the resolution
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
   Main function, calls the temperatures in a loop.
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
  while (Serial.available()){
    Serial.read();
  }
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
