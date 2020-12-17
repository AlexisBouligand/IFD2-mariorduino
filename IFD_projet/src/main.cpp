#include <Arduino.h>

int pinRED = A2;
int pinGREEN = A3;
int pinBLUE = A4;

void setup() {
  Serial.begin(115200);
  analogWrite(pinRED,255);
  analogWrite(pinGREEN,255);
  analogWrite(pinBLUE,255);
}


int photoRes = A0;
int thermistance = A1;

int red = 0;
int green = 0;
int blue = 0;

int button = 0;
int interruptor = 7;

String inString;
char instring;

unsigned long timer = 0;

void loop(){
  delay(1000);
  if((millis()-timer)>=1000){
    timer = millis();
    Serial.println("Valeur thermistance:");
    Serial.println(String(analogRead(thermistance)));
    Serial.println("Valeur photoresistance:");
    Serial.println(String(analogRead(photoRes)));
  }
  /*if(digitalRead(button)){
    Serial.println("Boutton préssé");
  }*/
  if(digitalRead(interruptor)){
    Serial.println("Interrupteur préssé");
  }
  while(Serial.available()){
    char inChar = Serial3.read();
    inString += inChar;
    if(inChar == '/'){

      if(inString.indexOf("R")!=-1){
        red = inString.substring(2,5).toInt();
        analogWrite(pinRED,red);
      }

      if(inString.indexOf("G")!=-1){
        green = inString.substring(2,5).toInt();
        analogWrite(pinGREEN,green);
      }

      if(inString.indexOf("B")!=-1){
        blue = inString.substring(2,5).toInt();
        analogWrite(pinBLUE,blue);
      }


      inString = "";
    }
  }

}