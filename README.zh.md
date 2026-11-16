# Project Acceleracers Trainer — Recode（中文）

适用于 **Project Acceleracers**（UE4 4.27）的内部修改器 DLL，使用 C++ 编写，带有完全
自定义的 ImGui 覆盖层。它挂钩游戏渲染器，在游戏线程上执行各项功能，并包含一个
**主题分类**，可以基于本地 `imGUIS/` 文件夹中收集的 UI 包，一键切换整个 ImGui 外观。

> 单机工具。教育用途项目。无多人联机功能、无反作弊规避、无任何加壳或隐藏——
> 源代码按原样编译。

---

## 功能

### 玩家
- 无限 Nitrox（每帧自动回满）
- 无限 Jump Jets（Accelecharger 使用次数回满，冷却清零）
- 完美操控（轮胎摩擦力拉满，抓地力可调滑块）
- Accelecharger 充能（可调计数）
- 引擎 HP 强化（清除 `EngineBlown`）
- 穿墙 Noclip（WASD + 空格/Ctrl，速度可调）
- 上帝模式（免疫 Storm 闪电、尖刺和强风推挤）
- Hydro Tires：水中不减速 / Spike Tires：沙地不卡滞

### 进度
- 解锁全部车辆（`ECarList` 全部条目 0–112）
- 解锁全部 AcceleChargers（`EAccelechargers` 全部条目 0–16）
- 设置全部领域记录（对 47 个领域调用 `SaveGameTime`）
- 通过游戏自带的 `Save_Game` 保存进度

### 世界
- 实时识别当前领域（传送门 `LevelOut` 名称 + 转储表）
- 完成当前领域（将车辆停入出口传送门）
- 领域列表，带坐标与实时状态
- 引擎诊断：世界／关卡／Actor／传送门计数，游戏线程 ID 校验

### 主题分类
第四个标签页 **THEME**，使用从 `imGUIS/` 中各 UI 包提取的调色板，即时重建 ImGui 样式：

| 主题 | 来源包 | 外观 |
|---|---|---|
| Default | 原版修改器 | 深蓝灰底，橙色强调 |
| Neverlose | `neverlose-last`（`gui.hpp`、`neverlose_menu.cpp`） | 藏青卡片，蓝色强调 |
| Skeet | `skeet.cc-master`（衍生——该包内为原版 ImGui） | 近黑底，绿色强调 |
| Onetap | `onetap ui remake`（`ui/core/ui_theme.h`） | 炭灰底，红色强调 |
| Fatal UI | `project-139`（`FatalUITest/menu/Menu.cpp`） | 炭灰底，蓝紫强调 |

## imGUIS 安全审计

在使用任何包之前，已对全部五个压缩包中的每一个源文件扫描了网络调用、进程创建、
持久化以及混淆载荷。

| 包 | 结论 | 备注 |
|---|---|---|
| `neverlose-last (1).rar` | **干净** | 原版 ImGui 分叉 + 自绘菜单代码 |
| `neverlose-last-main.zip` | **干净**，1 处悬空引用 | `examples/example_win32_directx11/build_win32.bat` 调用了 `..\..\run_menu.ps1`，但该脚本在压缩包内并不存在（已被移除或从未附带）。整个 `examples/` 目录修改器也不会使用。 |
| `skeet.cc-master.zip` | **干净** | 未修改的原版 ImGui，不含主题代码 |
| `onetap ui remake.zip` | **干净** | ImGui + 控件重绘，纯绘图代码 |
| `project-139.rar` | **干净** | ImGui + FatalWidgets 绘图库 |

在可编译代码中未发现任何网络套接字、下载行为、注册表写入、计划任务、进程注入或
编码载荷。修改器只取用其中的调色板和样式常量——各包自身的代码不会被链接进 DLL。

## 构建

环境要求：Visual Studio 2022（或 2026），含 C++ 桌面开发工作负载；Windows 10/11 x64。

1. 打开 `AcceleracersTrainer.sln`。
2. 选择 **Release / x64**。
3. 编译。输出：`bin\Release\AcceleracersTrainer.dll`。

命令行：

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

ImGui（DX11 + Win32 后端）与 MinHook 已内置于 `third_party/`——无需包管理器、无需
外部下载，一条编译命令。

## 安装与使用

1. 启动 **Project Acceleracers**（窗口化或无边框模式效果最佳）。
2. 使用任意标准 DLL 注入器注入 `AcceleracersTrainer.dll`。
3. 按 **INSERT** 打开/关闭菜单。
4. 在 **THEME** 标签页选择外观；在其他标签页开启各项功能。

所有功能都在游戏线程上运行（通过 `UObject::ProcessEvent` 挂钩）；若修改器错误地
占用了其他线程，功能将被拒绝执行——参见 WORLD 标签页中的 `GT`／`Render` 读数。

## 项目结构

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── src/
│   ├── dllmain.cpp      # 入口点：启动线程
│   ├── hooks.cpp/.hpp   # Present/Resize/ProcessEvent 挂钩，D3D11 覆盖层
│   ├── features.cpp/.hpp# 功能引擎 + 队列动作
│   ├── menu.cpp/.hpp    # 手绘 ImGui 菜单，4 个标签页
│   ├── themes.cpp/.hpp  # 调色板系统（THEME 标签页）
│   ├── realms.hpp       # 领域传送门表（47 个领域）
│   ├── game.hpp         # 游戏专用偏移与辅助函数
│   ├── ue.hpp           # 精简 UE4 核心：GNames、GWorld、FName、ProcessEvent
│   └── gamethread.hpp   # 游戏线程占用
└── third_party/
    ├── imgui/           # Dear ImGui + DX11/Win32 后端
    └── minhook/         # MinHook
```

## 语言

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

每种语言均有独立 README：`README.de.md`、`README.ru.md`、`README.pt.md`、
`README.pt-BR.md`、`README.es.md`、`README.ja.md`、`README.zh.md`。

## 免责声明

仅供研究与单机使用。风险自担；作者对存档损坏或账号处罚不承担任何责任。游戏版权归
各自权利方所有。
