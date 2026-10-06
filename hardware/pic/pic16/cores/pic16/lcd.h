/*
 * lcd.h - Libreria LCD 16x2 (HD44780) en modo 4 bits para el core PIC16.
 *
 * Conexion por defecto (pines Arduino del core -> ver Arduino.h):
 *   RS  = pin 8  (RB0)
 *   EN  = pin 9  (RB1)
 *   D4  = pin 10 (RB2)
 *   D5  = pin 11 (RB3)
 *   D6  = pin 12 (RB4)
 *   D7  = pin 13 (RB5)
 * (El pin RW del LCD va a GND.)
 *
 * Si quieres otros pines, llama lcd_config(rs,en,d4,d5,d6,d7) antes de lcd_begin().
 */
#ifndef LCD_H
#define LCD_H

void lcd_config(unsigned char rs, unsigned char en,
                unsigned char d4, unsigned char d5,
                unsigned char d6, unsigned char d7);
void lcd_begin(void);                       /* inicializa el LCD 16x2 */
void lcd_clear(void);                        /* borra la pantalla */
void lcd_home(void);                         /* cursor a 0,0 */
void lcd_setCursor(unsigned char col, unsigned char row);
void lcd_write(char c);                      /* escribe un caracter */
void lcd_print(const char *s);               /* escribe una cadena */
void lcd_printInt(long n);                   /* escribe un entero */
void lcd_command(unsigned char cmd);         /* comando crudo */

#endif
