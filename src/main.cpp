#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ESP32Ping.h>

#define mq2_pin 34
#define ledYellow_pin 32
#define ledRed_pin 25
#define buzzer_pin 27
#define SYNC_DELAY 60000

WiFiClientSecure wifi;
HTTPClient http;
uint32_t lastSyncMillis;
bool bootSync;

const char *ca_cert =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEFTCCAv2gAwIBAgIUXZHljUhf107B3AkAH9is4HxbfLswDQYJKoZIhvcNAQEL\n"
    "BQAwgagxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpDYWxpZm9ybmlhMRYwFAYDVQQH\n"
    "Ew1TYW4gRnJhbmNpc2NvMRkwFwYDVQQKExBDbG91ZGZsYXJlLCBJbmMuMRswGQYD\n"
    "VQQLExJ3d3cuY2xvdWRmbGFyZS5jb20xNDAyBgNVBAMTK01hbmFnZWQgQ0EgNDlk\n"
    "N2Q2OTU1MGYwY2MzMDczMmExYjhmODM5MjJmY2YwHhcNMjIwNDA3MDUzMDAwWhcN\n"
    "MzIwNDA0MDUzMDAwWjAiMQswCQYDVQQGEwJVUzETMBEGA1UEAxMKQ2xvdWRmbGFy\n"
    "ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL8NMJ9Y3T5osO0rNeB9\n"
    "fNakbq0dlku3pTf/ggPo7fNb4xEaBACQRAgfmUg5ENN7R0z9vp9JABj0OU8FnpdP\n"
    "pPqzrjVag1A2BPSnhEMJL0TEkOemkkZr7efHTbjSuaoRg5jh8u/sDN86N7eFGMJp\n"
    "5cZKP4fw6BkSSpXh80YmQwvFzlE0KyolKAaK4bacCioi8Bpu+n0IUY6VkIcGCW/I\n"
    "ah+ePe6WIGIY51mv4MCqtVuW0duuU22G5XsBlfxcSTARm9zN3B1oFvI2ysRWMrfs\n"
    "FN1jPg2Ww0EcfT1euR+ByJYhSTHrIap7ms6T7rxBX4d1lx5Y+FtFqvV7aWFNVVg4\n"
    "AIMCAwEAAaOBuzCBuDATBgNVHSUEDDAKBggrBgEFBQcDAjAMBgNVHRMBAf8EAjAA\n"
    "MB0GA1UdDgQWBBQIVOb0Le3Q19a9VZrTKZdg6KQmIzAfBgNVHSMEGDAWgBReGfcS\n"
    "aEZyatcDb80HqNmnO5ai3TBTBgNVHR8ETDBKMEigRqBEhkJodHRwOi8vY3JsLmNs\n"
    "b3VkZmxhcmUuY29tLzc0MjQ2YzE0LTA2MWQtNDBhYi05MDZiLTUzZjliNGE2YWQ4\n"
    "NS5jcmwwDQYJKoZIhvcNAQELBQADggEBAI+ZSGmhzG54H+hZTr8NvtgrKrJVIIfb\n"
    "KGR/Qh3htyo/uCbfe9Y0UKrD0cB6YJSaqLh3FR4jynlB0RGmhS2oEg0E36rj6VRB\n"
    "A1hLac5Wxsbs5CoIgLkoqTmr6PO8Gp3V3L+HHRFsvMNzQxxZ+rOUUWEjZkYySUUX\n"
    "8DNaIaL50RP+qCN1zPZGFXY+6/PdbWOWR8DDmrTmb7eAMKlXS8PZP3bl0ldj4uB4\n"
    "E5TzpPkX9ujQj/lYxQ7Kb4pZgsDIMhMbbFqRa09S6dCwc+AU7CHDjLQo3M+NEBkF\n"
    "XUeXH3ST25knOZrjFeGQaO002hRHGYxhZemK7ZK8ABYOcMfPg6agv1I=\n"
    "-----END CERTIFICATE-----\n";

