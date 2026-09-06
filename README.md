# Corcell ZMK ファームウェア

Corcell は、PAW3222 トラックボールと乾電池駆動に対応した ZMK ファームウェアです。
キー配線、matrix transform、physical layout、charlieplex kscan は Corchibi 互換です。

**このブランチ（`dya-studio`）は DYA Studio 対応版です。**
通常版は `main` ブランチで管理します。

初めて使う方・接続で困っている方は **[接続・使い方ガイド](docs/connection-guide.md)** を参照してください。
USB は **右手側** に接続します。左右間の通信には Bluetooth を使います。

通常版との違いは DYA Studio 関連のみで、キー配線・センサー設定・ポインタの
効き方は `main` と揃えてあります。

- ZMK Studio を有効にし、ロックは無効（`CONFIG_ZMK_STUDIO_LOCKING=n`）にしています。
- DYA Studio が最初に読む device info は、unlock 前でも取得できるように
  `CONFIG_ZMK_DEVICE_INFO_STUDIO_RPC_REQUIRE_UNLOCK=n` を明示しています。
- Studio へは右手側の USB または Bluetooth で接続します。右手側では FPC 用と
  `studio-rpc-usb-uart` の両 snippet を、`snippet:` の空白区切り文字列で指定します。
- 2026-09-05 の `c2c3cb8` までの DYA 版は Studio の USB 接続に未対応です。
  USB でのキー入力と Studio 接続は別の機能です。更新方法は接続ガイドを参照してください。
- ポインタの倍率は `mouse_runtime_input_processor` / `scroll_runtime_input_processor`
  に持たせているので、DYA Studio から実機で調整できます。

## セッティングガイド

ボトムケースの開けかたと、チルトスタンドのサポート材の除去は
動画つきの別ページにまとめています。

**→ [セッティングガイド](docs/setup-guide.md)**

## ハードウェア構成

- `corcell_l` は split peripheral です。左側のモジュール入力を右側へ転送します。
- `corcell_r` は split central です。右側のモジュール入力と、左側から転送された入力を扱います。
- キー配線と kscan ピンは Corchibi と同じです。
- 6 ピン FPC スロットには、左右それぞれ 1 つずつ任意の入力モジュールを接続できます。
- デフォルトの FPC モジュールは PAW3222 トラックボールです。
- PAW3222 は `SCLK=P0.10`、`SDIO=P0.09`、`MOTION=P1.12` を使います。
- PAW3222 の NCS はデフォルトで GND 固定です。そのため、ファームウェア側では SPI chip-select GPIO を設定していません。
- PAW3222 で chip-select GPIO 制御が必要になった場合のみ、NCS を `RE_B` 側へジャンパして `&spi0` 配下に `cs-gpios = <&gpio0 5 GPIO_ACTIVE_LOW>;` を追加します。
- PAW3222 の CPI はファームウェア側で上書きせず、DYA Studio の runtime input processor
  倍率で調整します。既定値はカーソル `2/5`、スクロール `1/10` です。
- 基板上のロータリーエンコーダーは `RE_A=P0.04`、`RE_B=P0.05` で、デフォルトで有効です。
- FPC エンコーダーモジュールでは、PAW3222 の `NCS` 位置を A 相、`MOTION` 位置を B 相として使います。
- 現行回路では、エンコーダーモジュール使用時に `NCS` を `RE_B` 側へジャンパしてください。このときファームウェアは `A=P0.05`、`B=P1.12` として読みます。
- 乾電池の入力電圧は ADC0 / `P0.02` で読みます。

## FPC モジュールの切り替え

FPC モジュールは Zephyr/ZMK のスニペットで切り替えます。
通常の `build.yaml` では左右に PAW3222、右手に加えて Studio USB 用 snippet を指定します。
生成される UF2 は右手・左手・設定初期化用の 3 つです。

- 右手 PAW3222: `corcell-right-slot1-paw3222`
- 左手 PAW3222: `corcell-left-slot1-paw3222`
- 右手エンコーダー: `corcell-right-slot1-encoder`
- 左手エンコーダー: `corcell-left-slot1-encoder`

たとえば右手スロットをエンコーダーにする場合は、`build.yaml` の
`corcell-right-slot1-paw3222` を `corcell-right-slot1-encoder` に変更します。

ユーザー目線では次の流れです。

1. 左右それぞれ、FPC スロットに取り付けるモジュールを決めます。
2. `build.yaml` の `corcell_r` と `corcell_l` の FPC 用 snippet を選びます。右手側の `studio-rpc-usb-uart` は残します。
3. 変更を push します。
4. GitHub Actions の `Build` が完了したら、Artifacts から UF2 をダウンロードします。
5. `Corcell_R-...uf2` を右手、`Corcell_L-...uf2` を左手に書き込みます。

たとえば右手をエンコーダー、左手を PAW3222 にする場合は次のようにします。

```yaml
include:
  - board: xiao_ble/nrf52840/zmk
    shield: corcell_r
    artifact-name: Corcell_R-dya-studio-xiao_ble_zmk
    snippet: corcell-right-slot1-encoder studio-rpc-usb-uart
  - board: xiao_ble/nrf52840/zmk
    shield: corcell_l
    artifact-name: Corcell_L-dya-studio-xiao_ble_zmk
    snippet: corcell-left-slot1-paw3222
```

