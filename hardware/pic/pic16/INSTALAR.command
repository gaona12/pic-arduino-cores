#!/bin/zsh
# INSTALAR.command - Doble clic en Finder para instalar el core PIC16.
# Mueve el core a la carpeta correcta de Arduino y ejecuta la instalacion
# de dependencias (XC8, DFP, pyserial).

cd "$(dirname "$0")"

echo "============================================"
echo "  Core PIC16 para Arduino IDE - Instalador"
echo "============================================"
echo ""

DEST="$HOME/Documents/Arduino/hardware/pic/pic16"
HERE="$(pwd)"

# Si no estamos ya en el destino, copiar ahi.
if [[ "$HERE" != "$DEST" ]]; then
    echo "Copiando el core a $DEST ..."
    mkdir -p "$HOME/Documents/Arduino/hardware/pic"
    rm -rf "$DEST"
    cp -R "$HERE" "$DEST"
    echo "[OK] Core copiado."
    cd "$DEST"
fi

# Permisos de ejecucion a los scripts
chmod +x "$DEST"/*.sh "$DEST"/*.command 2>/dev/null

# Ejecutar la instalacion de dependencias
zsh "$DEST/instalar.sh"

echo ""
echo "Puedes cerrar esta ventana. Abre el Arduino IDE y elige la placa"
echo "PIC16F877A en Tools > Board."
echo ""
read "?Presiona ENTER para salir..."
