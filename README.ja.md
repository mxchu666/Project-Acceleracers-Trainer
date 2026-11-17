# Project Acceleracers Trainer — Recode（日本語）

**Project Acceleracers**（UE4 4.27）向けの内部トレーナー DLL です。C++ で書かれた D3D11
ImGui オーバーレイに、**5 つのメニューレンダラー**を搭載しています。トレーナー独自の
メニューに加え、ローカルの `imGUIS/` フォルダーから収集したサードパーティ UI パック 4 種を
**バイト単位でそのまま**取り込み、実行時に切り替えられます。

> シングルプレイヤー専用ツール。教育目的のプロジェクト。マルチプレイ機能なし、アンチチート
> 回避なし、パックや隠蔽も一切なし — ソースコードはそのままビルドできます。

---

## 機能

### プレイヤー
- 無限 Nitrox（毎フレーム補充）
- 無限 Jump Jets（Accelecharger の使用回数を補充、クールダウン解除）
- Perfect Handling（タイヤ摩擦を最大化、グリップのスライダー調整可能）
- Accelecharger チャージ（回数を調整可能）
- エンジン HP ブースト（`EngineBlown` を解除）
- Noclip（WASD + Space/Ctrl、速度調整可能）
- God Mode（Storm の落雷・トゲ・突風を無効化）
- Hydro Tires：水中の減速なし / Spike Tires：砂地でのスタックなし

### プログレッション
- 全車両アンロック（`ECarList` の全エントリ 0–112）
- 全 AcceleChargers アンロック（`EAccelechargers` の全エントリ 0–16）
- 全 realm のレコード設定（47 realm すべてに `SaveGameTime` を呼び出し）
- ゲーム本体の `Save_Game` による進行状況の保存

### ワールド
- realm のライブ識別（ポータルの `LevelOut` 名 + ダンプテーブル）
- 現在の realm をクリア（車を出口ポータル内に停める）
- 座標とライブ状態つきの realm リスト
- エンジン診断：world/level/actor/portal 数、ゲームスレッド ID の確認

すべてのレンダラーが同じ機能エンジンを駆動します。メニューは設定の書き込みと単発
アクションのキューイングのみを行い、実際のゲーム処理はゲームスレッド
（`UObject::ProcessEvent` 経由でフック）上で実行されます。トレーナーが誤ったスレッドを
要求した場合は拒否されます — Default メニューの WORLD タブにある `GT` / `Render` の
表示を参照してください。

## メニューレンダラー

| キー | 動作 |
|---|---|
| **INSERT** | アクティブなメニューを開く / 閉じる |
| **HOME** | コンパクトなレンダラーセレクターを開く |

Default メニューには、5 つのレンダラーとテーマパレットを一覧する **UI** タブもあります。
設定は 1 つの共有設定に保存されるため、セッション中にレンダラーを切り替えてもトグル
状態は引き継がれます。

| # | レンダラー | 収集元 | 外観 |
|---|---|---|---|
| 1 | **Default** | オリジナルトレーナー | 暗い青みのグレー、オレンジのアクセント |
| 2 | **Neverlose CS2** | `nl_cs2` パック（`gui.cpp`、`gui.hpp`） | ネイビーカード、ブルーのアクセント |
| 3 | **Neverlose Legacy** | `nl_legacy` パック（`neverlose_menu.cpp`） | ネイビーカード、ブルーのアクセント、ブラー背景 |
| 4 | **Onetap** | `onetap ui remake`（`draw.cpp`、`gui_layers.cpp`、`group_layers.cpp`） | チャコール、レッドのアクセント |
| 5 | **Fatal UI** | `project-139`（`FatalWidgets.cpp`、`menu/Menu.cpp`） | チャコール、青紫のアクセント |

> `skeet.cc-master` は無修正のストック ImGui で独自のメニューコードを持たないため、
> レンダラーでは**ありません** — Default メニューの UI タブでテーマパレットとして
> 生き続けています。

### レンダラーごとの機能マッピング

取り込んだ 4 パックは、同じ 9 つのライブトグルと Accelecharger チャージのスライダーを、
それぞれのパック独自のウィジェットで提供します：

