#!/bin/zsh
# make_board_package.sh - Empaqueta los cores PIC y genera package_pic_index.json
# para instalar desde el Board Manager del Arduino IDE (como el ESP32).
#
# Produce en board-manager/:
#   - pic16-<ver>.tar.bz2   (core PIC16)
#   - pic18-<ver>.tar.bz2   (core PIC18)
#   - package_pic_index.json
#
# El JSON lleva URLs que apuntan a los assets de un GitHub Release. Sube los
# .tar.bz2 como adjuntos de un Release y el JSON ya los referenciara.
#
# Uso:
#   zsh tools/make_board_package.sh [GITHUB_USER] [REPO] [RELEASE_TAG]
# Por defecto: gaona12 / pic-arduino-cores / v1.0.0
#
set -u
SCRIPT_DIR="${0:A:h}"
REPO_ROOT="${SCRIPT_DIR:h}"
SRC="$REPO_ROOT/hardware/pic"
OUT="$REPO_ROOT/board-manager"

GH_USER="${1:-gaona12}"
GH_REPO="${2:-pic-arduino-cores}"
TAG="${3:-v1.0.0}"
BASE_URL="https://github.com/$GH_USER/$GH_REPO/releases/download/$TAG"

PIC16_VER=$(grep -E '^version=' "$SRC/pic16/platform.txt" | head -1 | cut -d= -f2)
PIC18_VER=$(grep -E '^version=' "$SRC/pic18/platform.txt" | head -1 | cut -d= -f2)
: "${PIC16_VER:=1.1.0}"
: "${PIC18_VER:=1.0.0}"

rm -rf "$OUT"; mkdir -p "$OUT"
STAGE=$(mktemp -d)

echo "Empaquetando cores..."
# Arduino espera que el contenido del tar este dentro de una carpeta raiz.
# Usamos "pic16-<ver>" / "pic18-<ver>" como esa carpeta.
cp -R "$SRC/pic16" "$STAGE/pic16-$PIC16_VER"
cp -R "$SRC/pic18" "$STAGE/pic18-$PIC18_VER"
# limpiar binarios/basura que no deben viajar
find "$STAGE" \( -name 'transpile' -o -name 'libcheck' -o -name 'tinybld_load' \
     -o -name '*_build' -o -name '*.elf' -o -name '*.o' -o -name '.DS_Store' \) \
     -exec rm -rf {} + 2>/dev/null

TAR16="pic16-$PIC16_VER.tar.bz2"
TAR18="pic18-$PIC18_VER.tar.bz2"
tar -C "$STAGE" -cjf "$OUT/$TAR16" "pic16-$PIC16_VER"
tar -C "$STAGE" -cjf "$OUT/$TAR18" "pic18-$PIC18_VER"
rm -rf "$STAGE"

# checksums y tamanos
sha() { shasum -a 256 "$1" | cut -d' ' -f1; }
sz()  { stat -f%z "$1"; }
S16=$(sha "$OUT/$TAR16"); Z16=$(sz "$OUT/$TAR16")
S18=$(sha "$OUT/$TAR18"); Z18=$(sz "$OUT/$TAR18")

echo "Generando package_pic_index.json..."
cat > "$OUT/package_pic_index.json" <<JSON
{
  "packages": [
    {
      "name": "pic",
      "maintainer": "$GH_USER (PIC Arduino Cores)",
      "websiteURL": "https://github.com/$GH_USER/$GH_REPO",
      "email": "",
      "help": { "online": "https://github.com/$GH_USER/$GH_REPO/issues" },
      "platforms": [
        {
          "name": "PIC16F Boards (XC8)",
          "architecture": "pic16",
          "version": "$PIC16_VER",
          "category": "Contributed",
          "help": { "online": "https://github.com/$GH_USER/$GH_REPO/issues" },
          "url": "$BASE_URL/$TAR16",
          "archiveFileName": "$TAR16",
          "checksum": "SHA-256:$S16",
          "size": "$Z16",
          "boards": [
            { "name": "PIC16F877A (JT40P)" }, { "name": "PIC16F887" },
            { "name": "PIC16F886" }, { "name": "PIC16F876A" },
            { "name": "PIC16F874A" }, { "name": "PIC16F873A" },
            { "name": "PIC16F88" }, { "name": "PIC16F628A" },
            { "name": "PIC16F84A" }, { "name": "PIC16F690" }
          ],
          "toolsDependencies": []
        },
        {
          "name": "PIC18F Boards (XC8)",
          "architecture": "pic18",
          "version": "$PIC18_VER",
          "category": "Contributed",
          "help": { "online": "https://github.com/$GH_USER/$GH_REPO/issues" },
          "url": "$BASE_URL/$TAR18",
          "archiveFileName": "$TAR18",
          "checksum": "SHA-256:$S18",
          "size": "$Z18",
          "boards": [
            { "name": "PIC18F4550" }, { "name": "PIC18F2550" },
            { "name": "PIC18F4455" }, { "name": "PIC18F2455" },
            { "name": "PIC18F4520" }, { "name": "PIC18F2520" },
            { "name": "PIC18F4620" }, { "name": "PIC18F2620" },
            { "name": "PIC18F4525" }, { "name": "PIC18F452" },
            { "name": "PIC18F458" }, { "name": "PIC18F4580" }
          ],
          "toolsDependencies": []
        }
      ],
      "tools": []
    }
  ]
}
JSON

echo ""
echo "=================================================="
echo "Listo. Archivos en: $OUT"
echo "  - $TAR16  ($Z16 bytes)"
echo "  - $TAR18  ($Z18 bytes)"
echo "  - package_pic_index.json"
echo ""
echo "SIGUIENTE PASO:"
echo "  1) Crea un GitHub Release con el tag '$TAG'."
echo "  2) Sube $TAR16 y $TAR18 como adjuntos del Release."
echo "  3) Sube package_pic_index.json (al repo o al Release)."
echo "  4) En Arduino IDE: Preferences -> Additional boards manager URLs ->"
echo "     https://raw.githubusercontent.com/$GH_USER/$GH_REPO/main/board-manager/package_pic_index.json"
echo "=================================================="
