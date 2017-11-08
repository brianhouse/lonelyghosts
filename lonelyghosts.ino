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

// constants
const float Pi = 3.141593;
const int LED = 14; // 14 for external, 0 for red, 2 for blue
const int PITCH = round((pow(((ESP.getChipId() % 6000) / 6000.0), 3.0) * 6000.0) + 2000.0);

const float INCREMENT = 0.01;
const float COIT = 0.10;
const int BUMP_DELAY = (ESP.getChipId() % 10) + 1; // shouldnt be zero. 10=100ms spread (theres a 10ms gap betwen each)
const int RESIST = 5 * 1000; // 5 seconds

float BUMP = 0.01;
int NEIGHBOR_RANGE = -40;

// state
float phase = 0.0;
float capacitor = 0.0;
int start_t = 0;
int resist_t = 0;
int lit = 0;
int bumping = 0;
boolean tilt = false;
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
  tilt = digitalRead(13);
  start_t = millis();

}

void loop() {

  // are we tilting?
  int tl = digitalRead(13);
  if (tl != tilt) {
    scan();
    tilt = tl;
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

  // communicate, unless we're resisting
  int packetSize = Udp.parsePacket();  
  if (packetSize) {
    char packetBuffer[packetSize];
    Udp.read(packetBuffer, packetSize);
    String action = String(packetBuffer).substring(0, 4);
    if (action == "bump") {  // dont know why substring is necessary
      if (millis() > resist_t + RESIST) {   // are we resisting?
        bumping = BUMP_DELAY;
      }
    }
    else if (action == "rang") {
      int r = String(packetBuffer).substring(5, 7).toInt();
      NEIGHBOR_RANGE = -1 * r;
      Serial.print("--> neighbor range is ");
      Serial.println(NEIGHBOR_RANGE);
    }
    else if (action == "setb") {
      float r = String(packetBuffer).substring(4).toFloat();
      BUMP = r;
      Serial.print("--> bump is ");
      Serial.println(BUMP);
    }
  }
  
  // delayed bumping
  if (bumping > 0) {
    if (bumping == 1) {
      bump();
    }
    bumping--;
  }
  
  start_t = millis();
  
}

void increment() {
  phase = _min(phase + INCREMENT, 1.0);
  capacitor = f(phase);
  if (phase == 1.0) {
    Serial.println("--> fire");
    lit = 15;
    phase = 0.0;
    connectToWifi();
    Udp.beginPacket(host, port);
    String dataString = id + "," + String(WiFi.RSSI()) + ",fire";
    char dataBuf[dataString.length()+1];
    dataString.toCharArray(dataBuf, dataString.length()+1);
    Udp.write(dataBuf);
    Udp.endPacket();    
  }
}

void bump() {
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
  if (WiFi.status() != WL_CONNECTED) {
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      if (i % 10 == 0) {
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
  Serial.println("Scanning...");
  neighbors = "";
  int n = WiFi.scanNetworks();
  for (int i=0; i<n; i++) {
    Serial.println(String(WiFi.SSID(i)) + ":" + String(WiFi.RSSI(i)));
    if (WiFi.SSID(i).substring(0, 6) == "flolg_" and WiFi.RSSI(i) >= NEIGHBOR_RANGE) {
      neighbors += WiFi.SSID(i).substring(6) + ":" + String(WiFi.RSSI(i)) + ";";
    }
  }
  Serial.print("--> neighbors: ");
  Serial.print(neighbors);
  Serial.println();
  Udp.beginPacket(host, port);
  String dataString = id + "," + String(WiFi.RSSI()) + ",scan," + neighbors;
  char dataBuf[dataString.length()+1];
  dataString.toCharArray(dataBuf, dataString.length()+1);
  Udp.write(dataBuf);
  Udp.endPacket();         
  digitalWrite(LED, LOW);
  resist_t = millis();
}

