/*
   ESP8266 + Pulse Sensor BPM
*/

const int pulsePin = A0;

int threshold = 700;

bool beatDetected = false;

unsigned long lastBeatTime = 0;

int BPM = 0;

void setup() {

  Serial.begin(115200);

  Serial.println("Pulse Sensor Started");
}

void loop() {

  // Take average of 10 readings
  long total = 0;

  for (int i = 0; i < 10; i++) {

    total += analogRead(A0);

    delay(2);
  }

  int pulseValue = total / 10;

  Serial.print("Signal: ");
  Serial.print(pulseValue);

  // No finger
  if (pulseValue > 1000) {

    Serial.print("   --> No Finger");
  }

  else {

    Serial.print("   --> Finger Detected");

    // Beat detection
    if (pulseValue > threshold && !beatDetected) {

      beatDetected = true;

      unsigned long currentTime = millis();

      unsigned long interval = currentTime - lastBeatTime;

      lastBeatTime = currentTime;

      BPM = 60000 / interval;

      // Accept realistic BPM only
      if (BPM >= 40 && BPM <= 180) {

        Serial.print("   | BPM: ");
        Serial.print(BPM);
      }
    }

    // Reset detection
    if (pulseValue < threshold - 50) {

      beatDetected = false;
    }
  }

  Serial.println();

  delay(2000);
}
