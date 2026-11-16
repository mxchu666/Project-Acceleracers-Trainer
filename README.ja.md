# Project Acceleracers Trainer — Recode（日本語）

**Project Acceleracers**（UE4 4.27）向けの内部トレーナー DLL です。C++ で書かれ、完全
カスタムの ImGui オーバーレイを備えています。ゲームのレンダラーをフックし、ゲーム
スレッド上で機能を実行します。さらに **テーマカテゴリ** を搭載しており、ローカルの
`imGUIS/` フォルダーにある UI パックをベースに、ImGui の見た目を丸ごと切り替えられます。

> シングルプレイヤー用ツール。教育目的のプロジェクト。マルチプレイヤー機能、
> アンチチート回避、パック化や隠蔽は一切ありません。ソースコードはそのまま
> コンパイルできます。

---

## 機能

### プレイヤー
- 無限 Nitrox（毎フレーム補充）
- 無限 Jump Jets（Accelecharger の使用回数を補充、クールダウン解除）
- パーフェクト ハンドリング（タイヤの摩擦を最大化、グリップ調整スライダー付き）
- Accelecharger チャージ（回数を調整可能）
- エンジン HP ブースト（`EngineBlown` を解除）
- Noclip（WASD + Space/Ctrl、速度調整可能）
- ゴッドモード（Storm の雷、スパイク、突風に耐性）
- Hydro Tires：水中での減速なし / Spike Tires：砂地での停止なし

### プログレッション
- 全車両アンロック（`ECarList` の全エントリー 0–112）
- 全 AcceleChargers アンロック（`EAccelechargers` の全エントリー 0–16）
- 全レルムの記録設定（47 レルムすべてに `SaveGameTime` を呼び出し）
- ゲーム本来の `Save_Game` 経由で進行状況を保存

### ワールド
- 現在のレルムをリアルタイム識別（ポータルの `LevelOut` 名 + ダンプテーブル）
- 現在のレルムをクリア（車を出口ポータル内に停める）
- 座標と状態付きのレルム一覧
- エンジン診断：ワールド／レベル／アクター／ポータル数、ゲームスレッド ID の確認

### テーマカテゴリ
4 番目のタブ **THEME** は、`imGUIS/` の UI パックから抽出したパレットで ImGui の
スタイルを即座に再構築します。

| テーマ | 参照パック | 外観 |
|---|---|---|
| Default | オリジナルのトレーナー | ダークブルーグレー＋オレンジのアクセント |
| Neverlose | `neverlose-last`（`gui.hpp`、`neverlose_menu.cpp`） | ネイビーのカード＋ブルーのアクセント |
| Skeet | `skeet.cc-master`（派生 — パックは素の ImGui を同梱） | ほぼ黒＋グリーンのアクセント |
| Onetap | `onetap ui remake`（`ui/core/ui_theme.h`） | チャコール＋レッドのアクセント |
| Fatal UI | `project-139`（`FatalUITest/menu/Menu.cpp`） | チャコール＋青紫のアクセント |

## imGUIS のセキュリティ監査

使用前に、5 つのアーカイブ内のすべてのソースファイルを対象に、ネットワーク通信、
プロセス起動、永続化、難読化ペイロードのスキャンを実施しました。

| パック | 判定 | 備考 |
|---|---|---|
| `neverlose-last (1).rar` | **クリーン** | 素の ImGui フォーク＋独自のメニュー描画コード |
| `neverlose-last-main.zip` | **クリーン**、宙ぶらりんの参照 1 件 | `examples/example_win32_directx11/build_win32.bat` が `..\..\run_menu.ps1` を呼び出しますが、このスクリプトはアーカイブ内のどこにも存在しません（削除済み、または最初から同梱されず）。なお `examples/` ツリー自体、トレーナーでは未使用です。 |
| `skeet.cc-master.zip` | **クリーン** | 無改変の素の ImGui。テーマコードは含まれず |
| `onetap ui remake.zip` | **クリーン** | ImGui ＋ウィジェットリメイク。純粋な描画コード |
| `project-139.rar` | **クリーン** | ImGui ＋ FatalWidgets 描画ライブラリ |

コンパイル可能なコードに、ネットワークソケット・ダウンロード・レジストリ書き込み・
スケジュールタスク・プロセスインジェクション・エンコード済み blob は一切
見つかりませんでした。トレーナーが参照するのはカラーパレットとスタイル定数のみで、
パック本体のコードは DLL にリンクされません。

## ビルド

必要環境：Visual Studio 2022（または 2026）＋ C++ デスクトップ ワークロード、
Windows 10/11 x64。

1. `AcceleracersTrainer.sln` を開く。
2. **Release / x64** を選択。
3. ビルド。出力：`bin\Release\AcceleracersTrainer.dll`。

コマンドライン：

```bat
msbuild AcceleracersTrainer.vcxproj /p:Configuration=Release /p:Platform=x64
```

ImGui（DX11 + Win32 バックエンド）と MinHook は `third_party/` に同梱。パッケージ
マネージャー不要、外部ダウンロードなし、ビルドコマンドは 1 行。

## インストールと使い方

1. **Project Acceleracers** を起動（ウィンドウモード／ボーダーレス推奨）。
2. 任意の標準的な DLL インジェクターで `AcceleracersTrainer.dll` をインジェクト。
3. **INSERT** キーでメニューの開閉。
4. **THEME** タブで見た目を選択。他のタブで機能をオンにします。

機能はゲームスレッド（`UObject::ProcessEvent` 経由でフック）上で実行され、誤った
スレッドを掴んだ場合は拒否されます。WORLD タブの `GT`／`Render` 表示を確認して
ください。

## プロジェクト構成

```
AcceleracersTrainer/
├── AcceleracersTrainer.sln
├── AcceleracersTrainer.vcxproj
├── src/
│   ├── dllmain.cpp      # エントリポイント：ブートスレッド
│   ├── hooks.cpp/.hpp   # Present/Resize/ProcessEvent フック、D3D11 オーバーレイ
│   ├── features.cpp/.hpp# 機能エンジン＋キューされたアクション
│   ├── menu.cpp/.hpp    # カスタム描画の ImGui メニュー、4 タブ
│   ├── themes.cpp/.hpp  # パレットシステム（THEME タブ）
│   ├── realms.hpp       # レルムポータルテーブル（47 レルム）
│   ├── game.hpp         # ゲーム固有のオフセットとヘルパー
│   ├── ue.hpp           # 最小限の UE4 コア：GNames、GWorld、FName、ProcessEvent
│   └── gamethread.hpp   # ゲームスレッドのクレーム
└── third_party/
    ├── imgui/           # Dear ImGui ＋ DX11/Win32 バックエンド
    └── minhook/         # MinHook
```

## 言語

Deutsch · Русский · Português · Português (Brasil) · Español · English · 日本語 · 中文

各言語の README：`README.de.md`、`README.ru.md`、`README.pt.md`、
`README.pt-BR.md`、`README.es.md`、`README.ja.md`、`README.zh.md`。

## 免責事項

研究およびシングルプレイヤー用途限定。使用は自己責任で行ってください。セーブデータ
の破損やアカウントへの措置について、作者は一切の責任を負いません。ゲームの権利は
各権利者に帰属します。
