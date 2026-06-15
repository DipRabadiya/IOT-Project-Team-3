#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 8;

byte rates[RATE_SIZE];
byte rateSpot = 0;
byte validCount = 0;

long lastBeat = 0;

float bpm = 0;
int beatAvg = 0;

void setup()
{
  Serial.begin(115200);

  Serial.println();
  Serial.println("MAX30102 BPM TEST");

  Wire.begin(4, 5);

  if (!particleSensor.begin(Wire))
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

  Serial.println("MAX30102 READY");
}

void loop()
{
  long irValue = particleSensor.getIR();

  // Finger detection
  if (irValue < 10000)
  {
    Serial.println("NO FINGER");
    delay(500);
    return;
  }

  if (checkForBeat(irValue))
  {
    long delta = millis() - lastBeat;

    lastBeat = millis();

    bpm = 60.0 / (delta / 1000.0);

    // Ignore unrealistic BPM
    if (bpm > 50 && bpm < 180)
    {
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

      Serial.print("IR: ");
      Serial.print(irValue);

      Serial.print("   Raw BPM: ");
      Serial.print(bpm);

      Serial.print("   Avg BPM: ");
      Serial.println(beatAvg);
    }
  }

  delay(10);
}
