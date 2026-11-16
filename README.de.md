# Project Acceleracers Trainer — Recode (Deutsch)

Interne Trainer-DLL für **Project Acceleracers** (UE4 4.27), geschrieben in C++ mit einem
vollständig eigenen ImGui-Overlay. Sie hookt den Renderer des Spiels, führt die
Gameplay-Features auf dem Game-Thread aus und enthält eine **Theme-Kategorie**, mit der
das gesamte Aussehen des ImGui zwischen den UI-Paketen aus dem lokalen Ordner `imGUIS/`
umgeschaltet werden kann.

> Single-Player-Werkzeug. Bildungszweck. Keine Multiplayer-Funktionen, keine
> Anti-Cheat-Umgehung, nichts gepackt oder versteckt — der Quellcode kompiliert
> unverändert.

---

## Funktionen

### Spieler
- Unbegrenzter Nitrox (jede Frame aufgefüllt)
- Unbegrenzte Jump Jets (Accelecharger-Ladungen aufgefüllt, Cooldowns zurückgesetzt)
- Perfektes Handling (maximierter Reifengrip, einstellbarer Grip-Slider)
- Accelecharger-Ladungen (zählbar einstellbar)
- Motor-HP-Boost (setzt `EngineBlown` zurück)
- Noclip (WASD + Leertaste/Ctrl, Geschwindigkeit einstellbar)
- God Mode (immun gegen Sturm-Blitze, Spike-Reifen und Windstoß)
- Hydro-Tires: kein Wasserverlangsamung / Spike-Tires: kein Sand-Stillstand

### Fortschritt
- Alle Autos freischalten (jeder `ECarList`-Eintrag 0–112)
- Alle AcceleChargers freischalten (jeder `EAccelechargers`-Eintrag 0–16)
- Alle Realm-Rekorde setzen (ruft `SaveGameTime` für alle 47 Realms auf)
- Fortschritt über das eigene `Save_Game` des Spiels speichern

### Welt
- Live-Erkennung des aktuellen Realms (Portal-`LevelOut`-Name + Dump-Tabelle)
- Aktuellen Realm abschließen (parkt das Auto im Ausgangsportal)
- Realm-Liste mit Koordinaten und Live-Status
- Engine-Diagnose: Welt-/Level-/Actor-/Portal-Zähler, Game-Thread-ID-Prüfung

### Kategorie Theme
Ein vierter Tab, **THEME**, baut den ImGui-Stil sofort aus den Paletten um, die aus den
UI-Paketen in `imGUIS/` extrahiert wurden:

| Theme | Quellpaket | Look |
|---|---|---|
| Default | Original-Trainer | dunkles Blaugrau, oranger Akzent |
| Neverlose | `neverlose-last` (`gui.hpp`, `neverlose_menu.cpp`) | Navy-Karten, blauer Akzent |
| Skeet | `skeet.cc-master` (abgeleitet — das Paket enthält Standard-ImGui) | fast schwarz, grüner Akzent |
| Onetap | `onetap ui remake` (`ui/core/ui_theme.h`) | Anthrazit, roter Akzent |
| Fatal UI | `project-139` (`FatalUITest/menu/Menu.cpp`) | Anthrazit, blauvioletter Akzent |

## Sicherheitsprüfung der imGUIS

Bevor ein Paket verwendet wurde, wurden alle Quelldateien in allen fünf Archiven auf
Netzwerkaufrufe, Prozessstarts, Persistenz und verschleierte Payloads geprüft.

| Paket | Ergebnis | Hinweise |
|---|---|---|
| `neverlose-last (1).rar` | **sauber** | Standard-ImGui-Fork + eigener Menü-Zeichencode |
| `neverlose-last-main.zip` | **sauber**, 1 toter Verweis | `examples/example_win32_directx11/build_win32.bat` ruft `..\..\run_menu.ps1` auf, das im Archiv nirgends existiert. Das Skript fehlt (entfernt oder nie mitgeliefert), und der ganze `examples/`-Baum wird vom Trainer nicht benutzt. |
| `skeet.cc-master.zip` | **sauber** | unverändertes Standard-ImGui, kein Theme-Code enthalten |
| `onetap ui remake.zip` | **sauber** | ImGui + Widget-Remake, reiner Zeichencode |
| `project-139.rar` | **sauber** | ImGui + FatalWidgets-Zeichnbibliothek |

Keine Netzwerk-Sockets, Downloads, Registry-Schreibvorgänge, geplanten Aufgaben,
Prozess-Injektionen oder kodierten Blobs in kompilierbarem Code gefunden. Der Trainer
verwendet nur die Farbpaletten und Stil-Konstanten — der Code der Pakete selbst wird
nicht in die DLL gelinkt.

## Build

Voraussetzungen: Visual Studio 2022 (oder 2026) mit C++-Desktop-Workload, Windows 10/11 x64.

1. `AcceleracersTrainer.sln` öffnen.
2. **Release / x64** auswählen.
3. Bauen. Ausgabe: `bin\Release\AcceleracersTrainer.dll`.

Kommandozeile:

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

ImGui (DX11- + Win32-Backends) und MinHook liegen in `third_party/` bei — kein
Paketmanager, keine externen Downloads, ein Build-Befehl.

## Installation & Benutzung

1. **Project Acceleracers** starten (fenstermodus oder randlos funktioniert am besten).
2. `AcceleracersTrainer.dll` mit einem beliebigen Standard-DLL-Injector injizieren.
3. **INSERT** drücken, um das Menü zu öffnen/schließen.
4. Look im Tab **THEME** wählen; Features in den anderen Tabs aktivieren.

Die Features laufen auf dem Game-Thread (gehookt über `UObject::ProcessEvent`) und werden
verweigert, wenn der Trainer jemals den falschen Thread beansprucht — siehe die
`GT`/`Render`-Anzeige im Tab WORLD.

## Projektsstruktur

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── src/
│   ├── dllmain.cpp      # Einstiegspunkt: Boot-Thread
│   ├── hooks.cpp/.hpp   # Present/Resize/ProcessEvent-Hooks, D3D11-Overlay
│   ├── features.cpp/.hpp# Feature-Engine + Warteschlangen-Aktionen
│   ├── menu.cpp/.hpp    # selbst gezeichnetes ImGui-Menü, 4 Tabs
│   ├── themes.cpp/.hpp  # Palettensystem (der THEME-Tab)
│   ├── realms.hpp       # Realm-Portal-Tabelle (47 Realms)
│   ├── game.hpp         # spielspezifische Offsets und Helfer
│   ├── ue.hpp           # minimaler UE4-Kern: GNames, GWorld, FName, ProcessEvent
│   └── gamethread.hpp   # Game-Thread-Claim
└── third_party/
    ├── imgui/           # Dear ImGui + DX11/Win32-Backends
    └── minhook/         # MinHook
```

## Sprachen

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

Jede Sprache hat ein eigenes README: `README.de.md`, `README.ru.md`, `README.pt.md`,
`README.pt-BR.md`, `README.es.md`, `README.ja.md`, `README.zh.md`.

## Haftungsausschluss

Nur für Forschung und Single-Player. Nutzung auf eigene Gefahr; die Autoren übernehmen
keine Haftung für Speicherstand-Korruption oder Kontoaktionen. Das Spiel gehört seinen
jeweiligen Eigentümern.
