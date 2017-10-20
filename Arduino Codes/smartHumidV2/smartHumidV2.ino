//Name: smartHumidV2
//Author: David Velasquez
//Date: 15/09/2017
//Description: This program sends Temperature and Humidity to Thingworx Server.

//Libraries
#include <WiFi101.h>
#include <ThingworxWiFi101.h>
#include <ArduinoJson.h>
#include <DHT.h>

//I/O pin labeling
#define DHTPIN 2  //DHT sensor connected to Arduino digital pin 2

//Server Constants
const unsigned long TPOST = 3000;  //Time between requests to TWX server (every 2 secs)
const unsigned int sensorCount = 3;  //Number of sensor vars to send to TWX server (2)
char* ssid = "IoT-B19"; //WiFi SSID
char* password = "meca2017*"; //WiFi Pass
char* host = "iot.dis.eafit.edu.co";  //TWX Host
unsigned int port = 80; //TWX host port
const size_t bufferSize = JSON_ARRAY_SIZE(1) + 3 * JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 160;

//Variables
//->TWX Vars
char appKey[] = "4d8f01a5-5d21-45ab-a63e-9093ca219468"; //API Key from TWX
char thingName[] = "weatherThing";  //Thing name from TWX
char serviceName[] = "weatherService";  //Thing service name from TWX
char* propertyNames[] = {"in1", "in2", "anom"};  //Input names vector created on service from TWX
float propertyValues[sensorCount]; //Vector to store sensor vars to be sent to TWX
//->DHT11 Vars
#define DHTTYPE DHT11 //Use DHT11 sensor variant
DHT dht(DHTPIN, DHTTYPE); //DHT object var
//->Timing Vars
unsigned long lastConnectionTime = 0; //Last connection ms time between server requests
//->ThingworxWiFi101 Vars
ThingWorx myThing(host, port, appKey, thingName, serviceName);  //Declare the TWX object with his corresponding properties
//->Azure Vars
WiFiSSLClient azureml;
String inputJson = "";  //Variable to store Input Properties JSON
float anomaly = 0;

//Subroutines & functions
String POST(float temperature, float humidity) {
  String url = "/workspaces/895fa8d3bb90430f922f93009ce55c1f/services/3e33e9ed861e49b1ade3915ef6c96130/execute?api-version=2.0&details=true";
  String body = "{\"Inputs\": {\"input1\": {\"ColumnNames\": [\"Temperature\", \"Humidity\", \"Year\", \"Month\", \"Day\", \"Military Time\", \"Anomaly\"], \"Values\": [[";
  body += "\"" + String((int)temperature) + "\"," + "\"" + String((int)humidity) + "\"," + "\"" + "" + "\"," + "\"" + "" + "\"," + "\"" + "" + "\"," + "\"" + "" + "\"," + "\"" + "" + "\"" + "]]}}}";
  if (azureml.connect("ussouthcentral.services.azureml.net", 443)) {
    Serial.println(F("Connected to: ussouthcentral.services.azureml.net"));
    //Send the HTTP POST request:
    //    azureml.print(String("POST ") + url + " HTTP/1.1\r\n" +
    //                  "Host: ussouthcentral.services.azureml.net\r\n" +
    //                  "Authorization: Bearer ubj7KmBnvDqUE+qAFVssQik8tJWUYCNX1GQQweNWsaPT/BXbm9wZEb7rouNvRUs1R/YAB72wGrMVRY5zlMYtUg==\r\n" +
    //                  "Content-Type: application/json\r\n" +
    //                  "Content-Length: " + String(body.length()) + "\r\n\r\n" +
    //                  body + "\r\n\r\n");

    //    Serial.print(String("POST ") + url + " HTTP/1.1\r\n" +
    //                 "Host: ussouthcentral.services.azureml.net\r\n" +
    //                 "Authorization: Bearer ubj7KmBnvDqUE+qAFVssQik8tJWUYCNX1GQQweNWsaPT/BXbm9wZEb7rouNvRUs1R/YAB72wGrMVRY5zlMYtUg==\r\n" +
    //                 "Content-Type: application/json\r\n" +
    //                 "Content-Length: " + String(body.length()) + "\r\n\r\n" +
    //                 body + "\r\n\r\n");

    azureml.print(F("POST "));
    azureml.print(url);
    azureml.println(F(" HTTP/1.1"));
    azureml.println(F("Host: ussouthcentral.services.azureml.net"));
    azureml.println(F("Authorization: Bearer ubj7KmBnvDqUE+qAFVssQik8tJWUYCNX1GQQweNWsaPT/BXbm9wZEb7rouNvRUs1R/YAB72wGrMVRY5zlMYtUg=="));
    azureml.println(F("Content-Type: application/json"));
    azureml.print(F("Content-Length: "));
    azureml.println(body.length());
    azureml.println();
    azureml.print(body);
    azureml.println();

    Serial.print(F("POST "));
    Serial.print(url);
    Serial.println(F(" HTTP/1.1"));
    Serial.println(F("Host: ussouthcentral.services.azureml.net"));
    Serial.println(F("Authorization: Bearer ubj7KmBnvDqUE+qAFVssQik8tJWUYCNX1GQQweNWsaPT/BXbm9wZEb7rouNvRUs1R/YAB72wGrMVRY5zlMYtUg=="));
    Serial.println(F("Content-Type: application/json"));
    Serial.print(F("Content-Length: "));
    Serial.println(body.length());
    Serial.println();
    Serial.print(body);
    Serial.println();

    unsigned long timeout = millis();
    while (azureml.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(F(">>> Client Timeout !"));
        azureml.stop();
        return "Error";
      }
    }
    String json = "";
    String reply = "";
    boolean httpBody = false;
    while (azureml.available()) {
      String line = azureml.readStringUntil('\r');
      reply += line;
      //Serial.print(line);
      if (!httpBody && line.charAt(1) == '{') {
        httpBody = true;
      }
      if (httpBody) {
        json += line;
        httpBody = false;
      }
    }
    azureml.stop();
    Serial.println(reply);
    return json;
  }
  else {
    Serial.println(F("The connection could not be established"));
    azureml.stop();
    return "Error";
  }
}

