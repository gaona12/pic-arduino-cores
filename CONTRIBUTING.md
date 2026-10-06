# Contributing / Cómo colaborar

> [🇬🇧 English](#english) · [🇪🇸 Español](#español)

<a name="english"></a>
## 🇬🇧 English

Thanks for your interest! This is a community project and all help is welcome —
you don't need to be an expert.

### Ways to help

1. **Test on real hardware.** We've only verified flashing on a PIC16F877A. If you
   own any of the other 21 chips, flash an example and report back by opening an
   [issue](../../issues) with the chip, programmer (PICkit/bootloader) and result.
2. **Port a library to C.** Single-instance libraries (one LCD, one sensor) are the
   easy wins — see "Porting a library" below.
3. **Windows / Linux support.** The build wrappers are zsh shell scripts; porting
   them to `.bat`/`bash` would open the project to more people.
4. **Board Manager package.** Package the cores as a `package_pic_index.json` so
   users can install with one click like the ESP32.
5. **Docs, examples, typo fixes.** Always appreciated.

### Dev setup

- macOS with the Arduino IDE (or `arduino-cli`).
- MPLAB XC8 installed (`hardware/pic/pic16/instalar.sh` can fetch it).
- A C compiler (`clang`, already on macOS) — the tools auto-compile on first build.

To test a change:
```bash
arduino-cli compile --fqbn pic:pic16:jt40p path/to/YourSketch
```

### Porting a library (single-instance)

A library lives in `hardware/pic/<core>/pic_libraries/MyLib/` with four files:

| File | Purpose |
|------|---------|
| `MyLib.h` / `MyLib.c` | the C implementation (uses the core HAL: `digitalWrite`, `Wire_*`, ...) |
| `MyLib.compat` | per-chip compatibility manifest (`requires=`, `min_ram=`, `chips_excluidos=`) |
| `MyLib.map` | tells the transpiler how to translate object syntax (`class=`, `prefix=`, optional `ctor=`) |

With the `.map`, a sketch written as `MyLib obj; obj.method(x);` is auto-translated
to `MyLib_method(x);`. See `pic_libraries/Servo/` as a complete example.

> **Known limit:** the transpiler translates *syntax*, not per-object *state*. A
> single global instance works; multiple instances of the same class share state
> unless the C library is designed with an explicit handle/id parameter.

### Pull requests

- Keep the **shipping runtime** (transpiler, uploader, checker that users run) in C and
  dependency-light, so a user only needs XC8. **Exception:** the ambitious full
  **C++→C translator** (to reuse Arduino libraries at 100%) MAY be written in Python or
  any language — correctness and completeness come first there. See ROADMAP.
- One feature/fix per PR; describe what you tested and on which chip.
- Be kind. We're all learning embedded here.

---

<a name="español"></a>
## 🇪🇸 Español

¡Gracias por tu interés! Este es un proyecto comunitario y toda ayuda es bienvenida —
no necesitas ser experto.

### Formas de ayudar

1. **Probar en hardware real.** Solo verificamos el grabado en un PIC16F877A. Si tienes
   alguno de los otros 21 chips, graba un ejemplo y cuéntanos abriendo un
   [issue](../../issues) con el chip, el programador (PICkit/bootloader) y el resultado.
2. **Portar una librería a C.** Las librerías de instancia única (un LCD, un sensor) son
   lo más fácil — mira "Portar una librería" abajo.
3. **Soporte Windows / Linux.** Los wrappers de compilación son scripts zsh; portarlos a
   `.bat`/`bash` abriría el proyecto a más gente.
4. **Paquete de Board Manager.** Empaquetar los cores como un `package_pic_index.json`
   para instalar con un clic, como el ESP32.
5. **Documentación, ejemplos, erratas.** Siempre se agradece.

### Entorno de desarrollo

- macOS con el Arduino IDE (o `arduino-cli`).
- MPLAB XC8 instalado (`hardware/pic/pic16/instalar.sh` lo descarga).
- Un compilador C (`clang`, ya viene en macOS) — las herramientas se auto-compilan en el
  primer build.

Para probar un cambio:
```bash
arduino-cli compile --fqbn pic:pic16:jt40p ruta/a/TuSketch
```

### Portar una librería (instancia única)

Una librería vive en `hardware/pic/<core>/pic_libraries/MiLib/` con cuatro archivos:

| Archivo | Para qué |
|---------|----------|
| `MiLib.h` / `MiLib.c` | la implementación en C (usa la HAL del core: `digitalWrite`, `Wire_*`, ...) |
| `MiLib.compat` | manifiesto de compatibilidad por chip (`requires=`, `min_ram=`, `chips_excluidos=`) |
| `MiLib.map` | le dice al transpilador cómo traducir la sintaxis de objeto (`class=`, `prefix=`, `ctor=` opcional) |

Con el `.map`, un sketch escrito como `MiLib obj; obj.metodo(x);` se traduce solo a
`MiLib_metodo(x);`. Mira `pic_libraries/Servo/` como ejemplo completo.

> **Límite conocido:** el transpilador traduce la *sintaxis*, no el *estado* por objeto.
> Una instancia global única funciona; varias instancias de la misma clase comparten
> estado, a menos que la librería en C se diseñe con un parámetro explícito de handle/id.

### Pull requests

- Mantén el **runtime que se entrega** (transpilador, cargador, verificador que el
  usuario ejecuta) en C y con pocas dependencias, para que el usuario solo necesite XC8.
  **Excepción:** el ambicioso **traductor C++→C completo** (para reutilizar librerías de
  Arduino al 100%) PUEDE escribirse en Python o cualquier lenguaje — ahí lo primero es la
  correctitud y la cobertura total. Ver ROADMAP.
- Una función/arreglo por PR; describe qué probaste y en qué chip.
- Sé amable. Aquí todos estamos aprendiendo sistemas embebidos.
