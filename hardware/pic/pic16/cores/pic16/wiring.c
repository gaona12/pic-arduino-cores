/*
 * wiring.c - Core multi-chip estilo Arduino para PIC de 8 bits (XC8).
 * Usa board_config.h para activar solo los perifericos de cada chip.
 */
#include "Arduino.h"
#include "board_config.h"

/* ===================== Mapa de pines ===================== *
 * El mapa depende de los puertos disponibles. Chips de 8 pines (12F)
 * usan solo GPIO. Chips medianos/grandes usan A..E segun existan.
 */
static volatile unsigned char * port_of(unsigned char pin) {
#if defined(HAS_GPIO_ONLY)
    return &GPIO;                       /* 12F6xx: todo en GPIO */
#else
  #if defined(HAS_PORTD)
    if (pin <= 7)  return &PORTD;
  #endif
    if (pin <= 15) return &PORTB;
  #if defined(HAS_PORTA)
    if (pin <= 21) return &PORTA;
  #endif
  #if defined(HAS_PORTE)
    if (pin <= 24) return &PORTE;
  #endif
  #if defined(HAS_PORTC)
    return &PORTC;
  #endif
    return &PORTB;
#endif
}
static volatile unsigned char * tris_of(unsigned char pin) {
#if defined(HAS_GPIO_ONLY)
    return &TRISIO;
#else
  #if defined(HAS_PORTD)
    if (pin <= 7)  return &TRISD;
  #endif
    if (pin <= 15) return &TRISB;
  #if defined(HAS_PORTA)
    if (pin <= 21) return &TRISA;
  #endif
  #if defined(HAS_PORTE)
    if (pin <= 24) return &TRISE;
  #endif
  #if defined(HAS_PORTC)
    return &TRISC;
  #endif
    return &TRISB;
#endif
}
static unsigned char bit_of(unsigned char pin) {
#if defined(HAS_GPIO_ONLY)
    return pin;                         /* GP0..GP5 */
#else
    if (pin <= 7)  return pin;
    if (pin <= 15) return (unsigned char)(pin - 8);
    if (pin <= 21) return (unsigned char)(pin - 16);
    if (pin <= 24) return (unsigned char)(pin - 22);
    return (unsigned char)(pin - 25);
#endif
}

static unsigned char initialized = 0;
static void core_init(void) {
    if (initialized) return;
    /* Poner todos los pines como DIGITALES. El registro depende del chip:
       - ADCON1 (877A, 88): PCFG para digital
       - ANSEL/ANSELH (887, 690, 88): 0 = digital
       - CMCON/CMCON0: comparadores off */
#if defined(ADCON1)
    ADCON1 = 0x06;
#endif
#if defined(ANSEL)
    ANSEL = 0x00;
#endif
#if defined(ANSELH)
    ANSELH = 0x00;
#endif
#if defined(CMCON)
    CMCON = 0x07;
#elif defined(CMCON0)
    CMCON0 = 0x07;
#endif
    initialized = 1;
}

/* ===================== Digital I/O ===================== */
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
    return (unsigned char)((*port & (unsigned char)(1u << b)) ? HIGH : LOW);
}
void togglePin(unsigned char pin) {
    volatile unsigned char *port = port_of(pin);
    *port ^= (unsigned char)(1u << bit_of(pin));
}

/* ===================== Tiempo ===================== */
void delay(unsigned long ms) { while (ms--) __delay_ms(1); }
void delayMicroseconds(unsigned long us) { while (us--) __delay_us(1); }

static volatile unsigned long _micros_acc = 0;
static unsigned char timer_started = 0;
#define T0_US_PER_OVF  ((256UL * 64UL * 4UL) / (_XTAL_FREQ / 1000000UL))

static void start_millis_timer(void) {
    if (timer_started) return;
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PSA  = 0;
    OPTION_REGbits.PS   = 0b101;   /* 1:64 */
    TMR0 = 0;
    CORE_TMR0IF = 0;              /* alias universal (ver board_config.h) */
    CORE_TMR0IE = 1;
    INTCONbits.GIE = 1;
    timer_started = 1;
}
unsigned long millis(void) { start_millis_timer(); return _micros_acc / 1000UL; }
unsigned long micros(void) { start_millis_timer(); return _micros_acc; }

static void (*user_int0)(void) = 0;
void __interrupt() isr(void) {
    if (CORE_TMR0IE && CORE_TMR0IF) {
        CORE_TMR0IF = 0;
        _micros_acc += T0_US_PER_OVF;
    }
    if (CORE_INTE && CORE_INTF) {
        CORE_INTF = 0;
        if (user_int0) user_int0();
    }
}

