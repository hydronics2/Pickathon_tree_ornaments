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

void loop() {
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
        if(c == '['){
          Serial.println("start logging... ");
          startLogging = 1;
          logIncrement = 0;
        } else if(c == ']'){
          startLogging = 0;
          Serial.println("got the data....");
          passData = true;
        }
        if(startLogging > 0 && logIncrement < sizeDataLog){
          dataLog[logIncrement] = c;
          logIncrement++;
        }

        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
      if(passData == true){
        for(int i = 0; i<sizeDataLog; i++){
          Serial.print(dataLog[i]);
          Serial.print(",");
        }
        Serial.println();
        passData = false;
        dataLog[sizeDataLog] = {0};
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

