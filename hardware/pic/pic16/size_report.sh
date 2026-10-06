#!/bin/zsh
# size_report.sh - Reporta el tamanio del programa para que el IDE lo muestre.
# Cuenta las palabras de programa ocupadas leyendo el .hex de Intel.
#
# $1 = ruta al .hex
# Imonta una linea "Program:NNN bytes" que el regex de platform.txt captura.

HEX="$1"

if [[ ! -f "$HEX" ]]; then
    echo "Program:0 bytes"
    exit 0
fi

# Cuenta bytes de datos (record type 00) en el .hex, excluyendo la zona
# del bootloader (direcciones >= 0x3F00 en el hex = 0x1F80 en words) y
# el config word (0x400E).
python3 - "$HEX" <<'PY'
import sys
total = 0
with open(sys.argv[1]) as f:
    for line in f:
        line=line.strip()
        if not line.startswith(":"): continue
        count = int(line[1:3],16)
        addr  = int(line[3:7],16)
        rtype = int(line[7:9],16)
        if rtype != 0: continue
        # saltar config (0x400E) y zona alta del bootloader
        if addr >= 0x3F00: continue
        total += count
print("Program:%d bytes" % total)
PY
