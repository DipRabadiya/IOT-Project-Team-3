import paho.mqtt.client as mqtt
import time
import json
import random

client = mqtt.Client()

client.connect("localhost", 1883, 60)

while True:
    data = {
        "heart_rate": random.randint(60, 100),
        "temperature": round(random.uniform(36.0, 38.0), 2)
    }

    client.publish("icu/patient1", json.dumps(data))


    print("Data Sent:", data)

    time.sleep(2)