void printWifiStatus() {
  //Print SSID name
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  //Print ipv4 assigned to WiFi101 module
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  //Print signal strength for WiFi101 module
  long rssi = WiFi.RSSI();
  Serial.print(F("Signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}

void WiFiInit() {
  delay(1000);  //Wait 1 sec for module initialization

  //Check if WiFi Shield is connected to Arduino
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    //Infinite loop if shield is not detected
    while (true);
  }

  //Attempt a WiFi connection to desired access point at ssid, password
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to SSID: "));
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
  pinMode(LED_BUILTIN, OUTPUT);

  //Physical outputs initialization
  digitalWrite(LED_BUILTIN, LOW);

  //Communications
  Serial.begin(9600); //Serial communications with computer at 9600 bauds for debug purposes
  dht.begin();  //Initialize communications with DHT11 sensor
  WiFiInit(); //WiFi communications initialization
}

void loop() {
  if (millis() - lastConnectionTime > TPOST) {  //Send request to server every TPOST seconds
    propertyValues[0] = dht.readTemperature(); //Read DHT11 temperature
    propertyValues[1] = dht.readHumidity(); //Read DHT11 humidity
    propertyValues[2] = anomaly;  //Anomaly state
    inputJson = POST(propertyValues[0], propertyValues[1]); //Send current Temp & Humid to Azure ML PCA to check anomalies
    StaticJsonBuffer<bufferSize> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(inputJson);
    Serial.println(inputJson);
    if (root.success()) anomaly = root["Results"]["output1"]["value"]["Values"][0][1];  //Extract anomaly from Azure recieved JSON
    else Serial.println(F("Error parsing json"));
    Serial.println("Anomaly: " + String(anomaly));
    if (anomaly == 1) digitalWrite(LED_BUILTIN, HIGH);  //Turn ON the Built In LED if there is an anomaly
    else digitalWrite(LED_BUILTIN, LOW);  //Turn OFF Built In LED if everything is OK
    myThing.post(sensorCount, propertyNames, propertyValues); //Send all values to TWX platform
    lastConnectionTime = millis();  //Refresh last connection time
  }
}
