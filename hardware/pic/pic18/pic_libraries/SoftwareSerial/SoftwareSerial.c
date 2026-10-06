/*
 * SoftwareSerial.c - Serial por software para el core PIC (bit-banging).
 * Usa el HAL del core: pinMode, digitalWrite, digitalRead, delayMicroseconds.
 */
#include "Arduino.h"
#include "SoftwareSerial.h"

static unsigned char _tx, _rx;
static unsigned int  _bit_us;   /* microsegundos por bit = 1e6 / baud */

void SoftwareSerial_begin(unsigned char txPin, unsigned char rxPin, unsigned long baud) {
    _tx = txPin; _rx = rxPin;
    _bit_us = (unsigned int)(1000000UL / baud);
    pinMode(_tx, OUTPUT);
    digitalWrite(_tx, HIGH);      /* linea idle en alto */
    pinMode(_rx, INPUT);
}

/* Envia un byte: start(0) + 8 datos LSB primero + stop(1) */
void SoftwareSerial_write(char c) {
    unsigned char b = (unsigned char)c;
    digitalWrite(_tx, LOW);                 /* start bit */
    delayMicroseconds(_bit_us);
    for (unsigned char i = 0; i < 8; i++) {
        digitalWrite(_tx, (b & 1) ? HIGH : LOW);
        b >>= 1;
        delayMicroseconds(_bit_us);
    }
    digitalWrite(_tx, HIGH);                /* stop bit */
    delayMicroseconds(_bit_us);
}

void SoftwareSerial_print(const char *s) { while (*s) SoftwareSerial_write(*s++); }
void SoftwareSerial_println(const char *s) { SoftwareSerial_print(s); SoftwareSerial_write('\r'); SoftwareSerial_write('\n'); }
void SoftwareSerial_printInt(long n) {
    char buf[12]; unsigned char i = 0; unsigned long u;
    if (n < 0) { SoftwareSerial_write('-'); u = (unsigned long)(-n); } else u = (unsigned long)n;
    if (u == 0) { SoftwareSerial_write('0'); return; }
    while (u) { buf[i++] = (char)('0' + (u % 10)); u /= 10; }
    while (i) SoftwareSerial_write(buf[--i]);
}
void SoftwareSerial_printIntln(long n) { SoftwareSerial_printInt(n); SoftwareSerial_write('\r'); SoftwareSerial_write('\n'); }

/* Recepcion simple (bloqueante corta): espera un start bit y lee 8 bits.
 * Devuelve -1 si no hay actividad en un tiempo razonable. */
int SoftwareSerial_read(void) {
    /* espera el flanco de bajada (start) un tiempo limitado */
    unsigned int guard = 60000;
    while (digitalRead(_rx) == HIGH) {
        if (--guard == 0) return -1;
    }
    /* medio bit para muestrear en el centro */
    delayMicroseconds(_bit_us / 2);
    if (digitalRead(_rx) != LOW) return -1;   /* falso start */
    unsigned char v = 0;
    for (unsigned char i = 0; i < 8; i++) {
        delayMicroseconds(_bit_us);
        if (digitalRead(_rx)) v |= (unsigned char)(1 << i);
    }
    delayMicroseconds(_bit_us);               /* stop */
    return (int)v;
}
