# Project Acceleracers Trainer — Recode（中文）

面向 **Project Acceleracers**（UE4 4.27）的内部训练器 DLL，使用 C++ 编写，基于 D3D11 的
ImGui 覆盖层，内置**五个菜单渲染器**：训练器自带菜单，外加四个来自本地 `imGUIS/` 文件夹
的第三方 UI 包 —— 以**逐字节原样**方式引入，可在运行时切换。

> 单机工具。教育性项目。无多人功能、无反作弊规避、无任何打包或隐藏 —— 源码原样即可编译。

---

## 功能

### 玩家
- 无限 Nitrox（每帧回满）
- 无限 Jump Jets（Accelecharger 使用次数补满，冷却清零）
- Perfect Handling（轮胎摩擦力最大化，抓地力滑条可调）
- Accelecharger 充能次数（可调计数器）
- 引擎 HP 提升（清除 `EngineBlown`）
- Noclip（WASD + Space/Ctrl，速度可调）
- God Mode（免疫 Storm 闪电、尖刺与风吹推挤）
- Hydro Tires：水中不减速 / Spike Tires：沙地不卡住

### 进度
- 解锁全部车辆（`ECarList` 全部条目 0–112）
- 解锁全部 AcceleChargers（`EAccelechargers` 全部条目 0–16）
- 设置全部 realm 纪录（为全部 47 个 realm 调用 `SaveGameTime`）
- 通过游戏自带的 `Save_Game` 保存进度

### 世界
- 实时 realm 识别（传送门 `LevelOut` 名称 + 转储表）
- 完成当前 realm（将车辆停入出口传送门）
- 附带坐标与实时状态的 realm 列表
- 引擎诊断：world/level/actor/portal 计数、游戏线程 ID 校验

每个渲染器驱动的都是同一个功能引擎：菜单只负责写入设置和将一次性动作排队入列；实际的
游戏操作在游戏线程上执行（通过 `UObject::ProcessEvent` 挂钩），如果训练器占用了错误的
线程则会拒绝执行 —— 参见 Default 菜单 WORLD 标签页中的 `GT` / `Render` 读数。

## 菜单渲染器

| 按键 | 动作 |
|---|---|
| **INSERT** | 打开 / 关闭当前菜单 |
| **HOME** | 打开紧凑的渲染器选择器 |

Default 菜单还带有一个 **UI** 标签页，列出全部五个渲染器以及主题配色。设置保存在共享
配置中，因此会话中途切换渲染器时，开关状态会跟着你走。

| # | 渲染器 | 来源 | 外观 |
|---|---|---|---|
| 1 | **Default** | 训练器原生 | 深蓝灰色，橙色点缀 |
| 2 | **Neverlose CS2** | `nl_cs2` 包（`gui.cpp`、`gui.hpp`） | 藏青卡片，蓝色点缀 |
| 3 | **Neverlose Legacy** | `nl_legacy` 包（`neverlose_menu.cpp`） | 藏青卡片，蓝色点缀，模糊背景 |
| 4 | **Onetap** | `onetap ui remake`（`draw.cpp`、`gui_layers.cpp`、`group_layers.cpp`） | 炭黑，红色点缀 |
| 5 | **Fatal UI** | `project-139`（`FatalWidgets.cpp`、`menu/Menu.cpp`） | 炭黑，蓝紫色点缀 |

> `skeet.cc-master` 内含未修改的原版 ImGui，没有自己的菜单代码，因此**不是**渲染器 ——
> 它以主题配色的形式保留在 Default 菜单的 UI 标签页中。

### 各渲染器的功能映射

四个引入的包都提供相同的九个实时开关和 Accelecharger 充能滑条，各自通过本包自带的
控件呈现：