/* ===================== ADC ===================== */
#if defined(HAS_ADC)
unsigned int analogRead(unsigned char channel) {
    if (channel > 7) channel = 7;
    ADCON1 = 0x80;   /* ADFM=1, canales analogicos */
    ADCON0 = (unsigned char)(0x40 | (unsigned char)(channel << 3) | 0x01);
    __delay_us(20);
    GO_nDONE = 1;
    while (GO_nDONE) { }
    return (unsigned int)(((unsigned int)ADRESH << 8) | ADRESL);
}
#else
unsigned int analogRead(unsigned char channel) { (void)channel; return 0; }
#endif

/* ===================== PWM ===================== */
#if defined(HAS_PWM)
static unsigned char pwm_init_done = 0;
static void pwm_init(void) {
    if (pwm_init_done) return;
    PR2 = 255; T2CON = 0x04;
    pwm_init_done = 1;
}
void analogWrite(unsigned char pin, unsigned char value) {
    pwm_init();
  #if defined(_12F683)
    /* 12F683 tiene un solo CCP en GP2 */
    TRISIObits.TRISIO2 = 0;
    CCP1CON = (unsigned char)(0x0C | (unsigned char)((value & 0x03) << 4));
    CCPR1L  = (unsigned char)(value >> 2);
    (void)pin;
  #else
    if (pin == PWM1) {
        TRISC2 = 0;
        CCP1CON = (unsigned char)(0x0C | (unsigned char)((value & 0x03) << 4));
        CCPR1L  = (unsigned char)(value >> 2);
    }
    #if defined(CCP2CON)
    else if (pin == PWM2) {
        TRISC1 = 0;
        CCP2CON = (unsigned char)(0x0C | (unsigned char)((value & 0x03) << 4));
        CCPR2L  = (unsigned char)(value >> 2);
    }
    #endif
  #endif
}
void pwmWrite10(unsigned char pin, unsigned int duty10) {
    pwm_init();
    unsigned char high = (unsigned char)(duty10 >> 2);
    unsigned char low  = (unsigned char)(duty10 & 0x03);
  #if defined(_12F683)
    TRISIObits.TRISIO2 = 0;
    CCP1CON = (unsigned char)(0x0C | (unsigned char)(low << 4));
    CCPR1L  = high; (void)pin;
  #else
    if (pin == PWM1) {
        TRISC2 = 0;
        CCP1CON = (unsigned char)(0x0C | (unsigned char)(low << 4));
        CCPR1L  = high;
    }
    #if defined(CCP2CON)
    else if (pin == PWM2) {
        TRISC1 = 0;
        CCP2CON = (unsigned char)(0x0C | (unsigned char)(low << 4));
        CCPR2L  = high;
    }
    #endif
  #endif
}
void pwmSetPeriod(unsigned char pr2, unsigned char t2cfg) {
    pwm_init(); PR2 = pr2; T2CON = (unsigned char)(0x04 | (t2cfg & 0x7B));
}
#else
void analogWrite(unsigned char pin, unsigned char value) { (void)pin; (void)value; }
void pwmWrite10(unsigned char pin, unsigned int d) { (void)pin; (void)d; }
void pwmSetPeriod(unsigned char p, unsigned char t) { (void)p; (void)t; }
#endif

/* ===================== Utils ===================== */
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
long constrain(long x, long a, long b) {
    if (x < a) return a; if (x > b) return b; return x;
}

/* ===================== UART / Serial ===================== */
#if defined(HAS_UART)
void Serial_begin(unsigned long baud) {
    core_init();
    unsigned long v = (_XTAL_FREQ / (16UL * baud)) - 1UL;
    SPBRG = (unsigned char)v;
    /* Los pines TX/RX dependen del chip; el modulo EUSART los toma solo,
       pero algunos requieren configurar el TRIS del pin RX como entrada. */
#if defined(TRISC6)
    TRISC6 = 1; TRISC7 = 1;        /* 877A/887: TX=RC6, RX=RC7 */
#elif defined(TRISB2)
    TRISB1 = 1; TRISB2 = 1;        /* 628A/88: TX=RB2, RX=RB1 */
#endif
    BRGH = 1; SYNC = 0; SPEN = 1; TXEN = 1; CREN = 1;
}
void Serial_write(char c) { while (!TXIF) {} TXREG = (unsigned char)c; }
unsigned char Serial_available(void) { return (unsigned char)(RCIF ? 1 : 0); }
int Serial_read(void) {
    if (!RCIF) return -1;
    if (OERR) { CREN = 0; CREN = 1; }
    return (int)RCREG;
}
#else
/* Sin UART: funciones vacias para que los sketches compilen igual */
void Serial_begin(unsigned long baud) { (void)baud; }
void Serial_write(char c) { (void)c; }
unsigned char Serial_available(void) { return 0; }
int Serial_read(void) { return -1; }
#endif
void Serial_print(const char *s) { while (*s) Serial_write(*s++); }
void Serial_println(const char *s) { Serial_print(s); Serial_write('\r'); Serial_write('\n'); }
void Serial_printNumber(long n, unsigned char base) {
    char buf[34]; unsigned char i = 0; unsigned long u;
    if (n < 0 && base == 10) { Serial_write('-'); u = (unsigned long)(-n); }
    else u = (unsigned long)n;
    if (u == 0) { Serial_write('0'); return; }
    while (u > 0) {
        unsigned char d = (unsigned char)(u % base);
        buf[i++] = (char)(d < 10 ? ('0'+d) : ('A'+d-10));
        u /= base;
    }
    while (i > 0) Serial_write(buf[--i]);
}
void Serial_printInt(long n)   { Serial_printNumber(n, 10); }
void Serial_printIntln(long n) { Serial_printNumber(n, 10); Serial_write('\r'); Serial_write('\n'); }

