#include<Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 // contant for the OLED screen
#define OLED_RESET     5

// states in wich the lamp can be, idle by default
#define LOADING 0
#define IDLE 1
#define PHONEIN 2
int state = IDLE;

String inString; //Input string used to recieve commands from the ESP via serial3

Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);//creation of OLED screen instance

String timeStr;
int hour = 0;
int minutes = 0; //time values
int seconds = 0;
unsigned long int ticks = 0;

int red = 255;
int green = 255; // color values for the LED lights
int blue = 255;
int lightmode = 1;//lightmode, changes the way the lamp behaves: Breathing, temperature based, custom color and off
int lummode = 0;//change if the luminiosity is automatic or manual;
int lumvalue = 0;//the luminiosity value if lightmode is set to manual

//Value for the pins, to change according to the shield
int rgbPin1Red = 0;
int rgbPin1Green = 0;
int rgbPin1Blue = 0;
int rgbPin2Red = 0;
int rgbPin2Green = 0;
int rgbPin2Blue = 0;
int buttonPin = 0;
int potoPin = 0;
int photoPin = 0;
int tempPin = 0;
int interrupPin = 0;


void oledSetup(){//setup the oled screen, no arguments, no return values
  if(!OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  OLED.clearDisplay();
  OLED.setTextSize(2);                      // choix de la taille des caractères
  OLED.setTextWrap(true);                  // disable text wrap
  OLED.setTextColor(WHITE, BLACK);          // set text color to white and black background
  OLED.setCursor(0,0);
  OLED.display();
}


void oledPrint(String window, String text){//function to print text on the screen, takes to char* for the content, returns nothing.
  OLED.clearDisplay();
  OLED.setCursor(0,0);
  OLED.print(window);
  OLED.setCursor(0,20);
  OLED.print(text);
  OLED.display();
}


void oledPrintD(int x,int y, String text){//same as before, but uses a String type instead and allow to print at any wanted coordinates
  OLED.setCursor(x,y);
  OLED.print(text);
  OLED.display();
}


void setup() {
  oledSetup();
  Serial.begin(115200);//Start connection with the computer (for debugging purposes)
  Serial3.begin(115200);//Start connection with the ESP
}


void EspEvent(){//Fucntion used to handle messages recived from the ESP
  while (Serial3.available()) {    
    char inChar = Serial3.read();
    inString += inChar;
    if(inChar == '|'){//Read the string until the end of command char, here a "|"
      Serial.println(inString);

      if(inString.indexOf("w|")!=-1){//Esp is connecting to wifi
        state = LOADING;
        oledPrint("Loading...","Connecting to WiFi");
      }

      if(inString.indexOf("m|")!=-1){//Esp is connecting to MQTT
        state = LOADING;
        oledPrint("Loading...","Waiting   for MQTT");
      }

      if(inString.indexOf("s|")!=-1){//Esp succesfully connected to the network.
        oledPrint("Done :)","Succes");
        delay(300);
        state = IDLE;
      }

      if(inString.indexOf("/LAMP/IN/HOUR>")!=-1){//Time was updated on the MQTT client
        hour = inString.substring(14,16).toInt();
        minutes = 0;
        seconds = 0;
      }
      if(inString.indexOf("/LAMP/IN/MINUTES>")!=-1){
        minutes = inString.substring(17,19).toInt();
        seconds = 0;
      }
      if(inString.indexOf("/LAMP/IN/SECONDS>")!=-1){
        seconds = inString.substring(17,19).toInt();
      }

      if(inString.indexOf("/LAMP/IN/RED>")!=-1){//Color was updated on the MQTT client
        red = inString.substring(13,16).toInt();
      }
      if(inString.indexOf("/LAMP/IN/GREEN>")!=-1){
        green = inString.substring(15,18).toInt();
      }
      if(inString.indexOf("/LAMP/IN/BLUE>")!=-1){
        blue = inString.substring(14,17).toInt();
      }

      if(inString.indexOf("/LAMP/IN/LIGHTMODE>")!=-1){//lightmode was updated on the MQTT server
        lightmode = inString.substring(19,20).toInt();
      }
      if(inString.indexOf("/LAMP/IN/LUMMODE>")!=-1){//lummode was updated on the MQTT server
        lummode = inString.substring(17,18).toInt();
      }
      if(inString.indexOf("/LAMP/IN/LUMVALUE>")!=-1){//lumvalue was updated on the MQTT server
        lumvalue = inString.substring(18,19).toInt();
      }

      inString = "";//clear the string for next command
    }
  }
}


void time(){//calculate time


String time(){//Update the time and return a String of it
  if (millis() - ticks> 1000) {//update time
    ticks = millis();
    seconds++;
    if (seconds == 60){
      seconds = 0;
      minutes++;
      if (minutes == 60){
        minutes = 0;
        hour++;
        if (hour == 24){
          hour = 0;
        }
      }
    }
  }
  String str1 = String(hour);//print time
  String str2 = String(minutes);
  String str3 = String(seconds);
  String str = str1 +":"+ str2 +":"+ str3;
  return str;
}


void loop(){
  
  timeStr = time();
  time();
  EspEvent();
  //updateSensors() Create a function to update the values of the sensors, maybe run in every ten seconds
  //light(); creat a function to handle changing the mode, luminiosity ect ect...
  //checkphone(); check is a phone was inserted and handle the situation if it was

  if(state == IDLE){
    oledPrint(timeStr,"Hello World !");
  }

  if(state == PHONEIN){
    oledPrint(timeStr,"Phone in");
    //phonetimer(); handle the timer for the phone
  }


}


// MQTT topics:
// /LAMP/IN/
//   HOURS
//   MINUTES
//   SECONDS
//   RED
//   GREEN
//   BLUE
//   LIGHTMODE
//   LUMMODE
//   LUMVALUE
// /LAMP/OUT/
//   TIMER
//   PHONE
//   ALIVE