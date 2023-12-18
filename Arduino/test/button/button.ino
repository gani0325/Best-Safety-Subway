void setup() {
  pinMode(6,INPUT);
  pinMode(7,INPUT);
  Serial.begin(9600);
}

void loop() {
  int push1=digitalRead(6);
  Serial.print("push1 = ");
  Serial.println(push1);
  
  int push2=digitalRead(7);
  Serial.print("push2 = ");
  Serial.println(push2);
  delay(1000);
}
