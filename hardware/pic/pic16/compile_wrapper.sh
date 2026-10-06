#!/bin/zsh
# compile_wrapper.sh - Compila el sketch + core PIC16 con XC8 (autodetectado).
#
# Argumentos (desde platform.txt):
#   $1 = mcpu       (ej. 16F877A)
#   $2 = fcpu       (ej. 20000000UL)
#   $3 = core_path  (carpeta cores/pic16)
#   $4 = sketch_cpp (.ino.cpp del sketch)
#   $5 = out_elf    (salida)
#   $6 = platform_path (carpeta del core, para hallar find_tools.sh)

set -e

MCPU="$1"; FCPU="$2"; CORE="$3"; SKETCH="$4"; OUT="$5"; PLAT="$6"

# Autodetectar herramientas
eval "$("$PLAT/find_tools.sh")"

if [[ -z "$XC8" ]]; then
    echo "ERROR: no se encontro XC8 (xc8-cc). Instala MPLAB XC8." >&2
    exit 1
fi
if [[ -z "$DFP" ]]; then
    echo "ERROR: no se encontro el DFP PIC16Fxxx. Instala el pack PIC16Fxxx_DFP." >&2
    exit 1
fi

WORK="$(dirname "$OUT")/pic16_build"
mkdir -p "$WORK"

# --- Transpilador Arduino->C (en C, parte del ecosistema del core) ---
# Se auto-compila la primera vez con el clang del sistema; luego se reutiliza.
TRANS="$PLAT/tools/transpile"
if [[ ! -x "$TRANS" ]]; then
    clang -O2 -o "$TRANS" "$PLAT/tools/transpile.c" 2>/dev/null || true
fi

# XC8 solo compila .c. Pasamos el sketch por el transpilador.
# El 3er arg (pic_libraries) activa el Nivel 2: traduce la sintaxis de objeto
# de las librerias que declaran un .map (p.ej. Servo miServo; -> Servo_attach).
if [[ -x "$TRANS" ]]; then
    "$TRANS" "$SKETCH" "$WORK/sketch.c" "$PLAT/pic_libraries"
else
    cp "$SKETCH" "$WORK/sketch.c"   # fallback: sin transpilar
fi
cp "$CORE/wiring.c" "$WORK/wiring.c"
cp "$CORE/main.c"   "$WORK/main.c"
cp "$CORE/lcd.c"    "$WORK/lcd.c"

# --- Deteccion de librerias del core: #include "Lib.h" ---
# Verifica compatibilidad con el chip y agrega el .c de la libreria.
LIBDIR="$PLAT/pic_libraries"
LIBCHECK="$PLAT/tools/libcheck"
EXTRA_LIBS=()
EXTRA_INC=(-I"$CORE")
if [[ -d "$LIBDIR" ]]; then
    [[ -x "$LIBCHECK" ]] || clang -O2 -o "$LIBCHECK" "$PLAT/tools/libcheck.c" 2>/dev/null || true
    # buscar includes del sketch que coincidan con una libreria del core
    for lib in "$LIBDIR"/*/; do
        libname=$(basename "$lib")
        if grep -qE "#include[[:space:]]*[\"<]${libname}\.h[\">]" "$SKETCH"; then
            # verificar compatibilidad con el chip
            compat="$lib/$libname.compat"
            if [[ -x "$LIBCHECK" && -f "$compat" ]]; then
                # libcheck sale con codigo !=0 si es incompatible; no dejes que
                # 'set -e' mate el script antes de poder mostrar el mensaje.
                msg=$("$LIBCHECK" "$MCPU" "$compat") || true
                echo "$msg"
                if echo "$msg" | grep -qi "INCOMPATIBLE"; then
                    echo "ERROR: la libreria $libname no es compatible con el $MCPU." >&2
                    exit 1
                fi
            fi
            # agregar el .c de la libreria si existe
            if [[ -f "$lib/$libname.c" ]]; then
                cp "$lib/$libname.c" "$WORK/$libname.c"
                EXTRA_LIBS+=("$WORK/$libname.c")
                EXTRA_INC+=(-I"$lib")
            fi
        fi
    done
fi

"$XC8" -mcpu="$MCPU" -mdfp="$DFP" -D_XTAL_FREQ="$FCPU" \
    "${EXTRA_INC[@]}" \
    "$WORK/sketch.c" "$WORK/wiring.c" "$WORK/main.c" "$WORK/lcd.c" "${EXTRA_LIBS[@]}" \
    -o "$OUT"
