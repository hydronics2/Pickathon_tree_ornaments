#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
 
const char* ssid     = "IoB2016";
const char* password = "dbtc2016";
boolean wifiConnected = false;
 
const char* host = "192.168.16.200";

SoftwareSerial swSer(2, 0, false, 256); //pins 4 and 3 as marked on board

// UDP variables
unsigned int localPort = 8888;
WiFiUDP UDP;
boolean udpConnected = false;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged"; // a string to send back



byte inByte = 0;
int lastInByte = 0;

byte startLogging = 0;
byte logIncrement = 0;
const int sizeDataLog = 20;
byte dataLog[sizeDataLog];
boolean passData = 0;

//
void setup() {
  Serial.begin(115200);
  swSer.begin(115200);

  Serial.println("\nSoftware serial test started");

  delay(100);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.config(IPAddress(192, 168, 16, 1), IPAddress(192, 168, 16, 200), IPAddress(255, 255, 255, 0));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  udpConnected = connectUDP();
  if (udpConnected){
    Serial.println("UDP connected");
  }
  //UDP.remoteIP() = '192.168.16.200';
  //UDP.remotePort() = '8888';
}

void loop() {

  int packetSize = UDP.parsePacket();  //recieving UDP Broadcast from teensy server
  if (packetSize) {    
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From server at ");
    IPAddress remote = UDP.remoteIP();
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(UDP.remotePort());

    // read the packet into packetBufffer
    memset(packetBuffer, 0, sizeof(packetBuffer)); //clear array
    UDP.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents of UDP Broadcast Received:");
    for(int i = 0; i<packetSize; i++){
      int value = (int)packetBuffer[i];
      Serial.print(value);
      Serial.print(",");
      swSer.print(packetBuffer[i]);
    }
    Serial.println("");   
    Serial.println("");
  }
    
  while (swSer.available() > 0) {
    byte inByte = swSer.read();
    Serial.write(inByte);
    //swSer.write(swSer.read());
    if(inByte == '['){
      Serial.println("");
      memset(dataLog, 0, sizeof(dataLog)); //clear array
      Serial.println("logging data from Pendulum... ");
      startLogging = 1;
      logIncrement = 0;
    } else if(inByte == ']'){
      if(logIncrement < sizeDataLog){
        dataLog[logIncrement] = ']';
      }
      startLogging = 0;
      Serial.println(" - end token received, got the data....");
      passData = true;
    }
    if(startLogging > 0 && logIncrement < sizeDataLog){
      dataLog[logIncrement] = inByte;
      Serial.println((int)inByte);
      logIncrement++;
    }
  }

  if(passData == true){
    Serial.println("Printing out the data and sending UDP to server");
    for(int i = 0; i<logIncrement; i++){
      Serial.print(dataLog[i]);
      Serial.print(",");
    }
    Serial.println();
    passData = false;
    UDP.beginPacket("192.168.16.200", 8888);
    for(int i = 0; i<logIncrement; i++){
      UDP.write(dataLog[i]);
    }    
    UDP.endPacket();
    Serial.println("packet sent to server");
    Serial.println("");
  }
}

// connect to UDP – returns true if successful or false if not
boolean connectUDP(){
  boolean state = false;
  
  Serial.println("");
  Serial.println("Connecting to UDP");
  
    if(UDP.begin(localPort) == 1){
      Serial.println("Connection successful");
      state = true;
  }else{
    Serial.println("Connection failed");
  }
  return state;
} 

