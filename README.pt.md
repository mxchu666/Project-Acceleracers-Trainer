# Project Acceleracers Trainer — Recode (Português)

DLL de trainer interna para **Project Acceleracers** (UE4 4.27), escrita em C++ com um
overlay ImGui sobre D3D11 que traz **cinco renderizadores de menu**: o menu próprio do
trainer mais quatro pacotes de UI de terceiros da pasta local `imGUIS/`, integrados
**byte a byte** e comutáveis em tempo de execução.

> Ferramenta para um jogador. Projeto educativo. Sem funcionalidades multijogador, sem
> evasão de anti-cheat, nada empacotado ou escondido — o código-fonte compila tal como
> está.

---

## Funções

### Jogador
- Nitrox ilimitado (reposto a cada frame)
- Jump Jets ilimitados (usos do Accelecharger repostos, cooldowns limpos)
- Perfect Handling (fricção máxima dos pneus, slider de aderência ajustável)
- Cargas do Accelecharger (contador ajustável)
- Boost de HP do Motor (limpa `EngineBlown`)
- Noclip (WASD + Espaço/Ctrl, velocidade ajustável)
- God Mode (imune aos raios da Storm, espinhos e rajadas de vento)
- Hydro Tires: sem travão na água / Spike Tires: sem atolamento na areia

### Progressão
- Desbloquear todos os carros (cada entrada `ECarList` 0–112)
- Desbloquear todos os AcceleChargers (cada `EAccelechargers` 0–16)
- Definir todos os recordes de realm (chama `SaveGameTime` para os 47 realms)
- Guardar o progresso com o próprio `Save_Game` do jogo

### Mundo
- Identificação do realm em direto (nome `LevelOut` do portal + tabela de dump)
- Completar o realm atual (estaciona o carro dentro do portal de saída)
- Lista de realms com coordenadas e estado em direto
- Diagnóstico do motor: contagens de world/level/actor/portal, verificação do ID da thread

Cada renderizador comanda o mesmo motor de funções: os menus apenas escrevem definições e
colocam ações pontuais em fila; o trabalho de gameplay corre na thread do jogo (hook via
`UObject::ProcessEvent`) e é recusado se o trainer chegar a reivindicar a thread errada —
vê a leitura `GT`/`Render` no separador WORLD do menu Default.

## Renderizadores de menu

| Tecla | Ação |
|---|---|
| **INSERT** | abrir / fechar o menu ativo |
| **HOME** | abrir o seletor compacto de renderizadores |

O menu Default também tem um separador **UI** que lista os cinco renderizadores e as
paletas de tema. As definições vivem numa configuração partilhada, por isso os toggles
acompanham-te quando mudas de renderizador a meio da sessão.

| # | Renderizador | Origem | Aparência |
|---|---|---|---|
| 1 | **Default** | trainer original | cinza-azulado escuro, acento laranja |
| 2 | **Neverlose CS2** | pack `nl_cs2` (`gui.cpp`, `gui.hpp`) | cartões azul-marinho, acento azul |
| 3 | **Neverlose Legacy** | pack `nl_legacy` (`neverlose_menu.cpp`) | cartões azul-marinho, acento azul, fundo com blur |
| 4 | **Onetap** | `onetap ui remake` (`draw.cpp`, `gui_layers.cpp`, `group_layers.cpp`) | carvão, acento vermelho |
| 5 | **Fatal UI** | `project-139` (`FatalWidgets.cpp`, `menu/Menu.cpp`) | carvão, acento azul-violeta |

> `skeet.cc-master` traz ImGui de origem sem modificações e sem código de menu próprio,
> por isso **não** é um renderizador — sobrevive como paleta de tema no separador UI do
> menu Default.

### Mapeamento de funções por renderizador

Os quatro packs vendidos expõem os mesmos nove toggles ativos e o slider de cargas do
Accelecharger, cada um com os widgets do próprio pack:

