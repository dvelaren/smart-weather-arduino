//Name: smartHumidV1
//Author: David Velasquez
//Date: 25/07/2017
//Description: This program sends Temperature and Humidity to Thingworx Server.

//Libraries
#include <WiFi101.h>
#include <ThingworxWiFi101.h>
#include <ArduinoJson.h>
#include <DHT.h>

//I/O pin labeling
#define DHTPIN 2  //DHT sensor connected to Arduino digital pin 2

//Server Constants
const unsigned long TPOST = 2000;  //Time between requests to TWX server (every 2 secs)
const unsigned int sensorCount = 2;  //Number of sensor vars to send to TWX server (2)
char* ssid = "IoT-B19"; //WiFi SSID
char* password = "meca2017*"; //WiFi Pass
char* host = "iot.dis.eafit.edu.co";  //TWX Host
unsigned int port = 80; //TWX host port

//Variables
//->TWX Vars
char appKey[] = "4d8f01a5-5d21-45ab-a63e-9093ca219468"; //API Key from TWX
char thingName[] = "weatherThing";  //Thing name from TWX
char serviceName[] = "weatherService";  //Thing service name from TWX
char* propertyNames[] = {"in1", "in2"};  //Input names vector created on service from TWX
float propertyValues[sensorCount]; //Vector to store sensor vars to be sent to TWX
//->DHT11 Vars
#define DHTTYPE DHT11 //Use DHT11 sensor variant
DHT dht(DHTPIN, DHTTYPE); //DHT object var
//->Timing Vars
unsigned long lastConnectionTime = 0; //Last connection ms time between server requests
//->ThingworxWiFi101 Vars
ThingWorx myThing(host, port, appKey, thingName, serviceName);  //Declare the TWX object with his corresponding properties
//->Azure Vars
WiFiClient azureml;
String inputJson = "";  //Variable to store Input Properties JSON

//Subroutines & functions
String POST(float temperature, float humidity, unsigned int year, unsigned int month, unsigned int day, unsigned int miltime) {
  String url = "/workspaces/895fa8d3bb90430f922f93009ce55c1f/services/050c7d14e4cc4c67ac1be67a229a51a2/execute?api-version=2.0&details=true";
  String body = "{\"Inputs\": {\"input1\": {\"ColumnNames\": [\"Temperature\", \"Humidity\", \"Year\", \"Month\", \"Day\", \"Military Time\", \"Anomaly\"], \"Values\": [[";
  body += "\"" + String(temperature) + "\"," + "\"" + String(humidity) + "\"," + "\"" + String(year) + "\"," + "\"" + String(month) + "\"," + "\"" + String(day) + "\"," + "\"" + String(miltime) + "\", \"0\"], [";
  body += "\"" + String(temperature) + "\"," + "\"" + String(humidity) + "\"," + "\"" + String(year) + "\"," + "\"" + String(month) + "\"," + "\"" + String(day) + "\"," + "\"" + String(miltime) + "\", \"0\"]]}}}";
  if (azureml.connect("ussouthcentral.services.azureml.net", 443)) {
    Serial.println("Connected to: ussouthcentral.services.azureml.net");
    //Send the HTTP POST request:
    azureml.print(String("POST ") + url + " HTTP/1.1\r\n" +
                  "Host: ussouthcentral.services.azureml.net\r\n" +
                  "Authorization: Bearer RTLoD54tEcGGEqKgwUej2uhHyNz4da3u+BD9x9ZRCScRg7z2MgzuWC8FHgz3tsQJ4z6wyTjwLzEFZoOjKjDrsA==\r\n" +
                  "Content-Type: application/json\r\n\r\n" +
                  body + "\r\n\r\n");

    Serial.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: ussouthcentral.services.azureml.net\r\n" +
                 "Authorization: Bearer RTLoD54tEcGGEqKgwUej2uhHyNz4da3u+BD9x9ZRCScRg7z2MgzuWC8FHgz3tsQJ4z6wyTjwLzEFZoOjKjDrsA==\r\n" +
                 "Content-Type: application/json\r\n\r\n" +
                 body + "\r\n\r\n");

    unsigned long timeout = millis();
    while (azureml.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        azureml.stop();
        return "Error";
      }
    }
    String json = "";
    boolean httpBody = false;
    while (azureml.available()) {
      String line = azureml.readStringUntil('\r');
      Serial.print(line);
      if (!httpBody && line.charAt(1) == '{') {
        httpBody = true;
      }
      if (httpBody) {
        json += line;
        httpBody = false;
      }
    }
    return json;
  }
  else {
    Serial.println("The connection could not be established");
    azureml.stop();
    return "Error";
  }
}

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
    //propertyValues[0] = 24; //Read DHT11 temperature
    //propertyValues[1] = 35; //Read DHT11 humidity
    //myThing.post(sensorCount, propertyNames, propertyValues); //Send values to server platform
    inputJson = POST(propertyValues[0], propertyValues[1], 2017, 8, 22, 14);
    Serial.println(inputJson);
    lastConnectionTime = millis();  //Refresh last connection time
  }
}


