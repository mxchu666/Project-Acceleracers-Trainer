# Project Acceleracers Trainer — Recode (Español)

DLL de trainer interna para **Project Acceleracers** (UE4 4.27), escrita en C++ con un
overlay ImGui sobre D3D11 que trae **cinco renderizadores de menú**: el menú propio del
trainer más cuatro paquetes de UI de terceros de la carpeta local `imGUIS/`, integrados
**byte a byte** y conmutables en tiempo de ejecución.

> Herramienta para un jugador. Proyecto educativo. Sin funcionalidades multijugador, sin
> evasión de anti-cheat, nada empaquetado ni oculto — el código fuente compila tal cual.

---

## Funciones

### Jugador
- Nitrox ilimitado (se rellena en cada frame)
- Jump Jets ilimitados (usos del Accelecharger recargados, cooldowns limpiados)
- Manejo Perfecto (fricción máxima de neumáticos, slider de agarre ajustable)
- Cargas del Accelecharger (contador ajustable)
- Boost de HP del Motor (limpia `EngineBlown`)
- Noclip (WASD + Espacio/Ctrl, velocidad ajustable)
- God Mode (inmune a los rayos de Storm, púas y ráfagas de viento)
- Hydro Tires: sin freno en el agua / Spike Tires: sin atasco en la arena

### Progresión
- Desbloquear todos los coches (cada entrada `ECarList` 0–112)
- Desbloquear todos los AcceleChargers (cada entrada `EAccelechargers` 0–16)
- Establecer todos los récords de realm (llama a `SaveGameTime` para los 47 realms)
- Guardar el progreso con el propio `Save_Game` del juego

### Mundo
- Identificación del realm en vivo (nombre `LevelOut` del portal + tabla de dump)
- Completar el realm actual (estaciona el coche dentro del portal de salida)
- Lista de realms con coordenadas y estado
- Diagnóstico del motor: conteos de mundo/niveles/actores/portales, verificación del ID del hilo

Cada renderizador mueve el mismo motor de funciones: los menús solo escriben ajustes y
encolan acciones de un disparo; el trabajo de jugabilidad corre en el hilo del juego
(enganchado vía `UObject::ProcessEvent`) y se rechaza si el trainer llega a reclamar el
hilo equivocado — mira la lectura `GT`/`Render` en la pestaña WORLD del menú Default.

## Renderizadores de menú

| Tecla | Acción |
|---|---|
| **INSERT** | abrir / cerrar el menú activo |
| **HOME** | abrir el selector compacto de renderizadores |

El menú Default además lleva una pestaña **UI** que lista los cinco renderizadores y las
paletas de tema. Los ajustes viven en una configuración compartida, así que los toggles
te siguen si cambias de renderizador en plena sesión.

| # | Renderizador | Origen | Aspecto |
|---|---|---|---|
| 1 | **Default** | trainer original | gris azulado oscuro, acento naranja |
| 2 | **Neverlose CS2** | pack `nl_cs2` (`gui.cpp`, `gui.hpp`) | tarjetas azul marino, acento azul |
| 3 | **Neverlose Legacy** | pack `nl_legacy` (`neverlose_menu.cpp`) | tarjetas azul marino, acento azul, fondo con blur |
| 4 | **Onetap** | `onetap ui remake` (`draw.cpp`, `gui_layers.cpp`, `group_layers.cpp`) | carbón, acento rojo |
| 5 | **Fatal UI** | `project-139` (`FatalWidgets.cpp`, `menu/Menu.cpp`) | carbón, acento azul-violeta |

> `skeet.cc-master` trae ImGui de fábrica sin modificar y sin código de menú propio, así
> que **no** es un renderizador — sigue vivo como paleta de tema en la pestaña UI del
> menú Default.

### Mapeo de funciones por renderizador

Los cuatro packs vendidos exponen los mismos nueve toggles en vivo y el slider de cargas
del Accelecharger, cada uno con los widgets del propio pack:

