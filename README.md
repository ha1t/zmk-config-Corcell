# Corchibi2 ZMK config

This config is a PAW3222-based dry-cell variant that reuses Corchibi's keymap,
matrix transform, physical layout, and charlieplex kscan.

## Hardware split

- `Corchibi2_L` is the split peripheral and forwards the left PAW3222 input.
- `Corchibi2_R` is the split central and handles the local PAW3222 sensor plus
  the forwarded left input.
- Key and kscan pins match Corchibi.
- PAW3222 uses `SCLK=P0.10`, `SDIO=P0.09`, and `MOTION=P1.12`.
- PAW3222 NCS is tied to GND by default, so the firmware does not configure a
  SPI chip-select GPIO. If the jumper is changed to route NCS to `RE_B`, add
  `cs-gpios = <&gpio0 5 GPIO_ACTIVE_LOW>;` under `&spi0` and disable or move
  the encoder B pin.
- Rotary encoder uses `RE_A=P0.04` and `RE_B=P0.05`.
- `INPUT_VOLTAGE` is read on ADC0 / `P0.02`.

## Power policy

- ZMK sleep is enabled.
- BLE TX power is set to 0 dBm instead of the Corchibi +8 dBm setting.
- PAW3222 `force-awake` is not enabled.
- Logging, shell, and SPI shell are disabled.
- The insomnia behavior module is not included.
- Battery reporting follows DYA Dash's dry-cell voltage-divider style:
  `output-ohms = 470k`, `full-ohms = 1M + 470k`, with Ni-MH 1-cell
  millivolt-to-percent thresholds.

## 3D data

Case and related 3D model files are available under `3D_data/`.

## License

- Firmware source code, ZMK configuration files, and documentation outside
  `3D_data/` are licensed under the MIT License. See `LICENSE`.
- 3D model files under `3D_data/` are licensed separately under
  CC BY-NC-ND 4.0. Commercial use is not permitted, and modifications are
  limited to private, personal use. See `3D_data/LICENSE.md`.
