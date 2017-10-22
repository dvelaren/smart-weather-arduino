//Name: postThingWorxV2
//Author: David Velasquez
//Date: 15/09/2017
//Description: This program sends Temperature and Humidity to Thingworx Server.

//Libraries
#include <ThingworxWiFi101.h>
#include <DHT.h>

//I/O pin labeling
#define DHTPIN 2  //DHT sensor connected to Arduino digital pin 2

//Server Constants
const unsigned long TPOST = 2000;  //Time between requests to TWX server (every 2 secs)
const unsigned int sensorCount = 3;  //Number of sensor vars to send to TWX server (2)
char* ssid = "IoT-B19"; //WiFi SSID
char* password = "meca2017*"; //WiFi Pass
char* host = "iot.dis.eafit.edu.co";  //TWX Host
unsigned int port = 80; //TWX host port

//Variables
//->TWX Vars
char appKey[] = "4d8f01a5-5d21-45ab-a63e-9093ca219468"; //API Key from TWX
char thingName[] = "weatherThing";  //Thing name from TWX
char serviceName[] = "weatherService";  //Thing service name from TWX
char* propertyNames[] = {"in1", "in2", "anomaly"};  //Input names vector created on service from TWX
float propertyValues[sensorCount] = {0}; //Vector to store sensor vars to be sent to TWX
//->DHT11 Vars
#define DHTTYPE DHT11 //Use DHT11 sensor variant
DHT dht(DHTPIN, DHTTYPE); //DHT object var
//->Timing Vars
unsigned long lastConnectionTime = 0; //Last connection ms time between server requests
//->ThingworxWiFi101 Vars
ThingWorx myThing(host, port, appKey, thingName, serviceName);  //Declare the TWX object with his corresponding properties

//Subroutines & functions
void printWifiStatus() {
  //Print SSID name
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  //Print ipv4 assigned to WiFi101 module
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  //Print signal strength for WiFi101 module
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void WiFiInit() {
  delay(1000);  //Wait 1 sec for module initialization

  //Check if WiFi Shield is connected to Arduino
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    //Infinite loop if shield is not detected
    while (true);
  }

  //Attempt a WiFi connection to desired access point at ssid, password
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(ssid, password);
    delay(10000); //Wait 10 secs for establishing connection
  }
  //Print WiFi status
  printWifiStatus();
}

void setup() {
  //I/O configuration

  //Physical outputs initialization

  //Communications
  Serial.begin(9600); //Serial communications with computer at 9600 bauds for debug purposes
  dht.begin();  //Initialize communications with DHT11 sensor
  WiFiInit(); //WiFi communications initialization
}

void loop() {
  if (millis() - lastConnectionTime > TPOST) {  //Send request to server every TPOST seconds
    propertyValues[0] = dht.readTemperature(); //Read DHT11 temperature
    propertyValues[1] = dht.readHumidity(); //Read DHT11 humidity
    myThing.post(sensorCount, propertyNames, propertyValues); //Send values to server platform
    lastConnectionTime = millis();  //Refresh last connection time
  }
}


