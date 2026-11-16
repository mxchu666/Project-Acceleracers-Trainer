# Project Acceleracers Trainer — Recode (Português do Brasil)

DLL de trainer interna para **Project Acceleracers** (UE4 4.27), escrita em C++ com um
overlay ImGui totalmente personalizado. Ela engancha o renderizador do jogo, executa os
recursos de gameplay na thread do jogo e inclui uma **categoria de temas** que permite
trocar todo o visual do ImGui entre os pacotes de UI guardados na pasta local `imGUIS/`.

> Ferramenta para um jogador. Projeto educacional. Sem funcionalidades multiplayer, sem
> evasão de anti-cheat, nada empacotado ou escondido — o código-fonte compila como está.

---

## Recursos

### Jogador
- Nitrox infinito (recarregado a cada frame)
- Jump Jets infinitos (usos do Accelecharger recarregados, cooldowns zerados)
- Direção Perfeita (atrito máximo dos pneus, slider de aderência ajustável)
- Cargas do Accelecharger (contador ajustável)
- Boost de HP do Motor (limpa `EngineBlown`)
- Noclip (WASD + Espaço/Ctrl, velocidade ajustável)
- God Mode (imune a raios da Storm, espinhos e rajadas de vento)
- Hydro Tires: sem lentidão na água / Spike Tires: sem travada na areia

### Progressão
- Desbloquear todos os carros (cada entrada `ECarList` 0–112)
- Desbloquear todos os AcceleChargers (cada entrada `EAccelechargers` 0–16)
- Definir todos os recordes de realm (chama `SaveGameTime` para os 47 realms)
- Salvar o progresso pelo próprio `Save_Game` do jogo

### Mundo
- Identificação do realm em tempo real (nome `LevelOut` do portal + tabela de dump)
- Completar o realm atual (estaciona o carro no portal de saída)
- Lista de realms com coordenadas e status
- Diagnóstico do motor: contagens de mundo/níveis/atores/portais, verificação do ID da thread

### Categoria Tema
Uma quarta aba, **THEME**, reconstrói o estilo ImGui instantaneamente a partir das paletas
extraídas dos pacotes de UI em `imGUIS/`:

| Tema | Pacote de origem | Visual |
|---|---|---|
| Default | trainer original | cinza-azulado escuro, destaque laranja |
| Neverlose | `neverlose-last` (`gui.hpp`, `neverlose_menu.cpp`) | cartões azul-marinho, destaque azul |
| Skeet | `skeet.cc-master` (derivado — o pacote traz ImGui de fábrica) | quase preto, destaque verde |
| Onetap | `onetap ui remake` (`ui/core/ui_theme.h`) | grafite, destaque vermelho |
| Fatal UI | `project-139` (`FatalUITest/menu/Menu.cpp`) | grafite, destaque azul-violeta |

## Auditoria de segurança do imGUIS

Antes de qualquer pacote ser usado, todos os arquivos de código-fonte dos cinco arquivos
compactados foram verificados em busca de chamadas de rede, criação de processos,
persistência e payloads ofuscados.

| Pacote | Veredito | Notas |
|---|---|---|
| `neverlose-last (1).rar` | **limpo** | fork de ImGui padrão + código próprio de desenho do menu |
| `neverlose-last-main.zip` | **limpo**, 1 referência órfã | `examples/example_win32_directx11/build_win32.bat` chama `..\..\run_menu.ps1`, que não existe em nenhum lugar do arquivo. O script está faltando (removido ou nunca distribuído), e toda a árvore `examples/` não é usada pelo trainer. |
| `skeet.cc-master.zip` | **limpo** | ImGui padrão sem modificações, sem código de temas |
| `onetap ui remake.zip` | **limpo** | ImGui + remake de widgets, código de desenho puro |
| `project-139.rar` | **limpo** | ImGui + biblioteca de desenho FatalWidgets |

Nenhum socket de rede, download, escrita no registro, tarefa agendada, injeção de
processos ou blobs codificados foram encontrados em código compilável. O trainer consome
apenas as paletas de cores e as constantes de estilo — o código dos pacotes não é ligado
na DLL.

## Compilação

Requisitos: Visual Studio 2022 (ou 2026) com a carga de trabalho C++ para desktop,
Windows 10/11 x64.

1. Abrir `AcceleracersTrainer.sln`.
2. Selecionar **Release / x64**.
3. Compilar. Saída: `bin\Release\AcceleracersTrainer.dll`.

Linha de comando:

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

O ImGui (backends DX11 + Win32) e o MinHook estão incluídos em `third_party/` — sem
gerenciador de pacotes, sem downloads externos, um comando de compilação.

## Instalação e uso

1. Inicie o **Project Acceleracers** (em janela ou sem bordas funciona melhor).
2. Injete `AcceleracersTrainer.dll` com qualquer injetor de DLL padrão.
3. Pressione **INSERT** para abrir/fechar o menu.
4. Escolha o visual na aba **THEME**; ative os recursos nas outras abas.

Os recursos rodam na thread do jogo (enganchada via `UObject::ProcessEvent`) e são
recusados se o trainer alegar a thread errada — veja a leitura `GT`/`Render` na aba WORLD.

## Estrutura do projeto

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── src/
│   ├── dllmain.cpp      # ponto de entrada: thread de boot
│   ├── hooks.cpp/.hpp   # hooks Present/Resize/ProcessEvent, overlay D3D11
│   ├── features.cpp/.hpp# motor de recursos + ações em fila
│   ├── menu.cpp/.hpp    # menu ImGui desenhado à mão, 4 abas
│   ├── themes.cpp/.hpp  # sistema de paletas (a aba THEME)
│   ├── realms.hpp       # tabela de portais de realms (47 realms)
│   ├── game.hpp         # offsets e auxiliares específicos do jogo
│   ├── ue.hpp           # núcleo UE4 mínimo: GNames, GWorld, FName, ProcessEvent
│   └── gamethread.hpp   # claim da thread do jogo
└── third_party/
    ├── imgui/           # Dear ImGui + backends DX11/Win32
    └── minhook/         # MinHook
```

## Idiomas

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

Cada idioma tem o seu próprio README: `README.de.md`, `README.ru.md`, `README.pt.md`,
`README.pt-BR.md`, `README.es.md`, `README.ja.md`, `README.zh.md`.

## Aviso legal

Apenas para pesquisa e jogo individual. Use por sua conta e risco; os autores não se
responsabilizam por corrupção de saves ou ações na conta. O jogo pertence aos seus
respectivos proprietários.
