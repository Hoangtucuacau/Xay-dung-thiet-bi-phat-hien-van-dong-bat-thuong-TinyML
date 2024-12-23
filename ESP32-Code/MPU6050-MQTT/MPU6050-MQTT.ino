#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino_JSON.h>
#include <SimpleTimer.h>

#define FREQUENCY_HZ 60
#define INTERVAL_MS (1000 / (FREQUENCY_HZ + 1))

sensors_event_t a, g, temp;

const char* ssid = "HoangNgoc";
const char* password = "22072002";
const char* mqtt_server = "192.168.137.1";
const int mqtt_port = 1883;
const char* mqtt_username = "";
const char* mqtt_password = "";
const char* mqtt_clientname = "ESP32Client";

Adafruit_MPU6050 mpu;
WiFiClient espClient;
PubSubClient client(espClient);
JSONVar data;
SimpleTimer timer;

void connectInternet() {
  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi Wi-Fi.");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 10) {
    delay(1000);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Khong the ket noi Wi-Fi.");
  } else {
    Serial.println("Da ket noi Wi-Fi.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    if (client.connect(mqtt_clientname, mqtt_username, mqtt_password)) {
      Serial.println("Connected.");
    } else {
      Serial.println("Try again.");
    }
  }
}

void getMPUData() {
  mpu.getEvent(&a, &g, &temp);
}

void displayData() {
  Serial.print(a.acceleration.x);
  Serial.print(",");
  Serial.print(a.acceleration.y);
  Serial.print(",");
  Serial.println(a.acceleration.z);
}

void sendData() {
  data["x"] = a.acceleration.x;
  data["y"] = a.acceleration.y;
  data["z"] = a.acceleration.z;

  String jsonString = JSON.stringify(data);
  client.publish("xyz", jsonString.c_str());
}

void setup() {
  Serial.begin(115200);

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG:
      Serial.println("+- 2000 deg/s");
      break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
      Serial.println("260 Hz");
      break;
    case MPU6050_BAND_184_HZ:
      Serial.println("184 Hz");
      break;
    case MPU6050_BAND_94_HZ:
      Serial.println("94 Hz");
      break;
    case MPU6050_BAND_44_HZ:
      Serial.println("44 Hz");
      break;
    case MPU6050_BAND_21_HZ:
      Serial.println("21 Hz");
      break;
    case MPU6050_BAND_10_HZ:
      Serial.println("10 Hz");
      break;
    case MPU6050_BAND_5_HZ:
      Serial.println("5 Hz");
      break;
  }

  connectInternet();
  client.setServer(mqtt_server, mqtt_port);

  timer.setInterval(20, getMPUData);
  timer.setInterval(20, displayData);
  timer.setInterval(20, sendData);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  timer.run();
}