| Função do trainer | Neverlose CS2 | Neverlose Legacy | Onetap | Fatal UI |
|---|---|---|---|---|
| Nitrox ilimitado | checkbox `1` (Ragebot ▸ Crown) | Rage ▸ *Silent Aim* | `auto_bhop` | VEHICLE ▸ TUNING ▸ Unlimited Nitrox |
| Jump Jets ilimitados | checkbox `2` | Rage ▸ *Automatic Fire* | `auto_strafe` | VEHICLE ▸ TUNING ▸ Unlimited Jump Jets |
| Perfect Handling | checkbox `3` | Rage ▸ *Aim Through Walls* | `no_recoil` | VEHICLE ▸ TUNING ▸ Perfect Handling (+ slider Grip) |
| Noclip | checkbox `4` | Rage ▸ *History* | `rage_enable` | PLAYER ▸ MOVEMENT ▸ Noclip (+ slider Speed) |
| God Mode | checkbox `5` | Rage ▸ *Duck Peek* | `rage_silent` | PLAYER ▸ SURVIVAL ▸ God Mode |
| Hydro: sem travão na água | checkbox `6` | Rage ▸ *Quick Peek* | `visuals_no_smoke` | VEHICLE ▸ HAZARDS ▸ Hydro |
| Spike: sem atolamento na areia | checkbox `7` | Rage ▸ *Quick Stop* | `visuals_no_flash` | VEHICLE ▸ HAZARDS ▸ Spike (+ slider Sand Floor) |
| Boost do Accelecharger | checkbox `8` | Legit ▸ *Enabled* | `hitchance_enabled` | VEHICLE ▸ ACCELECHARGER ▸ toggle |
| Boost de HP do Motor | checkbox `9` | Legit ▸ *Aimbot* | `fake_lag` | VEHICLE ▸ ENGINE ▸ Engine HP Boost (+ slider HP) |
| Cargas do Accelecharger | `Slider` (Ragebot ▸ Feather) | Rage ▸ *Field of View* | `fov` | VEHICLE ▸ ACCELECHARGER ▸ Charges |

As ações pontuais (desbloquear todos os carros, desbloquear todos os AcceleChargers,
definir todos os recordes, guardar o progresso, completar o realm atual) estão no menu
Default, na Fatal UI (ACTIONS ▸ UNLOCKS / RECORDS) e no Onetap (botões Save / Unlock
inventory).

> **Nota sobre as fontes do Onetap.** O pack foi escrito para ImGui com o rasterizador
> monocromático do FreeType. Esta build usa o stb_truetype de origem; o adaptador mantém
> `PixelSnapH` e oversampling 1× para que as fontes de píxeles em `assets/onetap/`
> continuem nítidas. Ativar `IMGUI_ENABLE_FREETYPE` repõe a rasterização original exata.

## Auditoria de segurança do imGUIS

Antes de usar qualquer pack, todos os ficheiros de código-fonte dos cinco arquivos foram
analisados à procura de chamadas de rede, criação de processos, persistência e payloads
ofuscados.

| Pack | Veredicto | Notas |
|---|---|---|
| `neverlose-last (1).rar` | **limpo** | fork de ImGui padrão + código próprio de desenho do menu |
| `neverlose-last-main.zip` | **limpo**, 1 referência pendente | `examples/example_win32_directx11/build_win32.bat` chama `..\..\run_menu.ps1`, que não existe em lado nenhum do arquivo. O script falta (removido ou nunca distribuído) e toda a árvore `examples/` não é usada pelo trainer. |
| `skeet.cc-master.zip` | **limpo** | ImGui padrão sem modificar, sem código de temas |
| `onetap ui remake.zip` | **limpo** | ImGui + remake de widgets, código de desenho puro |
| `project-139.rar` | **limpo** | ImGui + biblioteca de desenho FatalWidgets |

Não foram encontrados sockets de rede, descargas, escritas no registo, tarefas
agendadas, injeção de processos nem blobs codificados em código compilável. Isto importa
mais do que nunca agora que o código dos packs é compilado dentro da DLL — byte a byte,
apenas com os acrescentos documentados abaixo.

## Código vendido: o que é byte a byte, o que mudou

Tudo sob `src/uis/vendor/` está versionado **byte a byte** a partir das extrações de
`imGUIS/`, exceto estes acrescentos documentados que fazem a ponte entre os packs e o
trainer:

| Ficheiro | Mudança | Motivo |
|---|---|---|
| `src/uis/vendor/nl_legacy/neverlose_menu.cpp` | bloco de accessors acrescentado no fim do ficheiro | expõe o armazenamento de toggles/sliders do menu ao adaptador `ui_nllegacy.cpp` |
| `src/uis/vendor/fatal/menu/Client.h` | a secção de dados demo do pack reescrita como árvore de bindings do trainer; `char8_t icon[4]` → `char icon[4]` | essa secção são as propriedades de exemplo do pack; agora transporta os bindings do trainer; a mudança de tipo acompanha `/Zc:char8_t-` |
| `src/uis/compat/imgui.h` | novo shim de include-path | o código vendido da era 1.92 chama `ImGui::GetFontBaked()`; o shim fornece-o para o 1.90.9 de origem em `third_party/imgui` |
| Blobs de fontes Fatal | os dois literais base85 de ~8,6 MB movidos de `font_medium.h` / `font_extrabold.h` para ficheiros de execução `assets/fatal/*.base85` | o front-end do MSVC esgota a heap (C1060) ao compilar literais de vários megabytes; `AddFontFromMemoryCompressedBase85TTF` carrega-os em runtime |
| Flags do compilador | `IMGUI_DEFINE_MATH_OPERATORS` e `/Zc:char8_t-` definidos globalmente | o código vendido depende de ambos (operadores ImVec2, literais `u8""` como `char`) |

