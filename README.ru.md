# Project Acceleracers Trainer — Recode (Русский)

Внутренняя DLL-тренер для **Project Acceleracers** (UE4 4.27) на C++ с оверлеем ImGui
поверх D3D11, в который встроены **пять рендереров меню**: собственное меню тренера плюс
четыре сторонних UI-пака из локальной папки `imGUIS/`, добавленные **байт в байт** и
переключаемые на ходу.

> Инструмент для одиночной игры. Образовательный проект. Никакого мультиплеера, никакого
> обхода анти-чита, ничего упакованного или скрытого — исходники компилируются как есть.

---

## Возможности

### Игрок
- Бесконечный Nitrox (восполняется каждый кадр)
- Бесконечные Jump Jets (заряды Accelecharger пополнены, кулдауны сброшены)
- Perfect Handling (максимальное сцепление шин, регулируемый ползунок Grip)
- Заряды Accelecharger (настраиваемый счётчик)
- Boost HP двигателя (снимает `EngineBlown`)
- Noclip (WASD + Space/Ctrl, регулируемая скорость)
- God Mode (иммунитет к молниям Storm, шипам и порывам ветра)
- Hydro Tires: без замедления в воде / Spike Tires: без застревания в песке

### Прогресс
- Открыть все машины (каждая запись `ECarList` 0–112)
- Открыть все AcceleChargers (каждый `EAccelechargers` 0–16)
- Установить все рекорды realm (`SaveGameTime` для всех 47 realm)
- Сохранить прогресс через собственный `Save_Game` игры

### Мир
- Определение realm на лету (имя `LevelOut` портала + таблица дампа)
- Завершить текущий realm (ставит машину в выходной портал)
- Список realm с координатами и живым статусом
- Диагностика движка: счётчики world/level/actor/portal, проверка ID игрового потока

Каждый рендерер управляет одним и тем же движком функций: меню только пишут настройки и
ставят разовые действия в очередь; сама игровая работа выполняется в игровом потоке
(хук через `UObject::ProcessEvent`) и отклоняется, если тренер вдруг захватил не тот
поток — см. показания `GT`/`Render` во вкладке WORLD меню Default.

## Рендереры меню

| Клавиша | Действие |
|---|---|
| **INSERT** | открыть / закрыть активное меню |
| **HOME** | открыть компактный селектор рендереров |

В меню Default также есть вкладка **UI** со списком всех пяти рендереров и тематическими
палитрами. Настройки живут в одном общем конфиге — переключатели переезжают с вами при
смене рендерера прямо посреди сессии.

| # | Рендерер | Источник | Внешний вид |
|---|---|---|---|
| 1 | **Default** | оригинальный тренер | тёмный сине-серый, оранжевый акцент |
| 2 | **Neverlose CS2** | пак `nl_cs2` (`gui.cpp`, `gui.hpp`) | тёмно-синие карточки, синий акцент |
| 3 | **Neverlose Legacy** | пак `nl_legacy` (`neverlose_menu.cpp`) | тёмно-синие карточки, синий акцент, размытый фон |
| 4 | **Onetap** | `onetap ui remake` (`draw.cpp`, `gui_layers.cpp`, `group_layers.cpp`) | графит, красный акцент |
| 5 | **Fatal UI** | `project-139` (`FatalWidgets.cpp`, `menu/Menu.cpp`) | графит, сине-фиолетовый акцент |

> `skeet.cc-master` — это немодифицированный стоковый ImGui без собственного кода меню,
> поэтому он **не** рендерер — он остаётся тематической палитрой во вкладке UI меню
> Default.

### Привязка функций по рендерерам

Четыре vendored-пака предоставляют те же девять живых переключателей и ползунок зарядов
Accelecharger — каждый через виджеты самого пака:

