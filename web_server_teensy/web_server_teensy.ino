/*
teensy3.2 connects to router... reads GET requests coming from ESP pendulums
teensy web server IP 192.168.16.200

data format is '[' start character, COCOON#, other data, other data, (up to 20 bytes), ']' end character
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 16, 200);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
unsigned int localPort = 8888;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

byte placeX[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};  // x position in feet^-1 so 20 equals 2 feet
byte placeY[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}; // y position in feet^-1
byte distanceArray[20];

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial1.begin(9600);


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  Udp.begin(localPort);
  
}

byte startLogging = 0;
byte logIncrement = 0;
const int sizeDataLog = 20;
char dataLog[sizeDataLog] = {0};
boolean passData = 0;

const int sizeDataLog2 = 20;
byte dataLog2[sizeDataLog2] = {0};
int incrementData2 = 0;

int startIncrement = 0;
int stopIncrement = 0;
boolean findInteger = 0;



void loop() {

  int packetSize = Udp.parsePacket();
  if (packetSize) {    
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    memset(packetBuffer, 0, sizeof(packetBuffer)); //clear array
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    for(int i = 0; i < packetSize; i++){
      int value = (int)packetBuffer[i];
      Serial.print(value);
      Serial.print(",");
    }
    byte activePendulum = 0;
    byte activeRed = 0;
    byte activeGreen = 0;
    byte activeBlue = 0;
    byte activeIntensity = 0;
    
    if(packetSize > 2){ //includes cocoon number
      activePendulum = (int)packetBuffer[1];
      Serial.print("active pendulum: ");
      Serial.println(activePendulum);
      
    }
    if(packetSize == 6){ //got red green blue too
      activeRed = (int)packetBuffer[2];
      activeGreen = (int)packetBuffer[3];
      activeBlue = (int)packetBuffer[4];
      activeIntensity = (int)packetBuffer[5];
    }

    findDistances(activePendulum);
    
    Serial.println("");
    Serial.println("");
    // send a reply to the IP address and port that sent us the packet we received
    Udp.beginPacket("255.255.255.255", 8888);
    Udp.write(91);  //signifies start of package... probalby goingn to leave out 
    Udp.write(activeRed);
    Udp.write(activeGreen);
    Udp.write(activeBlue);
    Udp.write(activeIntensity);
    for(int i=0; i<20; i++){
      Udp.write(distanceArray[i]);
    }
    Udp.endPacket();


    //sends random strings to teensy3.6 for audio response
    Serial1.print(91);
    Serial1.print(",");
    Serial1.print(random(0,20));
    Serial1.print(",");
    Serial1.print(100);
    Serial1.print(",");
    Serial1.print(93);
    Serial1.println();
  }
  delay(10);

//  while (Serial.available() > 0) {
//    byte inByte = Serial.read();
//    Serial.write(inByte);
//    //swSer.write(swSer.read());
//    if(inByte == '['){
//      memset(dataLog, 0, sizeof(dataLog)); //clear array
//      Serial.println("start logging... ");
//      startLogging = 1;
//      logIncrement = 0;
//    } else if(inByte == ']'){
//      if(logIncrement < sizeDataLog){
//        dataLog[logIncrement] = ']';
//      }      
//      startLogging = 0;
//      Serial.println("got the data....");
//      passData = true;
//    }
//    if(startLogging > 0 && logIncrement < sizeDataLog){
//      dataLog[logIncrement] = inByte;
//      logIncrement++;
//    }
//  }
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if(c == '/'){ // '['
          Serial.println("start logging... ");
          memset(dataLog, 0, sizeof(dataLog)); //clear array
          memset(dataLog2, 0, sizeof(dataLog2)); //clear array
          startLogging = 1;
          logIncrement = 0;
          startIncrement = 0;
          incrementData2 = 0;
        } else if(logIncrement == 20){
          startLogging = 0;
          Serial.println("got the data....");
          client.stop();
          logIncrement = 0;
          passData = true;
          //startIncrement = 0;
        }
        if( c == ','){
          startIncrement = stopIncrement+1;
          Serial.print("startIncrement: ");
          Serial.println(startIncrement);
          stopIncrement = logIncrement;
          findInteger = true;
          Serial.print("stopIncrement: ");
          Serial.println(stopIncrement);
        }
        if(startLogging > 0 && logIncrement < sizeDataLog){
          dataLog[logIncrement] = c;
          logIncrement++;
        }
 
        if(findInteger == true){
          findInteger = false;
          if(stopIncrement - startIncrement == 1){
            Serial.print("found one character byte: ");
            Serial.println(dataLog[startIncrement]);
            //int  tempInt = (int)dataLog[startIncrement];
            //dataLog2[incrementData2] = tempInt;
            String tempString = String(dataLog[startIncrement]);
            int tempInt = tempString.toInt();
            dataLog2[incrementData2] = (byte)tempInt;         
            findInteger = false;
            incrementData2++;
          } else if(stopIncrement - startIncrement == 2){
            //startIncrement = 
            char buffer[3];
            buffer[0] = dataLog[startIncrement];
            buffer[1] = dataLog[startIncrement+1];
            buffer[2] = '\0';
            int  n;
            n = atoi(buffer);
            Serial.print("integer: ");
            Serial.println(n);
            char tempChar = (char)n;
            dataLog2[incrementData2] = tempChar;
            incrementData2++;
            findInteger = false;
          } else if(stopIncrement - startIncrement == 3){
            //startIncrement = 
            char buffer[3];
            buffer[0] = dataLog[startIncrement];
            buffer[1] = dataLog[startIncrement+1];
            buffer[2] = dataLog[startIncrement+2];
            buffer[3] = '\0';
            int  n;
            n = atoi(buffer);
            Serial.print("integer: ");
            Serial.println(n);
            char tempChar = (char)n;
            dataLog2[incrementData2] = tempChar;
            incrementData2++;
            findInteger = false;
          }
        }
      }
      if(passData == true){ //dumbed it down for sanity
        Serial1.print(91);
        Serial1.print(",");
        Serial1.print(random(0,20));
        Serial1.print(",");
        Serial1.print(100);
        Serial1.print(",");
        Serial1.print(93);
        Serial1.println();
        passData = false;
        dataLog[sizeDataLog] = {0};
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    //client.stop();
    Serial.println("client disconnected");
  }
}

void findDistances(byte activePendulum){
  for(int i=0; i<20; i++){
    byte distance = sqrt(sq(placeX[i] - placeX[activePendulum]) + sq(placeY[i] - placeY[activePendulum]));
    Serial.print(i);
    Serial.print("-distance: ");
    Serial.println(distance);
    distanceArray[i] = distance;
  }
}


 

