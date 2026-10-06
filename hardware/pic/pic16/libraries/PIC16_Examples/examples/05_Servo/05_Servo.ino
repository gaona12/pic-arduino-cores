// 05_Servo - Control de un servomotor con PWM (pin PWM1 = RC2)
//
// Un servo se controla con pulsos de 1ms (0 grados) a 2ms (180 grados)
// cada 20ms. Ajustamos el periodo del PWM a ~20ms (50Hz) y el ancho de
// pulso con duty de 10 bits.
//
// Con Fosc=20MHz, Timer2 prescaler 1:16, PR2=255 -> periodo ~ 3.3ms (no 20ms).
// Para 50Hz real se requiere mas prescaler; aqui damos un barrido de demo
// usando pwmSetPeriod + pwmWrite10. Ajusta los valores a tu servo.

void setup() {
  // Periodo grande para acercarnos a 50Hz: PR2=255, Timer2 1:16, postscale.
  // t2cfg: bits <6:3> postscale, <1:0> prescale. 0x7A -> post 16, pre 16.
  pwmSetPeriod(255, 0x7A);
}

void loop() {
  // Barrido aproximado: valores de duty tipicos para servo (ajustar).
  pwmWrite10(PWM1, 30);    // ~ posicion minima
  delay(1000);
  pwmWrite10(PWM1, 75);    // ~ posicion media
  delay(1000);
  pwmWrite10(PWM1, 120);   // ~ posicion maxima
  delay(1000);
}
