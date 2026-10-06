// 04_FadePWM - Variar el brillo de un LED con PWM
// Conecta un LED (con resistencia) al pin PWM1 (RC2 / CCP1).

void setup() {
  // nada que configurar; analogWrite inicia el PWM solo
}

void loop() {
  // sube el brillo 0 -> 255
  for (int b = 0; b <= 255; b++) {
    analogWrite(PWM1, (unsigned char)b);
    delay(5);
  }
  // baja el brillo 255 -> 0
  for (int b = 255; b >= 0; b--) {
    analogWrite(PWM1, (unsigned char)b);
    delay(5);
  }
}
