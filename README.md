# Corcell ZMK ファームウェア

Corcell は、PAW3222 トラックボールと乾電池駆動に対応した ZMK ファームウェアです。
キー配線、matrix transform、physical layout、charlieplex kscan は Corchibi 互換です。

`main` ブランチは通常版ファームウェアです。
DYA Studio 対応版は `dya-studio` ブランチで管理します。

## ハードウェア構成

- `corcell_l` は split peripheral です。左側のモジュール入力を右側へ転送します。
- `corcell_r` は split central です。右側のモジュール入力と、左側から転送された入力を扱います。
- キー配線と kscan ピンは Corchibi と同じです。
- 6 ピン FPC スロットには、左右それぞれ 1 つずつ任意の入力モジュールを接続できます。
- デフォルトの FPC モジュールは PAW3222 トラックボールです。
- PAW3222 は `SCLK=P0.10`、`SDIO=P0.09`、`MOTION=P1.12` を使います。
- PAW3222 の NCS はデフォルトで GND 固定です。そのため、ファームウェア側では SPI chip-select GPIO を設定していません。
- PAW3222 で chip-select GPIO 制御が必要になった場合のみ、NCS を `RE_B` 側へジャンパして `&spi0` 配下に `cs-gpios = <&gpio0 5 GPIO_ACTIVE_LOW>;` を追加します。
- PAW3222 の CPI はファームウェア側で上書きせず、カーソル移動量は固定の input processor 倍率で調整します。
- FPC エンコーダーモジュールでは、PAW3222 の `NCS` 位置を A 相、`MOTION` 位置を B 相として使います。
- 現行回路では、エンコーダーモジュール使用時に `NCS` を `RE_B` 側へジャンパしてください。このときファームウェアは `A=P0.05`、`B=P1.12` として読みます。
- 乾電池の入力電圧は ADC0 / `P0.02` で読みます。

## FPC モジュールの切り替え

FPC モジュールは Zephyr/ZMK のスニペットで切り替えます。
通常の `build.yaml` では PAW3222 snippet だけを指定しているため、生成される UF2 の数は増えません。

- 右手 PAW3222: `corcell-right-slot1-paw3222`
- 左手 PAW3222: `corcell-left-slot1-paw3222`
- 右手エンコーダー: `corcell-right-slot1-encoder`
- 左手エンコーダー: `corcell-left-slot1-encoder`

たとえば右手スロットをエンコーダーにする場合は、`build.yaml` の
`corcell-right-slot1-paw3222` を `corcell-right-slot1-encoder` に変更します。

ユーザー目線では次の流れです。

1. 左右それぞれ、FPC スロットに取り付けるモジュールを決めます。
2. `build.yaml` の `corcell_r` と `corcell_l` に、取り付けたモジュールのスニペットを 1 つだけ指定します。
3. 変更を push します。
4. GitHub Actions の `Build` が完了したら、Artifacts から UF2 をダウンロードします。
5. `Corcell_R-...uf2` を右手、`Corcell_L-...uf2` を左手に書き込みます。

たとえば右手をエンコーダー、左手を PAW3222 にする場合は次のようにします。

```yaml
include:
  - board: xiao_ble/nrf52840/zmk
    shield: corcell_r
    artifact-name: Corcell_R-xiao_ble_zmk
    snippet: corcell-right-slot1-encoder
  - board: xiao_ble/nrf52840/zmk
    shield: corcell_l
    artifact-name: Corcell_L-xiao_ble_zmk
    snippet: corcell-left-slot1-paw3222
```

キーは `snippet:`（単数・文字列）です。`snippets:` のようにリストで書くと、
`zmkfirmware/zmk` の `build-user-config.yml` は `matrix.snippet` を空として扱い、
`west build` に `-S` が渡りません。その場合でもビルドは成功しますが、
スニペットの内容が丸ごと無視された UF2 が出力されます。

新しい FPC モジュールを増やす場合は、`snippets/` に右手用と左手用のスニペットを追加します。
`build.yaml` には実際に取り付けたモジュールのスニペットだけを書くため、モジュール候補が増えても UF2 の出力数は増えません。

## 電源設定

- ZMK sleep を有効にしています。
- BLE TX power は Corchibi の +8 dBm ではなく、0 dBm にしています。
- BLE preferred connection interval は `6-12`、latency は `0` にして、ポインタ操作の遅延を抑えています。
- PAW3222 の `force-awake` は有効にしていません。
- smooth scrolling は無効にしています。
- logging、shell、SPI shell は無効にしています。
- insomnia behavior module は含めていません。
- 通常版 `main` では DYA Studio 用の runtime input processor を含めていません。DYA Studio で保存したポインタ設定が通常版に影響しない構成です。
- 電池残量は乾電池向け voltage divider 構成で Bluetooth の battery level として報告します。
- 分圧抵抗は `output-ohms = 470k`、`full-ohms = 1M + 470k` です。
- 1 セル Ni-MH 向けの millivolt-to-percent thresholds で Bluetooth の battery level として報告します。

## ライセンス

このリポジトリ内のファームウェアソースコード、ZMK 設定ファイル、ドキュメントは MIT License です。
詳しくは `LICENSE` を確認してください。