| 训练器功能 | Neverlose CS2 | Neverlose Legacy | Onetap | Fatal UI |
|---|---|---|---|---|
| 无限 Nitrox | 复选框 `1`（Ragebot ▸ Crown） | Rage ▸ *Silent Aim* | `auto_bhop` | VEHICLE ▸ TUNING ▸ Unlimited Nitrox |
| 无限 Jump Jets | 复选框 `2` | Rage ▸ *Automatic Fire* | `auto_strafe` | VEHICLE ▸ TUNING ▸ Unlimited Jump Jets |
| Perfect Handling | 复选框 `3` | Rage ▸ *Aim Through Walls* | `no_recoil` | VEHICLE ▸ TUNING ▸ Perfect Handling（+ Grip 滑条） |
| Noclip | 复选框 `4` | Rage ▸ *History* | `rage_enable` | PLAYER ▸ MOVEMENT ▸ Noclip（+ Speed 滑条） |
| God Mode | 复选框 `5` | Rage ▸ *Duck Peek* | `rage_silent` | PLAYER ▸ SURVIVAL ▸ God Mode |
| Hydro：水中不减速 | 复选框 `6` | Rage ▸ *Quick Peek* | `visuals_no_smoke` | VEHICLE ▸ HAZARDS ▸ Hydro |
| Spike：沙地不卡住 | 复选框 `7` | Rage ▸ *Quick Stop* | `visuals_no_flash` | VEHICLE ▸ HAZARDS ▸ Spike（+ Sand Floor 滑条） |
| Accelecharger 提升 | 复选框 `8` | Legit ▸ *Enabled* | `hitchance_enabled` | VEHICLE ▸ ACCELECHARGER ▸ 开关 |
| 引擎 HP 提升 | 复选框 `9` | Legit ▸ *Aimbot* | `fake_lag` | VEHICLE ▸ ENGINE ▸ Engine HP Boost（+ HP 滑条） |
| Accelecharger 充能 | `Slider`（Ragebot ▸ Feather） | Rage ▸ *Field of View* | `fov` | VEHICLE ▸ ACCELECHARGER ▸ Charges |

一次性动作（解锁全部车辆、解锁全部 AcceleChargers、设置全部纪录、保存进度、完成当前
realm）可在 Default 菜单、Fatal UI（ACTIONS ▸ UNLOCKS / RECORDS）和 Onetap（Save /
Unlock inventory 按钮）中使用。

> **Onetap 字体说明。** 该包最初是为使用 FreeType 单色光栅器的 ImGui 编写的。本构建使用
> 原版 stb_truetype；适配器保留 `PixelSnapH` 和 1× 超采样，使 `assets/onetap/` 中的像素
> 字体保持锐利。启用 `IMGUI_ENABLE_FREETYPE` 可还原与原版完全一致的光栅化效果。

## imGUIS 安全审计

在引入任何包之前，我们对全部五个压缩包中的每个源码文件进行了扫描，检查网络调用、进程
创建、持久化和混淆载荷。

| 包 | 结论 | 备注 |
|---|---|---|
| `neverlose-last (1).rar` | **干净** | 原版 ImGui 分支 + 自定义菜单绘制代码 |
| `neverlose-last-main.zip` | **干净**，1 处悬空引用 | `examples/example_win32_directx11/build_win32.bat` 调用 `..\..\run_menu.ps1`，但该脚本在压缩包内不存在。脚本缺失（已被移除或从未发布），且整个 `examples/` 目录树均不被训练器使用。 |
| `skeet.cc-master.zip` | **干净** | 未修改的原版 ImGui，不含主题代码 |
| `onetap ui remake.zip` | **干净** | ImGui + 控件重制，纯绘制代码 |
| `project-139.rar` | **干净** | ImGui + FatalWidgets 绘制库 |

在所有可编译代码中均未发现网络套接字、下载、注册表写入、计划任务、进程注入或编码
载荷。既然包代码如今已编译进 DLL —— 逐字节原样，仅含下文记录的少量增补 —— 这一结论
比以往任何时候都更重要。

## 引入代码：哪些逐字节原样，哪些有改动

`src/uis/vendor/` 下的所有内容均从 `imGUIS/` 解包结果**逐字节原样**提交，仅以下列出的
增补用于将各包与训练器衔接：