| トレーナー機能 | Neverlose CS2 | Neverlose Legacy | Onetap | Fatal UI |
|---|---|---|---|---|
| 無限 Nitrox | チェックボックス `1`（Ragebot ▸ Crown） | Rage ▸ *Silent Aim* | `auto_bhop` | VEHICLE ▸ TUNING ▸ Unlimited Nitrox |
| 無限 Jump Jets | チェックボックス `2` | Rage ▸ *Automatic Fire* | `auto_strafe` | VEHICLE ▸ TUNING ▸ Unlimited Jump Jets |
| Perfect Handling | チェックボックス `3` | Rage ▸ *Aim Through Walls* | `no_recoil` | VEHICLE ▸ TUNING ▸ Perfect Handling（+ Grip スライダー） |
| Noclip | チェックボックス `4` | Rage ▸ *History* | `rage_enable` | PLAYER ▸ MOVEMENT ▸ Noclip（+ Speed スライダー） |
| God Mode | チェックボックス `5` | Rage ▸ *Duck Peek* | `rage_silent` | PLAYER ▸ SURVIVAL ▸ God Mode |
| Hydro：水中の減速なし | チェックボックス `6` | Rage ▸ *Quick Peek* | `visuals_no_smoke` | VEHICLE ▸ HAZARDS ▸ Hydro |
| Spike：砂地スタックなし | チェックボックス `7` | Rage ▸ *Quick Stop* | `visuals_no_flash` | VEHICLE ▸ HAZARDS ▸ Spike（+ Sand Floor スライダー） |
| Accelecharger ブースト | チェックボックス `8` | Legit ▸ *Enabled* | `hitchance_enabled` | VEHICLE ▸ ACCELECHARGER ▸ トグル |
| エンジン HP ブースト | チェックボックス `9` | Legit ▸ *Aimbot* | `fake_lag` | VEHICLE ▸ ENGINE ▸ Engine HP Boost（+ HP スライダー） |
| Accelecharger チャージ | `Slider`（Ragebot ▸ Feather） | Rage ▸ *Field of View* | `fov` | VEHICLE ▸ ACCELECHARGER ▸ Charges |

単発アクション（全車両アンロック、全 AcceleChargers アンロック、全 realm レコード設定、
進行状況の保存、現在の realm をクリア）は、Default メニュー、Fatal UI（ACTIONS ▸
UNLOCKS / RECORDS）、Onetap（Save / Unlock inventory ボタン）から実行できます。

> **Onetap のフォントについて。** このパックは FreeType のモノクロラスタライザを使う
> ImGui 向けに書かれました。本ビルドはストックの stb_truetype を使用し、アダプタが
> `PixelSnapH` と 1× オーバーサンプリングを維持するため、`assets/onetap/` のピクセル
> フォントもくっきり表示されます。`IMGUI_ENABLE_FREETYPE` を有効にすると、元の正確な
> ラスタライズが再現されます。

## imGUIS セキュリティ監査

パックを使用する前に、5 つのアーカイブすべてのソースファイルを、ネットワーク呼び出し、
プロセス起動、永続化、難読化されたペイロードの観点からスキャンしました。

| パック | 判定 | 備考 |
|---|---|---|
| `neverlose-last (1).rar` | **クリーン** | ストック ImGui フォーク + 独自のメニュー描画コード |
| `neverlose-last-main.zip` | **クリーン**、未解決参照 1 件 | `examples/example_win32_directx11/build_win32.bat` が `..\..\run_menu.ps1` を呼び出しますが、アーカイブ内のどこにも存在しません。スクリプトは欠落しており（削除済みまたは未公開）、`examples/` ツリー全体がトレーナーでは未使用です。 |
| `skeet.cc-master.zip` | **クリーン** | 無修正のストック ImGui、テーマコードなし |
| `onetap ui remake.zip` | **クリーン** | ImGui + ウィジェットリメイク、純粋な描画コード |
| `project-139.rar` | **クリーン** | ImGui + FatalWidgets 描画ライブラリ |

コンパイル可能なコードに、ネットワークソケット・ダウンロード・レジストリ書き込み・
スケジュールタスク・プロセスインジェクション・エンコード済み blob は一切見つかりません
でした。パックのコードが DLL に組み込まれる今、この結論はこれまで以上に重要です —
バイト単位そのまま、追加は下記の文書化された箇所のみです。

## 取り込みコード：バイト単位そのままの部分と変更点

`src/uis/vendor/` 以下は、`imGUIS/` の展開物から**バイト単位そのまま**コミットしてい
ます。例外は、パックとトレーナーを繋ぐ以下の文書化された追加のみです：

