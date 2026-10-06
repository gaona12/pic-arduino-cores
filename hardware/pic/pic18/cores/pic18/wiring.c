/*
 * wiring.c - Implementacion del core PIC18F estilo Arduino (XC8).
 */
#include "Arduino.h"

/* ===== Mapa de pines =====
 * Chips de 40 pines: PORTA..E. Chips de 28 pines (2550/2620): solo A,B,C. */
static volatile unsigned char * port_of(unsigned char pin) {
#if defined(PORTD)
    if (pin <= 7)  return &PORTD;
#endif
    if (pin <= 15) return &PORTB;
    if (pin <= 21) return &PORTA;
#if defined(PORTE)
    if (pin <= 24) return &PORTE;
#endif
    return &PORTC;
}
static volatile unsigned char * tris_of(unsigned char pin) {
#if defined(TRISD)
    if (pin <= 7)  return &TRISD;
#endif
    if (pin <= 15) return &TRISB;
    if (pin <= 21) return &TRISA;
#if defined(TRISE)
    if (pin <= 24) return &TRISE;
#endif
    return &TRISC;
}
static unsigned char bit_of(unsigned char pin) {
    if (pin <= 7)  return pin;
    if (pin <= 15) return (unsigned char)(pin - 8);
    if (pin <= 21) return (unsigned char)(pin - 16);
    if (pin <= 24) return (unsigned char)(pin - 22);
    return (unsigned char)(pin - 25);
}

static unsigned char initialized = 0;
static void core_init(void) {
    if (initialized) return;
    /* PIC18: dejar pines analogicos como digitales.
       Familias distintas usan ADCON1 o ANSEL. */
#if defined(ADCON1)
    ADCON1 = 0x0F;   /* todos digitales (PCFG=1111) */
#endif
#if defined(ANSELA)
    ANSELA = 0; ANSELB = 0; ANSELC = 0;
  #if defined(ANSELD)
    ANSELD = 0; ANSELE = 0;
  #endif
#endif
    initialized = 1;
}

/* ===== Digital ===== */
void pinMode(unsigned char pin, unsigned char mode) {
    core_init();
    volatile unsigned char *tris = tris_of(pin);
    unsigned char b = bit_of(pin);
    if (mode == OUTPUT) *tris &= (unsigned char)~(1u << b);
    else                *tris |=  (unsigned char)(1u << b);
}
void digitalWrite(unsigned char pin, unsigned char value) {
    volatile unsigned char *port = port_of(pin);
    unsigned char b = bit_of(pin);
    if (value) *port |=  (unsigned char)(1u << b);
    else       *port &= (unsigned char)~(1u << b);
}
unsigned char digitalRead(unsigned char pin) {
    volatile unsigned char *port = port_of(pin);
    unsigned char b = bit_of(pin);
    unsigned char v = (unsigned char)(*port & (unsigned char)(1u << b));
    return v ? HIGH : LOW;
}

/* ===== Tiempo ===== */
void delay(unsigned long ms) { while (ms--) __delay_ms(1); }
void delayMicroseconds(unsigned long us) { while (us--) __delay_us(1); }

/* ===== ADC (10 bits) ===== */
unsigned int analogRead(unsigned char channel) {
    core_init();
#if defined(ADCON0)
    /* Selecciona canal, enciende ADC. ADCON2 con justif. derecha. */
  #if defined(ADCON2)
    ADCON2 = 0xA9;  /* right justified, Tacq, Fosc/8 */
  #endif
    ADCON0 = (unsigned char)((channel << 2) | 0x01);  /* canal + ADON */
    __delay_us(20);
    ADCON0 |= 0x02;                 /* GO */
    while (ADCON0 & 0x02) { }       /* espera */
    return (unsigned int)(((unsigned int)ADRESH << 8) | ADRESL);
#else
    (void)channel; return 0;
#endif
}

/* ===== PWM (CCP) ===== */
static unsigned char pwm_started = 0;
void analogWrite(unsigned char pin, unsigned char value) {
#if defined(CCP1CON)
    if (!pwm_started) { PR2 = 255; T2CON = 0x04; pwm_started = 1; }
    if (pin == PWM1) {
        TRISC &= ~0x04;
        CCP1CON = (unsigned char)(0x0C | (unsigned char)((value & 0x03) << 4));
        CCPR1L  = (unsigned char)(value >> 2);
    }
  #if defined(CCP2CON)
    else if (pin == PWM2) {
        TRISC &= ~0x02;
        CCP2CON = (unsigned char)(0x0C | (unsigned char)((value & 0x03) << 4));
        CCPR2L  = (unsigned char)(value >> 2);
    }
  #endif
#else
    (void)pin; (void)value;
#endif
}