## Compilação

Requisitos: Visual Studio 2022 (ou 2026) com a carga de trabalho de desktop C++,
Windows 10/11 x64.

1. Abrir `AcceleracersTrainer.sln`.
2. Selecionar **Release / x64**.
3. Compilar. Saída: `bin\Release\AcceleracersTrainer.dll` mais `assets\` ao lado.

Linha de comandos:

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

O ImGui (backends DX11 + Win32) e o MinHook vêm incluídos em `third_party/` — sem gestor
de pacotes, sem descargas externas, um só comando de compilação. A build copia
`assets/fatal/` para junto da DLL automaticamente (target `CopyRuntimeAssets`).

Em execução, a DLL espera as pastas `assets/` que acompanha (`fatal`, `nl_legacy`,
`onetap`) ao lado; cada renderizador recorre à fonte integrada quando faltam ficheiros.

## Instalação e utilização

1. Inicia o **Project Acceleracers** (em janela ou sem bordas funciona melhor).
2. Injeta `AcceleracersTrainer.dll` com qualquer injetor de DLL padrão.
3. **INSERT** abre o menu ativo, **HOME** abre o seletor de renderizadores.
4. Escolhe um renderizador; ativa funções onde estiveres — a configuração partilhada
   acompanha-te.

## Estrutura do projeto

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── assets/
│   ├── fatal/             # blobs de fontes do pack Fatal (base85, carregados em runtime)
│   ├── nl_legacy/         # fontes SST + FontAwesome
│   └── onetap/            # fontes fs-tahoma / smallest_pixel / csgo_icons
├── src/
│   ├── dllmain.cpp        # ponto de entrada: thread de arranque
│   ├── hooks.cpp/.hpp     # hooks Present/Resize/ProcessEvent, overlay D3D11
│   ├── features.cpp/.hpp  # motor de funções + ações em fila
│   ├── menu.cpp/.hpp      # menu Default (separadores + separador UI)
│   ├── themes.cpp/.hpp    # sistema de paletas (paletas de tema)
│   ├── realms.hpp         # tabela de portais de realms (47 realms)
│   ├── game.hpp           # offsets e helpers específicos do jogo
│   ├── ue.hpp             # núcleo UE4 mínimo: GNames, GWorld, FName, ProcessEvent
│   ├── gamethread.hpp     # claim da thread do jogo
│   └── uis/
│       ├── ui_manager.cpp/.hpp  # comutação de renderizadores, seletor HOME, rebuild do atlas
│       ├── ui_assets.cpp/.hpp   # dispositivo D3D, carregador WIC, caminhos de assets
│       ├── ui_packs.hpp         # interface de adaptadores de renderizadores
│       ├── ui_nlcs2.cpp         # adaptadores que fazem a ponte packs ↔ trainer
│       ├── ui_nllegacy.cpp
│       ├── ui_onetap.cpp
│       ├── ui_fatal.cpp
│       ├── compat/imgui.h       # shim GetFontBaked (código vendor 1.92 sobre 1.90.9)
│       └── vendor/              # os quatro packs, byte a byte
│           ├── nl_cs2/ · nl_legacy/ · onetap/ · fatal/
└── third_party/
    ├── imgui/             # Dear ImGui 1.90.9 + backends DX11/Win32
    └── minhook/           # MinHook
```

## Idiomas

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

Cada idioma tem o seu próprio README: `README.de.md`, `README.ru.md`, `README.pt.md`,
`README.pt-BR.md`, `README.es.md`, `README.ja.md`, `README.zh.md`.

## Aviso legal

Apenas para investigação e jogo individual. Usa por tua conta e risco; os autores não se
responsabilizam por corrupção de saves nem por ações sobre a conta. O jogo pertence aos
seus respetivos proprietários. Todos os packs de UI vendidos pertencem aos respetivos
autores e são redistribuídos aqui sem modificações para este projeto educativo.
