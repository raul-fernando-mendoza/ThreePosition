  #include <SoftwareSerial.h>

  SoftwareSerial MySerial(0, 3); // RX, TX

void setup() {
  // put your setup code here, to run once:
  MySerial.begin(9600);
  MySerial.println("Hello");
}
int i;
void loop() {
  // put your main code here, to run repeatedly:
  MySerial.print("hello");
  MySerial.println(i++);
  double d = (double)d*0.3;
  delay(1000);
}
