#include <BTL-Thiet-ke-dien-tu-tien-tien_inferencing.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SimpleTimer.h>
#include <U8g2lib.h>

#define FREQUENCY_HZ 60
#define INTERVAL_MS (1000 / (FREQUENCY_HZ + 1))

sensors_event_t a, g, temp;

#define LEDG 16
#define LEDR 17
#define BUZZER 18

float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
size_t feature_ix = 0;
static unsigned long last_interval_ms = 0;

Adafruit_MPU6050 mpu;
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);
SimpleTimer timer;

void ei_printf(const char *format, ...) {
  static char print_buf[1024] = { 0 };

  va_list args;
  va_start(args, format);
  int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
  va_end(args);

  if (r > 0) {
    Serial.write(print_buf);
  }
}

void getMPUData() {
  mpu.getEvent(&a, &g, &temp);
  features[feature_ix++] = a.acceleration.x;
  features[feature_ix++] = a.acceleration.y;
  features[feature_ix++] = a.acceleration.z;
}

void detectMPUData() {
  if (feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
    Serial.println("Running the inference...");
    signal_t signal;
    ei_impulse_result_t result;
    int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
      ei_printf("Failed to create signal from buffer (%d)\n", err);
      return;
    }

    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, true);

    if (res != 0) return;

    ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms.)",
              result.timing.dsp, result.timing.classification);
    ei_printf(": \n");

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);

      if (result.classification[ix].value > 0.95) {
        if (result.classification[ix].label == "fall") {
          digitalWrite(LEDG, LOW);
          digitalWrite(LEDR, HIGH);
          digitalWrite(BUZZER, HIGH);

          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_helvB10_tr);
          u8g2.setCursor(5, 15);
          u8g2.print("Trang thai:");
          u8g2.setCursor(5, 30);
          u8g2.print("Da bi nga");
          u8g2.sendBuffer();

          delay(5000);
          digitalWrite(LEDG, HIGH);
          digitalWrite(LEDR, LOW);
          digitalWrite(BUZZER, LOW);
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_helvB10_tr);
          u8g2.setCursor(5, 15);
          u8g2.print("Trang thai:");
          u8g2.setCursor(5, 30);
          u8g2.print("Dung yen");
          u8g2.sendBuffer();
        } else if (result.classification[ix].label == "idle") {
          digitalWrite(LEDG, HIGH);
          digitalWrite(LEDR, LOW);
          digitalWrite(BUZZER, LOW);
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_helvB10_tr);
          u8g2.setCursor(5, 15);
          u8g2.print("Trang thai:");
          u8g2.setCursor(5, 30);
          u8g2.print("Dung yen.");
          u8g2.sendBuffer();
        } else if (result.classification[ix].label == "sitdown") {
          digitalWrite(LEDG, HIGH);
          digitalWrite(LEDR, LOW);
          digitalWrite(BUZZER, LOW);
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_helvB10_tr);
          u8g2.setCursor(5, 15);
          u8g2.print("Trang thai:");
          u8g2.setCursor(5, 30);
          u8g2.print("Ngoi xuong.");
          u8g2.sendBuffer();
        } else if (result.classification[ix].label == "standup") {
          digitalWrite(LEDG, HIGH);
          digitalWrite(LEDR, LOW);
          digitalWrite(BUZZER, LOW);
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_helvB10_tr);
          u8g2.setCursor(5, 15);
          u8g2.print("Trang thai:");
          u8g2.setCursor(5, 30);
          u8g2.print("Dung day.");
          u8g2.sendBuffer();
        } else if (result.classification[ix].label == "walk") {
          digitalWrite(LEDG, HIGH);
          digitalWrite(LEDR, LOW);
          digitalWrite(BUZZER, LOW);
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_helvB10_tr);
          u8g2.setCursor(5, 15);
          u8g2.print("Trang thai:");
          u8g2.setCursor(5, 30);
          u8g2.print("Di bo.");
          u8g2.sendBuffer();
        } else {
          digitalWrite(LEDG, HIGH);
          digitalWrite(LEDR, LOW);
          digitalWrite(BUZZER, LOW);
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_helvB10_tr);
          u8g2.setCursor(5, 15);
          u8g2.print("Trang thai:");
          u8g2.setCursor(5, 30);
          u8g2.print("Dang xac dinh...");
          u8g2.sendBuffer();
        }
      }
    }
    feature_ix = 0;
  }
}

void setup() {
  Serial.begin(115200);
  u8g2.begin();

  pinMode(LEDG, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(BUZZER, OUTPUT);

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

  Serial.print("Features: ");
  Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  Serial.print("Label count: ");
  Serial.println(EI_CLASSIFIER_LABEL_COUNT);

  timer.setInterval(20, getMPUData);
  timer.setInterval(20, detectMPUData);
}

void loop() {
  timer.run();
}
