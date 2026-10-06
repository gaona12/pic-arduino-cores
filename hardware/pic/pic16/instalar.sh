#!/bin/zsh
# ============================================================================
# instalar.sh - Instalador automatico del core PIC16 para Arduino IDE (macOS)
#
# Deja todo listo para programar el PIC16F877A desde el Arduino IDE:
#   1. Instala MPLAB XC8 (compilador) si no esta.
#   2. Descarga el DFP (soporte del chip) si no esta.
#   3. Verifica Python3 + pyserial (para grabar por USB).
#   4. Comprueba que el core esta en la carpeta correcta.
#
# Uso:  zsh instalar.sh
# ============================================================================

set -e
echo "============================================"
echo "  Instalador del core PIC16 para Arduino"
echo "============================================"

# ---------- 1. XC8 ----------
XC8=""
for base in "$HOME/microchip/xc8"; do
    [[ -d "$base" ]] && XC8=$(ls -d "$base"/v*/bin/xc8-cc 2>/dev/null | sort -V | tail -1)
done
[[ -z "$XC8" ]] && XC8=$(command -v xc8-cc 2>/dev/null || true)

if [[ -n "$XC8" ]]; then
    echo "[OK] XC8 encontrado: $XC8"
else
    echo "[..] XC8 no encontrado. Descargando instalador oficial de Microchip..."
    XC8_URL="https://ww1.microchip.com/downloads/aemDocuments/documents/DEV/ProductDocuments/SoftwareTools/xc8-v3.00-full-install-macos-x64-installer.dmg"
    TMP_DMG="/tmp/xc8_installer.dmg"
    echo "    (esto puede tardar varios minutos, ~200MB)"
    curl -L -o "$TMP_DMG" "$XC8_URL" || {
        echo "[ERROR] No se pudo descargar XC8."
        echo "        Descargalo manualmente de:"
        echo "        https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers/xc8"
        echo "        e instalalo en ~/microchip/xc8/"
        exit 1
    }
    echo "    Montando e instalando (puede pedir tu contrasena)..."
    MP=$(hdiutil attach "$TMP_DMG" | grep -o '/Volumes/.*' | head -1)
    INST=$(find "$MP" -name "installbuilder.sh" | head -1)
    "$INST" --mode unattended --unattendedmodeui minimal --prefix "$HOME/microchip/xc8/v3.00"
    hdiutil detach "$MP" >/dev/null 2>&1 || true
    rm -f "$TMP_DMG"
    echo "[OK] XC8 instalado en ~/microchip/xc8/"
fi

# ---------- 2. DFP PIC16Fxxx ----------
DFP=""
for d in "$HOME/.mchp_packs/Microchip/PIC16Fxxx_DFP"/*/xc8; do
    [[ -d "$d" ]] && DFP="$d"
done

if [[ -n "$DFP" ]]; then
    echo "[OK] DFP encontrado: $DFP"
else
    echo "[..] DFP PIC16Fxxx no encontrado. Descargando..."
    DFP_VER="1.9.176"
    DFP_DIR="$HOME/.mchp_packs/Microchip/PIC16Fxxx_DFP/$DFP_VER"
    mkdir -p "$DFP_DIR"
    curl -L -o "$DFP_DIR/pack.atpack" \
        "https://packs.download.microchip.com/Microchip.PIC16Fxxx_DFP.$DFP_VER.atpack"
    (cd "$DFP_DIR" && unzip -oq pack.atpack && rm -f pack.atpack)
    echo "[OK] DFP instalado en $DFP_DIR"
fi

# ---------- 3. Python + pyserial ----------
PY=""
for cand in /Library/Frameworks/Python.framework/Versions/*/bin/python3 \
            /opt/homebrew/bin/python3 /usr/local/bin/python3 /usr/bin/python3; do
    [[ -x "$cand" ]] && "$cand" -c "import serial" 2>/dev/null && PY="$cand" && break
done
if [[ -n "$PY" ]]; then
    echo "[OK] Python con pyserial: $PY"
else
    echo "[..] Instalando pyserial..."
    python3 -m pip install --user pyserial && echo "[OK] pyserial instalado" || \
        echo "[AVISO] Instala pyserial a mano: pip3 install pyserial"
fi

# ---------- 4. Ubicacion del core ----------
HW="$HOME/Documents/Arduino/hardware/pic/pic16"
if [[ -f "$HW/boards.txt" ]]; then
    echo "[OK] Core instalado en $HW"
else
    echo "[AVISO] El core deberia estar en:"
    echo "        $HW"
    echo "        Copia la carpeta 'pic' ahi y reinicia el Arduino IDE."
fi

echo ""
echo "============================================"
echo "  Listo. Abre (o reinicia) el Arduino IDE:"
echo "   Tools > Board > PIC16F877A (JT40P)"
echo "============================================"
