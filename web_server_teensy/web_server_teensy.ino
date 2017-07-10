/*
teensy3.2 connects to router... reads GET requests coming from ESP pendulums
teensy web server IP 192.168.16.200

data format is '[' start character, COCOON#, other data, other data, (up to 20 bytes), ']' end character
 
 */

#include <SPI.h>
#include <Ethernet.h>

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

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial1.begin(115200);


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
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
      if(passData == true){
        for(int i = 0; i<sizeDataLog; i++){
          Serial.print(dataLog2[i]);
          Serial.print(",");
          Serial1.print(dataLog2[i]);
          Serial1.print(",");
        }
        Serial.println();
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

