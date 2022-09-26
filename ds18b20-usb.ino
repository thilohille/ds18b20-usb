// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

//uncomment to enable demo-mode. temperature-values are random then, no sensors required. value is number of mock-sensors simulated
//#define DEMO 3

//wifi mesh configuration
#define   MESH_SSID       "your-essid"
#define   MESH_PASSWORD   "your-password"
#define   MESH_PORT       5555

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 15 //onewire gpio pin
#define TEMPERATURE_PRECISION 9
#define MAXSENSORS 50 //max device to search for on the bus or mesh.
#define MEASUREMENT_PERIOD 5*1000L //mearure every 5 sec.
#define MEASUREMENT_MAXAGE 60*1000L //expire record after 60 sec

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)

StaticJsonDocument<50*MAXSENSORS> doc;

// Prototypes
void localSensors(); 
void sendMessage(); 
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(); 
void nodeTimeAdjustedCallback(int32_t offset); 
void delayReceivedCallback(uint32_t from, int32_t delay);

Scheduler     userScheduler; // to control your personal task
Task taskSensorsLocal( 0, TASK_FOREVER, &localSensors); // start with a one second interval
Task taskSendMessage( TASK_SECOND * 1, TASK_FOREVER, &sendMessage ); // start with a one second interval

painlessMesh  mesh;

String msg;

bool calc_delay = false;
SimpleList<uint32_t> nodes;

OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress Thermometers[MAXSENSORS];
int detectedsensors = 0;
unsigned long target_time = 0L ;


void notify_num_sensors(int blink_times, int blink_delay) {
  #ifdef LED_BUILTIN
    for (int i=0; i < blink_times; i++){
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(blink_delay);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      delay(blink_delay);                       // wait for a second
    }
  #endif
}

//Sensordata struct
struct SensorData {
  uint64_t sensorid;
  float value;
  unsigned long timestamp;
};

SensorData SensorDB[MAXSENSORS];
int currentsensor = 0;

void setup(void)
{
  // start serial port
  Serial.begin(115200);
  //Serial.println("Dallas Temperature IC Control Library Demo");

  //mesh init
  mesh.setDebugMsgTypes(ERROR | DEBUG);  // set before init() so that you can see error messages
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  //userScheduler.addTask( taskSensorsLocal );
  //taskSensorsLocal.enable();

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();

  randomSeed(analogRead(A0));

  #ifdef DEMO
    detectedsensors = DEMO;
    //create fake sensor address from the chipid and a sensorcount
    uint32_t chipid = ESP.getEfuseMac();
    for (int i=0; i<DEMO; i++){
      Thermometers[i][0] = ((chipid >> (32 - 8)) & 0xff);
      Thermometers[i][1] = ((chipid >> (32 - 16)) & 0xff);
      Thermometers[i][2] = ((chipid >> (32 - 24)) & 0xff);
      Thermometers[i][3] = ((chipid >> (32 - 32)) & 0xff);
      Thermometers[i][4] = i + 1;
      Thermometers[i][5] = i + 1;
      Thermometers[i][6] = i + 1;
      Thermometers[i][7] = i + 1;
    }
    return;
  #endif

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

const String hexDigits = "0123456789ABCDEF";

uint64_t hextoint(String input){
  uint64_t result = 0;
  input.toUpperCase();
  for (int g = 0; g < input.length(); g++) {
    result <<= 4;
    result |= hexDigits.indexOf(input[g]);
  }
  return result;
}

uint64_t adresstoint(DeviceAddress deviceAddress){
  uint64_t result = deviceAddress[7] 
              | (deviceAddress[6] << 8) 
              | (deviceAddress[5] << 16) 
              | (deviceAddress[4] << 24)
              | (uint64_t(deviceAddress[3]) << 32) 
              | (uint64_t(deviceAddress[2]) << 40) 
              | (uint64_t(deviceAddress[1]) << 48) 
              | (uint64_t(deviceAddress[0]) << 56); 
  return result;
}



// main function to read information from a local sensor
int readLocalSensor(DeviceAddress deviceAddress)
{
  uint64_t addressint = adresstoint(deviceAddress);
  #ifdef DEMO
    float tempC = random(1500, 2800) / 100.0;
  #else
    float tempC = sensors.getTempC(deviceAddress);
  #endif
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    //queueSerial("Error: Could not read temperature data");
    return -1;
  }
  return registerSensor(addressint, tempC);
}

// main function to print information about a device
void formatSensorData(uint64_t sensorid, float value, String &line){
  line += "\"";
  line += String(((uint32_t)((sensorid >> 32) & 0xFFFFFFFF)),HEX);
  line += String(((uint32_t)(sensorid & 0xFFFFFFFF)),HEX);
  line+= "\": [\"";
  line += String(value);
  line += "\",\"";
  line += String(DallasTemperature::toFahrenheit(value));
  line += "\"]";
  //Serial.print("0f8989898": ["26.6","156"],)
}

