#define BLYNK_TEMPLATE_ID "TMPL6coLvjULI"
#define BLYNK_TEMPLATE_NAME "Temperature and Humidity Monitor"
#define BLYNK_AUTH_TOKEN "CkhNTPMO42Yc0kaJYJBfQqzgLz6wBZa3"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <esp_task_wdt.h>
#include <esp_sleep.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "HotSpot - :D";  // type your wifi name
char pass[] = "aradeahah";  // type your wifi password

int speakerPin = 5;
int speakerPin_1 = 18;

BlynkTimer timer;

#define DHTPIN 19 // Connect Out pin to D2 in NODE MCU
#define HOT 5
#define NORM 18
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define NORMAL_TEMPERATURE_THRESHOLD 27.0
#define HOT_TEMPERATURE_THRESHOLD 27.5

TaskHandle_t BlynkTask;
TaskHandle_t SensorTask;

void BlynkTaskFunction(void *pvParameters) {
  while (1) {
    Blynk.run();
    timer.run();
    vTaskDelay(1);
  }
}

void SensorTaskFunction(void *pvParameters) {
  while (1) {
    sendSensor();
    vTaskDelay(100L); // Adjust the delay according to your needs
  }
}

void sendSensor() {
  pinMode(speakerPin, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(speakerPin_1, OUTPUT);
  pinMode(18, OUTPUT);

  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // You can send any value at any time.
  // Please don't send more than 10 values per second.
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print("    Humidity: ");
  Serial.println(h);

  if (t <= NORMAL_TEMPERATURE_THRESHOLD) {
    Serial.println("Normal!");
    digitalWrite(NORM, HIGH);
    digitalWrite(speakerPin_1, HIGH);
    delay(1000);
    digitalWrite(speakerPin_1, LOW);
    digitalWrite(NORM, LOW);
  }

  if (t >= HOT_TEMPERATURE_THRESHOLD) {
    Serial.println("Too hot!");
    digitalWrite(HOT, HIGH);
    digitalWrite(speakerPin, HIGH);
    delay(1000);
    digitalWrite(speakerPin, LOW);
    digitalWrite(HOT, LOW);
  } else if (t >= NORMAL_TEMPERATURE_THRESHOLD) {
    digitalWrite(speakerPin_1, HIGH);
    delay(1000);
    digitalWrite(speakerPin_1, LOW);
  }

  // Put the ESP32 into deep sleep mode for 3 seconds
  esp_sleep_enable_timer_wakeup(3000000);
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  dht.begin();

  pinMode(speakerPin, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(speakerPin_1, OUTPUT);
  pinMode(18, OUTPUT);

  xTaskCreatePinnedToCore(BlynkTaskFunction, "BlynkTask", 4096, NULL, 1, &BlynkTask, 1);
  xTaskCreatePinnedToCore(SensorTaskFunction, "SensorTask", 4096, NULL, 1, &SensorTask, 0);

  timer.setInterval(100L, sendSensor);
}

void loop() {
  // This loop should be empty, as tasks are handling the functionality
}
