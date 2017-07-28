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
  Serial.begin(9600);
  Serial1.begin(9600);


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

const int sizeOfInData = 40;
char inData[sizeOfInData];
byte index = 0;

char *pwm1, *pwm2, *pwm3, *pwm4, *pwm5, *pwm6, *i;


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
      char aChar = client.read();
      if(aChar == '\n')
      {
        for(int i = 0; i < index; i++){
          Serial.write(inData[i]);
        }
        Serial.println();
          char* command = strtok(inData, ",");
          while (command != 0)
          {
            // Split the command in two values
            char* separator = strchr(command, ':');
            if (separator != 0){
              // Actually split the string in 2: replace ':' with 0
              *separator = 0;
              int pinId = atoi(command);
              ++separator;
              int pwmRate = atoi(separator);

              Serial.print("pinID: ");
              Serial.println(pinId);
              Serial.print("pwmRate: ");
              Serial.println(pwmRate);
              
            // Find the next command in input string
            command = strtok(0, "&");
          }
        index = 0;
        inData[index] = NULL;
      }else{
        if(index < sizeOfInData-2){  //safety to keep from overrunning the buffer
          inData[index] = aChar;
          index++;
        inData[index] = '\0'; // Keep the string NULL terminated
      }
    }
  }
}
    }
  }
}

