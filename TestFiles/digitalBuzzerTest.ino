int delayTime = 100;

void setup() {
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  pinMode(0, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  delayTime += 1;
  if(delayTime > 300)
    delayTime = 100;
  
  digitalWrite(2, HIGH);
  digitalWrite(0, LOW);
  delayMicroseconds(delayTime);
  digitalWrite(2, LOW);
  digitalWrite(0, HIGH);
  delayMicroseconds(delayTime);
}