| 文件 | 改动 | 原因 |
|---|---|---|
| `src/uis/vendor/nl_legacy/neverlose_menu.cpp` | 在文件末尾追加访问器代码块 | 向适配器 `ui_nllegacy.cpp` 暴露菜单的开关/滑条存储 |
| `src/uis/vendor/fatal/menu/Client.h` | 该包的示例属性数据段重写为训练器绑定树；`char8_t icon[4]` → `char icon[4]` | 该数据段是包的示例属性，现在承载训练器绑定；类型更改配合 `/Zc:char8_t-` |
| `src/uis/compat/imgui.h` | 新增的包含路径垫片 | 1.92 时代的包代码会调用 `ImGui::GetFontBaked()`；该垫片为 `third_party/imgui` 中的原版 1.90.9 提供此函数 |
| Fatal 字体数据块 | 两个约 8.6 MB 的 base85 字符串字面量从 `font_medium.h` / `font_extrabold.h` 移至运行时文件 `assets/fatal/*.base85` | MSVC 前端在编译数 MB 大小的字面量时会耗尽堆空间（C1060）；`AddFontFromMemoryCompressedBase85TTF` 在运行时加载 |
| 编译器标志 | 全局定义 `IMGUI_DEFINE_MATH_OPERATORS` 和 `/Zc:char8_t-` | 包代码同时依赖两者（ImVec2 运算符、作为 `char` 的 `u8""` 字面量） |

## 构建

要求：Visual Studio 2022（或 2026）及 C++ 桌面开发工作负载，Windows 10/11 x64。

1. 打开 `AcceleracersTrainer.sln`。
2. 选择 **Release / x64**。
3. 构建。输出：`bin\Release\AcceleracersTrainer.dll` 及旁边的 `assets\`。

命令行：

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

ImGui（DX11 + Win32 后端）与 MinHook 已内置于 `third_party/` —— 无包管理器、无外部
下载、一条构建命令。构建时会自动把 `assets/fatal/` 复制到 DLL 旁边
（`CopyRuntimeAssets` 目标）。

运行时，DLL 期望其自带的 `assets/` 文件夹（`fatal`、`nl_legacy`、`onetap`）位于旁边；
文件缺失时，每个渲染器都会回退到内置字体。

## 安装与使用

1. 启动 **Project Acceleracers**（窗口化或无边框效果最佳）。
2. 使用任意标准 DLL 注入器注入 `AcceleracersTrainer.dll`。
3. **INSERT** 打开当前菜单，**HOME** 打开渲染器选择器。
4. 选择一个渲染器；随处启用功能 —— 共享配置会跟随你。

## 项目结构

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── assets/
│   ├── fatal/             # Fatal 包字体数据块（base85，运行时加载）
│   ├── nl_legacy/         # SST 字体 + FontAwesome
│   └── onetap/            # fs-tahoma / smallest_pixel / csgo_icons 字体
├── src/
│   ├── dllmain.cpp        # 入口点：启动线程
│   ├── hooks.cpp/.hpp     # Present/Resize/ProcessEvent 挂钩，D3D11 覆盖层
│   ├── features.cpp/.hpp  # 功能引擎 + 排队动作
│   ├── menu.cpp/.hpp      # Default 菜单（标签页 + UI 标签页）
│   ├── themes.cpp/.hpp    # 配色系统（主题配色）
│   ├── realms.hpp         # realm 传送门表（47 个 realm）
│   ├── game.hpp           # 游戏专用偏移与辅助函数
│   ├── ue.hpp             # 最小 UE4 核心：GNames、GWorld、FName、ProcessEvent
│   ├── gamethread.hpp     # 游戏线程声明
│   └── uis/
│       ├── ui_manager.cpp/.hpp  # 渲染器切换、HOME 选择器、图集重建
│       ├── ui_assets.cpp/.hpp   # D3D 设备、WIC 纹理加载器、资源路径
│       ├── ui_packs.hpp         # 渲染器适配器接口
│       ├── ui_nlcs2.cpp         # 连接包 ↔ 训练器的适配器
│       ├── ui_nllegacy.cpp
│       ├── ui_onetap.cpp
│       ├── ui_fatal.cpp
│       ├── compat/imgui.h       # GetFontBaked 垫片（1.90.9 上的 1.92 包代码）
│       └── vendor/              # 四个包，逐字节原样
│           ├── nl_cs2/ · nl_legacy/ · onetap/ · fatal/
└── third_party/
    ├── imgui/             # Dear ImGui 1.90.9 + DX11/Win32 后端
    └── minhook/           # MinHook
```

## 语言

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

每种语言均有独立 README：`README.de.md`、`README.ru.md`、`README.pt.md`、
`README.pt-BR.md`、`README.es.md`、`README.ja.md`、`README.zh.md`。

## 免责声明

仅供研究与单机使用。风险自负；作者不对存档损坏或账号处分承担任何责任。游戏归其各自
所有者所有。所有引入的 UI 包归其各自作者所有，此处为该教育项目以未修改形式再分发。
