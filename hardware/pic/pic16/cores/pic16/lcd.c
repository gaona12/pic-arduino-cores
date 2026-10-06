/*
 * lcd.c - LCD 16x2 (HD44780) modo 4 bits para el core PIC16F877A.
 */
#include "Arduino.h"
#include "lcd.h"

/* Pines por defecto (numeracion Arduino del core) */
static unsigned char _rs = 8;   /* RB0 */
static unsigned char _en = 9;   /* RB1 */
static unsigned char _d4 = 10;  /* RB2 */
static unsigned char _d5 = 11;  /* RB3 */
static unsigned char _d6 = 12;  /* RB4 */
static unsigned char _d7 = 13;  /* RB5 */

void lcd_config(unsigned char rs, unsigned char en,
                unsigned char d4, unsigned char d5,
                unsigned char d6, unsigned char d7) {
    _rs = rs; _en = en; _d4 = d4; _d5 = d5; _d6 = d6; _d7 = d7;
}

static void lcd_pulse(void) {
    digitalWrite(_en, HIGH);
    __delay_us(2);
    digitalWrite(_en, LOW);
    __delay_us(50);
}

/* Envia un nibble (4 bits) por D4..D7 */
static void lcd_nibble(unsigned char nib) {
    digitalWrite(_d4, (nib >> 0) & 1);
    digitalWrite(_d5, (nib >> 1) & 1);
    digitalWrite(_d6, (nib >> 2) & 1);
    digitalWrite(_d7, (nib >> 3) & 1);
    lcd_pulse();
}

/* Envia un byte: rs=0 comando, rs=1 dato */
static void lcd_send(unsigned char value, unsigned char rs) {
    digitalWrite(_rs, rs);
    lcd_nibble((unsigned char)(value >> 4));
    lcd_nibble((unsigned char)(value & 0x0F));
}

void lcd_command(unsigned char cmd) { lcd_send(cmd, 0); if (cmd < 4) __delay_ms(2); }
void lcd_write(char c)              { lcd_send((unsigned char)c, 1); }

void lcd_begin(void) {
    pinMode(_rs, OUTPUT); pinMode(_en, OUTPUT);
    pinMode(_d4, OUTPUT); pinMode(_d5, OUTPUT);
    pinMode(_d6, OUTPUT); pinMode(_d7, OUTPUT);
    digitalWrite(_rs, LOW); digitalWrite(_en, LOW);

    __delay_ms(50);              /* espera de arranque del LCD */
    /* secuencia de init en 4 bits (segun datasheet HD44780) */
    digitalWrite(_rs, LOW);
    lcd_nibble(0x03); __delay_ms(5);
    lcd_nibble(0x03); __delay_us(150);
    lcd_nibble(0x03); __delay_us(150);
    lcd_nibble(0x02);            /* fija modo 4 bits */

    lcd_command(0x28);           /* 4 bits, 2 lineas, 5x8 */
    lcd_command(0x0C);           /* display ON, cursor OFF */
    lcd_command(0x06);           /* incremento de cursor */
    lcd_command(0x01);           /* clear */
    __delay_ms(2);
}

void lcd_clear(void) { lcd_command(0x01); __delay_ms(2); }
void lcd_home(void)  { lcd_command(0x02); __delay_ms(2); }

void lcd_setCursor(unsigned char col, unsigned char row) {
    unsigned char addr = (unsigned char)(col + (row ? 0x40 : 0x00));
    lcd_command((unsigned char)(0x80 | addr));
}

void lcd_print(const char *s) { while (*s) lcd_write(*s++); }

void lcd_printInt(long n) {
    char buf[12]; unsigned char i = 0; unsigned long u;
    if (n < 0) { lcd_write('-'); u = (unsigned long)(-n); }
    else u = (unsigned long)n;
    if (u == 0) { lcd_write('0'); return; }
    while (u > 0) { buf[i++] = (char)('0' + (u % 10)); u /= 10; }
    while (i > 0) lcd_write(buf[--i]);
}
