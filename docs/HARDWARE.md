# Cなし1.75モデル：公式コード確認記録

調査日：2026-10-07。公式リポジトリ： https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75

固定コミット：`e4344e70c2fa78a13e8a06566507f1ba8af6672a`。

## 確認したファイル

- [GPIO・I²C・メモリ仕様](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/HARDWARE_REFERENCE.md)
- [維持されているBSPのピン定義](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/firmware/brookesia/components/waveshare__esp32_s3_touch_amoled_1_75/include/bsp/esp32_s3_touch_amoled_1_75.h)
- [LCD初期化・輝度・タッチのBSP実装](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/firmware/brookesia/components/waveshare__esp32_s3_touch_amoled_1_75/esp32_s3_touch_amoled_1_75.c)
- [AXP2101公式サンプル](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/examples/esp-idf/01_AXP2101/main/port_axp2101.cpp)
- [PCF85063公式サンプル](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/examples/arduino/03_LVGL_PCF85063_simpleTime/03_LVGL_PCF85063_simpleTime.ino)
- [PCF85063レジスタ操作](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/examples/arduino/libraries/SensorLib/src/SensorPCF85063.hpp)
- [QMI8658レジスタ定義](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/examples/arduino/libraries/SensorLib/src/REG/QMI8658Constants.h)

## LCD

CO5300、466×466、RGB565、QSPI SPI2。CLK38、CS12、DATA4/5/6/7、RST39、TE13。公式BSPの初期化順序：

```
FE 20; 19 10; 1C A0;
FE 00; C4 80; 3A 55; 35 00; 53 20;
51 FF; 63 FF;
2A 00 06 01 D7;
2B 00 00 01 D1; wait 600 ms;
11; wait 600 ms; 29;
```

同じテーブルを採用し、`esp_lcd_panel_set_gap(panel,6,0)`、reset、init、display onの順。QSPIドライバの既定40MHzを使用（Flash/PSRAMの80MHzとは別）。タイル転送の開始/終端を偶数にそろえ、466端でも条件を維持します。

輝度は `esp_lcd_panel_io_tx_param(io, (0x02<<24)|(0x51<<8), &value, 1)`、value = percent×255/100。AMOLEDなのでLEDバックライト用GPIOはありません。起動後は8%へ下げてから時間帯設定を適用します。TE13は信号として予約し、今回の部分更新はTE同期を使用しません。

## I²C

SCL14、SDA15、400kHz。タッチ0x5A、PMIC0x34、IMU0x6B、RTC0x51、TCA9554 0x20、ES8311 0x18、ES7210 0x40。バスは共通ハンドルを再利用し、RTCの複数レジスタ操作はmutexで直列化します。

CST9217 RST40とLCD RST39は別信号。タッチは公式コンポーネント2.0.0の初期化・座標読出しを使用。INT11は設定し、入力は10msのポーリングで取り込みます。

PCF85063は0x04からseconds/minutes/hours/day/weekday/month/yearの7バイトを読み書き。seconds bit7(OS)、CTRL1 bit5(STOP)、bit1(12/24h)を検証。書込み時はSTOPで日付更新を保護し、最後に必ず解除します。GPIOで擬似RTCを作りません。

AXP2101は0x03のID=0x4Aを確認し、公式XPowersLibと同じADC_CHANNEL_CTRL=0x30のbit1をクリアして未接続TS測定を停止。公式サンプルの汎用電源レール変更部分はコメントアウトされており、時計でも勝手に電源電圧を変更しません。

QMI8658はWHO_AM_I(0x00)=0x05。CTRL1=0x40（連続読出し）、CTRL2=0x06（±2g、125Hz）、CTRL5=0x01（LPF mode 0）、CTRL7=0x01（加速度のみ）。0x35〜0x3Aを25Hzで読み、16384 LSB/gで換算します。INT2=GPIO21、INT1はTCA9554 P6。割込みは使わず別タスクでポーリングします。画面重力ベクトルは公式 `firmware/brookesia/components/Gravitysphere/Gravitysphere.cpp` のworkerLoopと同じ（−accelY, +accelX）。

## その他のGPIO（誤用しないための予約表）

| GPIO | 用途 |
|---|---|
| 0 | BOOT |
| 1/2/3 | microSD CMD/CLK/D0（LCD/TouchのRESETではない） |
| 8/9/10 | Audio DOUT/BCLK/DIN |
| 16/17/18 | 拡張端子（G版GPSの扱いは対象外） |
| 19/20 | native USB D−/D＋ |
| 21 | QMI8658 INT2 |
| 41 | microSD D3/CS |
| 42 | Audio MCLK |
| 43/44 | UART0 TX/RX |
| 45/46 | Audio LRCK/PA control |

TCA9554はP3=RTC_INT、P4=PWR状態、P5=AXP_IRQ、P6=QMI_INT1、P7=G版GPS_RST。音声、SD、GPS、expander割込み処理は時計には不要なため初期化しません。Wi-FiはESP32-S3内蔵無線です。

## 旧プロジェクトとの差分

旧コードは1.75Cと明記され、LCD_RST=1、TP_RST=2、Flash32MBでした。現在のソースはLCD_RST=39、TP_RST=40、Flash16MB。旧プロジェクト全体は `legacy_sand_sim/` に退避し、現在のコンポーネント探索対象外です。
