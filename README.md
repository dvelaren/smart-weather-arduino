# smart-weather-arduino
![alt text](https://raw.githubusercontent.com/tidusdavid/smart-weather-arduino/master/Resources/Architecture.png)
![alt text](https://raw.githubusercontent.com/tidusdavid/smart-weather-arduino/master/Resources/Device.jpg)
This project uses PCA machine learning from Azure ML to predict anomalies on a weather station system using Arduino.

## Testing

In order to test this project, upload the ArduinoCodes/postThingWorxV2/postThingWorxV2.ino to the Arduino MKR1000, edit the code header to match your SSID, Thingworx server, thing name, service name, api key, etc and then monitor through Thingworx Platform that the values change depending on the DHT11 sensor state.