/* ===== Utils ===== */
long map(long x, long a, long b, long c, long d) { return (x-a)*(d-c)/(b-a)+c; }
long constrain(long x, long a, long b) { if (x<a) return a; if (x>b) return b; return x; }

/* ===== Serial (EUSART) ===== */
void Serial_begin(unsigned long baud) {
    core_init();
    unsigned long v = (_XTAL_FREQ / (16UL * baud)) - 1UL;
    SPBRG = (unsigned char)v;
    TRISC |= 0xC0;      /* RC6/RC7 */
    TXSTA = 0x24;       /* BRGH=1, TXEN=1 */
    RCSTA = 0x90;       /* SPEN=1, CREN=1 */
}
void Serial_write(char c) { while (!PIR1bits.TXIF) {} TXREG = (unsigned char)c; }
void Serial_print(const char *s) { while (*s) Serial_write(*s++); }
void Serial_println(const char *s) { Serial_print(s); Serial_write('\r'); Serial_write('\n'); }
void Serial_printInt(long n) {
    char buf[12]; unsigned char i = 0; unsigned long u;
    if (n < 0) { Serial_write('-'); u = (unsigned long)(-n); } else u = (unsigned long)n;
    if (u == 0) { Serial_write('0'); return; }
    while (u) { buf[i++] = (char)('0' + (u % 10)); u /= 10; }
    while (i) Serial_write(buf[--i]);
}
void Serial_printIntln(long n) { Serial_printInt(n); Serial_write('\r'); Serial_write('\n'); }
unsigned char Serial_available(void) { return (unsigned char)(PIR1bits.RCIF ? 1 : 0); }
int Serial_read(void) {
    if (!PIR1bits.RCIF) return -1;
    if (RCSTAbits.OERR) { RCSTAbits.CREN = 0; RCSTAbits.CREN = 1; }
    return (int)RCREG;
}

/* Objeto Serial estilo Arduino Uno */
const SerialClass Serial = {
    Serial_begin, Serial_print, Serial_println,
    Serial_printInt, Serial_printIntln,
    Serial_write, Serial_available, Serial_read
};

/* ===== EEPROM (PIC18) ===== */
#if defined(EEADR)
unsigned char EEPROM_read(unsigned char addr) {
    EEADR = addr;
    EECON1bits.EEPGD = 0; EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    return EEDATA;
}
void EEPROM_write(unsigned char addr, unsigned char data) {
    EEADR = addr; EEDATA = data;
    EECON1bits.EEPGD = 0; EECON1bits.CFGS = 0; EECON1bits.WREN = 1;
    INTCONbits.GIE = 0;
    EECON2 = 0x55; EECON2 = 0xAA; EECON1bits.WR = 1;
    INTCONbits.GIE = 1;
    while (EECON1bits.WR) { }
    EECON1bits.WREN = 0;
}
#else
unsigned char EEPROM_read(unsigned char addr) { (void)addr; return 0; }
void EEPROM_write(unsigned char addr, unsigned char data) { (void)addr; (void)data; }
#endif

