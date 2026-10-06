// 03_AnalogRead - Leer un potenciometro con el ADC (10 bits)
// Conecta el potenciometro al canal AN0 (pin RA0). Rango 0..1023.

void setup() {
  Serial_begin(9600);
  Serial_println("Lectura ADC canal 0 (RA0)");
}

void loop() {
  unsigned int valor = analogRead(0);          // canal 0 = AN0 = RA0
  long porcentaje = map(valor, 0, 1023, 0, 100);

  Serial_print("ADC=");
  Serial_printInt(valor);
  Serial_print("  (");
  Serial_printInt(porcentaje);
  Serial_println("%)");
  delay(300);
}