/*
   Main function, calls the temperatures in a loop, if something has been received on the serialport.
*/
void loop(){
  //Serial.println("Main loop");
  if ((millis () - target_time >= MEASUREMENT_PERIOD ) || (currentsensor <= 0)  ){
    target_time += MEASUREMENT_PERIOD ; // change scheduled time exactly, no slippage will happen
    localSensors();
    cleanupSensors();
  }
  checkserial();
  mesh.update();
}

void checkserial(){
  // call sensors.requestTemperatures() to issue a global temperature
  //Serial.println("DONE");

  // print the device information
  if (!Serial.available()){
    return;
  }
  while (Serial.available()){
    Serial.read();
  }
  delay(50);
  if (currentsensor <= 0){
    Serial.println("No sensordata!");
    return;
  }
  String line ="";
  line += "{";  
  for (int f=0; f<currentsensor; f++){
    formatSensorData(SensorDB[f].sensorid,SensorDB[f].value, line);
    if (f < currentsensor-1){
      line += ","; 
    }
  }
  line += "}";
  Serial.println(line);
}

void localSensors()
{
  //"consume" the data
  // request to all devices on the bus
  #ifndef DEMO
    //Serial.println("Requesting temperatures...");
    sensors.requestTemperatures();
  #endif
  msg = "";
  String line ="";
  line += "{";  
  for (int a=0; a<detectedsensors; a++){
    int sensoridx = readLocalSensor(Thermometers[a]);
    formatSensorData(SensorDB[sensoridx].sensorid,SensorDB[sensoridx].value, line);
    delay(30);
    if (a < detectedsensors-1){
      line += ","; 
    }
  }
  line += "}";
  msg += line;
  //Serial.println(line);
#ifdef LED_BUILTIN
  notify_num_sensors(detectedsensors, 100);
#endif
}

int cleanupSensors(){
  for (int i=0; i<currentsensor; i++){
    if ((millis()-SensorDB[i].timestamp) > MEASUREMENT_MAXAGE){
      return deleteSensor(i);
    }
  }
}

int deleteSensor(int sensorindex){
  for (int i=sensorindex; i<currentsensor-1; i++){
    SensorDB[sensorindex].sensorid = SensorDB[sensorindex + 1].sensorid;
    SensorDB[sensorindex].value = SensorDB[sensorindex + 1].value;
    SensorDB[sensorindex].timestamp = SensorDB[sensorindex + 1].timestamp;
  }
  currentsensor--;
  if (currentsensor < 0){
    currentsensor = 0;
  }
  return currentsensor;
}

int lookupSensor(uint64_t sensorid){
  for (int i=0; i<currentsensor; i++){
    if (SensorDB[i].sensorid == sensorid){
      return i;
    }
  }
  return -1;
}



int registerSensor(uint64_t sensorid, float value){
  int sensorindex = lookupSensor(sensorid);
  if ( sensorindex >= 0){
    SensorDB[sensorindex].sensorid = sensorid;
    SensorDB[sensorindex].value = value;
    SensorDB[sensorindex].timestamp = millis();
  }
  else{
    SensorDB[currentsensor].sensorid = sensorid;
    SensorDB[currentsensor].value = value;
    SensorDB[currentsensor].timestamp = millis();
    if (currentsensor < MAXSENSORS){
      currentsensor++;
    }
    sensorindex = currentsensor;
  }
  return sensorindex;
}

void sendMessage() {
  mesh.sendBroadcast(msg);

  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }
  //Serial.printf("Sending message: %s\n", msg.c_str());
  taskSendMessage.setInterval( random(TASK_SECOND * 1, TASK_SECOND * 5));  // between 1 and 5 seconds
}

void receivedCallback(uint32_t from, String & msg) {
  //Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  DeserializationError error = deserializeJson(doc, msg.c_str());
  if (error){
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  JsonObject documentRoot = doc.as<JsonObject>(); 
  for (JsonPair keyValue : documentRoot){
    registerSensor(hextoint(keyValue.key().c_str()), keyValue.value()[0].as<float>());
    //Serial.println(keyValue.key().c_str());
    //Serial.println(keyValue.value()[0].as<float>());
    //hextoint(keyValue.key().c_str());
    //Serial.println(keyValue.value()[0].as<char*>());
  }
}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  //Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  //Serial.printf("--> startHere: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
}

void changedConnectionCallback() {
  //Serial.printf("Changed connections\n");
  // Reset blink task
  nodes = mesh.getNodeList();

  //Serial.printf("Num nodes: %d\n", nodes.size());
  //Serial.printf("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    //Serial.printf(" %u", *node);
    node++;
  }
  //Serial.println();
  calc_delay = true;
}

void nodeTimeAdjustedCallback(int32_t offset) {
  //Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  //Serial.printf("Delay to node %u is %d us\n", from, delay);
}
