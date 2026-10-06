// 06_EEPROM - Guardar y leer datos en la EEPROM interna (256 bytes)
//
// Cuenta cuantas veces se ha reiniciado la placa, guardando el contador
// en la EEPROM (no se pierde al apagar). Lo muestra por serial.

void setup() {
  Serial_begin(9600);

  unsigned char arranques = EEPROM_read(0);   // lee direccion 0
  arranques++;                                 // una vez mas
  EEPROM_write(0, arranques);                  // guarda

  Serial_print("La placa ha arrancado ");
  Serial_printInt(arranques);
  Serial_println(" veces.");
}

void loop() {
  // nada
}
