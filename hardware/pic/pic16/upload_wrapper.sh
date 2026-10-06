#!/bin/zsh
# upload_wrapper.sh - Graba el .hex por USB con el bootloader, usando el
# cargador en C (tinybld_load). Sin dependencia de Python.
#
# Argumentos:
#   $1 = puerto serie (ej. /dev/cu.usbserial-130)
#   $2 = ruta al .hex
#   $3 = platform_path (carpeta del core)

set -e
PORT="$1"; HEX="$2"; PLAT="$3"

LOADER="$PLAT/tools/tinybld_load"
# Auto-compilar el cargador en C la primera vez (clang viene con macOS)
if [[ ! -x "$LOADER" ]]; then
    clang -O2 -o "$LOADER" "$PLAT/tools/tinybld_load.c" 2>/dev/null || {
        echo "ERROR: no se pudo compilar el cargador (tinybld_load.c)." >&2
        exit 1
    }
fi

exec "$LOADER" -p "$PORT" -f "$HEX"
