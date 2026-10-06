/*
 * main.c - Punto de entrada del core PIC16. Llama a setup() una vez
 *          y luego loop() para siempre, igual que Arduino.
 */
#include "Arduino.h"

void main(void) {
    setup();
    while (1) {
        loop();
    }
}