| Función | Neverlose CS2 | Neverlose Legacy | Onetap | Fatal UI |
|---|---|---|---|---|
| Nitrox ilimitado | checkbox `1` (Ragebot ▸ Crown) | Rage ▸ *Silent Aim* | `auto_bhop` | VEHICLE ▸ TUNING ▸ Unlimited Nitrox |
| Jump Jets ilimitados | checkbox `2` | Rage ▸ *Automatic Fire* | `auto_strafe` | VEHICLE ▸ TUNING ▸ Unlimited Jump Jets |
| Manejo Perfecto | checkbox `3` | Rage ▸ *Aim Through Walls* | `no_recoil` | VEHICLE ▸ TUNING ▸ Perfect Handling (+ slider Grip) |
| Noclip | checkbox `4` | Rage ▸ *History* | `rage_enable` | PLAYER ▸ MOVEMENT ▸ Noclip (+ slider Speed) |
| God Mode | checkbox `5` | Rage ▸ *Duck Peek* | `rage_silent` | PLAYER ▸ SURVIVAL ▸ God Mode |
| Hydro: sin freno en agua | checkbox `6` | Rage ▸ *Quick Peek* | `visuals_no_smoke` | VEHICLE ▸ HAZARDS ▸ Hydro |
| Spike: sin atasco en arena | checkbox `7` | Rage ▸ *Quick Stop* | `visuals_no_flash` | VEHICLE ▸ HAZARDS ▸ Spike (+ slider Sand Floor) |
| Boost del Accelecharger | checkbox `8` | Legit ▸ *Enabled* | `hitchance_enabled` | VEHICLE ▸ ACCELECHARGER ▸ toggle |
| Boost de HP del Motor | checkbox `9` | Legit ▸ *Aimbot* | `fake_lag` | VEHICLE ▸ ENGINE ▸ Engine HP Boost (+ slider HP) |
| Cargas del Accelecharger | `Slider` (Ragebot ▸ Feather) | Rage ▸ *Field of View* | `fov` | VEHICLE ▸ ACCELECHARGER ▸ Charges |

Las acciones de un disparo (desbloquear todos los coches, desbloquear todos los
AcceleChargers, establecer todos los récords, guardar progreso, completar el realm
actual) están en el menú Default, en la Fatal UI (ACTIONS ▸ UNLOCKS / RECORDS) y en
Onetap (botones Save / Unlock inventory).

> **Nota de fuentes de Onetap.** El pack se escribió para ImGui con el rasterizador
> monocromo de FreeType. Esta build usa el stb_truetype de fábrica; el adaptador
> mantiene `PixelSnapH` y oversampling 1× para que las fuentes de píxeles de
> `assets/onetap/` sigan nítidas. Activando `IMGUI_ENABLE_FREETYPE` se recupera el
> rasterizado original exacto.

## Auditoría de seguridad del imGUIS

Antes de usar cualquier paquete, se escaneó cada archivo de código fuente de los cinco
archivos comprimidos en busca de llamadas de red, creación de procesos, persistencia y
payloads ofuscados.

| Paquete | Veredicto | Notas |
|---|---|---|
| `neverlose-last (1).rar` | **limpio** | fork de ImGui estándar + código propio de dibujo del menú |
| `neverlose-last-main.zip` | **limpio**, 1 referencia colgante | `examples/example_win32_directx11/build_win32.bat` llama a `..\..\run_menu.ps1`, que no existe en ningún lugar del archivo. Falta el script (eliminado o nunca distribuido) y todo el árbol `examples/` no lo usa el trainer. |
| `skeet.cc-master.zip` | **limpio** | ImGui estándar sin modificar, sin código de temas |
| `onetap ui remake.zip` | **limpio** | ImGui + remake de widgets, código de dibujo puro |
| `project-139.rar` | **limpio** | ImGui + librería de dibujo FatalWidgets |

No se encontraron sockets de red, descargas, escrituras al registro, tareas programadas,
inyección de procesos ni blobs codificados en código compilable. Esto importa más que
nunca ahora que el código de los paquetes se compila dentro de la DLL — byte a byte,
solo con los añadidos documentados más abajo.

## Código vendido: qué es byte a byte y qué cambió

Todo lo hay bajo `src/uis/vendor/` está versionado **byte a byte** desde los extractos de
`imGUIS/`, salvo estos añadidos documentados que puentean los packs con el trainer:

| Fichero | Cambio | Motivo |
|---|---|---|
| `src/uis/vendor/nl_legacy/neverlose_menu.cpp` | bloque de accessors añadido al final del fichero | expone el almacenamiento de toggles/sliders del menú al adaptador `ui_nllegacy.cpp` |
| `src/uis/vendor/fatal/menu/Client.h` | la sección de datos demo del pack reescrita como árbol de bindings del trainer; `char8_t icon[4]` → `char icon[4]` | esa sección son las propiedades de ejemplo del pack; ahora lleva los bindings del trainer; el cambio de tipo va con `/Zc:char8_t-` |
| `src/uis/compat/imgui.h` | fichero nuevo (shim de rutas de include) | el código vendido de la era 1.92 llama a `ImGui::GetFontBaked()`; el shim lo provee para el 1.90.9 de fábrica en `third_party/imgui` |
| Blobs de fuentes Fatal | los dos literales base85 de ~8,6 MB movidos fuera de `font_medium.h` / `font_extrabold.h` a ficheros en tiempo de ejecución `assets/fatal/*.base85` | el front-end de MSVC agota el montón (C1060) compilando literales de varios megabytes; `AddFontFromMemoryCompressedBase85TTF` los carga en runtime |
| Flags de compilación | `IMGUI_DEFINE_MATH_OPERATORS` y `/Zc:char8_t-` definidos globalmente | el código vendido depende de ambos (operadores ImVec2, literales `u8""` como `char`) |

