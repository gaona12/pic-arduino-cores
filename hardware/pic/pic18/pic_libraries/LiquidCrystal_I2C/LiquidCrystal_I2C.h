/*
 * LiquidCrystal_I2C.h - LCD 16x2 por I2C (modulo PCF8574) para el core PIC.
 * Equivalente a la libreria LiquidCrystal_I2C de Arduino, en C.
 *
 * Requiere I2C hardware (MSSP). Conecta SDA=RC4, SCL=RC3 con pull-ups 4.7k.
 *
 * Uso (estilo Arduino):
 *   lcdi2c_begin(0x27);          // direccion del modulo (0x27 o 0x3F)
 *   lcdi2c_print("Hola");
 *   lcdi2c_setCursor(0,1);
 *   lcdi2c_printInt(123);
 *   lcdi2c_clear();
 */
#ifndef LIQUIDCRYSTAL_I2C_H
#define LIQUIDCRYSTAL_I2C_H

void lcdi2c_begin(unsigned char addr);
void lcdi2c_clear(void);
void lcdi2c_setCursor(unsigned char col, unsigned char row);
void lcdi2c_print(const char *s);
void lcdi2c_printInt(long n);
void lcdi2c_backlight(unsigned char on);

#endif
