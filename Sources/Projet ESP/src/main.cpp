#include <Arduino.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Constants

WiFiClient wificlient;
PubSubClient client(wificlient);

const char ssid[] = "NUMERICABLE-1790"; //SSID of the wifi
const char pwd[] = "19900EC0E6";  //PASSWORD of the wifi
const char hostname[14] = "192.168.0.20"; //IP adress of the server
const char* topicIn = "/LAMP/IN/#"; //The topics the ESP needs to subscribe to;
const char* clientName = ""; //the name of the client (Preferably not MesC*******SurTonFront)

String inString;
unsigned long lastMillis = 0;

int buffer = 0;

void callback(char* topic, byte* payload, unsigned int length) { // fucntion that is called when a mesage is recieved from the server
  Serial.print(topic);
  Serial.print('>');
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.print('|');
}


void connectWIFI(){// connect to wifi
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("w|");
    delay(100);
  }
  Serial.print("s|");
}


void connectMQTT() {//connect to MQTT;
  Serial.print("m|");
  while (!client.connect(clientName)) {
    Serial.print("m|");
    delay(100);
  }
  client.publish("/LAMP/OUT/ALIVE", "Bonsoir");
  client.subscribe(topicIn);
  Serial.print("s|");
}



void alive(){//Check if the ESSP is still connected to the borker and sends an heartbeat every 3 seceonds
  if (millis() - lastMillis > 3000) {
    lastMillis = millis();

    if(!client.connected()){
      connectMQTT();
    }
  
    if(WiFi.status() != WL_CONNECTED){
      connectWIFI();
    }
    client.publish("/LAMP/OUT/ALIVE", "Bonsoir");
    client.loop();
  }
}


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pwd);
  client.setKeepAlive(30);
  client.setServer(hostname, 1883);
  client.setCallback(callback);
  connectWIFI();
  connectMQTT();
}

void MegaEvent(){
  while (Serial.available()) {    
    char inChar = Serial.read();
    inString += inChar;
    if(inChar == '|'){ 
      if(inString.indexOf("PHONE")!=-1){//Esp is connecting to wifi
        client.publish("/LAMP/OUT/PHONE","1")
      }
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