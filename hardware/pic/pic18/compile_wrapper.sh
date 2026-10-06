#!/bin/zsh
# compile_wrapper.sh - Compila el sketch + core PIC18 con XC8 (autodetectado).
#
# Argumentos (desde platform.txt):
#   $1 = mcpu       (ej. 18F4550)
#   $2 = fcpu
#   $3 = core_path  (cores/pic18)
#   $4 = sketch_cpp
#   $5 = out_elf
#   $6 = platform_path

set -e
MCPU="$1"; FCPU="$2"; CORE="$3"; SKETCH="$4"; OUT="$5"; PLAT="$6"

eval "$("$PLAT/find_tools.sh")"

if [[ -z "$XC8" ]]; then
    echo "ERROR: no se encontro XC8 (xc8-cc). Instala MPLAB XC8." >&2
    exit 1
fi
if [[ -z "$DFP" ]]; then
    echo "ERROR: no se encontro el DFP PIC18Fxxxx. Instala el pack PIC18Fxxxx_DFP." >&2
    exit 1
fi

WORK="$(dirname "$OUT")/pic18_build"
mkdir -p "$WORK"

# Transpilador Arduino->C (auto-compila con clang la primera vez)
TRANS="$PLAT/tools/transpile"
if [[ ! -x "$TRANS" ]]; then
    clang -O2 -o "$TRANS" "$PLAT/tools/transpile.c" 2>/dev/null || true
fi
# El 3er arg (pic_libraries) activa el Nivel 2: traduce la sintaxis de objeto
# de las librerias que declaran un .map (p.ej. Servo miServo; -> Servo_attach).
if [[ -x "$TRANS" ]]; then
    "$TRANS" "$SKETCH" "$WORK/sketch.c" "$PLAT/pic_libraries"
else
    cp "$SKETCH" "$WORK/sketch.c"
fi
cp "$CORE/wiring.c" "$WORK/wiring.c"
cp "$CORE/main.c"   "$WORK/main.c"

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
    "$WORK/sketch.c" "$WORK/wiring.c" "$WORK/main.c" "${EXTRA_LIBS[@]}" \
    -o "$OUT"
