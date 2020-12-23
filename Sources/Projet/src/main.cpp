#include<Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET     5

#define LOADING 0
#define IDLE 1
#define PHONEIN 2

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

int state = IDLE;
String inString; //Input string

int hour = 0;
int minutes = 0; //time values
int seconds = 0;
unsigned long int ticks = 0;

int red = 255;
int green = 255;
int blue = 255;

int analogPin = A2 ; //analog sensor
int tension = 0 ; //variable to store the value read
unsigned long int lux = 0 ;


Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void oledSetup(){
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

void oledPrint(const char* window, const char* text){
  OLED.clearDisplay();
  OLED.setCursor(0,0);
  OLED.print(window);
  OLED.setCursor(0,20);
  OLED.print(text);
  OLED.display();
}

void oledPrintD(int x,int y, String text){
  OLED.setCursor(x,y);
  OLED.print(text);
  OLED.display();
}

void setup() {
  oledSetup();
  Serial.begin(115200);
  Serial3.begin(115200);
}


void EspEvent(){
  while (Serial3.available()) {    
    char inChar = Serial3.read();
    inString += inChar;
    if(inChar == '|'){
      Serial.println(inString);

      if(inString.indexOf("w|")!=-1){
        state = LOADING;
        oledPrint("Loading...","Connecting to WiFi");
      }

      if(inString.indexOf("m|")!=-1){
        state = LOADING;
        oledPrint("Loading...","Waiting   for MQTT");
      }

      if(inString.indexOf("s|")!=-1){
        oledPrint("Done :)","Succes");
        delay(300);
        state = IDLE;
      }
      if(inString.indexOf("fs|")!=-1){
        oledPrint("Echec","Alive a foiré");
        delay(500);
      }


      if(inString.indexOf("/LAMP/IN/HOUR>")!=-1){
        hour = inString.substring(14,16).toInt();
        minutes = 0;
        seconds = 0;
      }

      if(inString.indexOf("/LAMP/DEBUG>")!=-1){
        Serial3.println("p|");
        if(state == PHONEIN){
          state = IDLE;
        }else{
          state == PHONEIN;
        }
      }

      if(inString.indexOf("/LAMP/IN/MINUTES>")!=-1){
        minutes = inString.substring(17,19).toInt();
        seconds = 0;
      }
      if(inString.indexOf("/LAMP/IN/SECONDS>")!=-1){
        seconds = inString.substring(17,19).toInt();
      }
      if(inString.indexOf("/LAMP/IN/RED>")!=-1){
        red = inString.substring(13,16).toInt();
      }
      if(inString.indexOf("/LAMP/IN/GREEN>")!=-1){
        green = inString.substring(15,18).toInt();
      }
      if(inString.indexOf("/LAMP/IN/BLUE>")!=-1){
        blue = inString.substring(14,17).toInt();
      }






      inString = "";
    }
  }
}



void time(){
  if (millis() - ticks> 1000) {
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
}

String printTime(){
  String str1 = String(hour);
  String str2 = String(minutes);
  String str3 = String(seconds);
  String str = str1 +":"+ str2 +":"+ str3;
  return str;
}


void loop(){
  String hour;

  time();
  EspEvent();
  //checkForPhone();
  if(state == IDLE){
    hour = printTime();
    int str_len = hour.length() + 1;  
    char char_array[str_len];
    hour.toCharArray(char_array, str_len);
    String str = String(red)+"/"+String(green)+"/"+String(blue);
    oledPrint(char_array,"");
    oledPrintD(0,20,str);
  }
  if(state == PHONEIN){
    hour = printTime();
    int str_len = hour.length() + 1;  
    char char_array[str_len];
    hour.toCharArray(char_array, str_len);
    oledPrint(char_array,"Phone in");
  }
  tension = analogRead(analogPin) ; // read the input pin
  lux = map(tension,0,10,0,10000); //map tension to lux
  Serial.println(lux);
}
