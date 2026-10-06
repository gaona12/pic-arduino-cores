// 09_LCD_HolaMundo - Mostrar texto en un LCD 16x2 (HD44780) en modo 4 bits
//
// Conexion por defecto (pines Arduino del core):
//   LCD RS -> pin 8  (RB0)
//   LCD EN -> pin 9  (RB1)
//   LCD D4 -> pin 10 (RB2)
//   LCD D5 -> pin 11 (RB3)
//   LCD D6 -> pin 12 (RB4)
//   LCD D7 -> pin 13 (RB5)
//   LCD RW -> GND
//   LCD VSS -> GND, VDD -> 5V, V0 -> potenciometro de contraste
//
// Si usas otros pines, llama lcd_config(rs,en,d4,d5,d6,d7) antes de lcd_begin().

#include "lcd.h"

void setup() {
  lcd_begin();
  lcd_setCursor(0, 0);
  lcd_print("Hola Mundo!");
  lcd_setCursor(0, 1);
  lcd_print("PIC16F877A");
}

void loop() {
  // contador en la segunda linea
  static long n = 0;
  lcd_setCursor(10, 1);
  lcd_printInt(n);
  lcd_print("   ");
  n++;
  delay(1000);
}
