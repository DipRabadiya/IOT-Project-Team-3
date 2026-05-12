#include <WiFi.h>
#include <PubSubClient.h>

// wifi details//

const char* ssid = "your_WIFI";
const char* password = "your_PASSWORD";

// MQTT broker (free public)//
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

//connect to wifi//
void setup_wifi(){
  Serial.print("Connecting wifi");

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nwifi connected");
}

//connect to MQTT//

void reconnect(){
  while(!client.connected()){
    Serial.print("connecting MQTT...");
    if (client.connect("ESP32_ICU")){
      Serial.println("connected!");
    }else {
      Serial.println("failed, retrying...");
      delay(2000);
    }

  }
}
void setup(){
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server,1883);
  randomSeed(analogRead(0));
}
void loop(){
  if (!client.connected()){
    reconnect();
  }
  client.loop();

  //same sensor values//
  int heartrate = random(60, 120);    
  int spo2 = random(90,100);          
  float temperature = random(360, 380) / 10.0;

  //convert to JSON format
  String message = "{";
  message += "\"heartrate\":" + String(heartrate) + ",";
  message += "\"spo2\":" + String(spo2) + ",";
  message += "\"temperature\":" + String(temperature);
  message += "}";

  Serial.println("Sending:");
  Serial.println(message);

  //send data//
  client.publish("icu/patient1",message.c_str());

  delay(3000);
}

