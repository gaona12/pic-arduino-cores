#!/bin/zsh
# find_tools.sh - Autodetecta XC8 y el DFP PIC16Fxxx en cualquier Mac.
# (El resto del ecosistema es C: transpilador y cargador se compilan con clang.)
setopt NULL_GLOB 2>/dev/null
SELF_DIR="${0:A:h}"

# --- XC8 (xc8-cc) ---
XC8=""
for base in "$SELF_DIR/tools/xc8" "$HOME/microchip/xc8" \
            "/Applications/microchip/xc8" "/opt/microchip/xc8"; do
    if [[ -d "$base" ]]; then
        cand=$(ls -d "$base"/v*/bin/xc8-cc 2>/dev/null | sort -V | tail -1)
        [[ -z "$cand" ]] && cand=$(ls -d "$base"/bin/xc8-cc 2>/dev/null | tail -1)
        [[ -n "$cand" ]] && XC8="$cand" && break
    fi
done
[[ -z "$XC8" ]] && XC8=$(command -v xc8-cc 2>/dev/null)

# --- DFP PIC16Fxxx ---
DFP=""
for base in "$SELF_DIR/tools/dfp/xc8" \
            "$HOME/.mchp_packs/Microchip/PIC16Fxxx_DFP"/*/xc8 \
            /Applications/microchip/mplabx/*/packs/Microchip/PIC16Fxxx_DFP/*/xc8; do
    if [[ -d "$base" ]]; then DFP="$base"; fi
done

echo "XC8=$XC8"
echo "DFP=$DFP"