void saveReport(int level)
{
  if (WiFi.status() != WL_CONNECTED)
    return;
  http.begin("https://api-monoxide.ezralazuardy.com/v1/device/report");
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32");
  http.addHeader("Api-Key", "04d5d2f1-0caa-49dd-8293-a3c2466bed78");
  http.addHeader("Device-Key", "6e0dcdf1-95a5-4981-96ed-9d2291d7db16");
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
  http.begin("https://api-monoxide.ezralazuardy.com/v1/device/sync");
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32");
  http.addHeader("Api-Key", "04d5d2f1-0caa-49dd-8293-a3c2466bed78");
  http.addHeader("Device-Key", "6e0dcdf1-95a5-4981-96ed-9d2291d7db16");
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
      const char *ssid = "Jentayu-VTOL";
      const char *passwd = "vtoljuara";
      WiFi.begin(ssid, passwd);
      while (WiFi.status() != WL_CONNECTED)
      {
        digitalWrite(ledRed_pin, HIGH);
        digitalWrite(ledYellow_pin, HIGH);
        buzzer(2700, 100, 200, 1);
        digitalWrite(ledYellow_pin, LOW);
        delay(300);
      }
      digitalWrite(ledRed_pin, LOW);
      digitalWrite(ledYellow_pin, HIGH);
      buzzer(2700, 100, 100, 2);
      Serial.println("\nConnecting to wireless network.");
    }
    else if (!Ping.ping("8.8.8.8", 2))
    {
      digitalWrite(ledRed_pin, HIGH);
      digitalWrite(ledYellow_pin, HIGH);
      buzzer(2700, 100, 0, 1);
      digitalWrite(ledYellow_pin, LOW);
      delay(500);
      Serial.println("\nIntenet connection is unavailable!");
      if (WiFi.status() != WL_CONNECTED)
      {
        continue;
      }
    }
    else
    {
      digitalWrite(ledRed_pin, LOW);
      digitalWrite(ledYellow_pin, HIGH);
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
    sensorValue += analogRead(mq2_pin);
  }
  return sensorValue / 30;
}

void alertLow()
{
  saveReport(1);
  digitalWrite(ledYellow_pin, LOW);
  while (true)
  {
    digitalWrite(ledYellow_pin, LOW);
    digitalWrite(ledRed_pin, HIGH);
    buzzer(1000, 100, 50, 1);
    digitalWrite(ledRed_pin, LOW);
    delay(50);
    if (getSensorValue() <= 1500)
    {
      digitalWrite(ledYellow_pin, HIGH);
      digitalWrite(ledRed_pin, HIGH);
      buzzer(1000, 1000, 50, 1);
      digitalWrite(ledRed_pin, LOW);
      buzzer(2700, 100, 50, 3);
      break;
    }
  }
}

void alertMedium()
{
  saveReport(2);
  digitalWrite(ledYellow_pin, LOW);
  while (true)
  {
    digitalWrite(ledYellow_pin, LOW);
    digitalWrite(ledRed_pin, HIGH);
    buzzer(1000, 100, 50, 1);
    digitalWrite(ledRed_pin, LOW);
    delay(50);
    if (getSensorValue() <= 1500)
    {
      digitalWrite(ledYellow_pin, HIGH);
      digitalWrite(ledRed_pin, HIGH);
      buzzer(1000, 1000, 50, 1);
      digitalWrite(ledRed_pin, LOW);
      buzzer(2700, 100, 50, 3);
      break;
    }
  }
}

void alertHigh()
{
  saveReport(3);
  digitalWrite(ledYellow_pin, LOW);
  while (true)
  {
    digitalWrite(ledYellow_pin, LOW);
    digitalWrite(ledRed_pin, HIGH);
    buzzer(1000, 100, 50, 1);
    digitalWrite(ledRed_pin, LOW);
    delay(50);
    if (getSensorValue() <= 1500)
    {
      digitalWrite(ledYellow_pin, HIGH);
      digitalWrite(ledRed_pin, HIGH);
      buzzer(1000, 1000, 50, 1);
      digitalWrite(ledRed_pin, LOW);
      buzzer(2700, 100, 50, 3);
      break;
    }
  }
}

void setup()
{
  // Setup pin mode
  pinMode(ledYellow_pin, OUTPUT);
  pinMode(ledRed_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);

  // Setup tone
  ledcSetup(0, 14000, 8);
  ledcAttachPin(buzzer_pin, 0);

  // Initizlize WiFi
  wifi.setCACert(ca_cert);
  WiFi.mode(WIFI_STA);

  // Finish startup sign
  buzzer(2700, 100, 0, 1);
  buzzer(1650, 100, 0, 1);
  buzzer(2700, 100, 0, 1);
  for (int i = 0; i <= 10; i++)
  {
    digitalWrite(ledYellow_pin, HIGH);
    digitalWrite(ledRed_pin, LOW);
    delay(150);
    digitalWrite(ledYellow_pin, LOW);
    digitalWrite(ledRed_pin, HIGH);
    delay(150);
  }
  digitalWrite(ledYellow_pin, LOW);
  digitalWrite(ledRed_pin, LOW);

  // serial
  Serial.begin(9600);
  Serial.println();
}

void loop()
{
  internetConnection();
  int sensorValue = getSensorValue();
  if (sensorValue >= 1500 && sensorValue < 2500)
    alertLow();
  else if (sensorValue >= 2500 && sensorValue < 3500)
    alertMedium();
  else if (sensorValue >= 3500)
    alertHigh();
}