| ファイル | 変更 | 理由 |
|---|---|---|
| `src/uis/vendor/nl_legacy/neverlose_menu.cpp` | ファイル末尾にアクセッサブロックを追記 | メニューのトグル/スライダー格納領域をアダプタ `ui_nllegacy.cpp` に公開 |
| `src/uis/vendor/fatal/menu/Client.h` | パックのデモプロップデータセクションをトレーナーのバインディングツリーとして書き直し。`char8_t icon[4]` → `char icon[4]` | このセクションはパックのサンプルプロパティです。今回トレーナーのバインディングを担います。型変更は `/Zc:char8_t-` に付随 |
| `src/uis/compat/imgui.h` | 新規インクルードパスシム | 1.92 時代のベンダーコードは `ImGui::GetFontBaked()` を呼びます。`third_party/imgui` のストック 1.90.9 向けに提供します |
| Fatal のフォント blob | 約 8.6 MB の base85 文字列リテラル 2 件を `font_medium.h` / `font_extrabold.h` から実行時ファイル `assets/fatal/*.base85` へ移動 | MSVC のフロントエンドは数メガバイトのリテラルでヒープを使い果たします（C1060）。`AddFontFromMemoryCompressedBase85TTF` が実行時に読み込みます |
| コンパイラフラグ | `IMGUI_DEFINE_MATH_OPERATORS` と `/Zc:char8_t-` をグローバルに定義 | ベンダーコードは両方に依存します（ImVec2 演算子、`char` としての `u8""` リテラル） |

## ビルド

必要要件：C++ デスクトップワークロード入り Visual Studio 2022（または 2026）、
Windows 10/11 x64。

1. `AcceleracersTrainer.sln` を開く。
2. **Release / x64** を選択。
3. ビルド。出力：`bin\Release\AcceleracersTrainer.dll` とその隣に `assets\`。

コマンドライン：

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

ImGui（DX11 + Win32 バックエンド）と MinHook は `third_party/` に同梱 — パッケージ
マネージャー不要、外部ダウンロードなし、ビルドコマンド 1 発。ビルド時に
`assets/fatal/` が DLL の隣へ自動コピーされます（`CopyRuntimeAssets` ターゲット）。

実行時、DLL は同梱の `assets/` フォルダー（`fatal`、`nl_legacy`、`onetap`）を自分の
隣に期待します。ファイルが見つからない場合、各レンダラーは内蔵フォントにフォールバック
します。

## インストールと使い方

1. **Project Acceleracers** を起動（ウィンドウモード / ボーダーレスが最適）。
2. 任意の標準 DLL インジェクターで `AcceleracersTrainer.dll` を注入。
3. **INSERT** でアクティブなメニュー、**HOME** でレンダラーセレクターを開く。
4. レンダラーを選び、その場で機能を有効化 — 共有設定があなたについていきます。

## プロジェクト構成

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── assets/
│   ├── fatal/             # Fatal パックのフォント blob（base85、実行時に読み込み）
│   ├── nl_legacy/         # SST フォント + FontAwesome
│   └── onetap/            # fs-tahoma / smallest_pixel / csgo_icons フォント
├── src/
│   ├── dllmain.cpp        # エントリポイント：ブートスレッド
│   ├── hooks.cpp/.hpp     # Present/Resize/ProcessEvent フック、D3D11 オーバーレイ
│   ├── features.cpp/.hpp  # 機能エンジン + キューイングされたアクション
│   ├── menu.cpp/.hpp      # Default メニュー（タブ + UI タブ）
│   ├── themes.cpp/.hpp    # パレットシステム（テーマパレット）
│   ├── realms.hpp         # realm ポータルテーブル（47 realm）
│   ├── game.hpp           # ゲーム固有のオフセットとヘルパー
│   ├── ue.hpp             # 最小限の UE4 コア：GNames、GWorld、FName、ProcessEvent
│   ├── gamethread.hpp     # ゲームスレッドの要求
│   └── uis/
│       ├── ui_manager.cpp/.hpp  # レンダラー切替、HOME セレクター、アトラス再構築
│       ├── ui_assets.cpp/.hpp   # D3D デバイス、WIC テクスチャローダー、アセットパス
│       ├── ui_packs.hpp         # レンダラーアダプタインターフェース
│       ├── ui_nlcs2.cpp         # パック ↔ トレーナーを繋ぐアダプタ
│       ├── ui_nllegacy.cpp
│       ├── ui_onetap.cpp
│       ├── ui_fatal.cpp
│       ├── compat/imgui.h       # GetFontBaked シム（1.90.9 上の 1.92 ベンダーコード）
│       └── vendor/              # 4 パック、バイト単位そのまま
│           ├── nl_cs2/ ・ nl_legacy/ ・ onetap/ ・ fatal/
└── third_party/
    ├── imgui/             # Dear ImGui 1.90.9 + DX11/Win32 バックエンド
    └── minhook/           # MinHook
```

## 言語

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

各言語には専用の README があります：`README.de.md`、`README.ru.md`、`README.pt.md`、
`README.pt-BR.md`、`README.es.md`、`README.ja.md`、`README.zh.md`。

## 免責事項

研究およびシングルプレイヤー用途専用。自己責任でご利用ください。セーブデータの破損や
アカウントへの措置について、作者は一切の責任を負いません。ゲームの権利は各権利者に
帰属します。取り込んだすべての UI パックの権利はそれぞれの作者に帰属し、この教育
プロジェクトのために無修正のまま再配布しています。
