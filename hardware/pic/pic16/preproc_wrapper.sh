#!/bin/zsh
# preproc_wrapper.sh - Preprocesado para la deteccion de librerias del IDE.
# Usa clang e incluye el core + todas las carpetas de pic_libraries para que
# encuentre los headers de las librerias (#include "SoftwareSerial.h", etc.)
#
# Args: $1=core_path  $2=fcpu  $3=source  $4=salida  $5=platform_path
setopt NULL_GLOB 2>/dev/null
CORE="$1"; FCPU="$2"; SRC="$3"; OUT="$4"; PLAT="$5"

INCS=(-I"$CORE")
for lib in "$PLAT"/pic_libraries/*/; do
    INCS+=(-I"$lib")
done

/usr/bin/clang -E -x c -nostdinc "${INCS[@]}" \
    -DARDUINO_IDE_LIBDETECT -D_XTAL_FREQ="$FCPU" \
    "$SRC" -o "$OUT"
