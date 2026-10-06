/*
 * LiquidCrystal_I2C.c - LCD 16x2 via PCF8574 (I2C) para el core PIC.
 * Usa el Wire del core. El PCF8574 mapea sus 8 pines al LCD asi:
 *   P0=RS  P1=RW  P2=EN  P3=backlight  P4..P7=D4..D7
 */
#include "Arduino.h"
#include "LiquidCrystal_I2C.h"

static unsigned char _addr;
static unsigned char _bl = 0x08;   /* bit de backlight (P3) */

/* escribe un byte crudo al PCF8574 por I2C */
static void pcf_write(unsigned char data) {
    Wire_beginTransmission(_addr);
    Wire_write(data | _bl);
    Wire_endTransmission();
}

/* pulso del EN (P2) para latch */
static void lcd_pulse(unsigned char data) {
    pcf_write(data | 0x04);        /* EN=1 */
    __delay_us(2);
    pcf_write(data & ~0x04);       /* EN=0 */
    __delay_us(50);
}

/* envia un nibble (en P4..P7) con RS dado (P0) */
static void lcd_nibble(unsigned char nib, unsigned char rs) {
    unsigned char data = (unsigned char)((nib << 4) | (rs ? 0x01 : 0x00));
    pcf_write(data);
    lcd_pulse(data);
}

static void lcd_send(unsigned char value, unsigned char rs) {
    lcd_nibble((unsigned char)(value >> 4), rs);
    lcd_nibble((unsigned char)(value & 0x0F), rs);
}

static void lcd_cmd(unsigned char c) { lcd_send(c, 0); if (c < 4) __delay_ms(2); }

void lcdi2c_begin(unsigned char addr) {
    _addr = addr;
    Wire_begin();
    __delay_ms(50);
    /* secuencia de init 4 bits */
    lcd_nibble(0x03, 0); __delay_ms(5);
    lcd_nibble(0x03, 0); __delay_us(150);
    lcd_nibble(0x03, 0); __delay_us(150);
    lcd_nibble(0x02, 0);
    lcd_cmd(0x28);   /* 4 bits, 2 lineas */
    lcd_cmd(0x0C);   /* display on, cursor off */
    lcd_cmd(0x06);   /* incremento */
    lcd_cmd(0x01);   /* clear */
    __delay_ms(2);
}
void lcdi2c_clear(void) { lcd_cmd(0x01); __delay_ms(2); }
void lcdi2c_setCursor(unsigned char col, unsigned char row) {
    unsigned char a = (unsigned char)(col + (row ? 0x40 : 0x00));
    lcd_cmd((unsigned char)(0x80 | a));
}
void lcdi2c_print(const char *s) { while (*s) lcd_send((unsigned char)*s++, 1); }
void lcdi2c_printInt(long n) {
    char buf[12]; unsigned char i = 0; unsigned long u;
    if (n < 0) { lcd_send('-',1); u = (unsigned long)(-n); } else u = (unsigned long)n;
    if (u == 0) { lcd_send('0',1); return; }
    while (u) { buf[i++] = (char)('0' + (u % 10)); u /= 10; }
    while (i) lcd_send((unsigned char)buf[--i], 1);
}
void lcdi2c_backlight(unsigned char on) { _bl = on ? 0x08 : 0x00; pcf_write(0); }
