# Corcell DYA 版 点検記録（2026-09-06）

## 対象と結論

- 対象リポジトリ：`yuchamichami/zmk-config-Corcell`
- 対象ブランチ：`dya-studio`
- 調査開始時 HEAD：`c2c3cb8bca8f061432a5976d0a7e396e535792b1`
- 既存の成功ビルド：[33941205268](https://github.com/yuchamichami/zmk-config-Corcell/actions/runs/33941205268)
- 点検範囲：左右の役割、USB／BLE／Studio、キーマップと物理配置、FPC 入力、電源・復帰、電池表示、依存関係、ビルド設定・生成ログ、README。
- Discord の実際の相談履歴、出荷個体のファーム、実機の再現試験は未確認。下記はコード・生成設定から確定できる事実と、利用者の症状の原因候補を分けて記載する。

## 確認事項と対処

| 優先度 | 確認した事実 | 利用者への影響／対処 |
| --- | --- | --- |
| 高 | 右手の既存生成設定は `CONFIG_ZMK_USB=y`、Studio transport は BLE のみ。Studio 用 UART chosen がない | USB の通常入力は対応するが USB Studio は未対応。右手の snippet に `studio-rpc-usb-uart` を追加 |
| 高 | README は FPC と Studio USB の snippet を併用不能と説明 | v0.3 workflow は `-S "${snippet}"` を渡し、Zephyr は空白をリストへ展開する。空白区切り文字列で併用し、説明を訂正 |
| 高 | Studio ロック無効時、`ZMK_STUDIO_LOCK_BLE_DIRECT_ADVERTISING_ON_UNLOCK` も既定で無効。接続済みプロファイルは広告を止める | OS 接続後に Web Bluetooth の候補へ出ない原因候補。同設定を明示的に有効にし、既存 `studio_unlock` キーで広告を開始する手順を追加 |
| 高 | 初期配列に `&out` がない。出力先は保存され、RPC の transport も現在の出力先に追従する | BLE 優先で USB が効かない、USB 給電中に BLE Studio が応答しない原因候補。レイヤー 3 の右 U／I に USB／BLE を追加 |
| 高 | 右が central、左が peripheral。左の `ZMK_USB=y` 要求が無効になる警告は役割の依存条件によるもの | 左 USB で通常の HID 入力ができないのは構成上の仕様。右へ挿す案内と、左右間は常に BLE という説明を追加 |
| 中 | Studio 保存済みの配列があると、ファームの新しい初期配列がそのまま使われない | 更新だけで新しい出力キーが現れるとは案内しない。既存配列への手動追加と、保存内容の退避・初期化の違いを案内 |
| 中 | README の本文は 0 dBm／6–12、実設定は +8 dBm／12–12 | 現在の値に訂正。15 ms は要求値で、実リンクでの保証値ではない |
| 中 | 右のキースキャン割り込みとセンサー復帰対策は既にマージ済み | 同じ修正を重ねない。古い出荷版なら更新対象。左右・復帰前後で実機確認する |
| 中 | 自動マウスレイヤーは固定 processor、倍率は runtime processor | Studio 側の自動レイヤー設定で全動作を自由に変えられるとは説明できない。倍率共有と調整範囲を明記 |
| 中 | ZMK と DYA モジュールの多くが可動ブランチ `main`／`main+dya` 参照 | 同じ config コミットの再ビルドでも依存更新により結果が変わる。配布前には成功ビルド成果物と解決済み依存 SHA を保存し、固定化を別途検討 |
| 低 | Studio ロックは意図的に無効 | 初回解除が必須という汎用 ZMK 手順は本構成に合わない。今回この方針は変更しない |
| 低 | 電池は Ni-MH 換算、通知間隔 3600 秒 | アルカリ使用や交換直後の残量を厳密な値と扱わない。起動 LED と残量表示も分けて案内 |

## 全体点検の補足

- 左右の charlieplex 入力と物理配置を比較し、各レイヤー 50 入力と transform／physical layout の 50 位置を確認済み。
- 右の `col-offset=6`、左右の `interrupt-gpios=P1.13`、`wakeup-source` がある。
- PAW3222 の SPI は SCLK=P0.10、SDIO=P0.09、MOTION=P1.12。センサーの NCS は GND 固定の構成。ドライバは復帰対策済み `fc946760...` に固定されている。
- 左トラックボールは split input で右へ送る。右の FPC が PAW3222 の標準構成を今回の検証対象とする。FPC エンコーダーへの差し替えなど全組合せの動作保証は含まない。
- XIAO には既存のコンソール用 CDC ACM ノードがあるため、旧版でもシリアルポートが見える場合がある。USB snippet は別の Studio 用ポートを追加する。ポートの存在だけでは Studio 対応を判定できず、改善版で複数候補が出る場合の選び直しも案内する。
- デバイスツリーには Studio の physical layout があり、chosen の matrix-transform は使っていない。
- 独自 C 実装は電池換算・起動 LED・縦横スクロール処理を確認。接続のために変更する必要を示す証拠はなかった。ADC 精度、電池寿命、斜めスクロールの操作感は実機で評価する。
- 既存右ビルドは Flash 368,260 B / 788 KiB、RAM 163,324 B / 256 KiB。USB 追加後も生成結果で確認する。
- 既存ログには deprecated KSCAN/NFCT、上流モジュール等のコンパイル警告がある。ビルド成功を無警告・実機正常の証明として扱わない。
- 今回は無線の送信出力、接続間隔、スリープ時間、センサーの速度・配線を変更しない。原因が未確認の症状に対して一括でパラメータを変更しない。

## 今回の変更

1. `build.yaml`：右の PAW3222 snippet と Studio USB snippet を同時に渡す。
2. `config/corcell.keymap`：レイヤー 3 の U 位置に `OUT_USB`、I 位置に `OUT_BLE`。通常レイヤーと既存の Bluetooth 登録操作を維持。
3. `corcell_r.conf`：ロック無効でも解除キーによる BLE ブラウザ検出を有効にする。
4. README／接続ガイド／Discord 告知案：旧版と改善版、通常入力と Studio、初回接続と初期化を区別して説明。

## 検証状況

- 検証コミット：`e073cfb217965b2b243b2de435efa81ce26f3190`（これ以降の本 PR 内の追記は説明・検証記録のみ）。
- [変更版 Build 34040690890](https://github.com/yuchamichami/zmk-config-Corcell/actions/runs/34040690890)：右手・左手・settings_reset・成果物結合すべて成功。
- 右手の生成設定：`ZMK_USB=y`、`ZMK_STUDIO_TRANSPORT_UART=y`、`ZMK_STUDIO_TRANSPORT_BLE=y`、`ZMK_STUDIO_LOCK_BLE_DIRECT_ADVERTISING_ON_UNLOCK=y`、`PAW3222=y` を確認。
- 右手の生成 devicetree：`zmk,studio-rpc-uart` が追加した CDC ACM ノードを参照することを確認。
- 両 snippet の適用をビルドログで確認。左手の `PAW3222=y` 維持と Studio UART 非搭載も確認。
- 右手メモリ：Flash 375,024 B / 788 KiB（46.48%）、RAM 166,908 B / 256 KiB（63.67%）。容量超過なし。
- キーマップ：6 レイヤー各 50 キー、physical layout／transform 各 50 位置、変更位置が L3 の 7・8 のみであることを照合。接続ガイドの操作位置と既存 Bluetooth 操作も照合。
- [キーマップ図の自動生成](https://github.com/yuchamichami/zmk-config-Corcell/actions/runs/34040611542) 成功。U／I の OUT USB／OUT BLE 表示を確認。
- ドキュメントの相対リンクと差分の空白エラーを確認。
- 実機未確認のため、配布済み・実機確認済みとは扱わない。

## 配布前の実機チェック

| 試験 | 手順と合格条件 | 結果 |
| --- | --- | --- |
| 新規 Bluetooth | 空きプロファイルで OS に登録し、左右の文字・左右ボールが入力できる | 未実施 |
| 右 USB HID | データケーブルで接続し USB 優先。左右のキーとボールが動く | 未実施 |
| USB Studio | PC Chrome／Edge でポート選択→読出し→編集→保存→再起動後保持 | 未実施 |
| BLE Studio | USB を外して BLE 出力にし、解除キーを押すとブラウザに候補が現れ、読出し・保存できる | 未実施 |
| 出力切替 | USB＋BLE 接続中に L3＋U／I で所定の機器に出力される。切替先で Studio 再接続できる | 未実施 |
| 既存利用者の更新 | Studio 配列・BLE 優先保存済みから UF2 更新。保存内容と手動キー追加を確認 | 未実施 |
| 左電源 OFF | 右の入力・接続操作は可能、左は無反応。左 ON 後に復帰する | 未実施 |
| 左右再接続 | 両側再起動、片側再起動、距離を離して戻す場合の再接続を確認 | 未実施 |
| スリープ復帰 | 15 分以上放置後に左右から起こし、キーと PAW3222 の両方が復帰する | 未実施 |
| センサー維持 | USB snippet 追加後も両ボール・スクロール・クリック・エンコーダーが動く | 未実施 |
| 設定初期化 | 検証個体で両側 reset→左右ファーム復旧→OS 再登録で使える | 未実施 |

最低限、Windows または macOS の実際の販売対象環境で USB／Bluetooth の両方を確認する。報告ごとに OS・配布ファーム・入力と Studio のどちらかを記録し、同じ原因と決めつけない。

## 参照した一次資料

- [対象ブランチ](https://github.com/yuchamichami/zmk-config-Corcell/tree/c2c3cb8bca8f061432a5976d0a7e396e535792b1)
- [使用中のビルド workflow](https://github.com/zmkfirmware/zmk/blob/v0.3/.github/workflows/build-user-config.yml)
- [対象 Zephyr の snippets 処理](https://github.com/cormoran/zephyr/blob/v4.1.0%2Bzmk-fixes%2Bnrf-half-duplex-uart/cmake/modules/snippets.cmake)
- [ZMK Studio](https://zmk.dev/docs/features/studio)
- [ZMK 接続トラブル](https://zmk.dev/docs/troubleshooting/connection-issues)
- [DYA fork の出力先処理](https://github.com/cormoran/zmk/blob/main%2Bdya/app/src/endpoints.c)
- [DYA fork の RPC transport 選択](https://github.com/cormoran/zmk/blob/main%2Bdya/app/src/studio/rpc.c)
- [DYA Studio](https://github.com/cormoran/dya-studio)

BLE 検出の根拠：DYA fork の `app/src/studio/Kconfig` は検出用広告の既定を locking＋BLE 時だけ有効にし、`app/src/studio/core.c` の unlock はこの設定の有効時だけ `zmk_ble_set_directed_advertising(true)` を呼ぶ。`app/src/ble.c` は active profile 接続済みのとき、このフラグがなければ広告を停止する。ロック無効でもこの設定を個別に有効化できる。
