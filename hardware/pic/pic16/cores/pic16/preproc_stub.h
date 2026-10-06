/*
 * preproc_stub.h - Solo para la fase de deteccion de librerias del IDE (clang).
 * Hace que <xc.h> y los #pragma config no estorben a clang. NO se usa en la
 * compilacion real (esa la hace XC8 con el xc.h verdadero).
 */
#ifndef PREPROC_STUB_H
#define PREPROC_STUB_H
/* Evita que clang intente abrir <xc.h> durante la deteccion */
#define __XC_H
#define _XC_H_
#endif
