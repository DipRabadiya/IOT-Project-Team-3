/*
   ==========================================
   ESP8266 ICU Monitoring System
   DS18B20 + Pulse Sensor
   InfluxDB 2.x
   ==========================================
*/
 
#include <ESP8266WiFi.h>
 
 
 
// =====================================
// WIFI
// =====================================
const char* ssid = "icunetwork";
const char* password = "Team3@123";
 
  Serial.println("WiFi Connected");
 
