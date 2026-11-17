# Project Acceleracers Trainer — Recode

Internal trainer DLL for **Project Acceleracers** (UE4 4.27), written in C++ with a D3D11
ImGui overlay that ships **five menu renderers**: the trainer's own menu plus four
third-party UI packs from the local `imGUIS/` folder, vendored **byte-for-byte** and
switchable at runtime.

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

Every renderer drives the same feature engine: the menus only write settings and queue
one-shot actions; the gameplay work runs on the game thread (hooked through
`UObject::ProcessEvent`) and is refused if the trainer ever ends up claiming the wrong
thread — see the `GT` / `Render` readout in the Default menu's WORLD tab.

## Menu renderers

| Key | Action |
|---|---|
| **INSERT** | open / close the active menu |
| **HOME** | open the compact renderer selector |

The Default menu also carries a **UI** tab listing all five renderers plus the theme
palettes. Settings live in one shared config, so toggles follow you when you switch
renderers mid-session.

| # | Renderer | Vendored from | Look |
|---|---|---|---|
| 1 | **Default** | original trainer | dark blue-grey, orange accent |
| 2 | **Neverlose CS2** | `nl_cs2` pack (`gui.cpp`, `gui.hpp`) | navy cards, blue accent |
| 3 | **Neverlose Legacy** | `nl_legacy` pack (`neverlose_menu.cpp`) | navy cards, blue accent, blur backdrop |
| 4 | **Onetap** | `onetap ui remake` (`draw.cpp`, `gui_layers.cpp`, `group_layers.cpp`) | charcoal, red accent |
| 5 | **Fatal UI** | `project-139` (`FatalWidgets.cpp`, `menu/Menu.cpp`) | charcoal, blue-violet accent |

> `skeet.cc-master` ships unmodified stock ImGui with no menu code of its own, so it is
> **not** a renderer — it lives on as a theme palette in the Default menu's UI tab.

### Feature mapping per renderer

The four vendored packs expose the same nine live toggles and the Accelecharger charge
slider, each through the pack's own widgets:

| Trainer feature | Neverlose CS2 | Neverlose Legacy | Onetap | Fatal UI |
|---|---|---|---|---|
| Unlimited Nitrox | checkbox `1` (Ragebot ▸ Crown) | Rage ▸ *Silent Aim* | `auto_bhop` | VEHICLE ▸ TUNING ▸ Unlimited Nitrox |
| Unlimited Jump Jets | checkbox `2` | Rage ▸ *Automatic Fire* | `auto_strafe` | VEHICLE ▸ TUNING ▸ Unlimited Jump Jets |
| Perfect Handling | checkbox `3` | Rage ▸ *Aim Through Walls* | `no_recoil` | VEHICLE ▸ TUNING ▸ Perfect Handling (+ Grip slider) |
| Noclip | checkbox `4` | Rage ▸ *History* | `rage_enable` | PLAYER ▸ MOVEMENT ▸ Noclip (+ Speed slider) |
| God Mode | checkbox `5` | Rage ▸ *Duck Peek* | `rage_silent` | PLAYER ▸ SURVIVAL ▸ God Mode |
| Hydro: no water slow | checkbox `6` | Rage ▸ *Quick Peek* | `visuals_no_smoke` | VEHICLE ▸ HAZARDS ▸ Hydro |
| Spike: no sand stall | checkbox `7` | Rage ▸ *Quick Stop* | `visuals_no_flash` | VEHICLE ▸ HAZARDS ▸ Spike (+ Sand Floor slider) |
| Accelecharger Boost | checkbox `8` | Legit ▸ *Enabled* | `hitchance_enabled` | VEHICLE ▸ ACCELECHARGER ▸ toggle |
| Engine HP Boost | checkbox `9` | Legit ▸ *Aimbot* | `fake_lag` | VEHICLE ▸ ENGINE ▸ Engine HP Boost (+ HP slider) |
| Accelecharger Charges | `Slider` (Ragebot ▸ Feather) | Rage ▸ *Field of View* | `fov` | VEHICLE ▸ ACCELECHARGER ▸ Charges |

The one-shot actions (unlock all cars, unlock all AcceleChargers, set all realm records,
save progress, complete current realm) are reachable from the Default menu, the Fatal UI
(ACTIONS ▸ UNLOCKS / RECORDS) and Onetap (Save / Unlock inventory buttons).

> **Onetap font note.** The pack was written for ImGui with FreeType's monochrome
> rasterizer. This build uses the stock stb_truetype; the adapter keeps `PixelSnapH`
> and 1× oversampling so the pixel fonts shipped in `assets/onetap/` stay crisp.
> Enabling `IMGUI_ENABLE_FREETYPE` restores the exact original rasterization.

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
encoded blobs were found in any compilable source. This matters more than ever now that
the pack code is compiled into the DLL — verbatim, with only the documented additions
below.

