// 11_LCD_I2C - LCD 16x2 por I2C (modulo PCF8574)
//
// Muestra texto en un LCD 16x2 conectado por I2C con un backpack PCF8574.
// Solo 2 cables de datos (SDA/SCL) en vez de 6.
//
// Libreria: LiquidCrystal_I2C  (ver pic_libraries/LiquidCrystal_I2C)
//   requires=MSSP  -> necesita I2C por hardware.
//   NO compila en 16F628A ni 16F84A (no tienen MSSP); el verificador avisa.
//
// Cableado: SDA=RC4, SCL=RC3 con pull-ups de ~4.7k a VCC.
// Direccion tipica del modulo: 0x27 (algunos usan 0x3F).

#include "LiquidCrystal_I2C.h"

void setup() {
  lcdi2c_begin(0x27);          // direccion del backpack PCF8574
  lcdi2c_backlight(1);         // enciende la retroiluminacion
  lcdi2c_print("Hola Uno!");   // primera linea
  lcdi2c_setCursor(0, 1);      // columna 0, fila 1
  lcdi2c_print("PIC + I2C");
}

void loop() {
  // Muestra un contador en la segunda linea
  static long n = 0;
  lcdi2c_setCursor(10, 1);
  lcdi2c_printInt(n++);
  delay(1000);
}
