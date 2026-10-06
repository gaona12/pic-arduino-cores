# Roadmap

> [🇬🇧 English](#english) · [🇪🇸 Español](#español)

<a name="english"></a>
## 🇬🇧 English

### Done ✅
- PIC16 core (10 chips) + PIC18 core (12 chips), all peripherals.
- Arduino-Uno syntax via transpiler — Level 1 (core objects) + Level 2 (library objects).
- Library system with per-chip compatibility checker (`libcheck`).
- 100% C ecosystem (transpiler, uploader, checker auto-compile with clang).
- Verified flashing on PIC16F877A (PICkit 2 + USB bootloader).

### Next 🔜 (help wanted)
- [ ] **Hardware testing** on the other 21 chips.
- [ ] **Board Manager package** (`package_pic_index.json`) for one-click install.
- [ ] **Windows / Linux** build wrappers.
- [ ] **More ported libraries**: OneWire, DHT, NeoPixel-style, HC-SR04, DS18B20.
- [ ] **Transpiler Level 3 — full C++ → C translator (OPEN GOAL).** The big prize:
      a complete C++-to-plain-C translator (Cfront-style) so that **existing Arduino
      libraries can be reused at 100%**, unmodified. This would handle classes with
      per-object state, multiple instances, inheritance and templates by lowering them
      to C structs + function pointers that XC8 can compile.
      **This project is NOT locked to C-only tooling for this task.** The current
      ecosystem is 100% C for the runtime pieces (transpiler/uploader/checker), but the
      full C++→C translator MAY be built in **Python or any other language** if that
      gets us a correct, complete interpreter faster. Correctness and completeness come
      first here. Contributions in any language are welcome — open an issue to discuss
      the design before starting.
- [ ] **PIC12F** family support (needs another DFP).

### Non-goals
- A native C++ compiler/back-end that emits PIC machine code directly (that's a
  multi-year effort; XC8 already does C→PIC and we reuse it).
- **Note:** a C++→C *translator* (so XC8 still does the final C→PIC step) is NOT a
  non-goal — it's the open goal above, and it may use Python or any other language.

---

<a name="español"></a>
## 🇪🇸 Español

### Hecho ✅
- Core PIC16 (10 chips) + core PIC18 (12 chips), todos los periféricos.
- Sintaxis Arduino Uno vía transpilador — Nivel 1 (objetos del core) + Nivel 2 (objetos de librería).
- Sistema de librerías con verificador de compatibilidad por chip (`libcheck`).
- Ecosistema 100% en C (transpilador, cargador, verificador se auto-compilan con clang).
- Grabado verificado en PIC16F877A (PICkit 2 + bootloader USB).

### Lo que sigue 🔜 (buscamos ayuda)
- [ ] **Pruebas en hardware** de los otros 21 chips.
- [ ] **Paquete de Board Manager** (`package_pic_index.json`) para instalar de un clic.
- [ ] **Wrappers para Windows / Linux**.
- [ ] **Más librerías portadas**: OneWire, DHT, tipo NeoPixel, HC-SR04, DS18B20.
- [ ] **Transpilador Nivel 3 — intérprete C++ → C completo (META ABIERTA).** El gran
      objetivo: un traductor completo de C++ a C normal (estilo Cfront) para poder
      **reutilizar al 100% las librerías ya hechas para Arduino**, sin modificarlas.
      Manejaría clases con estado por objeto, múltiples instancias, herencia y templates,
      bajándolos a `struct` + punteros a función en C que XC8 sí compila.
      **Este proyecto NO está cerrado a usar solo C para esta tarea.** El ecosistema
      actual es 100% C en las piezas de runtime (transpilador/cargador/verificador), pero
      el traductor C++→C completo PUEDE construirse en **Python o cualquier otro lenguaje**
      si eso nos da un intérprete correcto y completo más rápido. Aquí lo primero es la
      correctitud y la cobertura total. Se aceptan aportes en cualquier lenguaje — abre un
      issue para discutir el diseño antes de empezar.
- [ ] Soporte de la familia **PIC12F** (necesita otro DFP).

### No-objetivos
- Un compilador/back-end de C++ nativo que genere código máquina de PIC directamente
  (eso son años de trabajo; XC8 ya hace C→PIC y lo reutilizamos).
- **Nota:** un *traductor* C++→C (donde XC8 sigue haciendo el paso final C→PIC) NO es un
  no-objetivo — es la meta abierta de arriba, y puede usar Python o cualquier otro lenguaje.
