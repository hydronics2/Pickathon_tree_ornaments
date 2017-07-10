/*
teensy3.6 recieves data from teensy3.2 web server

data format is '[' start character, COCOON#, other data, other data, (up to 20 bytes), ']' end character
 
 */


byte logIncrement = 0;
const int sizeDataLog = 20;
byte dataLog[sizeDataLog] = {0};


void setup() {
  Serial1.begin(115200);
  Serial1.setTimeout(50);
  Serial.begin(115200);
}

void loop() {

  if (Serial1.available()) {
    int foundInt = Serial1.parseInt();
    //Serial.println(foundInt);
    if(foundInt == 91){ //new data
      //Serial.println("new data");
      logIncrement = 0;
    }
    dataLog[logIncrement] = foundInt;
    logIncrement++;
    if(foundInt == 93){ //got all the data
      //Serial.println("got all the data");
      printData();
    }
  }
}

void printData(){
  for(int i = 0; i<logIncrement; i++){
    Serial.print(dataLog[i]);
    Serial.print(",");
  }
  Serial.println();

  
}

