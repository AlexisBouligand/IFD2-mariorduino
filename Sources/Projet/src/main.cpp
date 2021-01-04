#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string.h>
#include <math.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 // contant for the OLED screen
#define OLED_RESET     5

// states in wich the lamp can be, idle by default
#define LOADING 0
#define IDLE 1
#define PHONEIN 2

// Different lightmode
#define OFF 0
#define TEMPERATURE 1
#define FIX 2
#define CHANGING 3

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
int lightmode = 2;//lightmode, changes the way the lamp behaves: Breathing, temperature based, custom color and off
float red_ratio = 0.5; //ratio of red when the mode of the light depend of the temperature
unsigned long int ticks_color = 0;
int lummode = 0;//change if the luminiosity is automatic or manual;
int lumvalue = 0;//the luminiosity value if lightmode is set to manual
int speed = 0;//speed of the changing colors

//Variables linked with the sensor of luminosity and temperature
int enlightment = 0;
int temperature = 0;
unsigned long int ticks_sensor = 0;

//Value for the pins, to change according to the shield
int rgbPinRed = 2;
int rgbPinGreen = 3;
int rgbPinBlue = 4;

int buttonPin = 0;
int potoPin = 0;
int photoPin = A0;
int tempPin = A1;
int interrupPin = 7;

//variables for the phone timer
int defaultValue = 20; //nombre de minutes par défaut
int timerMinutes = 20;
int timerSeconds = 0;
unsigned long int timerTicks = 0;
bool isDone = false;

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
      if(inString.indexOf("/LAMP/IN/SPEED>")!=-1){
        speed = inString.substring(15,16).toInt();
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

bool checkphone(){
  if(digitalRead(interrupPin) == HIGH){
    return true;
  }else{
    return false;
  }
}

void phonetimer(){
  if(isDone == false){
    if((millis() - timerTicks) > 1000){
      timerTicks = millis();
      timerSeconds--;
      if(timerSeconds <= -1){
        timerSeconds = 59;
        timerMinutes--;
        if(timerMinutes <= -1){
          isDone = true;
        }
      }
    }

    oledPrint(String(timerMinutes)+":"+String(timerSeconds),"Continuez comme ca!");
  }else{
    oledPrint("Fini!","Félicitation !");
  }
}


String time(){//Update the time and return a String of it
  if ((millis() - ticks) > 1000) {
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

void updateSensors(){
  //we will read the datas of the sensors
  if((millis() - ticks_sensor)>10000){ //We read datas of the sensors every 10 seconds
    ticks_sensor = millis();
    //Check for the temperature sensor
    temperature = map(analogRead(tempPin),0,255,0,100); // 0 to 100 is the new scale (it has no unit)
    Serial.print("The temperature is : ");
    Serial.println(temperature);

    //Check for the light sensor
    enlightment = map(analogRead(photoPin),0,1023,0,100); //0 to 100 is the new scale (it has no unit)
    Serial.print("The enlightment is : ");
    Serial.println(analogRead(photoPin));
  }
}



void light(int lightmode, int* r_indent, int* b_indent, int* g_indent, int speed){
  if(lightmode == OFF){

    //programm if we want to switch off the lights
    analogWrite(rgbPinRed, 0);
    analogWrite(rgbPinBlue, 0);
    analogWrite(rgbPinGreen, 0);
  } else if(lightmode == TEMPERATURE){

    //program if we want to update the led according to the environment
    analogWrite(rgbPinGreen, 127);
    red_ratio = (float) temperature * 0.01;
    *r_indent = round(255*red_ratio);
    *b_indent = 255 - *r_indent;
    analogWrite(rgbPinRed, *r_indent);
    analogWrite(rgbPinBlue, *b_indent);
  } else if(lightmode == FIX){

    //program which set an unique color according to the user request
    analogWrite(rgbPinRed, red);
    analogWrite(rgbPinBlue, blue);
    analogWrite(rgbPinGreen, green);
  } else if(lightmode == CHANGING){

    //program which set different color into a loop a certain speed which is chosen by the user
    if((millis() - ticks_color)>speed){
      ticks_color = millis();

      analogWrite(rgbPinRed, *r_indent);
      analogWrite(rgbPinBlue, *b_indent);
      analogWrite(rgbPinGreen, *g_indent);
      if((*r_indent == *b_indent)&&(*r_indent == *g_indent)){
        if(*r_indent >255){
          *r_indent = 0;
          *b_indent = 0;
          *g_indent = 0;
        } else{
          *r_indent++;
          *b_indent++;
          *g_indent++;
        }
      } else if(*r_indent == *b_indent){
        if(*r_indent >255){
          *r_indent = 0;
          *b_indent = 0;
          *g_indent++;
        } else if(*g_indent >255){
          *r_indent++;
          *b_indent++;
          *g_indent = 0;
        } else{
          *r_indent++;
          *b_indent++;
          *g_indent++;
        }
      } else if(*r_indent == *g_indent){
        if(*r_indent >255){
          *r_indent = 0;
          *b_indent++;
          *g_indent = 0;
        } else if(*b_indent >255){
          *r_indent++;
          *b_indent = 0;
          *g_indent++;
        } else{
          *r_indent++;
          *b_indent++;
          *g_indent++;
        }
      } else if(*g_indent == *b_indent){
        if(*g_indent >255){
          *r_indent++;
          *b_indent = 0;
          *g_indent = 0;
        } else if(*r_indent >255){
          *r_indent = 0;
          *b_indent++;
          *g_indent++;
        } else{
          *r_indent++;
          *b_indent++;
          *g_indent++;
        }
      } else{
        if(*r_indent > 255){
          *r_indent = 0;
          *b_indent++;
          *g_indent++;
        } else if(*b_indent > 255){
          *b_indent = 0;
          *r_indent++;
          *g_indent++;
        } else if(*g_indent >255){
          *g_indent = 0;
          *r_indent++;
          *b_indent++;
        } else{
          *r_indent++;
          *b_indent++;
          *g_indent++;
        }
      }
      
    }
    
  }
}

void loop(){
  timeStr = time();
  time();
  EspEvent();
  updateSensors(); //Create a function to update the values of the sensors, maybe run in every ten seconds
  light(lightmode, &red, &blue, &green, speed); //create a function to handle changing the mode, luminiosity ect ect...
  

  if(state == IDLE){
    oledPrint(timeStr,"Hello World !");
    if(checkphone()){
      state = PHONEIN;
    }
  }

  if(state == PHONEIN){
    phonetimer();
    if(!checkphone()){
      if(isDone == false){
        oledPrint("Oh non","Vous avez échoué");
        delay(1000);
      }
      timerTicks = 0;
      timerMinutes = defaultValue;
      timerSeconds = 0;
      isDone = false;
      state = IDLE;
    }

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