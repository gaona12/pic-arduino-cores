/*
 * board_config.h - Detecta el chip (via macro de XC8) y define que
 * perifericos/puertos tiene disponibles. Asi el mismo core compila para
 * varios PIC, activando solo lo que cada uno soporta.
 *
 * XC8 define automaticamente una macro como _16F877A segun -mcpu.
 */
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/* ---- Familia de 40 pines con todo: A,B,C,D,E + ADC + CCP + MSSP ---- */
#if defined(_16F877A) || defined(_16F874A) || defined(_16F877) || defined(_16F874) || \
    defined(_16F887) || defined(_16F874A)
  #define HAS_PORTA 1
  #define HAS_PORTB 1
  #define HAS_PORTC 1
  #define HAS_PORTD 1
  #define HAS_PORTE 1
  #define HAS_ADC   1
  #define HAS_PWM   1   /* CCP1/CCP2 */
  #define HAS_MSSP  1   /* I2C/SPI */
  #define HAS_UART  1
  #define HAS_CMCON 1

/* ---- 28 pines: A,B,C,E(parcial) + ADC + CCP + MSSP ---- */
#elif defined(_16F876A) || defined(_16F873A) || defined(_16F886) || defined(_16F883)
  #define HAS_PORTA 1
  #define HAS_PORTB 1
  #define HAS_PORTC 1
  #define HAS_ADC   1
  #define HAS_PWM   1
  #define HAS_MSSP  1
  #define HAS_UART  1
  #define HAS_CMCON 1

/* ---- 18 pines con ADC: 16F88 (A,B) ---- */
#elif defined(_16F88) || defined(_16F818) || defined(_16F819)
  #define HAS_PORTA 1
  #define HAS_PORTB 1
  #define HAS_ADC   1
  #define HAS_UART  1   /* 16F88 tiene AUSART */

/* ---- 18 pines sin ADC: 16F628A/16F84A (A,B) ---- */
#elif defined(_16F628A) || defined(_16F628) || defined(_16F648A)
  #define HAS_PORTA 1
  #define HAS_PORTB 1
  #define HAS_UART  1
  #define HAS_CMCON 1

#elif defined(_16F84A) || defined(_16F84)
  #define HAS_PORTA 1
  #define HAS_PORTB 1
  /* sin ADC, sin UART, sin CMCON */

/* ---- 20 pines: 16F690 (A,B,C) + ADC ---- */
#elif defined(_16F690) || defined(_16F685) || defined(_16F687) || defined(_16F689)
  #define HAS_PORTA 1
  #define HAS_PORTB 1
  #define HAS_PORTC 1
  #define HAS_ADC   1
  #define HAS_UART  1

/* ---- 8 pines: 12F675 (solo GPIO) / 12F683 (GPIO+ADC+CCP) ---- */
#elif defined(_12F683)
  #define HAS_GPIO_ONLY 1
  #define HAS_ADC   1
  #define HAS_PWM   1

#elif defined(_12F675) || defined(_12F629)
  #define HAS_GPIO_ONLY 1
  #if defined(_12F675)
    #define HAS_ADC 1
  #endif

/* ---- Desconocido: asumir minimo (A,B) para no romper ---- */
#else
  #define HAS_PORTA 1
  #define HAS_PORTB 1
  #warning "Chip no reconocido en board_config.h: se asume PORTA/PORTB basico."
#endif

/* ---------------------------------------------------------------------------
 * Alias universales para bits de INTCON. Cada familia usa un nombre distinto
 * en INTCONbits (unos T0IF, otros TMR0IF). Detectamos cual existe via los
 * #define _INTCON_..._MASK que XC8 genera en el header del chip.
 * ------------------------------------------------------------------------- */
#if defined(_INTCON_TMR0IF_MASK)
  #define CORE_TMR0IF INTCONbits.TMR0IF
  #define CORE_TMR0IE INTCONbits.TMR0IE
#else
  #define CORE_TMR0IF INTCONbits.T0IF
  #define CORE_TMR0IE INTCONbits.T0IE
#endif

#if defined(_INTCON_INT0IF_MASK)
  #define CORE_INTF INTCONbits.INT0IF
  #define CORE_INTE INTCONbits.INT0IE
#elif defined(_INTCON_INTF_MASK)
  #define CORE_INTF INTCONbits.INTF
  #define CORE_INTE INTCONbits.INTE
#else
  #define CORE_INTF INTCONbits.INTF
  #define CORE_INTE INTCONbits.INTE
#endif

#endif /* BOARD_CONFIG_H */
