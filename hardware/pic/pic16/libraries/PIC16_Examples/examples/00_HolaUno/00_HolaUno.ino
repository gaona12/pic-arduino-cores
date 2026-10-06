// 00_HolaUno - Igual que el Arduino Uno
// Usa Serial.begin / Serial.print / Serial.println tal cual el Uno.
// Para numeros usa Serial.printInt(...) (unica diferencia por ser C).

void setup() {
  Serial.begin(9600);
  Serial.println("soy un pic");
}

void loop() {
  Serial.print("segundos encendido: ");
  Serial.printInt(millis() / 1000);
  Serial.println("");
  delay(1000);
}