## Compilación

Requisitos: Visual Studio 2022 (o 2026) con la carga de trabajo de escritorio C++,
Windows 10/11 x64.

1. Abrir `AcceleracersTrainer.sln`.
2. Seleccionar **Release / x64**.
3. Compilar. Salida: `bin\Release\AcceleracersTrainer.dll` más `assets\` junto a él.

Línea de comandos:

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

ImGui (backends DX11 + Win32) y MinHook vienen incluidos en `third_party/` — sin gestor
de paquetes, sin descargas externas, un solo comando de compilación. La build copia
`assets/fatal/` junto a la DLL automáticamente (target `CopyRuntimeAssets`).

En tiempo de ejecución la DLL espera las carpetas `assets/` que trae (`fatal`,
`nl_legacy`, `onetap`) a su lado; cada renderizador cae a la fuente integrada si faltan
sus ficheros.

## Instalación y uso

1. Inicia **Project Acceleracers** (en ventana o sin bordes funciona mejor).
2. Inyecta `AcceleracersTrainer.dll` con cualquier inyector de DLL estándar.
3. **INSERT** abre el menú activo, **HOME** abre el selector de renderizadores.
4. Elige un renderizador; activa funciones donde estés — la configuración compartida te
   sigue.

## Estructura del proyecto

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── assets/
│   ├── fatal/             # blobs de fuentes del pack Fatal (base85, cargados en runtime)
│   ├── nl_legacy/         # fuentes SST + FontAwesome
│   └── onetap/            # fuentes fs-tahoma / smallest_pixel / csgo_icons
├── src/
│   ├── dllmain.cpp        # punto de entrada: hilo de arranque
│   ├── hooks.cpp/.hpp     # hooks Present/Resize/ProcessEvent, overlay D3D11
│   ├── features.cpp/.hpp  # motor de funciones + acciones en cola
│   ├── menu.cpp/.hpp      # menú Default (pestañas + pestaña UI)
│   ├── themes.cpp/.hpp    # sistema de paletas (paletas de tema)
│   ├── realms.hpp         # tabla de portales de realms (47 realms)
│   ├── game.hpp           # offsets y helpers específicos del juego
│   ├── ue.hpp             # núcleo UE4 mínimo: GNames, GWorld, FName, ProcessEvent
│   ├── gamethread.hpp     # claim del hilo del juego
│   └── uis/
│       ├── ui_manager.cpp/.hpp  # conmutación de renderizadores, selector HOME, rebuild de atlas
│       ├── ui_assets.cpp/.hpp   # dispositivo D3D, cargador WIC, rutas de assets
│       ├── ui_packs.hpp         # interfaz de adaptadores de renderizadores
│       ├── ui_nlcs2.cpp         # adaptadores que puentea packs ↔ trainer
│       ├── ui_nllegacy.cpp
│       ├── ui_onetap.cpp
│       ├── ui_fatal.cpp
│       ├── compat/imgui.h       # shim GetFontBaked (código vendor 1.92 sobre 1.90.9)
│       └── vendor/              # los cuatro packs, byte a byte
│           ├── nl_cs2/ · nl_legacy/ · onetap/ · fatal/
└── third_party/
    ├── imgui/             # Dear ImGui 1.90.9 + backends DX11/Win32
    └── minhook/           # MinHook
```

## Idiomas

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

Cada idioma tiene su propio README: `README.de.md`, `README.ru.md`, `README.pt.md`,
`README.pt-BR.md`, `README.es.md`, `README.ja.md`, `README.zh.md`.

## Aviso legal

Solo para investigación y juego individual. Úsalo bajo tu propio riesgo; los autores no
se responsabilizan de corrupción de guardados ni de acciones sobre la cuenta. El juego
pertenece a sus respectivos propietarios. Todos los packs de UI vendidos pertenecen a
sus respectivos autores y se redistribuyen aquí sin modificar para este proyecto
educativo.
