#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>
 
const char* ssid     = "IoB2016";
const char* password = "dbtc2016";
 
const char* host = "192.168.16.200";

SoftwareSerial swSer(2, 0, false, 256); //pins 4 and 3 as marked on board

byte inByte = 0;
int lastInByte = 0;

byte startLogging = 0;
byte logIncrement = 0;
const int sizeDataLog = 20;
byte dataLog[sizeDataLog];
boolean passData = 0;


void setup() {
  Serial.begin(115200);
  swSer.begin(115200);

  Serial.println("\nSoftware serial test started");

  for (char ch = ' '; ch <= 'z'; ch++) {
    swSer.write(ch);
  }
  swSer.println("");

  delay(100);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  while (swSer.available() > 0) {
    byte inByte = swSer.read();
    Serial.write(inByte);
    //swSer.write(swSer.read());
    if(inByte == '['){
      memset(dataLog, 0, sizeof(dataLog)); //clear array
      Serial.println("start logging... ");
      startLogging = 1;
      logIncrement = 0;
    } else if(inByte == ']'){
      if(logIncrement < sizeDataLog){
        dataLog[logIncrement] = ']';
      }
      startLogging = 0;
      Serial.println("got the data....");
      passData = true;
    }
    if(startLogging > 0 && logIncrement < sizeDataLog){
      dataLog[logIncrement] = inByte;
      logIncrement++;
    }
  }

    while (Serial.available() > 0) {
    byte inByte = Serial.read();
    Serial.write(inByte);
    //swSer.write(swSer.read());
    if(inByte == '['){
      memset(dataLog, 0, sizeof(dataLog)); //clear array
      Serial.println("start logging... ");
      startLogging = 1;
      logIncrement = 0;
    } else if(inByte == ']'){
      if(logIncrement < sizeDataLog){
        dataLog[logIncrement] = ']';
      }      
      startLogging = 0;
      Serial.println("got the data....");
      passData = true;
    }
    if(startLogging > 0 && logIncrement < sizeDataLog){
      dataLog[logIncrement] = inByte;
      logIncrement++;
    }
  }
//  while (Serial.available() > 0) {
//    swSer.write(Serial.read());
//  }
//  if(inByte != lastInByte){
//    swSer.write(inByte);
//    lastInByte = inByte;
//  }

  if(passData == true){
    for(int i = 0; i<sizeDataLog; i++){
      Serial.print(dataLog[i]);
      Serial.print(",");
    }
    Serial.println();
    passData = false;
    Serial.print("connecting to ");
    Serial.println(host);
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }
    
    // We now create a URI for the request
    String url = String(dataLog[0]) + "," + String(dataLog[1]) + "," + String(dataLog[2]) + "," + String(dataLog[3]) + "," + String(dataLog[4]) + "," + String(dataLog[5]) + "," + String(dataLog[6]) + "," + String(dataLog[7]) + "," + String(dataLog[8]) + "," + String(dataLog[9]);
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
    // This will send the request to the server
    client.print(String("GET ") + "/" + url + "\r\n" +
                 "Host: " + host + "\r\n");
    delay(50);
    
    // Read all the lines of the reply from server and print them to Serial
  //  while(client.available()){
  //    String line = client.readStringUntil('\r');
  //    Serial.print(line);
  //  }
    
    Serial.println();
    Serial.println("closing connection");
    
    
  }


}
