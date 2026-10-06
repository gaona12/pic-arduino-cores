# Installation / Instalación

> [🇬🇧 English](#english) · [🇪🇸 Español](#español)

<a name="english"></a>
## 🇬🇧 English

### Requirements
- macOS (Intel or Apple Silicon). Windows/Linux not supported yet.
- Arduino IDE 2.x or `arduino-cli`.
- MPLAB XC8 compiler (free edition is fine).
- A PIC programmer: a **PICkit 2** (via `pk2cmd`) or a board with a USB bootloader.

### Steps

1. **Place the cores.** Copy the core folders into your Arduino sketchbook so the
   `boards.txt` files land exactly here:
   ```
   ~/Documents/Arduino/hardware/pic/pic16/boards.txt
   ~/Documents/Arduino/hardware/pic/pic18/boards.txt
   ```

2. **Install XC8.** Either run the helper:
   ```bash
   zsh ~/Documents/Arduino/hardware/pic/pic16/instalar.sh
   ```
   or install it yourself from Microchip and the core will auto-detect it
   (`find_tools.sh` looks in the standard locations).

3. **Restart the Arduino IDE.** Then **Tools → Board** → choose your PIC
   (e.g. "PIC16F877A (JT40P)").

4. **Select the port** (for USB bootloader upload): **Tools → Port → /dev/cu.usbserial-...**

5. **Verify / Upload** as usual. For bootloader boards, press RESET when prompted.

### Flashing options
- **USB bootloader (TinyBootloader):** no extra hardware; the chip must already have
  the bootloader. Only pre-tested on the PIC16F877A.
- **PICkit 2 (ICSP):** uses `pk2cmd`. Works for any supported chip. The `-T` flag
  (power target) is required or it writes zeros.

---

<a name="español"></a>
## 🇪🇸 Español

### Requisitos
- macOS (Intel o Apple Silicon). Windows/Linux aún no soportados.
- Arduino IDE 2.x o `arduino-cli`.
- Compilador MPLAB XC8 (la edición gratis sirve).
- Un programador de PIC: un **PICkit 2** (vía `pk2cmd`) o una placa con bootloader USB.

### Pasos

1. **Coloca los cores.** Copia las carpetas de los cores en tu sketchbook de Arduino
   para que los `boards.txt` queden exactamente aquí:
   ```
   ~/Documents/Arduino/hardware/pic/pic16/boards.txt
   ~/Documents/Arduino/hardware/pic/pic18/boards.txt
   ```

2. **Instala XC8.** Ejecuta el ayudante:
   ```bash
   zsh ~/Documents/Arduino/hardware/pic/pic16/instalar.sh
   ```
   o instálalo tú desde Microchip y el core lo detectará solo
   (`find_tools.sh` busca en las rutas estándar).

3. **Reinicia el Arduino IDE.** Luego **Tools → Board** → elige tu PIC
   (p.ej. "PIC16F877A (JT40P)").

4. **Selecciona el puerto** (para grabar por bootloader USB):
   **Tools → Port → /dev/cu.usbserial-...**

5. **Verify / Upload** como siempre. En placas con bootloader, pulsa RESET cuando lo pida.

### Opciones de grabado
- **Bootloader USB (TinyBootloader):** sin hardware extra; el chip ya debe tener el
  bootloader grabado. Solo probado en el PIC16F877A.
- **PICkit 2 (ICSP):** usa `pk2cmd`. Funciona con cualquier chip soportado. El flag `-T`
  (alimentar el target) es obligatorio o graba en ceros.

---

## Publishing the Board Manager package / Publicar el paquete Board Manager

> For maintainers. / Para mantenedores.

🇬🇧 The one-click Board Manager install relies on two things: compressed core
archives hosted on a GitHub Release, and a `package_pic_index.json` that points
to them with their SHA-256 checksums.

🇪🇸 La instalación de un clic por Board Manager depende de dos cosas: los cores
comprimidos alojados en un GitHub Release, y un `package_pic_index.json` que los
referencia con sus checksums SHA-256.

**Steps / Pasos:**

1. Generate the archives and index / Genera los comprimidos y el índice:
   ```bash
   zsh tools/make_board_package.sh gaona12 pic-arduino-cores v1.0.0
   ```
   This creates `board-manager/pic16-*.tar.bz2`, `pic18-*.tar.bz2` and
   `package_pic_index.json`.

2. Create a GitHub Release with tag `v1.0.0` and **upload both `.tar.bz2`** as
   assets. / Crea un Release con el tag `v1.0.0` y **sube los dos `.tar.bz2`**
   como adjuntos.

3. Commit `board-manager/package_pic_index.json` to the repo (on `main`). /
   Haz commit del `package_pic_index.json` en el repo (en `main`).

4. Users add this URL / Los usuarios agregan esta URL:
   ```
   https://raw.githubusercontent.com/gaona12/pic-arduino-cores/main/board-manager/package_pic_index.json
   ```

> If the checksum or size in the JSON doesn't match the uploaded file, Arduino
> refuses to install. Always regenerate the JSON with the script after changing
> a core. / Si el checksum o el tamaño del JSON no coinciden con el archivo
> subido, Arduino se niega a instalar. Regenera siempre el JSON con el script
> tras cambiar un core.
