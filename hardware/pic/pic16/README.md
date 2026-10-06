# Core PIC16 para Arduino IDE — PIC16F877A (placa JT40P)

Programa el **PIC16F877A** desde el **Arduino IDE**, con sketches estilo
`setup()` / `loop()`, compilando con **MPLAB XC8** y grabando por **USB**
con un bootloader (sin necesitar un PICkit cada vez).

Expone **todo el hardware** del PIC16F877A con una API tipo Arduino:
GPIO, ADC, PWM, UART (Serial), I2C, SPI, EEPROM, interrupciones, timers y
una librería LCD 16x2 incluida.

---

## Instalación rápida

1. Descomprime este paquete en:
   `~/Documents/Arduino/hardware/pic/pic16`
   (debe quedar el `boards.txt` en esa ruta exacta).
2. Ejecuta el instalador (instala XC8 + DFP si faltan):
   ```bash
   zsh ~/Documents/Arduino/hardware/pic/pic16/instalar.sh
   ```
3. Abre (o reinicia) el **Arduino IDE**.
4. **Tools → Board → "PIC16F877A (JT40P - bootloader USB)"**.
5. **Tools → Port →** el puerto de tu placa (`/dev/cu.usbserial-XXX`).

> La primera vez, el bootloader debe estar grabado en el PIC. Si es un chip
> nuevo, grábalo una sola vez con un PICkit (ver `bootloader/` y la guía
> `COMO_GRABAR.md`). Después ya cargas por USB desde el IDE.

---

## Uso

Escribe tu sketch, pulsa **Verify** (compilar) y **Upload** (grabar).
Al subir, **pulsa el botón RESET** de la placa cuando lo pida (tienes ~25 s).

Ejemplos listos en **File → Examples → PIC16_Examples**.

---

## API disponible

### Digital
```c
pinMode(pin, OUTPUT|INPUT);
digitalWrite(pin, HIGH|LOW);
digitalRead(pin);
togglePin(pin);
```

### Analógico (ADC 10 bits)
```c
unsigned int v = analogRead(canal);   // canal 0..7 -> AN0..AN7, v = 0..1023
```

### PWM
```c
analogWrite(PWM1, 0..255);     // PWM1 = RC2 (CCP1), PWM2 = RC1 (CCP2)
pwmWrite10(PWM1, 0..1023);     // duty de 10 bits (servos)
pwmSetPeriod(pr2, t2cfg);      // ajustar frecuencia del PWM
```

### Tiempo
```c
delay(ms);
delayMicroseconds(us);
unsigned long t = millis();
unsigned long u = micros();
```

### Serial (UART)
```c
Serial_begin(9600);
Serial_print("texto");
Serial_println("texto");
Serial_printInt(numero);        // decimal
Serial_printIntln(numero);
Serial_printNumber(n, HEX);     // DEC / HEX / BIN
Serial_write('A');
if (Serial_available()) { int c = Serial_read(); }
```

### I2C (SDA=RC4, SCL=RC3)
```c
Wire_begin();
Wire_beginTransmission(addr);
Wire_write(dato);
Wire_endTransmission();
Wire_requestFrom(addr, 1);
unsigned char d = Wire_read(1);  // 1=ACK, 0=NACK (ultimo byte)
```

### SPI (SCK=RC3, SDO=RC5, SDI=RC4)
```c
SPI_begin();
unsigned char r = SPI_transfer(0x55);
```

### EEPROM interna (256 bytes)
```c
EEPROM_write(direccion, valor);   // direccion 0..255
unsigned char v = EEPROM_read(direccion);
```

### Interrupciones
```c
void miISR() { ... }
attachInterrupt_INT0(miISR, FALLING);  // o RISING. Entrada en RB0.
enableInterrupts();
disableInterrupts();
```

### LCD 16x2 (HD44780, modo 4 bits)
```c
#include "lcd.h"
lcd_begin();
lcd_setCursor(col, fila);
lcd_print("Hola");
lcd_printInt(123);
lcd_clear();
// pines por defecto: RS=8, EN=9, D4..D7=10..13 (puerto B)
// cambiar: lcd_config(rs, en, d4, d5, d6, d7) antes de lcd_begin()
```

### Utilidades
```c
map(x, in_min, in_max, out_min, out_max);
constrain(x, a, b);
min(a,b)  max(a,b)  abs(x)
bitRead(v,b)  bitSet(v,b)  bitClear(v,b)  bitWrite(v,b,x)
```

