from sense_hat import SenseHat
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import requests
import math
import time
# =====================================
# SENSE HAT INITIALIZATION
# =====================================
sense = SenseHat()
 
sense.low_light = True
sense.clear((0, 0, 0))
# =====================================
# INFLUXDB SETTINGS
# =====================================
#url = "http://192.168.137.110:8086"
url = "http://192.168.137.1:8086"
#token = "2deztI2SQso_JlxSeAls5qNDz9F278-ZKP6u4rWR5FLEnIc21ymPDSxe17LGOcTJP1gK2uEuTdCm_iBUlGPJCQ=="
org = "iot-org"
bucket = "iot-body-monitor"
measurement = "icu_room_monitor"
# =====================================
# CONNECT TO INFLUXDB
# =====================================
client = InfluxDBClient(
    url=url,
    token=token,
    org=org
)
write_api = client.write_api(
    write_options=SYNCHRONOUS
)
print("===================================")
print(" ICU ROOM MONITOR STARTED ")
print("===================================")
while True:
    try:
        # ---------------------------------
        # READ SENSE HAT VALUES
        # ---------------------------------
        room_temperature = sense.get_temperature()
        room_humidity = sense.get_humidity()
        room_pressure = sense.get_pressure()
        accel = sense.get_accelerometer_raw()
        motion_level = math.sqrt(
            accel["x"] ** 2 +
            accel["y"] ** 2 +
            accel["z"] ** 2
        )
        # ---------------------------------
        # PRINT VALUES
        # ---------------------------------
        print("\n-----------------------------------")
        print(
            "Room Temperature :",
            round(room_temperature, 2),
            "ï¿½C"
        )
        print(
            "Room Humidity    :",
            round(room_humidity, 2),
            "%"
        )
        print(
            "Room Pressure    :",
            round(room_pressure, 2),
            "hPa"
        )
        print(
            "Motion Level     :",
            round(motion_level, 3)
        )
        # ---------------------------------
        # LED STATUS
        # ---------------------------------
 
        def show_center_4x4(color):
             # Turn OFF all 64 LEDs
            for x in range(2, 6):      # Columns 2,3,4,5
                 for y in range(2, 6):  # Rows 2,3,4,5
                    sense.set_pixel(x, y, color)
 
        if motion_level < 1.05:
 
        # GREEN
 
            show_center_4x4((0, 255, 0))
 
        elif motion_level < 1.30:
 
        # YELLOW
 
            show_center_4x4((255, 255, 0))
 
        else:
 
        # RED
 
            show_center_4x4((255, 0, 0))
        # ---------------------------------
        # CREATE DATA POINT
        # ---------------------------------
        point = (
            Point(measurement)
            .field(
                "room_temperature",
                float(room_temperature)
            )
            .field(
                "room_humidity",
                float(room_humidity)
            )
            .field(
                "room_pressure",
                float(room_pressure)
            )
            .field(
                "motion_level",
                float(motion_level)
            )
        )
        # ---------------------------------
        # WRITE TO INFLUXDB
        # ---------------------------------
        write_api.write(
            bucket=bucket,
            record=point
        )
        print("InfluxDB Write: SUCCESS")
 
        payload = {
            "source": "raspberry_pi",
            "room_temperature": round(room_temperature, 2),
            "room_humidity": round(room_humidity, 2),
            "room_pressure": round(room_pressure, 2),
            "motion_level": round(motion_level, 3)
        }
        try:
            response = requests.post(
            "http://192.168.0.111:1880/iot-data",
            json=payload,
            timeout=5
            )
            print(
             "Node-RED Status:",
            response.status_code
                )
        except Exception as e:
            print(
        "Node-RED Error:",
        str(e)
            )

    except Exception as e:
        print("\nERROR OCCURRED")
        print(str(e))
    time.sleep(1)