キーは `snippet:`（単数・文字列）です。`snippets:` のようにリストで書くと、
`zmkfirmware/zmk` の `build-user-config.yml` は `matrix.snippet` を空として扱い、
`west build` に `-S` が渡りません。その場合でもビルドは成功しますが、
スニペットの内容が丸ごと無視された UF2 が出力されます。

単数キーでも、文字列の中に空白で区切って複数の snippet 名を指定できます。
Zephyr が各名前に分解するため、FPC モジュールと Studio USB は併用できます。

新しい FPC モジュールを増やす場合は、`snippets/` に右手用と左手用のスニペットを追加します。
`build.yaml` には実際に取り付けたモジュールのスニペットだけを書くため、モジュール候補が増えても UF2 の出力数は増えません。

## 電源設定

- ZMK sleep を有効にしています。
- BLE TX power は +8 dBm です。
- BLE preferred connection interval は `12-12`（15 ms）、latency は `0` です。
  これは要求値であり、ホストとの実際の接続条件は接続先にも依存します。
- 30 秒で idle、15 分で deep sleep に入る設定です。保存済みの DYA 設定がある場合はそちらも確認してください。
- PAW3222 の `force-awake` は有効にしていません。
- smooth scrolling は無効にしています。
- logging、shell、SPI shell は無効にしています。
- insomnia behavior module は含めていません。
- DYA Studio 用の runtime input processor を含めています。通常版 `main` には含めていないため、DYA Studio で保存したポインタ設定は通常版に影響しません。
- 電池残量は乾電池向け voltage divider 構成で Bluetooth の battery level として報告します。
- 分圧抵抗は `output-ohms = 470k`、`full-ohms = 1M + 470k` です。
- 1 セル Ni-MH 向けの millivolt-to-percent thresholds で Bluetooth の battery level として報告します。

## 電源投入 LED

電池を入れると、XIAO の緑 LED が 2 秒だけ点灯して消えます。
組み立て時に、ペアリングしなくても電池と昇圧回路が生きているか確認できます。

- 点灯後は GPIO を切り離すので、消えたあとの消費電流はありません。
- 点灯時間は `CONFIG_CORCELL_POWER_ON_LED_MS`（既定 2000、100〜10000 ms）で変えられます。
- 不要なら `CONFIG_CORCELL_POWER_ON_LED=n` を conf に書けば丸ごと無効化できます。
- 左右どちらの半身でも点灯します。USB 給電でも同じく点灯します。

## 更新履歴

書き込みが必要な側を「対象」に書いています。記載がない項目は左右とも書き換えてください。

### 未リリース — 接続改善

- 右手側の Studio USB 通信を有効にし、FPC センサー設定と併用しました。（対象: 右手）
- 初期キーマップのレイヤー 3 に、U 位置の USB 優先と I 位置の Bluetooth 優先を追加しました。
  保存済みキーマップがある場合は自動で追加されません。接続ガイドの移行手順を参照してください。（対象: 右手）
- 初回接続、旧版と新版の違い、USB の切り分け、再ペアリング、更新と初期化の手順を追加しました。
- [点検結果と実機確認項目](docs/firmware-audit-2026-09-06.md) をまとめました。

### 2026-09-05

- 無線接続時にカーソルがカクつく問題を修正しました。BLE の送信出力を 0 dBm から
  +8 dBm へ引き上げ、接続間隔を 15 ms 固定にしています。（対象: 左右）
- スリープから復帰したあとトラックボールが反応しなくなる問題を修正しました。
  復帰時にセンサーの電源管理が復旧しないため、電池を抜くまで復帰しませんでした。
  ドライバ側で対処しています。（対象: 右手）
- スリープ後にキーを押しても復帰しない問題を修正しました。右手側のキー読み取りに
  割り込み設定が抜けており、スリープ中にキー入力を検知できませんでした。（対象: 右手）

### 2026-09-03

- セッティングガイドを追加しました。ボトムケースの取り外し、マグネットの取り付け、
  チルトスタンドのサポート材除去を、動画つきで別ページにまとめています。

### 2026-08-31

- 電池を入れると XIAO の緑 LED が 2 秒点灯するようにしました。組み立て時に、
  ペアリングせずに電池と昇圧回路の動作を確認できます。（対象: 左右）
- トラックボールがまったく動作しない問題を修正しました。`build.yaml` の記述が
  ビルド側の想定と食い違っており、センサーの設定が丸ごと無視された UF2 が
  出力されていました。（対象: 左右）
- カーソル速度を調整し、基板上のロータリーエンコーダーを既定で有効にしました。
- 通常版から DYA Studio 用の設定を分離しました。DYA Studio 対応版は
  `dya-studio` ブランチで管理します。

## ライセンス

このリポジトリ内のファームウェアソースコード、ZMK 設定ファイル、ドキュメントは MIT License です。
詳しくは `LICENSE` を確認してください。
