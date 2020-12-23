#include <Arduino.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Constants

WiFiClient wificlient;
PubSubClient client(wificlient);
const char ssid[] = "NUMERICABLE-1790";
const char pwd[] = "19900EC0E6";
const char hostname[14] = "192.168.0.20";
const char* topicIn = "/LAMP/IN/#";

String inString;

//void messageReceived(String &topic, String &payload) {
//  String str = topic + ">" + payload + "|" ;
//  Serial.print(str);
//}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(topic);
  Serial.print('>');
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.print('|');
}

void connectWIFI(){
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("w|");
    delay(100);
  }
  Serial.print("s|");
}



void connectMQTT() {
  
  Serial.print("m|");
  while (!client.connect("MesCouillesSurTonFront")) {
    Serial.print("m|");
    delay(100);
  }
  client.publish("/LAMP/OUT/ALIVE", "Bonsoir");
  client.subscribe(topicIn);
  Serial.print("s|");
}

unsigned long lastMillis = 0;
void alive(){
  if (millis() - lastMillis > 3000) {
    lastMillis = millis();

    if(!client.connected()){
      connectMQTT();
    }
  
    if(WiFi.status() != WL_CONNECTED){
      connectWIFI();
    }
    client.publish("/LAMP/OUT/ALIVE", "Bonsoir");
    client.flush();
    client.loop();
  }
}

void

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pwd);
  client.setKeepAlive(120);
  client.setServer(hostname, 1883);
  client.setCallback(callback);
  connectWIFI();
  connectMQTT();
}

void EspEvent(){
  while (Serial3.available()) {    
    char inChar = Serial3.read();
    inString += inChar;
    if(inChar == '|'){
      Serial.println(inString);

      if(inString.indexOf("p|")!=-1){
        client.publish("/LAMP/OUT/PHONE", "1");
      }

    

      inString = "";
    }
  }
}

void loop() {
  if(!client.connected()){
    connectMQTT();
  }
  
  if(WiFi.status() != WL_CONNECTED){
    connectWIFI();
  }
  alive();
}