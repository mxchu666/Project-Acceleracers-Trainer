# Project Acceleracers Trainer — Recode (Deutsch)

Interne Trainer-DLL für **Project Acceleracers** (UE4 4.27), geschrieben in C++ mit einem
D3D11-ImGui-Overlay, das **fünf Menü-Renderer** mitbringt: das eigene Trainer-Menü plus
vier UI-Pakete von Drittanbietern aus dem lokalen Ordner `imGUIS/` — **byte-identisch**
übernommen und zur Laufzeit umschaltbar.

> Einzelspieler-Werkzeug. Bildungsprojekt. Keine Multiplayer-Funktionen, keine
> Anti-Cheat-Umgehung, nichts gepackt oder versteckt — der Quellcode kompiliert so, wie
> er ist.

---

## Funktionen

### Spieler
- Unbegrenztes Nitrox (jede Frame aufgefüllt)
- Unbegrenzte Jump Jets (Accelecharger-Ladungen aufgefüllt, Cooldowns entfernt)
- Perfect Handling (maximale Reifenhaftung, einstellbarer Grip-Regler)
- Accelecharger-Ladungen (zählbarer Wert)
- Engine-HP-Boost (löscht `EngineBlown`)
- Noclip (WASD + Space/Ctrl, einstellbare Geschwindigkeit)
- God Mode (immun gegen Storm-Blitze, Stacheln und Windstoß)
- Hydro Tires: kein Wasserausbremsen / Spike Tires: kein Sand-Steckenbleiben

### Fortschritt
- Alle Autos freischalten (jeder `ECarList`-Eintrag 0–112)
- Alle AcceleChargers freischalten (jeder `EAccelechargers`-Eintrag 0–16)
- Alle Realm-Rekorde setzen (`SaveGameTime` für alle 47 Realms)
- Fortschritt speichern über das eigene `Save_Game` des Spiels

### Welt
- Live-Realm-Erkennung (Portal-`LevelOut`-Name + Dump-Tabelle)
- Aktuelles Realm abschließen (parkt das Auto im Ausgangsportal)
- Realm-Liste mit Koordinaten und Live-Status
- Engine-Diagnose: World/Level/Actor/Portal-Zähler, Game-Thread-ID-Prüfung

Jeder Renderer treibt dieselbe Feature-Engine: Die Menüs schreiben nur Einstellungen und
legen Einmal-Aktionen in die Warteschlange; die Gameplay-Arbeit läuft auf dem
Game-Thread (über `UObject::ProcessEvent` gehookt) und wird verweigert, wenn der Trainer
jemals den falschen Thread beansprucht — siehe die `GT`/`Render`-Anzeige im WORLD-Tab
des Default-Menüs.

## Menü-Renderer

| Taste | Aktion |
|---|---|
| **INSERT** | aktives Menü öffnen / schließen |
| **HOME** | kompakten Renderer-Selector öffnen |

Das Default-Menü hat außerdem einen **UI**-Tab mit allen fünf Renderern plus den
Theme-Paletten. Die Einstellungen liegen in einer gemeinsamen Konfiguration — Toggles
wandern also mit, wenn du mitten in der Sitzung den Renderer wechselst.

| # | Renderer | Übernommen aus | Look |
|---|---|---|---|
| 1 | **Default** | Original-Trainer | dunkles Blaugrau, oranger Akzent |
| 2 | **Neverlose CS2** | `nl_cs2`-Pack (`gui.cpp`, `gui.hpp`) | Navy-Karten, blauer Akzent |
| 3 | **Neverlose Legacy** | `nl_legacy`-Pack (`neverlose_menu.cpp`) | Navy-Karten, blauer Akzent, Blur-Hintergrund |
| 4 | **Onetap** | `onetap ui remake` (`draw.cpp`, `gui_layers.cpp`, `group_layers.cpp`) | Anthrazit, roter Akzent |
| 5 | **Fatal UI** | `project-139` (`FatalWidgets.cpp`, `menu/Menu.cpp`) | Anthrazit, blauvioletter Akzent |

> `skeet.cc-master` bringt unverändertes Stock-ImGui ohne eigenen Menü-Code mit, ist also
> **kein** Renderer — es lebt als Theme-Palette im UI-Tab des Default-Menüs weiter.

### Feature-Zuordnung pro Renderer

Die vier übernommenen Packs stellen dieselben neun Live-Toggles und den
Accelecharger-Ladungs-Regler bereit — jeweils mit den Widgets des eigenen Packs:

