// 10_SoftwareSerial - Segundo puerto serie por software (bit-banging)
//
// Util cuando el UART hardware esta ocupado o quieres un serial extra en
// pines arbitrarios. Funciona en TODOS los PIC del core (solo usa GPIO).
//
// Libreria: SoftwareSerial  (ver pic_libraries/SoftwareSerial)
//   requires=DIGITAL   -> compatible con cualquier chip
//
// Cableado de ejemplo: TX en el pin 0, RX en el pin 1.
//   El baudrate maximo depende del cristal (a 20 MHz, 9600 va comodo).

#include "SoftwareSerial.h"

void setup() {
  SoftwareSerial_begin(0, 1, 9600);   // txPin, rxPin, baud
  SoftwareSerial_println("Serial por software listo");
}

void loop() {
  // Eco: si llega un byte por RX, lo reenvia por TX
  int c = SoftwareSerial_read();       // -1 si no hay dato
  if (c != -1) {
    SoftwareSerial_write((char)c);
  }
  SoftwareSerial_print("contador=");
  static long n = 0;
  SoftwareSerial_printIntln(n++);
  delay(1000);
}
