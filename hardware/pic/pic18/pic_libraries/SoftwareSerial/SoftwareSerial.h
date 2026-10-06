/*
 * SoftwareSerial.h - Puerto serie por software (bit-banging) para el core PIC.
 * Equivalente a la libreria SoftwareSerial de Arduino, en C.
 *
 * Permite TX/RX serial en pines arbitrarios (no solo los del UART hardware).
 * Util cuando el UART hardware esta ocupado o quieres un segundo serial.
 *
 * Uso (estilo Arduino):
 *   SoftwareSerial_begin(txPin, rxPin, 9600);
 *   SoftwareSerial_print("hola");
 *   SoftwareSerial_println("mundo");
 *   SoftwareSerial_printInt(123);
 *   char c = SoftwareSerial_read();   // -1 si no hay
 */
#ifndef SOFTWARE_SERIAL_H
#define SOFTWARE_SERIAL_H

void SoftwareSerial_begin(unsigned char txPin, unsigned char rxPin, unsigned long baud);
void SoftwareSerial_write(char c);
void SoftwareSerial_print(const char *s);
void SoftwareSerial_println(const char *s);
void SoftwareSerial_printInt(long n);
void SoftwareSerial_printIntln(long n);
int  SoftwareSerial_read(void);        /* lee un byte, -1 si no llega */

#endif