---

## Mapa de pines (número Arduino → pin PIC → header de la placa)

| Arduino | PIC    | Header | Notas                         |
|---------|--------|--------|-------------------------------|
| 0..7    | RD0..RD7 | D0..D7 | salida normal (5V/0V)       |
| 8..15   | RB0..RB7 | B0..B7 | RB0 = interrupcion externa  |
| 16..21  | RA0..RA5 | A0..A5 | RA0..RA5 tambien ADC (AN0..) |
| 22..24  | RE0..RE2 | E0..E2 | tambien ADC (AN5..AN7)      |
| 25..30  | RC0..RC5 | C0..C5 | RC1=PWM2, RC2=PWM1, RC3/4=I2C/SPI |
| —       | RC6/RC7  | TX/RX  | reservados para Serial       |

Canales ADC: `analogRead(0..7)` = AN0..AN7.
PWM: `PWM1` = RC2, `PWM2` = RC1.

---

## Ejemplos incluidos (File → Examples → PIC16_Examples)

| # | Ejemplo | Qué muestra |
|---|---------|-------------|
| 01 | Blink | Parpadeo de un pin |
| 02 | SerialHello | Enviar texto y números por serial |
| 03 | AnalogRead | Leer potenciómetro con el ADC |
| 04 | FadePWM | Variar brillo de un LED con PWM |
| 05 | Servo | Control de servomotor con PWM |
| 06 | EEPROM | Guardar/leer datos no volátiles |
| 07 | Interrupt | Interrupción externa en RB0 |
| 08 | I2C_scan | Escáner de dispositivos I2C |
| 09 | LCD_HolaMundo | Texto en LCD 16x2 (paralelo) |
| 10 | SoftwareSerial | Segundo serial por software en pines GPIO |
| 11 | LCD_I2C | LCD 16x2 por I2C (módulo PCF8574) |

Todos verificados compilando sin errores.

---

## Librerías incluidas (sistema con filtro de compatibilidad)

Las librerías viven en `pic_libraries/`. Cada una trae un manifiesto `.compat`
que declara qué necesita (I2C, RAM, chips excluidos…). Al compilar, el
verificador en C `tools/libcheck` comprueba si la librería es compatible con el
chip elegido: si no lo es, **cancela la compilación con un mensaje claro** en
vez de fallar con un error críptico de XC8.

Para usar una librería basta con incluir su cabecera en el sketch
(`#include "SoftwareSerial.h"`); el core la detecta, verifica y enlaza sola.

| Librería | Incluir | Requiere | Chips donde NO va |
|----------|---------|----------|-------------------|
| SoftwareSerial | `"SoftwareSerial.h"` | solo GPIO | ninguno (todos) |
| LiquidCrystal_I2C | `"LiquidCrystal_I2C.h"` | I2C (MSSP) | 16F628A, 16F84A |
| Wire (I2C) | ya en el core | I2C (MSSP) | 16F628A, 16F88, 16F84A, 16F690 |
| SPI | ya en el core | MSSP | 16F628A, 16F88, 16F84A, 16F690 |
| LiquidCrystal (paralelo) | `"lcd.h"` | solo GPIO + RAM | 16F84A (poca RAM) |

Ejemplo (idéntico a Arduino, se verifica solo al compilar):

```c
#include "LiquidCrystal_I2C.h"   // si el chip no tiene I2C, el IDE avisa y para
void setup() {
  lcdi2c_begin(0x27);
  lcdi2c_print("Hola I2C");
}
void loop() {}
```

Si eliges, por ejemplo, el **PIC16F628A** (sin MSSP) y usas
`LiquidCrystal_I2C.h`, la compilación se detiene con:

```
INCOMPATIBLE: 'LiquidCrystal_I2C' no soporta el 16F628A (excluido explicitamente).
ERROR: la libreria LiquidCrystal_I2C no es compatible con el 16F628A.
```

### Añadir una librería nueva

1. Crea `pic_libraries/MiLib/` con `MiLib.h`, `MiLib.c` y `MiLib.compat`.
2. En el `.compat` declara `name=`, `requires=` (DIGITAL/MSSP/ADC/PWM/UART),
   `min_ram=`, `chips_excluidos=`.
3. Incluye `"MiLib.h"` en tu sketch: el core la detecta y verifica sola.

