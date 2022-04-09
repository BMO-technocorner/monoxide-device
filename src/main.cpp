#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ESP32Ping.h>

#define XENV(x) #x
#define ENV(x) XENV(x)
#define MQ2_PIN 34
#define LED_YELLOW_PIN 32
#define LED_RED_PIN 25
#define BUZZER_PIN 27
#define SCAN_DELAY 500
#define SYNC_DELAY 60000
#define LOW_DETECTION_DELAY 10000
#define HIGH_DETECTION_DELAY 20000
#define LOW_DETECTION_SENSITIVITY_START 1500
#define LOW_DETECTION_SENSITIVITY_END 3000
#define HIGH_DETECTION_SENSITIVITY_START 3000
#define PING_DNS "8.8.8.8"

const char* wifi_ssid = ENV(WIFI_SSID);
const char* wifi_password = ENV(WIFI_PASSWORD);
const char* ca_cert = ENV(CA_CERT);
const char* api_key = ENV(API_KEY);
const char* device_key = ENV(DEVICE_KEY);
const char* user_agent = ENV(USER_AGENT);

WiFiClientSecure wifi;
HTTPClient http;
uint32_t lastDetectionMillis;
uint32_t lastSyncMillis;
bool bootSync;
bool reported;

void saveReport(int level)
{
  if (WiFi.status() != WL_CONNECTED)
    return;
  Serial.println("\nGas detected. Saving report...");
  http.begin("https://api-monoxide.ezralazuardy.com/v1/device/report");
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", user_agent);
  http.addHeader("Api-Key", api_key);
  http.addHeader("Device-Key", device_key);
  Serial.println();
  Serial.println(http.POST("{\"detectionLevel\": " + String(level) + "}"));
  Serial.println(http.getString());
  http.end();
}

void synchronize()
{
  uint32_t currentMillis = millis();
  if (bootSync)
  {
    if (currentMillis - lastSyncMillis < SYNC_DELAY)
      return;
    lastSyncMillis = currentMillis;
  }
  bootSync = true;
  if (WiFi.status() != WL_CONNECTED)
    return;
  Serial.println("\nSynchronizing...");
  http.begin("https://api-monoxide.ezralazuardy.com/v1/device/sync");
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", user_agent);
  http.addHeader("Api-Key", api_key);
  http.addHeader("Device-Key", device_key);
  Serial.println();
  Serial.println(http.POST("{}"));
  Serial.println(http.getString());
  http.end();
}

void buzzer(int frequency, int duration, int pause, int times)
{
  for (int time = 1; time <= times; time++)
  {
    ledcWriteTone(0, frequency);
    delay(duration);
    ledcWriteTone(0, 0);
    delay(pause);
  }
}

void internetConnection()
{
  while (true)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(wifi_ssid, wifi_password);
      while (WiFi.status() != WL_CONNECTED)
      {
        digitalWrite(LED_RED_PIN, HIGH);
        digitalWrite(LED_YELLOW_PIN, HIGH);
        buzzer(2700, 100, 200, 1);
        digitalWrite(LED_YELLOW_PIN, LOW);
        delay(300);
      }
      digitalWrite(LED_RED_PIN, LOW);
      digitalWrite(LED_YELLOW_PIN, HIGH);
      buzzer(2700, 100, 100, 2);
      Serial.println("\nConnecting to wireless network.");
    }
    else if (!Ping.ping(PING_DNS, 2))
    {
      digitalWrite(LED_RED_PIN, HIGH);
      digitalWrite(LED_YELLOW_PIN, HIGH);
      buzzer(2700, 100, 0, 1);
      digitalWrite(LED_YELLOW_PIN, LOW);
      delay(500);
      Serial.println("\nIntenet connection is unavailable!");
      if (WiFi.status() != WL_CONNECTED)
      {
        continue;
      }
    }
    else
    {
      digitalWrite(LED_RED_PIN, LOW);
      digitalWrite(LED_YELLOW_PIN, LOW);
      Serial.println("\nIntenet connection is available!");
      synchronize();
      break;
    }
  }
}

int getSensorValue()
{
  int sensorValue = 0;
  for (int i = 0; i <= 30; i++)
  {
    sensorValue += analogRead(MQ2_PIN);
  }
  return sensorValue / 30;
}

void alertLow()
{
  uint32_t lastDetectionMillis = millis();
  digitalWrite(LED_YELLOW_PIN, LOW);
  while (true)
  {
    digitalWrite(LED_YELLOW_PIN, LOW);
    digitalWrite(LED_RED_PIN, HIGH);
    if (!reported)
    {
      ledcWriteTone(0, 1000);
      saveReport(1);
      ledcWriteTone(0, 0);
      reported = true;
    }
    buzzer(1000, 600, 50, 1);
    digitalWrite(LED_RED_PIN, LOW);
    if (getSensorValue() < 1500 && millis() - lastDetectionMillis >= LOW_DETECTION_DELAY)
    {
      reported = false;
      digitalWrite(LED_YELLOW_PIN, HIGH);
      digitalWrite(LED_RED_PIN, HIGH);
      buzzer(1000, 1000, 50, 1);
      digitalWrite(LED_RED_PIN, LOW);
      buzzer(2700, 100, 50, 3);
      break;
    }
  }
}

void alertHigh()
{
  uint32_t lastDetectionMillis = millis();
  digitalWrite(LED_YELLOW_PIN, LOW);
  while (true)
  {
    digitalWrite(LED_YELLOW_PIN, LOW);
    digitalWrite(LED_RED_PIN, HIGH);
    if (!reported)
    {
      ledcWriteTone(0, 1000);
      saveReport(2);
      ledcWriteTone(0, 0);
      reported = true;
    }
    buzzer(1000, 100, 50, 1);
    digitalWrite(LED_RED_PIN, LOW);
    if (getSensorValue() < 1500 && millis() - lastDetectionMillis >= HIGH_DETECTION_DELAY)
    {
      reported = false;
      digitalWrite(LED_YELLOW_PIN, HIGH);
      digitalWrite(LED_RED_PIN, HIGH);
      buzzer(1000, 1000, 50, 1);
      digitalWrite(LED_RED_PIN, LOW);
      buzzer(2700, 100, 50, 3);
      break;
    }
  }
}

void setup()
{
  // Setup pin mode
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Setup tone
  ledcSetup(0, 14000, 8);
  ledcAttachPin(BUZZER_PIN, 0);

  // Initizlize WiFi
  wifi.setCACert(ca_cert);
  WiFi.mode(WIFI_STA);

  // Finish startup sign
  buzzer(2700, 100, 0, 1);
  buzzer(1650, 100, 0, 1);
  buzzer(2700, 100, 0, 1);
  for (int i = 0; i <= 10; i++)
  {
    digitalWrite(LED_YELLOW_PIN, HIGH);
    digitalWrite(LED_RED_PIN, LOW);
    delay(150);
    digitalWrite(LED_YELLOW_PIN, LOW);
    digitalWrite(LED_RED_PIN, HIGH);
    delay(150);
  }
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);

  // serial
  Serial.begin(9600);
  Serial.println();
}

void loop()
{
  internetConnection();
  int sensorValue = getSensorValue();
  Serial.println("\nSensor value: " + String(sensorValue));
  if (sensorValue >= LOW_DETECTION_SENSITIVITY_START && sensorValue < LOW_DETECTION_SENSITIVITY_END)
    alertLow();
  else if (sensorValue >= HIGH_DETECTION_SENSITIVITY_START)
    alertHigh();
  delay(SCAN_DELAY);
}