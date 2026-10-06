// 01_Blink - Parpadeo basico de un pin
// Conecta un LED (con resistencia ~330R a GND) al pin 0 (RD0 / header "D0").

void setup() {
  pinMode(0, OUTPUT);
}

void loop() {
  digitalWrite(0, HIGH);
  delay(500);
  digitalWrite(0, LOW);
  delay(500);
}
