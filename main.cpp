#include <Arduino.h>
#include <heltec.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <bsec.h>
#include <Wire.h>
#include <ArduinoJson.h>
// WiFi
WiFiClient wifiClient;
const char *ssid = "THMnet";                   // put in your WIFI SSID, provided in lab
const char *password = "bVqh7rbb9AxbTpKMLfQN"; // put in WIFI password

// MQTT
MqttClient mqttClient(wifiClient);
const char broker[] = "mqtt.ei.thm.de";           // MQTT Broker FQDN, will be provided in lab
int port = 1993;                                  // MQTT Port
const char topic[] = "THM/IoTLab/Atharva/BME680"; // MQTT Topic
// BME680
//  *** Task 4 – "Read Sensor Data" ***
#define SDA2 21              // Data_BME680
#define SCL2 13              // Clock_BME680
TwoWire I2Ctwo = TwoWire(1); // for BME680
// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);
// Create an object of the class Bsec
Bsec iaqSensor;
String output; // output string: error msg or sensor data
// *** Task 5 – "Show Data on OLED" ***
//declarations Heltec
#define BAND 868E6  //you can set band here directly,e.g. 868E6,915E6
#define SDA1 SDA_OLED
#define SCL1 SCL_OLED
TwoWire I2Cone = TwoWire(0);

// *** Task 6 – "Send JSON via MQTT " ***
//JSON
JsonDocument doc;


