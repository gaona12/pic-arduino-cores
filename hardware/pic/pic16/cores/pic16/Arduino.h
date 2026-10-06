/*
 * Arduino.h - Core completo estilo Arduino para PIC16F877A (XC8)
 *
 * Expone TODO el hardware del PIC16F877A con API tipo Arduino:
 *   - GPIO:        pinMode, digitalWrite, digitalRead, togglePin
 *   - Analogico:   analogRead (ADC 10 bits)
 *   - PWM:         analogWrite (modulos CCP1/CCP2)
 *   - Tiempo:      delay, delayMicroseconds, millis, micros
 *   - Serial:      Serial_begin/print/println/printInt/read/available
 *   - I2C:         Wire_begin/beginTransmission/write/endTransmission/requestFrom/read
 *   - SPI:         SPI_begin/transfer
 *   - EEPROM:      EEPROM_read/EEPROM_write
 *   - Interrupts:  attachInterrupt (INT externo RB0), enableInterrupts
 *   - Utils:       map, constrain, min, max, abs, bitRead/Set/Clear
 *
 * Mapa de "pines Arduino" -> pines del PIC (headers de la placa JT40P):
 *   0..7   = RD0..RD7     8..15 = RB0..RB7     16..21 = RA0..RA5
 *   22..24 = RE0..RE2     25..30 = RC0..RC5    (RC6/RC7 = UART)
 *
 * PWM: CCP1 = pin RC2 (17 en Arduino: 25+2=27), CCP2 = RC1 (26).
 */
#ifndef ARDUINO_H
#define ARDUINO_H

#ifndef ARDUINO_IDE_LIBDETECT
#include <xc.h>
#endif

/* --- Config bits por chip (verificados contra los .cfgdata de XC8) --- */
#if defined(_16F877A) || defined(_16F874A) || defined(_16F877) || defined(_16F874) || \
    defined(_16F876A) || defined(_16F873A)
  /* FOSC WDTE PWRTE BOREN LVP CPD WRT CP */
  #pragma config FOSC=HS, WDTE=OFF, PWRTE=OFF, BOREN=ON, LVP=ON, CPD=OFF, WRT=OFF, CP=OFF
#elif defined(_16F887) || defined(_16F886) || defined(_16F884) || defined(_16F883)
  /* FOSC WDTE PWRTE MCLRE CP CPD BOREN IESO FCMEN LVP BOR4V WRT */
  #pragma config FOSC=HS, WDTE=OFF, PWRTE=OFF, MCLRE=ON, CP=OFF, CPD=OFF
  #pragma config BOREN=ON, IESO=OFF, FCMEN=OFF, LVP=ON, BOR4V=BOR40V, WRT=OFF
#elif defined(_16F628A) || defined(_16F628) || defined(_16F648A)
  /* FOSC WDTE PWRTE MCLRE BOREN LVP CPD CP */
  #pragma config FOSC=HS, WDTE=OFF, PWRTE=OFF, MCLRE=ON, BOREN=ON, LVP=ON, CPD=OFF, CP=OFF
#elif defined(_16F88) || defined(_16F818) || defined(_16F819)
  /* FOSC WDTE PWRTE MCLRE BOREN LVP CPD WRT CCPMX CP FCMEN IESO */
  #pragma config FOSC=HS, WDTE=OFF, PWRTE=OFF, MCLRE=ON, BOREN=ON, LVP=ON
  #pragma config CPD=OFF, WRT=OFF, CCPMX=RB0, CP=OFF, FCMEN=OFF, IESO=OFF
#elif defined(_16F84A) || defined(_16F84)
  /* FOSC WDTE PWRTE CP */
  #pragma config FOSC=HS, WDTE=OFF, PWRTE=OFF, CP=OFF
#elif defined(_16F690) || defined(_16F685) || defined(_16F687) || defined(_16F689)
  /* FOSC WDTE PWRTE MCLRE CP CPD BOREN IESO FCMEN */
  #pragma config FOSC=HS, WDTE=OFF, PWRTE=OFF, MCLRE=ON, CP=OFF, CPD=OFF
  #pragma config BOREN=ON, IESO=OFF, FCMEN=OFF
#else
  #pragma config FOSC=HS, WDTE=OFF, PWRTE=OFF
#endif

#ifndef _XTAL_FREQ
#define _XTAL_FREQ 20000000UL
#endif