/* ===== I2C (MSSP master) ===== */
#if defined(SSPCON2)
static void i2c_wait(void) { while ((SSPCON2 & 0x1F) || (SSPSTAT & 0x04)) ; }
void Wire_begin(void) {
    SSPCON1 = 0x28; SSPCON2 = 0x00;
    SSPADD = (unsigned char)((_XTAL_FREQ / (4UL * 100000UL)) - 1UL);
    SSPSTAT = 0x00;
    TRISC |= 0x18;    /* RC3=SCL, RC4=SDA */
}
void Wire_beginTransmission(unsigned char addr) {
    i2c_wait(); SSPCON2bits.SEN = 1;
    i2c_wait(); SSPBUF = (unsigned char)(addr << 1); i2c_wait();
}
void Wire_write(unsigned char data) { i2c_wait(); SSPBUF = data; i2c_wait(); }
void Wire_endTransmission(void) { i2c_wait(); SSPCON2bits.PEN = 1; i2c_wait(); }
unsigned char Wire_requestFrom(unsigned char addr) {
    i2c_wait(); SSPCON2bits.SEN = 1;
    i2c_wait(); SSPBUF = (unsigned char)((addr << 1) | 1); i2c_wait();
    return 1;
}
unsigned char Wire_read(unsigned char ack) {
    unsigned char d;
    i2c_wait(); SSPCON2bits.RCEN = 1;
    i2c_wait(); d = SSPBUF;
    SSPCON2bits.ACKDT = ack ? 0 : 1;
    SSPCON2bits.ACKEN = 1;
    i2c_wait(); return d;
}
void SPI_begin(void) {
    TRISC &= ~0x28;   /* RC3 SCK, RC5 SDO salidas */
    TRISC |= 0x10;    /* RC4 SDI entrada */
    SSPSTAT = 0x40; SSPCON1 = 0x20;
}
unsigned char SPI_transfer(unsigned char data) {
    SSPBUF = data;
    while (!SSPSTATbits.BF) ;
    return SSPBUF;
}
#else
void Wire_begin(void){}
void Wire_beginTransmission(unsigned char a){ (void)a; }
void Wire_write(unsigned char d){ (void)d; }
void Wire_endTransmission(void){}
unsigned char Wire_requestFrom(unsigned char a){ (void)a; return 0; }
unsigned char Wire_read(unsigned char ack){ (void)ack; return 0; }
void SPI_begin(void){}
unsigned char SPI_transfer(unsigned char d){ (void)d; return 0; }
#endif

/* ===== LCD 16x2 (HD44780, 4 bits) =====
 * RS=8(RB0) EN=9(RB1) D4..D7=10..13(RB2..RB5). RW a GND. */
#define LCD_RS 8
#define LCD_EN 9
#define LCD_D4 10
static void lcd_pulse(void){ digitalWrite(LCD_EN,1); __delay_us(2); digitalWrite(LCD_EN,0); __delay_us(50); }
static void lcd_nibble(unsigned char nib){
    digitalWrite(LCD_D4+0,(nib>>0)&1);
    digitalWrite(LCD_D4+1,(nib>>1)&1);
    digitalWrite(LCD_D4+2,(nib>>2)&1);
    digitalWrite(LCD_D4+3,(nib>>3)&1);
    lcd_pulse();
}
static void lcd_send(unsigned char v, unsigned char rs){
    digitalWrite(LCD_RS, rs);
    lcd_nibble((unsigned char)(v>>4));
    lcd_nibble((unsigned char)(v&0x0F));
}
static void lcd_cmd(unsigned char c){ lcd_send(c,0); if(c<4) __delay_ms(2); }
void lcd_begin(void){
    pinMode(LCD_RS,OUTPUT); pinMode(LCD_EN,OUTPUT);
    pinMode(LCD_D4,OUTPUT); pinMode(LCD_D4+1,OUTPUT);
    pinMode(LCD_D4+2,OUTPUT); pinMode(LCD_D4+3,OUTPUT);
    digitalWrite(LCD_RS,0); digitalWrite(LCD_EN,0);
    __delay_ms(50);
    lcd_nibble(0x03); __delay_ms(5);
    lcd_nibble(0x03); __delay_us(150);
    lcd_nibble(0x03); __delay_us(150);
    lcd_nibble(0x02);
    lcd_cmd(0x28); lcd_cmd(0x0C); lcd_cmd(0x06); lcd_cmd(0x01); __delay_ms(2);
}
void lcd_clear(void){ lcd_cmd(0x01); __delay_ms(2); }
void lcd_setCursor(unsigned char col, unsigned char row){
    unsigned char a = (unsigned char)(col + (row ? 0x40 : 0x00));
    lcd_cmd((unsigned char)(0x80 | a));
}
void lcd_print(const char *s){ while(*s) lcd_send((unsigned char)*s++,1); }
void lcd_printInt(long n){
    char buf[12]; unsigned char i=0; unsigned long u;
    if(n<0){ lcd_send('-',1); u=(unsigned long)(-n); } else u=(unsigned long)n;
    if(u==0){ lcd_send('0',1); return; }
    while(u){ buf[i++]=(char)('0'+(u%10)); u/=10; }
    while(i) lcd_send((unsigned char)buf[--i],1);
}
