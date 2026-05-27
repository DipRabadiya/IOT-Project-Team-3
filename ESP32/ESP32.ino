/*
   ==========================================
   ESP8266 ICU Monitoring System
   DS18B20 + Pulse Sensor
   InfluxDB 2.x
   ==========================================
*/
 
#include <ESP8266WiFi.h>
 
#include <OneWire.h>
#include <DallasTemperature.h>
 
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
 
// =====================================
// WIFI
// =====================================
const char* ssid = "icunetwork";
const char* password = "Team3@123";
 
// =====================================
// InfluxDB 2.x Settings
// =====================================
 
// Friend Laptop IP
#define INFLUXDB_URL "http://192.168.137.110:8086"
 
// Organization
#define INFLUXDB_ORG "iot-org"
 
// Bucket
#define INFLUXDB_BUCKET "iot-body-monitor"
 
// Token
// #define INFLUXDB_TOKEN "JSsROqrguG6lfYdP88KNzDbBm65cIO1mUhuZKAg7RUlSLYerXkDyAyXj9r8uGuGtjDFlBhJdqjFhf8ozAHE4vg=="
 
// =====================================
// Influx Client
// =====================================
InfluxDBClient client(
  INFLUXDB_URL,
  INFLUXDB_ORG,
  INFLUXDB_BUCKET,
  INFLUXDB_TOKEN
);
 
// =====================================
// Data Point
// =====================================
Point sensor("patient");
 
// =====================================
// DS18B20 Setup
// =====================================
#define ONE_WIRE_BUS 2
 
OneWire oneWire(ONE_WIRE_BUS);
 
DallasTemperature tempSensor(&oneWire);
 
// =====================================
// Pulse Sensor
// =====================================
const int pulsePin = A0; //For ADC
 
int threshold = 700;
 
bool beatDetected = false;
 
unsigned long lastBeatTime = 0;
 
int BPM = 0;
 
void setup()
{
  Serial.begin(115200);
 
  Serial.println();
  Serial.println("ICU Monitoring Started");
 
  // ------------------------------
  // WIFI CONNECT
  // ------------------------------
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println();
  Serial.println("WiFi Connected");
 
  // ------------------------------
  // Start Temperature Sensor
  // ------------------------------
  tempSensor.begin();
 
  // ------------------------------
  // Check InfluxDB Connection
  // ------------------------------
  if (client.validateConnection())
  {
    Serial.println("InfluxDB Connected");
  }
  else
  {
    Serial.print("InfluxDB Error: ");
 
    Serial.println(
      client.getLastErrorMessage()
    );
  }
}
 
void loop()
{
  // ==================================
  // READ TEMPERATURE
  // ==================================
 
  tempSensor.requestTemperatures();
 
  float tempC =
    tempSensor.getTempCByIndex(0);
 
  // ==================================
  // READ PULSE SENSOR
  // ==================================
 
  long total = 0;
 
  for (int i = 0; i < 10; i++)
  {
    total += analogRead(A0);
 
    delay(2);
  }
 
  int pulseValue = total / 10;
 
  // ==================================
  // BPM
  // ==================================
 
  if (pulseValue > threshold &&
      !beatDetected)
  {
    beatDetected = true;
 
    unsigned long currentTime =
      millis();
 
    unsigned long interval =
      currentTime - lastBeatTime;
 
    lastBeatTime = currentTime;
 
    BPM = 60000 / interval;
  }
 
  if (pulseValue < threshold - 50)
  {
    beatDetected = false;
  }
 
  // ==================================
  // SERIAL PRINT
  // ==================================
 
  Serial.println("----------------");
 
  Serial.print("Temperature: ");
  Serial.println(tempC);
 
  Serial.print("Pulse: ");
  Serial.println(pulseValue);
 
  Serial.print("BPM: ");
  Serial.println(BPM);
 
  // ==================================
  // PREPARE DATA
  // ==================================
 
  sensor.clearFields();
 
  sensor.addField(
    "temperature",
    tempC
  );
 
  sensor.addField(
    "pulse",
    pulseValue
  );
 
  sensor.addField(
    "bpm",
    BPM
  );
 
  // ==================================
  // WRITE TO INFLUXDB
  // ==================================
 
  if (!client.writePoint(sensor))
  {
    Serial.print("InfluxDB Write Failed: ");
 
    Serial.println(
      client.getLastErrorMessage()
    );
  }
  else
  {
    Serial.println("Data Written Successfully");
  }
 
  delay(5000);
}