/* Objeto Serial estilo Arduino: conecta los metodos a las funciones. */
const SerialClass Serial = {
    Serial_begin,
    Serial_print,
    Serial_println,
    Serial_printInt,
    Serial_printIntln,
    Serial_write,
    Serial_available,
    Serial_read
};

/* ===================== I2C ===================== */
#if defined(HAS_MSSP)
static void i2c_wait(void) { while ((SSPCON2 & 0x1F) || (SSPSTAT & 0x04)) {} }
void Wire_begin(void) {
    SSPCON = 0x28; SSPCON2 = 0x00;
    SSPADD = (unsigned char)((_XTAL_FREQ / (4UL * 100000UL)) - 1UL);
    SSPSTAT = 0x00; TRISC3 = 1; TRISC4 = 1;
}
void Wire_beginTransmission(unsigned char addr) {
    i2c_wait(); SEN = 1; i2c_wait(); SSPBUF = (unsigned char)(addr << 1); i2c_wait();
}
void Wire_write(unsigned char data) { i2c_wait(); SSPBUF = data; i2c_wait(); }
void Wire_endTransmission(void) { i2c_wait(); PEN = 1; i2c_wait(); }
unsigned char Wire_requestFrom(unsigned char addr, unsigned char n) {
    (void)n; i2c_wait(); SEN = 1; i2c_wait();
    SSPBUF = (unsigned char)((addr << 1) | 1); i2c_wait(); return 1;
}
unsigned char Wire_read(unsigned char ack) {
    i2c_wait(); RCEN = 1; i2c_wait();
    unsigned char d = SSPBUF;
    ACKDT = (unsigned char)(ack ? 0 : 1); ACKEN = 1; i2c_wait();
    return d;
}
void SPI_begin(void) {
    TRISC3 = 0; TRISC5 = 0; TRISC4 = 1; SSPSTAT = 0x40; SSPCON = 0x20;
}
unsigned char SPI_transfer(unsigned char data) {
    SSPBUF = data; while (!SSPSTATbits.BF) {} return SSPBUF;
}
#else
void Wire_begin(void) {}
void Wire_beginTransmission(unsigned char a) { (void)a; }
void Wire_write(unsigned char d) { (void)d; }
void Wire_endTransmission(void) {}
unsigned char Wire_requestFrom(unsigned char a, unsigned char n) { (void)a; (void)n; return 0; }
unsigned char Wire_read(unsigned char ack) { (void)ack; return 0; }
void SPI_begin(void) {}
unsigned char SPI_transfer(unsigned char d) { (void)d; return 0; }
#endif

/* ===================== EEPROM ===================== */
#if defined(EEADR)
unsigned char EEPROM_read(unsigned char addr) {
    EEADR = addr;
  #if defined(EEPGD)
    EEPGD = 0;
  #endif
    RD = 1; return EEDATA;
}
void EEPROM_write(unsigned char addr, unsigned char data) {
    EEADR = addr; EEDATA = data;
  #if defined(EEPGD)
    EEPGD = 0;
  #endif
    WREN = 1;
    INTCONbits.GIE = 0; EECON2 = 0x55; EECON2 = 0xAA; WR = 1; INTCONbits.GIE = 1;
    while (WR) {} WREN = 0;
}
#else
unsigned char EEPROM_read(unsigned char addr) { (void)addr; return 0; }
void EEPROM_write(unsigned char addr, unsigned char data) { (void)addr; (void)data; }
#endif

/* ===================== Interrupciones ===================== */
void attachInterrupt_INT0(void (*fn)(void), unsigned char mode) {
    user_int0 = fn;
#if defined(TRISB0)
    TRISB0 = 1;                 /* RB0 es la entrada INT en 877A/88/628A */
#elif defined(TRISA2)
    TRISA2 = 1;                 /* en 16F690 la INT externa esta en RA2 */
#endif
    OPTION_REGbits.INTEDG = (mode == RISING) ? 1 : 0;
    CORE_INTF = 0;
    CORE_INTE = 1;
    INTCONbits.GIE = 1;
}
void enableInterrupts(void)  { INTCONbits.GIE = 1; }
void disableInterrupts(void) { INTCONbits.GIE = 0; }