/* --- Constantes --- */
#define HIGH 1
#define LOW  0
#define INPUT  1
#define OUTPUT 0
#define true  1
#define false 0
#define boolean unsigned char
#define bool    unsigned char
#define byte    unsigned char
#define word    unsigned int
#define DEC 10
#define HEX 16
#define BIN 2
#define CHANGE 1
#define RISING 2
#define FALLING 3

/* Canales ADC estilo A0..A7 */
#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define A5 5

/* Pines PWM */
#define PWM1 27   /* RC2 / CCP1 */
#define PWM2 26   /* RC1 / CCP2 */

/* --- Macros de bits (como Arduino) --- */
#define bitRead(v,b)   (((v) >> (b)) & 0x01)
#define bitSet(v,b)    ((v) |=  (1u << (b)))
#define bitClear(v,b)  ((v) &= ~(1u << (b)))
#define bitWrite(v,b,x) ((x) ? bitSet(v,b) : bitClear(v,b))
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x)   ((x)>0?(x):-(x))

/* --- Digital I/O --- */
void pinMode(unsigned char pin, unsigned char mode);
void digitalWrite(unsigned char pin, unsigned char value);
unsigned char digitalRead(unsigned char pin);
void togglePin(unsigned char pin);

/* --- Tiempo --- */
void delay(unsigned long ms);
void delayMicroseconds(unsigned long us);
unsigned long millis(void);
unsigned long micros(void);

/* --- Analogico (ADC 10 bits) --- */
unsigned int analogRead(unsigned char channel);

/* --- PWM (CCP1/CCP2) --- */
void analogWrite(unsigned char pin, unsigned char value);  /* 0..255 */
void pwmWrite10(unsigned char pin, unsigned int duty10);   /* duty 0..1023 */
void pwmSetPeriod(unsigned char pr2, unsigned char t2cfg); /* control de periodo */

/* --- Utilidades matematicas --- */
long map(long x, long in_min, long in_max, long out_min, long out_max);
long constrain(long x, long a, long b);

/* --- UART / Serial --- */
void Serial_begin(unsigned long baud);
void Serial_write(char c);
void Serial_print(const char *s);
void Serial_println(const char *s);
void Serial_printInt(long n);
void Serial_printIntln(long n);
void Serial_printNumber(long n, unsigned char base);
unsigned char Serial_available(void);
int  Serial_read(void);

/* ===========================================================================
 * "Serial" estilo Arduino Uno.
 *
 * Para que Serial.print() acepte texto Y numeros con el MISMO metodo (igual
 * que el Uno), el metodo print del objeto apunta a una funcion que decide
 * en tiempo de ejecucion si el argumento es un puntero a texto o un numero.
 * Lo logramos con un tipo "variant" implicito: print recibe un valor y una
 * marca de tipo. Pero para que sea TRANSPARENTE al escribir, el objeto usa
 * punteros a funcion y ademas definimos helpers.
 *
 * USO (identico al Uno):
 *     Serial.begin(9600);
 *     Serial.print("texto");     Serial.print(123);
 *     Serial.println("texto");   Serial.println(123);
 *     Serial.write('A');
 *     if (Serial.available()) c = Serial.read();
 * =========================================================================== */
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

/* --- I2C (MSSP master) --- */
void Wire_begin(void);
void Wire_beginTransmission(unsigned char addr);
void Wire_write(unsigned char data);
void Wire_endTransmission(void);
unsigned char Wire_requestFrom(unsigned char addr, unsigned char nack_last);
unsigned char Wire_read(unsigned char ack);

/* --- SPI (MSSP master) --- */
void SPI_begin(void);
unsigned char SPI_transfer(unsigned char data);

/* --- EEPROM interna (256 bytes) --- */
unsigned char EEPROM_read(unsigned char addr);
void EEPROM_write(unsigned char addr, unsigned char data);

/* --- Interrupciones --- */
void attachInterrupt_INT0(void (*fn)(void), unsigned char mode); /* INT en RB0 */
void enableInterrupts(void);
void disableInterrupts(void);

/* --- LCD 16x2 (declaradas aqui para usarlas sin #include extra) --- */
void lcd_begin(void);
void lcd_clear(void);
void lcd_home(void);
void lcd_setCursor(unsigned char col, unsigned char row);
void lcd_write(char c);
void lcd_print(const char *s);
void lcd_printInt(long n);
void lcd_command(unsigned char cmd);
void lcd_config(unsigned char rs, unsigned char en,
                unsigned char d4, unsigned char d5,
                unsigned char d6, unsigned char d7);

/* El usuario define: */
void setup(void);
void loop(void);

#endif
