# Core PIC18F para Arduino IDE (XC8)

Programa microcontroladores **PIC18F** desde el Arduino IDE, con sketches
estilo `setup()`/`loop()`, compilando con MPLAB XC8. Elige la placa en el
menú como un ESP32.

## Chips soportados (12)

| Placa | Chip | Pines | Flash | Notas |
|-------|------|-------|-------|-------|
| PIC18F4550 | 18F4550 | 40 | 32K | USB nativo (el más popular) |
| PIC18F2550 | 18F2550 | 28 | 32K | USB |
| PIC18F4455 | 18F4455 | 40 | 24K | USB |
| PIC18F2455 | 18F2455 | 28 | 24K | USB |
| PIC18F4520 | 18F4520 | 40 | 32K | — |
| PIC18F2520 | 18F2520 | 28 | 32K | — |
| PIC18F4620 | 18F4620 | 40 | 64K | mucha memoria |
| PIC18F2620 | 18F2620 | 28 | 64K | — |
| PIC18F4525 | 18F4525 | 40 | 48K | — |
| PIC18F452  | 18F452  | 40 | 32K | clásico |
| PIC18F458  | 18F458  | 40 | 32K | con CAN |
| PIC18F4580 | 18F4580 | 40 | 32K | con CAN |

Config bits verificados contra los archivos oficiales de XC8 (3 familias:
USB, moderna y clásica).

## Requisitos

- MPLAB XC8 (lo instala `instalar.sh` del core PIC16, o `brew install --cask mplab-xc8`)
- DFP PIC18Fxxxx (se instala con el core; o descárgalo de packs.download.microchip.com)
- No necesita Python: el cargador por USB y el transpilador son C que se
  auto-compilan con el `clang` de macOS.

## Uso

Igual que el core PIC16: eliges la placa en Tools → Board, escribes el
sketch y Verify/Upload.

```c
void setup() {
  Serial_begin(9600);
  pinMode(0, OUTPUT);         // pin 0 = RD0
  Serial_println("PIC18F listo!");
}
void loop() {
  digitalWrite(0, HIGH); delay(500);
  digitalWrite(0, LOW);  delay(500);
  Serial_printInt(analogRead(0));
}
```

## API (completa, igual que el core PIC16)

```c
// Digital
pinMode/digitalWrite/digitalRead
// Tiempo
delay/delayMicroseconds
// Analogico
analogRead(canal)              // ADC 10 bits
analogWrite(PWM1, 0..255)      // PWM (CCP1/CCP2)
// Serial
Serial_begin/print/println/printInt/write
// I2C (SDA=RC4, SCL=RC3)
Wire_begin/beginTransmission/write/endTransmission/requestFrom/read
// SPI (SCK=RC3, SDO=RC5, SDI=RC4)
SPI_begin/transfer
// EEPROM interna
EEPROM_read/EEPROM_write
// LCD 16x2 (RS=8, EN=9, D4..D7=10..13)
lcd_begin/clear/setCursor/print/printInt
// Utils
map/constrain, bitRead/Set/Clear
```

Mapa de pines (40 pines): 0..7=RD, 8..15=RB, 16..21=RA, 22..24=RE, 25..31=RC.
En chips de 28 pines (2550/2520/2620/2455) solo hay PORTA/B/C.

## Librerías incluidas (sistema con filtro de compatibilidad)

Igual que el core PIC16: las librerías viven en `pic_libraries/`, cada una con
un manifiesto `.compat`. Al compilar, el verificador en C `tools/libcheck`
comprueba la compatibilidad con el chip y, si no encaja, detiene la compilación
con un mensaje claro. Para usar una librería, incluye su cabecera en el sketch;
el core la detecta, verifica y enlaza sola.

| Librería | Incluir | Requiere | Notas |
|----------|---------|----------|-------|
| SoftwareSerial | `"SoftwareSerial.h"` | solo GPIO | compatible con todos los PIC18 |
| LiquidCrystal_I2C | `"LiquidCrystal_I2C.h"` | I2C (MSSP) | todos los PIC18 lo tienen |
| Wire (I2C) | ya en el core | I2C (MSSP) | — |
| SPI | ya en el core | MSSP | — |

Ejemplo:

```c
#include "SoftwareSerial.h"
#include "LiquidCrystal_I2C.h"
void setup() {
  SoftwareSerial_begin(0, 1, 9600);
  lcdi2c_begin(0x27);
  lcdi2c_print("Hola I2C");
}
void loop() {}
```

Todos los PIC18 de este core tienen MSSP, así que las librerías de I2C son
compatibles con los 12 chips. El filtro sigue activo por si añades un chip sin
el periférico, o una librería con otros requisitos.

## Grabado

- Por **PICkit** (ICSP) con pk2cmd — recomendado para PIC18.
- Por **bootloader USB** si el chip tiene uno compatible grabado.
  (Nota: el bootloader incluido es para PIC16F877A; para PIC18 con USB
  nativo como el 4550 existen bootloaders USB-HID dedicados.)

## Limitaciones

- Validado en compilación; grabado en hardware pendiente de probar.
- Solo macOS por ahora (scripts .sh).
- Los chips de 28 pines (2550/2520/2620/2455) no tienen PORTD/E; el LCD usa
  pines de PORTB (RB0..RB5), que sí tienen.