| Trainer-Feature | Neverlose CS2 | Neverlose Legacy | Onetap | Fatal UI |
|---|---|---|---|---|
| Unbegrenztes Nitrox | Checkbox `1` (Ragebot ▸ Crown) | Rage ▸ *Silent Aim* | `auto_bhop` | VEHICLE ▸ TUNING ▸ Unlimited Nitrox |
| Unbegrenzte Jump Jets | Checkbox `2` | Rage ▸ *Automatic Fire* | `auto_strafe` | VEHICLE ▸ TUNING ▸ Unlimited Jump Jets |
| Perfect Handling | Checkbox `3` | Rage ▸ *Aim Through Walls* | `no_recoil` | VEHICLE ▸ TUNING ▸ Perfect Handling (+ Grip-Regler) |
| Noclip | Checkbox `4` | Rage ▸ *History* | `rage_enable` | PLAYER ▸ MOVEMENT ▸ Noclip (+ Speed-Regler) |
| God Mode | Checkbox `5` | Rage ▸ *Duck Peek* | `rage_silent` | PLAYER ▸ SURVIVAL ▸ God Mode |
| Hydro: kein Wasserausbremsen | Checkbox `6` | Rage ▸ *Quick Peek* | `visuals_no_smoke` | VEHICLE ▸ HAZARDS ▸ Hydro |
| Spike: kein Sand-Steckenbleiben | Checkbox `7` | Rage ▸ *Quick Stop* | `visuals_no_flash` | VEHICLE ▸ HAZARDS ▸ Spike (+ Sand-Floor-Regler) |
| Accelecharger-Boost | Checkbox `8` | Legit ▸ *Enabled* | `hitchance_enabled` | VEHICLE ▸ ACCELECHARGER ▸ Toggle |
| Engine-HP-Boost | Checkbox `9` | Legit ▸ *Aimbot* | `fake_lag` | VEHICLE ▸ ENGINE ▸ Engine HP Boost (+ HP-Regler) |
| Accelecharger-Ladungen | `Slider` (Ragebot ▸ Feather) | Rage ▸ *Field of View* | `fov` | VEHICLE ▸ ACCELECHARGER ▸ Charges |

Die Einmal-Aktionen (alle Autos freischalten, alle AcceleChargers freischalten, alle
Realm-Rekorde setzen, Fortschritt speichern, aktuelles Realm abschließen) erreichst du im
Default-Menü, in der Fatal UI (ACTIONS ▸ UNLOCKS / RECORDS) und in Onetap (Save- /
Unlock-inventory-Buttons).

> **Onetap-Schrift-Hinweis.** Das Pack wurde für ImGui mit FreeTypes monochromem
> Rasterizer geschrieben. Dieser Build nutzt das Stock-stb_truetype; der Adapter hält
> `PixelSnapH` und 1× Oversampling, damit die Pixel-Fonts aus `assets/onetap/` scharf
> bleiben. Mit aktiviertem `IMGUI_ENABLE_FREETYPE` kehrt das exakte Original-Rastering
> zurück.

## imGUIS-Sicherheitsaudit

Bevor ein Paket verwendet wurde, wurde jede Quelldatei aller fünf Archive auf
Netzwerkaufrufe, Prozessstarts, Persistenz und verschleierte Payloads gescannt.

| Pack | Urteil | Anmerkungen |
|---|---|---|
| `neverlose-last (1).rar` | **sauber** | Stock-ImGui-Fork + eigener Menü-Zeichencode |
| `neverlose-last-main.zip` | **sauber**, 1 toter Verweis | `examples/example_win32_directx11/build_win32.bat` ruft `..\..\run_menu.ps1` auf, das nirgendwo im Archiv existiert. Das Skript fehlt (entfernt oder nie ausgeliefert), und der ganze `examples/`-Baum wird vom Trainer nicht verwendet. |
| `skeet.cc-master.zip` | **sauber** | unverändertes Stock-ImGui, kein Theme-Code |
| `onetap ui remake.zip` | **sauber** | ImGui + Widget-Remake, reiner Zeichencode |
| `project-139.rar` | **sauber** | ImGui + FatalWidgets-Zeichnungsbibliothek |

Keine Netzwerk-Sockets, Downloads, Registry-Schreibvorgänge, geplanten Aufgaben,
Prozess-Injektionen oder kodierten Blobs in kompilierbarem Code gefunden. Das zählt mehr
als je zuvor, seit der Pack-Code in die DLL kompiliert wird — byte-identisch, nur mit den
unten dokumentierten Ergänzungen.

## Übernommener Code: was byte-identisch ist, was sich änderte

Alles unter `src/uis/vendor/` wird **byte-identisch** aus den `imGUIS/`-Extrakten
versioniert — mit Ausnahme dieser dokumentierten Ergänzungen, die die Packs mit dem
Trainer verbrücken:

