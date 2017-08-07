#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>

ADC_MODE(ADC_VCC); // detect battery life

const String id = String(ESP.getChipId());

// wifi client
WiFiUDP Udp;
const char* ssid     = "GL-MT300N-5cb";
const char* password = "goodlife";
const char* host     = "192.168.8.235";
const int port       = 23232; // both send and receive

// wifi server
ESP8266WebServer server(80);

// constants
const float Pi = 3.141593;
const float INCREMENT = 0.01;
const float BUMP = 0.05;
const float COIT = 0.10;
const int NEIGHBOR = -50;

const int PITCHES[] = {2500, 3750, 5000, 6667, 8333, 11250};

const int PITCH = PITCHES[ESP.getChipId() % sizeof(PITCHES - 1)];
const int LED = 14; // 14 for external, 0 for red, 2 for blue



// state
float phase = 0.0;
float capacitor = 0.0;
int start_t = 0;
int lit = 0;
boolean tilt = false;
String neighbors = "";
int bat = 0;



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
  scan();
  tilt = digitalRead(13);
  connectToWifi();
  Udp.begin(port);

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

  // how's the battery?
  if (lit == 15) {
    bat = ESP.getVcc();
  }
  
  // blink and click
  if (lit == 15) {
    digitalWrite(LED, HIGH);  // on
    tone(12, PITCH, 150);  
    lit--;   
  } else if (lit > 0) {
    lit--;
  } else {
    digitalWrite(LED, LOW);  // off
  }
  
  // update the algorithm
  increment();

  // communicate
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
  
  start_t = millis();
  
}

void increment() {
  phase = _min(phase + INCREMENT, 1.0);
  capacitor = f(phase);
  if (phase == 1.0) {
    Serial.println("--> fire");
    lit = 15;
    phase = 0.0;
    Udp.beginPacket(host, port);
    String dataString = id + "," + String(WiFi.RSSI()) + "," + String(bat) + "," + "fire" + "," + neighbors;
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
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (i % 20 == 0) {
      Serial.print("Connecting");
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
  Serial.println("--> connected");
  digitalWrite(0, HIGH);
}

void scan() {
  digitalWrite(LED, HIGH);
  Serial.println("Scanning...");
  neighbors = "";
  int n = WiFi.scanNetworks();
  for (int i=0; i<n; i++) {
    Serial.println(String(WiFi.SSID(i)) + ":" + String(WiFi.RSSI(i)));
    if (WiFi.SSID(i).substring(0, 6) == "flolg_" and WiFi.RSSI(i) >= NEIGHBOR) {
      neighbors += WiFi.SSID(i).substring(6) + ":" + String(WiFi.RSSI(i)) + ";";
    }
  }
  Serial.print("--> neighbors: ");
  Serial.print(neighbors);
  Serial.println();
  digitalWrite(LED, LOW);
}



