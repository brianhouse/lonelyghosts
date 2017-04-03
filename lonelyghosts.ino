#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

String idString = "1";

const char* ssid     = "treehouse";
const char* password = "10happysquirrels";
const char* host     = "192.168.1.4";

WiFiUDP Udp;
const int port       = 23232; // both send and receive

const float INCREMENT = 0.01;
const float BUMP = 0.05;

float phase = 0.0;
float capacitor = 0.0;
int lit = 0;

const float Pi = 3.141593;

void setup() {
//  tone(8, 4, 4);
  Serial.begin(115200);
  delay(10);
  pinMode(0, OUTPUT);  
  connectToWifi();
  Udp.begin(port);
}

void increment() {
  phase = _min(phase + (INCREMENT * 1.0), 1.0);
  capacitor = f(phase);
  if (phase == 1.0) {
    Serial.println("--> fire");
    lit = 10;
    phase = 0.0;
    Udp.beginPacket(host, port);
    String dataString = idString + "," + String(WiFi.RSSI()) + "," + "fire";
    char dataBuf[dataString.length()+1];
    dataString.toCharArray(dataBuf, dataString.length()+1);
    Udp.write(dataBuf);
    Udp.endPacket();    
  }
}

void bump() {
  Serial.print("--> bump");
  if (phase > INCREMENT * 10) {
    capacitor = _min(capacitor + BUMP, 1.0);
    phase = f_inv(capacitor);
  } else {
    Serial.print(" (ignored)");
  }
  Serial.println();
}

float f(float x) {
  return sin((Pi / 2) * x);
}

float f_inv(float y) {
  return (2/Pi) * asin(y);
}

void loop() {
  if (lit == 10) {
    pinMode(2, HIGH);  
    lit--;
  } else if (lit > 0) {
    lit--;
  } else {
    pinMode(2, LOW);
  }
  int packetSize = Udp.parsePacket();  
  if (packetSize) {
    char packetBuffer[packetSize];
    Udp.read(packetBuffer, packetSize);
    if (String(packetBuffer) == "bump") {
      bump();
    }
  }
  increment();
  delay(10);
}

void connectToWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println();
    Serial.println();
    Serial.print("Attempting to connect to: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    int tries = 0;
    while (tries < 20 && WiFi.status() != WL_CONNECTED) {
      digitalWrite(0, HIGH);
      delay(250);
      digitalWrite(0, LOW);
      delay(250);      
      Serial.print(".");
      tries++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();
      tries = 0;
      while (tries < 20) {
        digitalWrite(0, HIGH);
        delay(250);
        digitalWrite(0, LOW);
        delay(250);
        tries++;
      }
    }
  }
  Serial.println();
  Serial.println("--> connected to wifi");
  digitalWrite(0, HIGH);
  printWifiStatus();
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

// https://www.arduino.cc/en/Tutorial/ToneMelody?from=Tutorial.Tone
