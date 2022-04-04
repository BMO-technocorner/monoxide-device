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


void internetConnection(){
  while (true){
    if(WiFi.status()!=WL_CONNECTED){
      const char* ssid="Jentayu-VTOL";
      const char* passwd="vtoljuara";
      WiFi.begin(ssid,passwd);
      while (WiFi.status()!=WL_CONNECTED){
        digitalWrite(ledRed_pin,HIGH);
        digitalWrite(ledYellow_pin,HIGH);
        buzzer(2700,100,200,1);
        digitalWrite(ledYellow_pin,LOW);
        delay(300);
      }
      digitalWrite(ledRed_pin,LOW);
      digitalWrite(ledYellow_pin,HIGH);
      buzzer(2700,100,100,2);
    }
    else if (!Ping.ping("8.8.8.8",2)){
      digitalWrite(ledRed_pin,HIGH);
      digitalWrite(ledYellow_pin,HIGH);
      buzzer(2700,100,0,1);
      digitalWrite(ledYellow_pin,LOW);
      delay(500);
      if(WiFi.status()!=WL_CONNECTED){
        continue;
      }
    }
    else{
      digitalWrite(ledRed_pin,LOW);
      digitalWrite(ledYellow_pin,HIGH);
      break;
    }

  }

}

void alertOn(){
  digitalWrite(ledYellow_pin,LOW);
  digitalWrite(ledRed_pin,HIGH);
  buzzer(1000,100,50,1);
  digitalWrite(ledRed_pin,LOW);
  delay(50);
}

void alertOff(){
  digitalWrite(ledYellow_pin,HIGH);
  digitalWrite(ledRed_pin,HIGH);
  buzzer(1000,1000,50,1);
  digitalWrite(ledRed_pin,LOW);
  buzzer(2700,100,50,3);
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
    buzzer(2700,100,0,1);
    buzzer(1650,100,0,1);
    buzzer(2700,100,0,1);
    for (int i=0; i<=10; i++){
      digitalWrite(ledYellow_pin,HIGH);
      digitalWrite(ledRed_pin,LOW);
      delay(150);
      digitalWrite(ledYellow_pin,LOW);
      digitalWrite(ledRed_pin,HIGH);
      delay(150);
    }
    digitalWrite(ledYellow_pin,LOW);
    digitalWrite(ledRed_pin,LOW);

}




void loop() {
  internetConnection();
  for(int i=1;i<=20;i++){
    alertOn();
  }
  alertOff();
  delay(5000);
  
}