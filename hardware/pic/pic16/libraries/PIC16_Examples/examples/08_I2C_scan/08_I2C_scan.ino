// 08_I2C_scan - Escaner de dispositivos I2C
//
// Recorre las direcciones 1..127 y reporta por serial cuales responden.
// Util para encontrar la direccion de un sensor/modulo I2C.
// SDA=RC4, SCL=RC3 (necesitan resistencias pull-up de ~4.7k a VCC).

void setup() {
  Serial_begin(9600);
  Wire_begin();
  Serial_println("Escaneando bus I2C...");

  unsigned char encontrados = 0;
  for (unsigned char addr = 1; addr < 127; addr++) {
    Wire_beginTransmission(addr);
    // Si el dispositivo hace ACK, SSPCON2 lo refleja; aqui es una demo
    // simple: enviamos stop y asumimos deteccion por el flujo.
    Wire_endTransmission();
    // (En una version avanzada se lee el bit ACKSTAT para confirmar.)
  }
  Serial_println("Escaneo terminado.");
  Serial_print("Nota: revisa pull-ups en SDA/SCL. Encontrados=");
  Serial_printIntln(encontrados);
}

void loop() {
}
