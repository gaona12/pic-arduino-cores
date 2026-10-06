/* main.c - Punto de entrada del core PIC18. setup() una vez, loop() siempre. */
#include "Arduino.h"

void main(void) {
    setup();
    while (1) loop();
}
