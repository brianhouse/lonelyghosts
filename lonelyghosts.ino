#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

ADC_MODE(ADC_VCC);

const int id = ESP.getChipId();

// wifi
WiFiUDP Udp;
const char* ssid     = "3V8VC";
const char* password = "7YYGM8V3R65V52FJ";
const char* host     = "192.168.1.9";
const int port       = 23232; // both send and receive

// constants
const float INCREMENT = 0.01;
const float BUMP = 0.05;
const float COIT = 0.10;
const float INSIST = (10 * 1000) / 10; // seconds * millis / loop

// state
float phase = 0.0;
float capacitor = 0.0;
int lit = 0;
int insist = 0;
bool holding = false;
int buttonValue = 1;
int prevButtonValue = 1;

const float Pi = 3.141593;


void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(0, OUTPUT);  // set as output to use red LED (LOW is on, HIGH is off)
  pinMode(2, OUTPUT);  // set as output to use blue LED (LOW is on, HIGH is off)
  pinMode(12, OUTPUT); // piezo, using with tone
  pinMode(13, INPUT_PULLUP);  // detecting manual click, use PULLUP to substitute for external resistor
  connectToWifi();
  Udp.begin(port);
}

void loop() {

  // blink and click
  if (!holding) {
    if (lit == 15) {
      digitalWrite(2, LOW);  // on
//      tone(12, 5000, 1);

//      tone(12, 5000, 250);
//      tone(12, 3337, 250);
//      tone(12, 2500, 250);     
      lit--;
    } else if (lit == 14) {              
      tone(12, 5000, 1);    
      lit--;
    } else if (lit == 11) {      
      tone(12, 4900, 1);  
      lit--;      
    } else if (lit == 8) {        
      tone(12, 5000, 1);  
      lit--;
    } else if (lit == 5) {
      tone(12, 4900, 1);    
 
      lit--;
    } else if (lit == 2) {
      tone(12, 4900, 1);    
      lit--;
    } else if (lit > 0) {
      lit--;
    } else {
      digitalWrite(2, HIGH);  // off
    }
  }
  
  // handle the button
  buttonValue = digitalRead(13);
  if (buttonValue == 0 && prevButtonValue == 1) {
    Serial.println("--> mousedown");
    phase = 1.0;  
    insist = INSIST;    
    holding = true;
    increment();
    digitalWrite(2, LOW);  // on
    tone(12, 5000, 1);    
  } else if (buttonValue == 1 && prevButtonValue == 0) {
    Serial.println("--> mouseup");
    digitalWrite(2, HIGH);  // off    
    tone(12, 4900, 1);    
    holding = false;
    lit = 0;
  } else {
    if (insist > 0) {
      insist--;    
    }
  }
  prevButtonValue = buttonValue;

  // update the algorithm
  if (!holding) {
    increment();
  }

  // handle network
  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();   
  }
  int packetSize = Udp.parsePacket();  
  if (packetSize) {
    char packetBuffer[packetSize];
    Udp.read(packetBuffer, packetSize);
    if (String(packetBuffer).substring(0, packetSize) == "bump") {  // dont know why substring is necessary
      bump();
    }    
  }
  
  delay(10);
  
}

void increment() {
  phase = _min(phase + INCREMENT, 1.0);
  capacitor = f(phase);
  if (phase == 1.0) {
    Serial.println("--> fire");
    lit = 15;
    phase = 0.0;
    Udp.beginPacket(host, port);
    String dataString = id + "," + String(WiFi.RSSI()) + "," + "fire";
    char dataBuf[dataString.length()+1];
    dataString.toCharArray(dataBuf, dataString.length()+1);
    Udp.write(dataBuf);
    Udp.endPacket();    
  }
}

void bump() {
  if (insist != 0) {
    Serial.println("--> insisting");
    return;
  }
  if (phase <= COIT) {    
    Serial.println("--> resting");
    return;
  }
  Serial.println("--> bump");
  capacitor = _min(capacitor + BUMP, 1.0);
  phase = f_inv(capacitor);
}

float f(float x) {
  return sin((Pi / 2) * x);
}

float f_inv(float y) {
  return (2/Pi) * asin(y);
}

void connectToWifi() {
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (i % 10 == 0) {
      Serial.println();
      Serial.print("--> attempting to connect to: ");
      Serial.println(ssid);      
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
    digitalWrite(0, HIGH);
    delay(250);
    digitalWrite(0, LOW);
    delay(250);      
    Serial.print(".");
    i++;
  }
  Serial.println();
  Serial.println("--> connected to wifi");
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.println(rssi);
}

// maybe need a rest period after firing when cant be bumped

// https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/pinouts

