/*
 * Arduino.h - Core estilo Arduino para PIC18F (XC8).
 *
 * Soporta los PIC18F mas usados. Config bits por chip (verificados contra
 * los .cfgdata de XC8). Reusa la API: pinMode/digitalWrite/analogRead/
 * Serial/PWM/EEPROM, etc.
 *
 * Mapa de pines (chips de 40 pines, p.ej. 18F4550):
 *   0..7=RD0..RD7  8..15=RB0..RB7  16..21=RA0..RA5  22..24=RE0..RE2
 *   25..31=RC0..RC7
 */
#ifndef ARDUINO_H
#define ARDUINO_H

#ifndef ARDUINO_IDE_LIBDETECT
#include <xc.h>
#endif

/* ===== Config bits por chip (3 familias de PIC18) ===== */
/* -- Grupo USB: 18F4550 / 18F2550 / 18F4455 / 18F2455 -- */
#if defined(_18F4550) || defined(_18F2550) || defined(_18F4455) || defined(_18F2455)
  #pragma config PLLDIV=5, CPUDIV=OSC1_PLL2, USBDIV=2
  #pragma config FOSC=HS, FCMEN=OFF, IESO=OFF
  #pragma config PWRT=OFF, BOR=ON, BORV=3, VREGEN=ON
  #pragma config WDT=OFF, WDTPS=32768
  #pragma config MCLRE=ON, LVP=OFF, XINST=OFF
  #pragma config CP0=OFF, CPB=OFF, WRTC=OFF

/* -- Grupo moderno: 18F4520/2520/4620/2620/4525/4523/4585 -- */
#elif defined(_18F4520) || defined(_18F2520) || defined(_18F4620) || \
      defined(_18F2620) || defined(_18F4523) || \
      defined(_18F4525) || defined(_18F4585)
  #pragma config OSC=HS, FCMEN=OFF, IESO=OFF
  #pragma config PWRT=OFF, WDT=OFF, WDTPS=32768
  #pragma config MCLRE=ON, LVP=OFF, XINST=OFF, STVREN=ON
  #pragma config CP0=OFF, CPB=OFF, WRTC=OFF

/* -- 18F4580/4585 (CAN): BOREN con valores distintos, lo omitimos -- */
#elif defined(_18F4580) || defined(_18F2580) || defined(_18F4680)
  #pragma config OSC=HS, FCMEN=OFF, IESO=OFF
  #pragma config PWRT=OFF, WDT=OFF, WDTPS=32768
  #pragma config MCLRE=ON, LVP=OFF, XINST=OFF, STVREN=ON
  #pragma config CP0=OFF, CPB=OFF, WRTC=OFF

/* -- Grupo clasico: 18F452 / 18F458 -- */
#elif defined(_18F452) || defined(_18F458)
  #pragma config OSC=HS, OSCS=OFF
  #pragma config PWRT=OFF, BOR=ON, BORV=27
  #pragma config WDT=OFF, WDTPS=128
  #pragma config LVP=OFF, STVR=ON
  #pragma config CP0=OFF, CPB=OFF, WRTC=OFF

#else
  /* Fallback: la mayoria de 18F aceptan OSC=HS */
  #pragma config WDT=OFF
#endif

#ifndef _XTAL_FREQ
#define _XTAL_FREQ 20000000UL
#endif

/* ===== Constantes ===== */
#define HIGH 1
#define LOW  0
#define INPUT 1
#define OUTPUT 0
#define true 1
#define false 0
#define boolean unsigned char
#define byte unsigned char
#define DEC 10
#define HEX 16
#define PWM1 27   /* RC2 / CCP1 */
#define PWM2 26   /* RC1 / CCP2 */

#define bitRead(v,b)  (((v)>>(b))&1)
#define bitSet(v,b)   ((v)|=(1u<<(b)))
#define bitClear(v,b) ((v)&=~(1u<<(b)))
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

/* ===== API ===== */
void pinMode(unsigned char pin, unsigned char mode);
void digitalWrite(unsigned char pin, unsigned char value);
unsigned char digitalRead(unsigned char pin);
void delay(unsigned long ms);
void delayMicroseconds(unsigned long us);
unsigned int analogRead(unsigned char channel);
void analogWrite(unsigned char pin, unsigned char value);
long map(long x, long a, long b, long c, long d);
long constrain(long x, long a, long b);

void Serial_begin(unsigned long baud);
void Serial_write(char c);
void Serial_print(const char *s);
void Serial_println(const char *s);
void Serial_printInt(long n);
void Serial_printIntln(long n);
unsigned char Serial_available(void);
int Serial_read(void);

/* Objeto Serial estilo Arduino Uno (igual que el core PIC16) */
typedef struct {
    void (*begin)(unsigned long baud);
    void (*print)(const char *s);
    void (*println)(const char *s);
    void (*printInt)(long n);
    void (*printIntln)(long n);
    void (*write)(char c);
    unsigned char (*available)(void);
    int (*read)(void);
} SerialClass;
extern const SerialClass Serial;

unsigned char EEPROM_read(unsigned char addr);
void EEPROM_write(unsigned char addr, unsigned char data);

/* I2C (MSSP master) */
void Wire_begin(void);
void Wire_beginTransmission(unsigned char addr);
void Wire_write(unsigned char data);
void Wire_endTransmission(void);
unsigned char Wire_requestFrom(unsigned char addr);
unsigned char Wire_read(unsigned char ack);

/* SPI (MSSP master) */
void SPI_begin(void);
unsigned char SPI_transfer(unsigned char data);

/* LCD 16x2 (HD44780, 4 bits) */
void lcd_begin(void);
void lcd_clear(void);
void lcd_setCursor(unsigned char col, unsigned char row);
void lcd_print(const char *s);
void lcd_printInt(long n);

void setup(void);
void loop(void);

#endif
