/*
=================================================
ICU PATIENT MONITOR
ESP8266 + MAX30102 + DS18B20
Node-RED + Telegram + InfluxDB
=================================================
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#include <OneWire.h>
#include <DallasTemperature.h>

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// =================================================
// WIFI
// =================================================

const char* ssid = "icunetwork";
const char* password = "Team3@123";

// =================================================
// NODE-RED
// =================================================

const char* nodeRedUrl =
"http://192.168.0.111:1880/iot-data";

// =================================================
// INFLUXDB
// =================================================

#define INFLUXDB_URL "http://192.168.137.1:8086"

//#define INFLUXDB_TOKEN "jV482-uDMiKBm2_Xm5x2IvOh9l1Jz8ho7Ch5h3_xVLwIiGQXStaGRTMXTcpsnhdDnXKuTQXRf7aMt_pzKm-0uA=="

#define INFLUXDB_ORG "iot-org"

#define INFLUXDB_BUCKET "iot-body-monitor"

// =================================================
// INFLUX CLIENT
// =================================================

InfluxDBClient client(
  INFLUXDB_URL,
  INFLUXDB_ORG,
  INFLUXDB_BUCKET,
  INFLUXDB_TOKEN
);

Point patient("icu_patient_monitor");

// =================================================
// DS18B20
// D4 = GPIO2
// =================================================

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature tempSensor(&oneWire);

// =================================================
// MAX30102
// SDA = D2(GPIO4)
// SCL = D1(GPIO5)
// =================================================

MAX30105 particleSensor;

const byte RATE_SIZE = 4;

byte rates[RATE_SIZE];

byte rateSpot = 0;

byte validCount = 0;

byte warmupBeats = 0;

long lastBeat = 0;

float bpm = 0;

int beatAvg = 0;

// =================================================

unsigned long lastSend = 0;

// =================================================

void setup()
{
  Serial.begin(115200);

  Serial.println();
  Serial.println("================================");
  Serial.println("ICU PATIENT MONITOR STARTING");
  Serial.println("================================");

  // --------------------------------
  // DS18B20
  // --------------------------------

  tempSensor.begin();

  // --------------------------------
  // MAX30102
  // --------------------------------

  Wire.begin(4, 5);

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD))
  {
    Serial.println("MAX30102 NOT FOUND");

    while (1)
    {
      delay(100);
    }
  }

  particleSensor.setup(
      60,
      4,
      2,
      100,
      411,
      4096
  );

  Serial.println("MAX30102 READY");

  // --------------------------------
  // WIFI
  // --------------------------------

  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // --------------------------------
  // INFLUXDB
  // --------------------------------

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

// =================================================

void loop()
{
  long irValue = particleSensor.getIR();

  if (irValue > 10000)
  {
    if (checkForBeat(irValue))
    {
      long delta = millis() - lastBeat;

      lastBeat = millis();

      bpm = 60.0 / (delta / 1000.0);

      if (bpm > 50 && bpm < 180)
      {
        warmupBeats++;

        if (warmupBeats < 4)
        {
          return;
        }

        rates[rateSpot] = (byte)bpm;

        rateSpot++;

        if (rateSpot >= RATE_SIZE)
        {
          rateSpot = 0;
        }

        if (validCount < RATE_SIZE)
        {
          validCount++;
        }

        int total = 0;

        for (int i = 0; i < validCount; i++)
        {
          total += rates[i];
        }

        beatAvg = total / validCount;

        Serial.print("Raw BPM: ");
        Serial.print(bpm);

        Serial.print("   Avg BPM: ");
        Serial.println(beatAvg);
      }
    }
  }
  else
  {
    beatAvg = 0;
  }

  // --------------------------------
  // SEND EVERY 5 SECONDS
  // --------------------------------

  if (millis() - lastSend < 5000)
  {
    return;
  }

  lastSend = millis();

  tempSensor.requestTemperatures();

  float body_temperature =
      tempSensor.getTempCByIndex(0);

  Serial.println("--------------------------------");

  Serial.print("Temperature: ");
  Serial.print(body_temperature);
  Serial.println(" C");

  Serial.print("Heart Rate: ");
  Serial.print(beatAvg);
  Serial.println(" BPM");

  // if (beatAvg < 50)
  // {
  //   Serial.println("Waiting for stable BPM...");
  //   return;
  // }

  // --------------------------------
  // WRITE TO INFLUXDB
  // --------------------------------

  patient.clearFields();

patient.addField(
    "body_temperature",
    body_temperature
);

if(beatAvg > 0)
{
    patient.addField(
        "heart_rate",
        beatAvg
    );
}

  if (!client.writePoint(patient))
  {
    Serial.print("Influx Write Failed: ");

    Serial.println(
      client.getLastErrorMessage()
    );
  }
  else
  {
    Serial.println(
      "InfluxDB Write SUCCESS"
    );
  }

  // --------------------------------
  // SEND TO NODE-RED
  // --------------------------------

  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClient wifiClient;

    HTTPClient http;

    http.begin(
      wifiClient,
      nodeRedUrl
    );

    http.addHeader(
      "Content-Type",
      "application/json"
    );

    String jsonData = "{";

jsonData += "\"body_temperature\":";
jsonData += String(body_temperature, 1);

if(beatAvg > 0)
{
    jsonData += ",\"heart_rate\":";
    jsonData += String(beatAvg);
}

jsonData += "}";

    Serial.print("Sending: ");
    Serial.println(jsonData);

    int httpCode =
      http.POST(jsonData);

    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    http.end();
}
  }
