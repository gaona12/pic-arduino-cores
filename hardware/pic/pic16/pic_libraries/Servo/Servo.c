/*
 * Servo.c - Servomotor por software (pulso 1-2 ms cada 20 ms) para el core PIC.
 * Usa solo la HAL del core: pinMode, digitalWrite, delayMicroseconds, delay.
 * Portado a C a mano (Nivel 2 del transpilador maneja la sintaxis de objeto).
 */
#include "Arduino.h"
#include "Servo.h"

static unsigned char _pin = 255;    /* 255 = sin asignar */
static unsigned int  _us  = 1500;   /* ancho de pulso actual (centro) */

void Servo_attach(unsigned char pin) {
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void Servo_writeMicroseconds(unsigned int us) {
    if (us < 500)  us = 500;
    if (us > 2500) us = 2500;
    _us = us;
    if (_pin == 255) return;
    /* un pulso: HIGH durante _us, luego completa el marco de ~20 ms en LOW */
    digitalWrite(_pin, HIGH);
    delayMicroseconds(_us);
    digitalWrite(_pin, LOW);
    /* resto del periodo (20 ms - pulso) para refrescar el servo */
    unsigned int resto = (unsigned int)(20000 - _us);
    delayMicroseconds(resto);
}

void Servo_write(unsigned char angle) {
    if (angle > 180) angle = 180;
    /* 0 grados = 1000 us, 180 grados = 2000 us (lineal) */
    unsigned int us = (unsigned int)(1000 + ((unsigned long)angle * 1000UL) / 180UL);
    Servo_writeMicroseconds(us);
}

void Servo_detach(void) {
    if (_pin != 255) digitalWrite(_pin, LOW);
    _pin = 255;
}
