#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

// wifi connection variables
const char* ssid = "testing";
const char* password = "hydronics";
boolean wifiConnected = false;

// UDP variables
unsigned int localPort = 8888;
WiFiUDP UDP;
boolean udpConnected = false;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged"; // a string to send back

void setup() {
  // Initialise Serial connection
  Serial.begin(115200);

  // Initialise wifi connection
  wifiConnected = connectWifi();

  // only proceed if wifi connection successful
  if(wifiConnected){
    udpConnected = connectUDP();
    if (udpConnected){
      // initialise pins
      pinMode(5,OUTPUT);
    }
  }
}

void loop() {
// check if the WiFi and UDP connections were successful
  if(wifiConnected){
    if(udpConnected){
    
    // if there’s data available, read a packet
      int packetSize = UDP.parsePacket();
      if(packetSize){
        Serial.println("");
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remote = UDP.remoteIP();
        for (int i =0; i < 4; i++){
          Serial.print(remote[i], DEC);
          if (i < 3){
            Serial.print(".");
          }
        } 
        Serial.print(", port ");
        Serial.println(UDP.remotePort());
    
        // read the packet into packetBufffer
        UDP.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
        Serial.println("Contents:");
        int value = packetBuffer[0]*256 + packetBuffer[1];
        //Serial.println(value);
        for(int i=0; i<packetSize; i++){
          Serial.print(packetBuffer[i]);
          Serial.print(",");
        }
        Serial.println("");
        
        // send a reply, to the IP address and port that sent us the packet we received
        UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
        UDP.write(ReplyBuffer);
        UDP.endPacket();
        
        // turn LED on or off depending on value recieved
        digitalWrite(5,value);
      }
      delay(10);
    }
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

// connect to wifi – returns true if successful or false if not
boolean connectWifi(){

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  WiFi.config(IPAddress(192, 168, 1, 60), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  boolean state = true;

  return state;
}
