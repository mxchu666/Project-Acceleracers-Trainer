# Project Acceleracers Trainer — Recode (Español)

DLL de trainer interna para **Project Acceleracers** (UE4 4.27), escrita en C++ con un
overlay ImGui completamente personalizado. Engancha el renderizador del juego, ejecuta
las funciones de jugabilidad en el hilo del juego e incluye una **categoría de temas**
que permite cambiar todo el aspecto del ImGui entre los paquetes de UI guardados en la
carpeta local `imGUIS/`.

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

### Categoría Tema
Una cuarta pestaña, **THEME**, reconstruye el estilo del ImGui al instante a partir de
las paletas extraídas de los paquetes de UI en `imGUIS/`:

| Tema | Paquete de origen | Aspecto |
|---|---|---|
| Default | trainer original | gris azulado oscuro, acento naranja |
| Neverlose | `neverlose-last` (`gui.hpp`, `neverlose_menu.cpp`) | tarjetas azul marino, acento azul |
| Skeet | `skeet.cc-master` (derivado — el paquete trae ImGui de fábrica) | casi negro, acento verde |
| Onetap | `onetap ui remake` (`ui/core/ui_theme.h`) | carbón, acento rojo |
| Fatal UI | `project-139` (`FatalUITest/menu/Menu.cpp`) | carbón, acento azul-violeta |

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
inyección de procesos ni blobs codificados en código compilable. El trainer solo consume
las paletas de colores y las constantes de estilo — el código de los paquetes no se
enlaza en la DLL.

## Compilación

Requisitos: Visual Studio 2022 (o 2026) con la carga de trabajo de escritorio C++,
Windows 10/11 x64.

1. Abrir `AcceleracersTrainer.sln`.
2. Seleccionar **Release / x64**.
3. Compilar. Salida: `bin\Release\AcceleracersTrainer.dll`.

Línea de comandos:

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

ImGui (backends DX11 + Win32) y MinHook vienen incluidos en `third_party/` — sin gestor
de paquetes, sin descargas externas, un solo comando de compilación.

## Instalación y uso

1. Inicia **Project Acceleracers** (en ventana o sin bordes funciona mejor).
2. Inyecta `AcceleracersTrainer.dll` con cualquier inyector de DLL estándar.
3. Pulsa **INSERT** para abrir/cerrar el menú.
4. Elige el aspecto en la pestaña **THEME**; activa las funciones en las demás pestañas.

Las funciones se ejecutan en el hilo del juego (enganchado vía `UObject::ProcessEvent`) y
se rechazan si el trainer llega a reclamar el hilo equivocado — mira la lectura
`GT`/`Render` en la pestaña WORLD.

## Estructura del proyecto

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── src/
│   ├── dllmain.cpp      # punto de entrada: hilo de arranque
│   ├── hooks.cpp/.hpp   # hooks Present/Resize/ProcessEvent, overlay D3D11
│   ├── features.cpp/.hpp# motor de funciones + acciones en cola
│   ├── menu.cpp/.hpp    # menú ImGui dibujado a mano, 4 pestañas
│   ├── themes.cpp/.hpp  # sistema de paletas (la pestaña THEME)
│   ├── realms.hpp       # tabla de portales de realms (47 realms)
│   ├── game.hpp         # offsets y helpers específicos del juego
│   ├── ue.hpp           # núcleo UE4 mínimo: GNames, GWorld, FName, ProcessEvent
│   └── gamethread.hpp   # claim del hilo del juego
└── third_party/
    ├── imgui/           # Dear ImGui + backends DX11/Win32
    └── minhook/         # MinHook
```

## Idiomas

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

Cada idioma tiene su propio README: `README.de.md`, `README.ru.md`, `README.pt.md`,
`README.pt-BR.md`, `README.es.md`, `README.ja.md`, `README.zh.md`.

## Aviso legal

Solo para investigación y juego individual. Úsalo bajo tu propio riesgo; los autores no
se responsabilizan de corrupción de guardados ni de acciones sobre la cuenta. El juego
pertenece a sus respectivos propietarios.