| Функция тренера | Neverlose CS2 | Neverlose Legacy | Onetap | Fatal UI |
|---|---|---|---|---|
| Бесконечный Nitrox | чекбокс `1` (Ragebot ▸ Crown) | Rage ▸ *Silent Aim* | `auto_bhop` | VEHICLE ▸ TUNING ▸ Unlimited Nitrox |
| Бесконечные Jump Jets | чекбокс `2` | Rage ▸ *Automatic Fire* | `auto_strafe` | VEHICLE ▸ TUNING ▸ Unlimited Jump Jets |
| Perfect Handling | чекбокс `3` | Rage ▸ *Aim Through Walls* | `no_recoil` | VEHICLE ▸ TUNING ▸ Perfect Handling (+ ползунок Grip) |
| Noclip | чекбокс `4` | Rage ▸ *History* | `rage_enable` | PLAYER ▸ MOVEMENT ▸ Noclip (+ ползунок Speed) |
| God Mode | чекбокс `5` | Rage ▸ *Duck Peek* | `rage_silent` | PLAYER ▸ SURVIVAL ▸ God Mode |
| Hydro: без замедления в воде | чекбокс `6` | Rage ▸ *Quick Peek* | `visuals_no_smoke` | VEHICLE ▸ HAZARDS ▸ Hydro |
| Spike: без застревания в песке | чекбокс `7` | Rage ▸ *Quick Stop* | `visuals_no_flash` | VEHICLE ▸ HAZARDS ▸ Spike (+ ползунок Sand Floor) |
| Boost Accelecharger | чекбокс `8` | Legit ▸ *Enabled* | `hitchance_enabled` | VEHICLE ▸ ACCELECHARGER ▸ переключатель |
| Boost HP двигателя | чекбокс `9` | Legit ▸ *Aimbot* | `fake_lag` | VEHICLE ▸ ENGINE ▸ Engine HP Boost (+ ползунок HP) |
| Заряды Accelecharger | `Slider` (Ragebot ▸ Feather) | Rage ▸ *Field of View* | `fov` | VEHICLE ▸ ACCELECHARGER ▸ Charges |

Разовые действия (открыть все машины, открыть все AcceleChargers, установить все
рекорды, сохранить прогресс, завершить текущий realm) доступны в меню Default, в Fatal
UI (ACTIONS ▸ UNLOCKS / RECORDS) и в Onetap (кнопки Save / Unlock inventory).

> **Примечание о шрифтах Onetap.** Пак писался под ImGui с монохромным растеризатором
> FreeType. Эта сборка использует стоковый stb_truetype; адаптер сохраняет `PixelSnapH`
> и 1× оверсэмплинг, чтобы пиксельные шрифты из `assets/onetap/` оставались чёткими.
> Включение `IMGUI_ENABLE_FREETYPE` возвращает точную оригинальную растеризацию.

## Аудит безопасности imGUIS

Перед использованием любого пака каждый исходный файл всех пяти архивов проверялся на
сетевые вызовы, запуск процессов, закрепление в системе и обфусцированные payload'ы.

| Пак | Вердикт | Примечания |
|---|---|---|
| `neverlose-last (1).rar` | **чисто** | форк стокового ImGui + собственный код отрисовки меню |
| `neverlose-last-main.zip` | **чисто**, 1 висячая ссылка | `examples/example_win32_directx11/build_win32.bat` вызывает `..\..\run_menu.ps1`, которого нет нигде в архиве. Скрипт отсутствует (удалён или никогда не поставлялся), и всё дерево `examples/` тренером не используется. |
| `skeet.cc-master.zip` | **чисто** | немодифицированный стоковый ImGui, без кода тем |
| `onetap ui remake.zip` | **чисто** | ImGui + ремейк виджетов, чистый код отрисовки |
| `project-139.rar` | **чисто** | ImGui + библиотека отрисовки FatalWidgets |

Сетевых сокетов, загрузок, записей в реестр, запланированных задач, инъекций процессов
или закодированных blob'ов в компилируемом коде не найдено. Теперь это важнее, чем
когда-либо: код паков компилируется в саму DLL — байт в байт, лишь с документированными
дополнениями ниже.

## Vendored-код: что байт в байт, а что изменилось

Всё в `src/uis/vendor/` закоммичено **байт в байт** из распаковок `imGUIS/` — за
исключением этих документированных дополнений, связывающих паки с тренером:

| Файл | Изменение | Причина |
|---|---|---|
| `src/uis/vendor/nl_legacy/neverlose_menu.cpp` | блок акцессоров добавлен в конец файла | открывает адаптеру `ui_nllegacy.cpp` хранилище переключателей/ползунков меню |
| `src/uis/vendor/fatal/menu/Client.h` | демо-секция данных пака переписана под дерево привязок тренера; `char8_t icon[4]` → `char icon[4]` | эта секция — примеры свойств пака; теперь она несёт привязки тренера; смена типа идёт в паре с `/Zc:char8_t-` |
| `src/uis/compat/imgui.h` | новый include-path шим | vendored-код эпохи 1.92 вызывает `ImGui::GetFontBaked()`; шим предоставляет его для стокового 1.90.9 в `third_party/imgui` |
| Шрифтовые блобы Fatal | два base85-литерала по ~8,6 МБ вынесены из `font_medium.h` / `font_extrabold.h` в файлы времени выполнения `assets/fatal/*.base85` | фронтенд MSVC исчерпывает кучу (C1060) на мегабайтных литералах; `AddFontFromMemoryCompressedBase85TTF` загружает их в рантайме |
| Флаги компилятора | `IMGUI_DEFINE_MATH_OPERATORS` и `/Zc:char8_t-` заданы глобально | vendored-код опирается на оба (операторы ImVec2, литералы `u8""` как `char`) |