## Vendored code: what is byte-for-byte, what changed

Everything under `src/uis/vendor/` is committed **byte-for-byte** from the `imGUIS/`
extracts, except these documented additions that bridge the packs to the trainer:

| File | Change | Why |
|---|---|---|
| `src/uis/vendor/nl_legacy/neverlose_menu.cpp` | accessor block appended at end of file | exposes the menu's toggle/slider storage to the adapter `ui_nllegacy.cpp` |
| `src/uis/vendor/fatal/menu/Client.h` | the pack's demo-prop data section rewritten as the trainer's binding tree; `char8_t icon[4]` → `char icon[4]` | that section is the pack's example properties; it now carries the trainer bindings; the type change goes with `/Zc:char8_t-` |
| `src/uis/compat/imgui.h` | new include-path shim | vendored 1.92-era code calls `ImGui::GetFontBaked()`; the shim provides it for the stock 1.90.9 in `third_party/imgui` |
| Fatal font blobs | the two ~8.6 MB base85 string literals moved out of `font_medium.h` / `font_extrabold.h` into runtime files `assets/fatal/*.base85` | MSVC's front end runs out of heap (C1060) compiling multi-megabyte literals; `AddFontFromMemoryCompressedBase85TTF` loads them at runtime |
| Compiler flags | `IMGUI_DEFINE_MATH_OPERATORS` and `/Zc:char8_t-` defined globally | the vendored code relies on both (ImVec2 operators, `u8""` literals as `char`) |

## Build

Requirements: Visual Studio 2022 (or 2026) with the C++ desktop workload, Windows 10/11 x64.

1. Open `AcceleracersTrainer.sln`.
2. Select **Release / x64**.
3. Build. Output: `bin\Release\AcceleracersTrainer.dll` plus `assets\` next to it.

Command line:

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

ImGui (DX11 + Win32 backends) and MinHook are vendored in `third_party/` — no package
manager, no external downloads, one build command. The build copies `assets/fatal/`
next to the DLL automatically (`CopyRuntimeAssets` target).

At runtime the DLL expects the `assets/` folders it ships with (`fatal`, `nl_legacy`,
`onetap`) beside it; every renderer falls back to the built-in font when its files are
missing.

## Install & usage

1. Launch **Project Acceleracers** (windowed or borderless works best).
2. Inject `AcceleracersTrainer.dll` with any standard DLL injector.
3. **INSERT** opens the active menu, **HOME** opens the renderer selector.
4. Pick a renderer; enable features wherever you are — the shared config follows you.

## Project structure

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── assets/
│   ├── fatal/             # Fatal pack font blobs (base85, loaded at runtime)
│   ├── nl_legacy/         # SST fonts + FontAwesome
│   └── onetap/            # fs-tahoma / smallest_pixel / csgo_icons fonts
├── src/
│   ├── dllmain.cpp        # entry point: boot thread
│   ├── hooks.cpp/.hpp     # Present/Resize/ProcessEvent hooks, D3D11 overlay
│   ├── features.cpp/.hpp  # feature engine + queued actions
│   ├── menu.cpp/.hpp      # Default menu (tabs + UI tab)
│   ├── themes.cpp/.hpp    # palette system (theme palettes)
│   ├── realms.hpp         # realm portal table (47 realms)
│   ├── game.hpp           # game-specific offsets and helpers
│   ├── ue.hpp             # minimal UE4 core: GNames, GWorld, FName, ProcessEvent
│   ├── gamethread.hpp     # game-thread claim
│   └── uis/
│       ├── ui_manager.cpp/.hpp  # renderer switching, HOME selector, atlas rebuild
│       ├── ui_assets.cpp/.hpp   # D3D device, WIC texture loader, asset paths
│       ├── ui_packs.hpp         # renderer adapter interface
│       ├── ui_nlcs2.cpp         # adapters bridging packs ↔ trainer
│       ├── ui_nllegacy.cpp
│       ├── ui_onetap.cpp
│       ├── ui_fatal.cpp
│       ├── compat/imgui.h       # GetFontBaked shim (1.92 vendor code on 1.90.9)
│       └── vendor/              # the four packs, byte-for-byte
│           ├── nl_cs2/ · nl_legacy/ · onetap/ · fatal/
└── third_party/
    ├── imgui/             # Dear ImGui 1.90.9 + DX11/Win32 backends
    └── minhook/           # MinHook
```

## Languages

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

Each language has its own README: `README.de.md`, `README.ru.md`, `README.pt.md`,
`README.pt-BR.md`, `README.es.md`, `README.ja.md`, `README.zh.md`.

## Disclaimer

For research and single-player use only. Use at your own risk; the authors take no
responsibility for save corruption or account actions. The game belongs to its
respective owners. All vendored UI packs belong to their respective authors and are
redistributed here unmodified for this educational project.
