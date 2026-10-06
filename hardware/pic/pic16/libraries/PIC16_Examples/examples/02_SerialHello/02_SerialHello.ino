// 02_SerialHello - Enviar texto y numeros por el puerto serie (UART)
// Abre el monitor serie a 9600 baud. TX=RC6 va por el CH340 al USB.

void setup() {
  Serial_begin(9600);
  Serial_println("PIC16F877A por Arduino IDE");
}

void loop() {
  static long contador = 0;
  Serial_print("Contador = ");
  Serial_printIntln(contador);
  contador++;
  delay(1000);
}
