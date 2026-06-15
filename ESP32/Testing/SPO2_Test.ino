#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;

uint32_t irBuffer[100];
uint32_t redBuffer[100];

int32_t spo2;
int8_t validSPO2;

int32_t heartRate;
int8_t validHeartRate;

void setup()
{
  Serial.begin(115200);

  Serial.println();
  Serial.println("MAX30102 SPO2 TEST");

  Wire.begin(4, 5);   // D2=SDA, D1=SCL

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD))
  {
    Serial.println("MAX30102 NOT FOUND");

    while (1)
    {
      delay(100);
    }
  }

  particleSensor.setup(
      60,     // LED brightness
      4,      // Sample average
      2,      // RED + IR
      100,    // Sample rate
      411,    // Pulse width
      4096    // ADC range
  );

  Serial.println("Sensor Ready");
  Serial.println("Place finger on sensor");

  delay(2000);
}

void loop()
{
  // Collect 100 samples

  for (int i = 0; i < 100; i++)
  {
    while (!particleSensor.available())
    {
      particleSensor.check();
      yield();
    }

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();

    particleSensor.nextSample();
  }

  // Finger check

  if (irBuffer[99] < 10000)
  {
    Serial.println("NO FINGER");

    delay(1000);

    return;
  }

  maxim_heart_rate_and_oxygen_saturation(
      irBuffer,
      100,
      redBuffer,
      &spo2,
      &validSPO2,
      &heartRate,
      &validHeartRate
  );

  Serial.println("--------------------");

  Serial.print("IR: ");
  Serial.println(irBuffer[99]);

  if (validSPO2)
  {
    Serial.print("SpO2: ");
    Serial.print(spo2);
    Serial.println("%");
  }
  else
  {
    Serial.println("SpO2 INVALID");
  }

  Serial.println("--------------------");

  delay(2000);
}
