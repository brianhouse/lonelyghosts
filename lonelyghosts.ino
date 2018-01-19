#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>

const String id = String(ESP.getChipId());

// wifi client
WiFiUDP Udp;
const char* ssid     = "GL-MT300N-5cb";
const char* password = "goodlife";
const char* host     = "192.168.8.155";
const int port       = 23232; // both send and receive

// wifi server
ESP8266WebServer server(80);

// device constants
const float Pi = 3.141593;
const int LED = 14; // 14 for external, 0 for red, 2 for blue
const int PITCH = round((pow(((ESP.getChipId() % 6000) / 6000.0), 3.0) * 6000.0) + 2000.0);
const float PHASE = ((ESP.getChipId() % 6000) / 6000.0) * 0.95;

// behavior constants
const float INCREMENT = 0.01;
const float COIT = 0.10;
const int RESIST = 5 * 1000;

// settable behavior constants
float SENSITIVITY = 0.01;
int RANGE = -40;

// state
float phase = 0.0;
float capacitor = 0.0;
int start_t = 0;
int resist_t = 0;
int lit = 0;
String neighbors = "";



void setup() {
  
  Serial.begin(115200);  
  delay(10);
  Serial.println();
  pinMode(0, OUTPUT);  // set as output to use red LED (LOW is on, HIGH is off)
  digitalWrite(0, LOW);
  pinMode(LED, OUTPUT);  // set as output to use external LED (HIGH is on, LOW is off)
  digitalWrite(LED, LOW);
  pinMode(12, OUTPUT); // piezo, using with tone
  pinMode(13, INPUT_PULLUP);  // detecting motion, use PULLUP to substitute for external resistor

  Serial.print("ID: ");  
  Serial.println(id);
  Serial.print("PITCH: ");  
  Serial.println(PITCH);
  Serial.print("PHASE: ");  
  Serial.println(PHASE);
  Serial.println();

  // server
  Serial.println("Activating AP...");
  String ap_name = "flolg_" + id;
  char ap_char[ap_name.length() + 1];
  ap_name.toCharArray(ap_char, ap_name.length() + 1);
  boolean result = WiFi.softAP(ap_char, "0123456789");
  if (result == true) {
    Serial.println("--> server active");  
  } else {
    Serial.println("--> server failed");
  }  
  
  // client
  connectToWifi();
  Udp.begin(port);
  scan();
  start_t = millis();

}

void loop() {

  // are we tilting?
  boolean tilt = !digitalRead(13);
  if (tilt) {
    Serial.println("\nTilting!");
    send(id + "," + String(WiFi.RSSI()) + ",tilt");
    scan();
  }

  int elapsed = millis() - start_t;
  if (elapsed < 10) {  
    return;
  }
  
  // blink and click
  if (lit == 15) {
    digitalWrite(LED, HIGH);  // on
    tone(12, PITCH*2, 20);  
    lit--;   
  } else if (lit == 13) {
    tone(12, PITCH, 130);  
    lit--;
  } else if (lit > 0) {
    lit--;
  } else {
    digitalWrite(LED, LOW);  // off
  }

  // make sure we're connected
  connectToWifi();   
  
  // update the algorithm
  increment();

  // receive
  int packetSize = Udp.parsePacket();  
  if (packetSize) {
    char packetBuffer[packetSize];
    Udp.read(packetBuffer, packetSize);
    String action = String(packetBuffer).substring(0, 4);
    if (action == "bump") {
      if (millis() > resist_t + RESIST) {   // are we resisting?
        bump();
      }
    }
    else if (action == "disr") {
//      phase = PHASE;
      Serial.print("DISR! ");
      Serial.println(PHASE);
      scan();
    }
    else if (action == "rang") {
      int r = String(packetBuffer).substring(4).toInt();
      RANGE = -1 * r;
      Serial.print("Set range to ");
      Serial.println(RANGE);
    }
    else if (action == "sens") {
      float r = String(packetBuffer).substring(4).toFloat();
      SENSITIVITY = r;
      Serial.print("Set sensitivity to ");
      Serial.println(SENSITIVITY);
    }
  }
  
  start_t = millis();
  
}

void increment() {
  phase = _min(phase + INCREMENT, 1.0);
  capacitor = f(phase);
  if (phase == 1.0) {
//    Serial.println("--> fire");
    lit = 15;
    phase = 0.0;
    send(id + "," + String(WiFi.RSSI()) + ",fire");
  }
}

void bump() {
  if (phase <= COIT) {    
//    Serial.println("--> resting");
    return;
  }
//  Serial.println("--> bump");
  capacitor = _min(capacitor + SENSITIVITY, 1.0);
  phase = f_inv(capacitor);
}

float f(float x) {
  return sin((Pi / 2) * x);
}

float f_inv(float y) {
  return (2/Pi) * asin(y);
}


void connectToWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      if (i % 10 == 0) {
        Serial.println();
        Serial.print("Reconnecting to wifi...");
        WiFi.disconnect();
        delay(250);
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
    digitalWrite(0, HIGH);
  }
}

void scan() {
  digitalWrite(LED, HIGH);
  Serial.println();
  Serial.println("Scanning...");
  neighbors = "";
  int n = WiFi.scanNetworks();
  for (int i=0; i<n; i++) {
    if (WiFi.SSID(i).substring(0, 6) == "flolg_" and WiFi.RSSI(i) >= RANGE) {
      Serial.println(String(WiFi.SSID(i)) + ":" + String(WiFi.RSSI(i)));
      neighbors += WiFi.SSID(i).substring(6) + ":" + String(WiFi.RSSI(i)) + ";";
    }
  }
  Serial.print("--> neighbors: ");
  Serial.print(neighbors);
  Serial.println();
  send(id + "," + String(WiFi.RSSI()) + ",scan," + neighbors);
  digitalWrite(LED, LOW);
  phase = PHASE;
  capacitor = f(phase);
  resist_t = millis();
}


void send(String dataString) {
  Udp.beginPacket(host, port);
  char dataBuf[dataString.length() + 1];
  dataString.toCharArray(dataBuf, dataString.length()+1);
  Udp.write(dataBuf);
  Udp.endPacket();         
}

