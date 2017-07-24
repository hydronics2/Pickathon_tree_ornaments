void setup() {
  // put your setup code here, to run once:
Serial1.begin(9600);
  

}

void loop() {
  Serial1.print("91,1,23,93");
  delay(1000);
  Serial1.print("91,2,244,93");
  delay(1000);
  Serial1.print("91,3,222,93");
  delay(1000);
  Serial1.print("91,4,200,93");
  delay(1000);
  Serial1.print("91,5,222,93");
  delay(1000);

}
