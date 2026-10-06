/*
 * Servo.h - Control de servomotor para el core PIC (estilo Arduino).
 *
 * Equivalente a la libreria Servo de Arduino, en C. Esta es una libreria de
 * "instancia unica" por servo: se usa como objeto estilo Uno y el
 * transpilador (Nivel 2) convierte la sintaxis de objeto a estas funciones.
 *
 * Uso estilo Arduino (lo que ESCRIBE el usuario en el sketch):
 *   Servo miServo;
 *   void setup() { miServo.attach(9); }
 *   void loop()  { miServo.write(90); delay(500); miServo.write(0); }
 *
 * El transpilador lo convierte a:
 *   Servo_attach(9);  Servo_write(90);  ...
 *
 * Genera el pulso de 1.0-2.0 ms (0-180 grados) por software (bit-banging),
 * asi funciona en cualquier pin de cualquier PIC (solo necesita GPIO).
 */
#ifndef SERVO_H
#define SERVO_H

void Servo_attach(unsigned char pin);   /* asocia el servo a un pin */
void Servo_write(unsigned char angle);  /* 0..180 grados */
void Servo_writeMicroseconds(unsigned int us); /* ancho de pulso directo */
void Servo_detach(void);

#endif
