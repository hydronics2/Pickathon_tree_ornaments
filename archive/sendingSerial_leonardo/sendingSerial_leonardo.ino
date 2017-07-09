/*


 */

void setup() {
  // initialize both serial ports:
  Serial.begin(115200);
  Serial1.begin(115200);
}

void loop() {
  // read from port 1, send to port 0:
  if (Serial.available()) {
    byte inByte = Serial.read();
    Serial.write(inByte);
    //Serial1.write(inByte);
    Serial1.write(inByte);
  }

  //Serial1.println("printing to serial1");
  //delay(1000);

  

}
