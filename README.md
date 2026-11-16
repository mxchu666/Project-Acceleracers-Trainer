# Project Acceleracers Trainer — Recode

Internal trainer DLL for **Project Acceleracers** (UE4 4.27), written in C++ with a fully
custom ImGui overlay. It hooks the game's renderer, runs its gameplay features on the
game thread, and includes a **theme category** that lets you switch the whole ImGui look
between the UI packs collected in the local `imGUIS/` folder.

> Single-player tool. Educational project. No multiplayer functionality, no anti-cheat
> evasion, nothing packed or hidden — the source compiles as-is.

---

## Features

### Player
- Unlimited Nitrox (refilled every frame)
- Unlimited Jump Jets (Accelecharger uses topped up, cooldowns cleared)
- Perfect Handling (max tire friction, adjustable grip slider)
- Accelecharger Charges (adjustable counter)
- Engine HP Boost (clears `EngineBlown`)
- Noclip (WASD + Space/Ctrl, adjustable speed)
- God Mode (immune to Storm lightning, spikes, wind push)
- Hydro Tires: no water slowdown / Spike Tires: no sand stall

### Progression
- Unlock all cars (every `ECarList` entry 0–112)
- Unlock all AcceleChargers (every `EAccelechargers` 0–16)
- Set all realm records (calls `SaveGameTime` for all 47 realms)
- Save progress through the game's own `Save_Game`

### World
- Live realm identification (portal `LevelOut` name + dump table)
- Complete current realm (parks the car inside the exit portal)
- Realm list with coordinates and live status
- Engine diagnostics: world/level/actor/portal counts, game-thread ID check

### Theme category
A fourth tab, **THEME**, rebuilds the ImGui style instantly from palettes extracted
from the UI packs in `imGUIS/`:

| Theme | Source pack | Look |
|---|---|---|
| Default | original trainer | dark blue-grey, orange accent |
| Neverlose | `neverlose-last` (`gui.hpp`, `neverlose_menu.cpp`) | navy cards, blue accent |
| Skeet | `skeet.cc-master` (derived — the pack ships stock ImGui) | near-black, green accent |
| Onetap | `onetap ui remake` (`ui/core/ui_theme.h`) | charcoal, red accent |
| Fatal UI | `project-139` (`FatalUITest/menu/Menu.cpp`) | charcoal, blue-violet accent |

## imGUIS security audit

Before any pack was used, every source file in all five archives was scanned for
network calls, process spawning, persistence, and obfuscated payloads.

| Pack | Verdict | Notes |
|---|---|---|
| `neverlose-last (1).rar` | **clean** | stock ImGui fork + custom menu drawing code |
| `neverlose-last-main.zip` | **clean**, 1 dangling reference | `examples/example_win32_directx11/build_win32.bat` calls `..\..\run_menu.ps1`, which does not exist anywhere in the archive. The script is missing (removed or never shipped), and the whole `examples/` tree is unused by the trainer. |
| `skeet.cc-master.zip` | **clean** | unmodified stock ImGui, no theme code included |
| `onetap ui remake.zip` | **clean** | ImGui + widget remake, pure drawing code |
| `project-139.rar` | **clean** | ImGui + FatalWidgets drawing library |

No network sockets, downloads, registry writes, scheduled tasks, process injection or
encoded blobs were found in any compilable source. The trainer only consumes the color
palettes and style constants — none of the packs' code is linked into the DLL.

## Build

Requirements: Visual Studio 2022 (or 2026) with the C++ desktop workload, Windows 10/11 x64.

1. Open `AcceleracersTrainer.sln`.
2. Select **Release / x64**.
3. Build. Output: `bin\Release\AcceleracersTrainer.dll`.

Command line:

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

ImGui (DX11 + Win32 backends) and MinHook are vendored in `third_party/` — no package
manager, no external downloads, one build command.

## Install & usage

1. Launch **Project Acceleracers** (windowed or borderless works best).
2. Inject `AcceleracersTrainer.dll` with any standard DLL injector.
3. Press **INSERT** to open/close the menu.
4. Pick a look in the **THEME** tab; enable features in the other tabs.

Features run on the game thread (hooked through `UObject::ProcessEvent`) and are
refused if the trainer ever ends up claiming the wrong thread — see the `GT` /
`Render` readout in the WORLD tab.

## Project structure

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── src/
│   ├── dllmain.cpp      # entry point: boot thread
│   ├── hooks.cpp/.hpp   # Present/Resize/ProcessEvent hooks, D3D11 overlay
│   ├── features.cpp/.hpp# feature engine + queued actions
│   ├── menu.cpp/.hpp    # custom-drawn ImGui menu, 4 tabs
│   ├── themes.cpp/.hpp  # palette system (the THEME tab)
│   ├── realms.hpp       # realm portal table (47 realms)
│   ├── game.hpp         # game-specific offsets and helpers
│   ├── ue.hpp           # minimal UE4 core: GNames, GWorld, FName, ProcessEvent
│   └── gamethread.hpp   # game-thread claim
└── third_party/
    ├── imgui/           # Dear ImGui + DX11/Win32 backends
    └── minhook/         # MinHook
```

## Languages

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

Each language has its own README: `README.de.md`, `README.ru.md`, `README.pt.md`,
`README.pt-BR.md`, `README.es.md`, `README.ja.md`, `README.zh.md`.

## Disclaimer

For research and single-player use only. Use at your own risk; the authors take no
responsibility for save corruption or account actions. The game belongs to its
respective owners.