---

## Sintaxis idéntica al Arduino Uno

Puedes escribir EXACTAMENTE como en el Arduino Uno, incluyendo numeros:

```c
void setup() {
  Serial.begin(9600);
  Serial.println("soy un pic");
}
void loop() {
  Serial.print("ADC = ");
  Serial.println(analogRead(0));   // numero, igual que el Uno
  delay(1000);
}
```

Un transpilador en C (ver abajo) convierte `Serial.print(...)`, `lcd.print(...)`,
`Wire.*`, `SPI.*`, `EEPROM.*` a las funciones del core, detectando si el
argumento es texto o numero. Es transparente: tu escribes estilo Uno.

## Ecosistema 100% en C (sin Python)

- **boards.txt / platform.txt**: definen las placas y las recetas del IDE.
- **cores/pic16/**: capa Arduino (`Arduino.h`, `wiring.c`, `main.c`, `lcd.c`).
- **tools/transpile.c**: transpilador Arduino->C (se auto-compila con clang
  la 1a vez). Traduce la sintaxis de objeto a funciones del core.
- **tools/tinybld_load.c**: cargador del bootloader por USB, en C puro
  (termios), sin pyserial. Se auto-compila con clang.
- **compile_wrapper.sh**: transpila el sketch y lo compila con XC8.
- **find_tools.sh**: autodetecta XC8 y el DFP. Sin rutas fijas → portable.
- **upload_wrapper.sh**: graba el `.hex` con el cargador en C.
- **bootloader/**: firmware Tiny Bootloader (hex) para grabar una vez.

Unica dependencia externa: MPLAB XC8 (compilador). Todo lo demas (transpilador,
cargador) es C que se compila con el clang que ya trae macOS.

---

## Requisitos

- macOS (Intel o Apple Silicon).
- Arduino IDE o arduino-cli.
- MPLAB XC8 (lo instala `instalar.sh`). Es la única dependencia externa.
- No necesita Python: el cargador por USB y el transpilador son C que se
  auto-compilan con el `clang` que ya trae macOS.

> **Nota de licencia:** XC8 es de Microchip y no se puede redistribuir dentro
> de este paquete. Por eso `instalar.sh` lo descarga de Microchip (aceptas sus
> términos al instalarlo), igual que el core del ESP32 descarga su toolchain.

---

## Chips soportados

Todos verificados compilando (config bits leídos de los archivos oficiales
de XC8, no inventados):

| Placa en el IDE | Chip | Pines | Memoria | ADC | PWM | UART | I2C/SPI |
|-----------------|------|-------|---------|-----|-----|------|---------|
| PIC16F877A (JT40P) | 16F877A | 40 | 8K | sí | sí | sí | sí |
| PIC16F887 | 16F887 | 40 | 8K | sí | sí | sí | sí |
| PIC16F886 | 16F886 | 28 | 8K | sí | sí | sí | sí |
| PIC16F628A | 16F628A | 18 | 2K | no | sí | sí | no |
| PIC16F88 | 16F88 | 18 | 4K | sí | sí | sí | no |
| PIC16F84A | 16F84A | 18 | 1K | no | no | no | no |
| PIC16F690 | 16F690 | 20 | 4K | sí | sí | sí | sí |
| PIC16F876A | 16F876A | 28 | 8K | sí | sí | sí | sí |
| PIC16F873A | 16F873A | 28 | 4K | sí | sí | sí | sí |
| PIC16F874A | 16F874A | 40 | 4K | sí | sí | sí | sí |

El core detecta el chip automáticamente (macro de XC8) y activa solo los
periféricos que existen; las funciones no soportadas por un chip quedan como
no-ops seguras (p.ej. `analogRead` en el 16F84A devuelve 0).

> El **grabado por bootloader USB** está probado en el PIC16F877A (placa
> JT40P). Para los demás chips, graba con un PICkit, o adapta el bootloader.

## Limitaciones

- XC8 compila **C**, no C++. Aun así puedes escribir en estilo Uno
  (`Serial.print(...)`, `lcd.print(...)`): el transpilador en C los convierte
  a las funciones del core. Internamente todo es C.
- Compatible con macOS. Para Windows/Linux habría que adaptar los scripts
  `.sh` (es viable, pero no está incluido).
- Los PIC12F de 8 pines requieren otro Device Family Pack; no incluidos aún.