| Datei | Änderung | Grund |
|---|---|---|
| `src/uis/vendor/nl_legacy/neverlose_menu.cpp` | Accessor-Block am Dateiende angehängt | legt den Toggle/Slider-Speicher des Menüs für den Adapter `ui_nllegacy.cpp` offen |
| `src/uis/vendor/fatal/menu/Client.h` | der Demo-Prop-Datenabschnitt des Packs als Trainer-Binding-Baum neu geschrieben; `char8_t icon[4]` → `char icon[4]` | dieser Abschnitt sind die Beispiel-Properties des Packs; er trägt jetzt die Trainer-Bindings; die Typänderung gehört zu `/Zc:char8_t-` |
| `src/uis/compat/imgui.h` | neue Include-Path-Shim | der 1.92-zeitige Vendor-Code ruft `ImGui::GetFontBaked()`; die Shim stellt ihn für das Stock-1.90.9 in `third_party/imgui` bereit |
| Fatal-Font-Blobs | die zwei ~8,6-MB-Base85-Stringliterale aus `font_medium.h` / `font_extrabold.h` in Laufzeitdateien `assets/fatal/*.base85` ausgelagert | MSVCs Frontend erschöpft den Heap (C1060) bei Megabyte-langen Literalen; `AddFontFromMemoryCompressedBase85TTF` lädt sie zur Laufzeit |
| Compiler-Flags | `IMGUI_DEFINE_MATH_OPERATORS` und `/Zc:char8_t-` global definiert | der Vendor-Code stützt sich auf beides (ImVec2-Operatoren, `u8""`-Literale als `char`) |

## Build

Voraussetzungen: Visual Studio 2022 (oder 2026) mit der C++-Desktop-Workload,
Windows 10/11 x64.

1. `AcceleracersTrainer.sln` öffnen.
2. **Release / x64** wählen.
3. Bauen. Ausgabe: `bin\Release\AcceleracersTrainer.dll` plus `assets\` daneben.

Kommandozeile:

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

ImGui (DX11 + Win32-Backends) und MinHook liegen in `third_party/` — kein
Paketmanager, keine externen Downloads, ein Build-Befehl. Der Build kopiert
`assets/fatal/` automatisch neben die DLL (`CopyRuntimeAssets`-Target).

Zur Laufzeit erwartet die DLL die mitgelieferten `assets/`-Ordner (`fatal`,
`nl_legacy`, `onetap`) neben sich; jeder Renderer fällt auf die eingebaute Schrift
zurück, wenn Dateien fehlen.

## Installation & Nutzung

1. **Project Acceleracers** starten (im Fenster oder rahmenlos funktioniert am besten).
2. `AcceleracersTrainer.dll` mit einem beliebigen Standard-DLL-Injector injizieren.
3. **INSERT** öffnet das aktive Menü, **HOME** den Renderer-Selector.
4. Renderer wählen; Features dort aktivieren, wo du gerade bist — die gemeinsame
   Konfiguration folgt dir.

## Projektstruktur

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── assets/
│   ├── fatal/             # Font-Blobs des Fatal-Packs (Base85, zur Laufzeit geladen)
│   ├── nl_legacy/         # SST-Fonts + FontAwesome
│   └── onetap/            # fs-tahoma-/smallest_pixel-/csgo_icons-Fonts
├── src/
│   ├── dllmain.cpp        # Einstiegspunkt: Boot-Thread
│   ├── hooks.cpp/.hpp     # Present/Resize/ProcessEvent-Hooks, D3D11-Overlay
│   ├── features.cpp/.hpp  # Feature-Engine + Warteschlangen-Aktionen
│   ├── menu.cpp/.hpp      # Default-Menü (Tabs + UI-Tab)
│   ├── themes.cpp/.hpp    # Palettensystem (Theme-Paletten)
│   ├── realms.hpp         # Realm-Portal-Tabelle (47 Realms)
│   ├── game.hpp           # spielspezifische Offsets und Helfer
│   ├── ue.hpp             # minimaler UE4-Kern: GNames, GWorld, FName, ProcessEvent
│   ├── gamethread.hpp     # Game-Thread-Claim
│   └── uis/
│       ├── ui_manager.cpp/.hpp  # Renderer-Wechsel, HOME-Selector, Atlas-Rebuild
│       ├── ui_assets.cpp/.hpp   # D3D-Device, WIC-Texturlader, Asset-Pfade
│       ├── ui_packs.hpp         # Renderer-Adapter-Interface
│       ├── ui_nlcs2.cpp         # Adapter, die Packs ↔ Trainer verbrücken
│       ├── ui_nllegacy.cpp
│       ├── ui_onetap.cpp
│       ├── ui_fatal.cpp
│       ├── compat/imgui.h       # GetFontBaked-Shim (1.92-Vendor-Code auf 1.90.9)
│       └── vendor/              # die vier Packs, byte-identisch
│           ├── nl_cs2/ · nl_legacy/ · onetap/ · fatal/
└── third_party/
    ├── imgui/             # Dear ImGui 1.90.9 + DX11/Win32-Backends
    └── minhook/           # MinHook
```

## Sprachen

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

Jede Sprache hat ihr eigenes README: `README.de.md`, `README.ru.md`, `README.pt.md`,
`README.pt-BR.md`, `README.es.md`, `README.ja.md`, `README.zh.md`.

## Haftungsausschluss

Nur für Forschung und Einzelspieler. Nutzung auf eigene Gefahr; die Autoren übernehmen
keine Verantwortung für Speicherkorruption oder Kontomaßnahmen. Das Spiel gehört seinen
jeweiligen Eigentümern. Alle übernommenen UI-Packs gehören ihren jeweiligen Autoren und
werden hier unverändert für dieses Bildungsprojekt weitervertrieben.