## Сборка

Требования: Visual Studio 2022 (или 2026) с рабочей нагрузкой «разработка классических
приложений на C++», Windows 10/11 x64.

1. Откройте `AcceleracersTrainer.sln`.
2. Выберите **Release / x64**.
3. Соберите. Результат: `bin\Release\AcceleracersTrainer.dll` плюс `assets\` рядом.

Командная строка:

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

ImGui (бэкенды DX11 + Win32) и MinHook лежат в `third_party/` — без пакетных менеджеров,
без внешних загрузок, одна команда сборки. Сборка автоматически копирует
`assets/fatal/` рядом с DLL (таргет `CopyRuntimeAssets`).

Во время работы DLL ожидает поставляемые папки `assets/` (`fatal`, `nl_legacy`,
`onetap`) рядом с собой; каждый рендерер откатывается к встроенному шрифту, если файлов
нет.

## Установка и использование

1. Запустите **Project Acceleracers** (в окне или без рамок — как удобнее).
2. Инъецируйте `AcceleracersTrainer.dll` любым стандартным DLL-инжектором.
3. **INSERT** открывает активное меню, **HOME** — селектор рендереров.
4. Выберите рендерер; включайте функции где угодно — общий конфиг следует за вами.

## Структура проекта

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── assets/
│   ├── fatal/             # шрифтовые блобы пака Fatal (base85, грузятся в рантайме)
│   ├── nl_legacy/         # шрифты SST + FontAwesome
│   └── onetap/            # шрифты fs-tahoma / smallest_pixel / csgo_icons
├── src/
│   ├── dllmain.cpp        # точка входа: boot-поток
│   ├── hooks.cpp/.hpp     # хуки Present/Resize/ProcessEvent, D3D11-оверлей
│   ├── features.cpp/.hpp  # движок функций + очередь действий
│   ├── menu.cpp/.hpp      # меню Default (вкладки + вкладка UI)
│   ├── themes.cpp/.hpp    # система палитр (тематические палитры)
│   ├── realms.hpp         # таблица порталов realm (47 realm)
│   ├── game.hpp           # игровые оффсеты и хелперы
│   ├── ue.hpp             # минимальное ядро UE4: GNames, GWorld, FName, ProcessEvent
│   ├── gamethread.hpp     # claim игрового потока
│   └── uis/
│       ├── ui_manager.cpp/.hpp  # переключение рендереров, HOME-селектор, ребилд атласа
│       ├── ui_assets.cpp/.hpp   # D3D-устройство, WIC-загрузчик текстур, пути к ассетам
│       ├── ui_packs.hpp         # интерфейс адаптеров рендереров
│       ├── ui_nlcs2.cpp         # адаптеры, связывающие пакы ↔ тренер
│       ├── ui_nllegacy.cpp
│       ├── ui_onetap.cpp
│       ├── ui_fatal.cpp
│       ├── compat/imgui.h       # шим GetFontBaked (vendor-код 1.92 на 1.90.9)
│       └── vendor/              # четыре пака, байт в байт
│           ├── nl_cs2/ · nl_legacy/ · onetap/ · fatal/
└── third_party/
    ├── imgui/             # Dear ImGui 1.90.9 + бэкенды DX11/Win32
    └── minhook/           # MinHook
```

## Языки

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

Для каждого языка свой README: `README.de.md`, `README.ru.md`, `README.pt.md`,
`README.pt-BR.md`, `README.es.md`, `README.ja.md`, `README.zh.md`.

## Отказ от ответственности

Только для исследований и одиночной игры. Используйте на свой риск; авторы не несут
ответственности за порчу сохранений или действия в отношении аккаунта. Игра принадлежит
её правообладателям. Все vendored-паки принадлежат своим авторам и распространяются здесь
без изменений в рамках этого образовательного проекта.
