#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#define ledYellow_pin 18
#define ledRed_pin 19
#define buzzer_pin 2



void buzzer(int frequency, int duration, int pause, int times){
  for(int time=1; time<=times; time++){
    ledcWriteTone(0,frequency);
    delay(duration);
    ledcWriteTone(0,0);
    delay(pause);
  }
}


void wifiConnect(){
  const char* ssid="Jentayu-VTOL";
  const char* passwd="vtoljuara";
  WiFi.begin(ssid,passwd);
  while (WiFi.status()!=WL_CONNECTED){
    digitalWrite(ledYellow_pin,HIGH);
    delay(300);
    digitalWrite(ledYellow_pin,LOW);
    delay(300);
  }
  digitalWrite(ledYellow_pin,HIGH);
  buzzer(2700,100,100,2);
}



void checkInternetConnection(){
  while (!Ping.ping("google.com",2)){
    digitalWrite(ledYellow_pin,HIGH);
    buzzer(2700,100,0,1);
    digitalWrite(ledYellow_pin,LOW);
    delay(500);
  }
  digitalWrite(ledYellow_pin,HIGH);
}

void setup() {
  // Setup pin mode
  pinMode(ledYellow_pin,OUTPUT);
  pinMode(ledRed_pin,OUTPUT);
  pinMode(buzzer_pin,OUTPUT);
  
  //Setup tone
  ledcSetup(0,14000,8);
  ledcAttachPin(buzzer_pin,0);
  
  // Initizlize WiFi
  WiFi.mode(WIFI_STA);


  //Finish startup sign
    for (int i=0; i<=15; i++){
      digitalWrite(ledYellow_pin,HIGH);
      digitalWrite(ledRed_pin,LOW);
      delay(150);
      digitalWrite(ledYellow_pin,LOW);
      digitalWrite(ledRed_pin,HIGH);
      delay(150);
    }
    digitalWrite(ledYellow_pin,LOW);
    digitalWrite(ledRed_pin,LOW);
    buzzer(2700,100,0,1);
    buzzer(1650,100,0,1);
    buzzer(2700,100,0,1);
    delay(500);
}




void loop() {
  if(WiFi.status()!=WL_CONNECTED){
    wifiConnect();
  }
  checkInternetConnection();
    
  
}