void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println("You're connected to the network");
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // MQTT
  //  You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  mqttClient.setId("2400212345");
  // You can provide a username and password for authentication
  mqttClient.setUsernamePassword("iotlab", "iotlab");
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);
  if (!mqttClient.connect(broker, port))
  {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1)
      ;
  }
  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
  // BME680
  // *** Task 4 – "Read Sensor Data" ***
  I2Ctwo.begin(SDA2, SCL2);
  iaqSensor.begin(BME68X_I2C_ADDR_HIGH, I2Ctwo);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();
  bsec_virtual_sensor_t sensorList[10] = {
      BSEC_OUTPUT_RAW_TEMPERATURE,
      BSEC_OUTPUT_RAW_PRESSURE,
      BSEC_OUTPUT_RAW_HUMIDITY,
      BSEC_OUTPUT_RAW_GAS,
      BSEC_OUTPUT_IAQ,
      BSEC_OUTPUT_STATIC_IAQ,
      BSEC_OUTPUT_CO2_EQUIVALENT,
      BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };
  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();

  // Print the header
  output = "Timestamp [ms], raw temperature [°C], pressure [hPa], raw relative humidity [%], gas [Ohm], IAQ, IAQ accuracy, temperature [°C], relative humidity [%], Static IAQ, CO2 equivalent, breath VOC equivalent";
  Serial.println(output);
  // ******************************
      // *** Task 6 – "Send JSON via MQTT " ***


 // OLED
  // *** Task 5 – "Show Data on OLED" ***
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Enable*/, true /*Serial Enable*/, true /*LoRa use PABOOST*/, BAND /*LoRa RF working band*/);
  Heltec.display -> clear();
  Heltec.display -> drawString(0, 0, "TEST");
  Heltec.display -> display();


}
void loop()
{
  // put your main code here, to run repeatedly:
  // ******************************
  // *** Task 2 – "Connect to local wifi" ***
  while (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0))
  {
    WiFi.reconnect();
    delay(5000);
    // *** Task 3 – "Connect MQTT Broker" ***
    if (!mqttClient.connected())
    {
      mqttClient.connect(broker, port);
    }
    delay(1000);
  }
  // ******************************

  // ******************************
  // *** Task 3 – "Connect MQTT Broker" ***
  if (!mqttClient.connected())
  {
    mqttClient.connect(broker, port);
  }
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // ******************************
  // Comment out when working on Task 6 !!!
 // mqttClient.beginMessage(topic);
  //mqttClient.print("TEST");
  //mqttClient.endMessage();
  //delay(1000);
  // ******************************
  // BME680
  // *** Task 4 – "Read Sensor Data" ***
  unsigned long time_trigger = millis();
  if (iaqSensor.run())
  { // If new data is available
    output = String(time_trigger);
    output += ", " + String(iaqSensor.iaq);
    output += ", " + String(iaqSensor.iaqAccuracy);
    output += ", " + String(iaqSensor.staticIaq);
    output += ", " + String(iaqSensor.co2Equivalent);
    output += ", " + String(iaqSensor.breathVocEquivalent);
    output += ", " + String(iaqSensor.rawTemperature);
    output += ", " + String(iaqSensor.pressure);
    output += ", " + String(iaqSensor.rawHumidity);
    output += ", " + String(iaqSensor.gasResistance);
    output += ", " + String(iaqSensor.stabStatus);
    output += ", " + String(iaqSensor.runInStatus);
    output += ", " + String(iaqSensor.temperature);
    output += ", " + String(iaqSensor.humidity);
    output += ", " + String(iaqSensor.gasPercentage);
    Serial.println(output);
  	// *** Task 5 – "Show Data on OLED" ***
    // OLED Display
    Heltec.display -> clear();
    Heltec.display -> setFont(ArialMT_Plain_10);
    Heltec.display -> drawString(0, 0, "Measurement BME680");
    Heltec.display -> drawString(0, 0, "Temp: " + String(iaqSensor.temperature)+ " °C");
    Heltec.display -> drawString(0, 10, "Humid: " + String(iaqSensor.humidity) + " %");
    Heltec.display -> drawString(0, 20, "IAQ: " + String(iaqSensor.iaq) + "IAQ_A: " + String(iaqSensor.iaqAccuracy));
    Heltec.display -> drawString(0, 30, "VOC: " + String(iaqSensor.breathVocEquivalent));
    Heltec.display -> drawString(0, 40, "Press: " + String(iaqSensor.pressure));
    Heltec.display -> drawString(0, 50, "CO2: " + String(iaqSensor.co2Equivalent));
    Heltec.display -> display();
    delay(100);
    Heltec.display -> clear();
    	// *** Task 6 – "Send JSON via MQTT " ***

      
//JSON via MQTT
doc["SensorID"] = "BME680_Weber";
doc["Temp_R"] = String(iaqSensor.rawTemperature);
doc["Pres_R"] = String(iaqSensor.pressure);
doc["Humid_R"] = String(iaqSensor.rawHumidity);
doc["GasResi_R"] = String(iaqSensor.gasResistance);
doc["IAQ"] = String(iaqSensor.iaq);
doc["IAQ_A"] = String(iaqSensor.iaqAccuracy);
doc["Temp"] = String(iaqSensor.temperature);
doc["Humid"] = String(iaqSensor.humidity);
doc["IAQ_S"] = String(iaqSensor.staticIaq);
doc["CO2"] = String(iaqSensor.co2Equivalent);
doc["VOC"] = String(iaqSensor.breathVocEquivalent);
mqttClient.beginMessage(topic);
serializeJson(doc, mqttClient);
mqttClient.endMessage();
doc.clear();

    //send JSON to MQTT Broker
    //mqttClient.beginMessage(topic);
    //serializeJson(doc,mqttClient);
    //mqttClient.print(buffer);
    //mqttClient.endMessage();
    //doc.clear();
   

  } else {
    checkIaqSensorStatus();
  }
    
}
// Helper function definitions
void checkIaqSensorStatus(void)
{
  if (iaqSensor.bsecStatus != BSEC_OK)
  {
    if (iaqSensor.bsecStatus < BSEC_OK)
    {
      output = "BSEC error code : " + String(iaqSensor.bsecStatus);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    }
    else
    {
      output = "BSEC warning code : " + String(iaqSensor.bsecStatus);
      Serial.println(output);
    }
  }
  if (iaqSensor.bme68xStatus != BME68X_OK)
  {
    if (iaqSensor.bme68xStatus < BME68X_OK)
    {
      output = "BME68X error code : " + String(iaqSensor.bme68xStatus);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    }
    else
    {
      output = "BME68X warning code : " + String(iaqSensor.bme68xStatus);
      Serial.println(output);
    }
  }
}

void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}
