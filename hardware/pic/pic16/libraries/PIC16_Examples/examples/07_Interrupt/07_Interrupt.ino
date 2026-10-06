// 07_Interrupt - Interrupcion externa en RB0 (INT0)
//
// Cuenta pulsos de un boton conectado a RB0 (pin 8). Cada flanco de bajada
// (al presionar) incrementa el contador, que se imprime por serial.
// Conecta el boton entre RB0 y GND (con pull-up externo o del propio PIC).

volatile long pulsos = 0;

void contarPulso() {
  pulsos++;
}

void setup() {
  Serial_begin(9600);
  attachInterrupt_INT0(contarPulso, FALLING);  // flanco de bajada
  Serial_println("Presiona el boton en RB0...");
}

void loop() {
  static long ultimo = -1;
  if (pulsos != ultimo) {
    ultimo = pulsos;
    Serial_print("Pulsos: ");
    Serial_printIntln(pulsos);
  }
  delay(50);
}
