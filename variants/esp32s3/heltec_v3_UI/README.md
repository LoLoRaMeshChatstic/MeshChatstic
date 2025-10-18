# Heltec WiFi LoRa 32 V3 - UI Graphical Interface

Custom variant of Heltec V3 with ILI9225 TFT display and graphical UI interface.

## Hardware Configuration

### Base Board

- **Board**: Heltec WiFi LoRa 32 V3
- **MCU**: ESP32-S3 (with PSRAM)
- **Flash**: 8MB
- **LoRa**: SX1262 (868MHz/915MHz)

### Display

- **Type**: ILI9225 TFT LCD
- **Size**: 2" (176x220 pixels)
- **Interface**: SPI
- **Orientation**: Landscape (220x176 after rotation)
- **Backlight**: PWM controlled

### Input Device

- **Type**: M5Stack CardKB
- **Interface**: I2C (address 0x5F)
- **Connection**: SDA=41, SCL=42

## Pin Configuration

### Display Pins (ILI9225)

- **CS**: GPIO 5
- **SCK**: GPIO 19
- **MOSI**: GPIO 20
- **MISO**: Not connected
- **DC**: GPIO 7
- **RST**: GPIO 33
- **BL**: GPIO 21 (Backlight)

### LoRa Pins (SX1262)

- **CS**: GPIO 8
- **SCK**: GPIO 9
- **MISO**: GPIO 11
- **MOSI**: GPIO 10
- **RESET**: GPIO 12
- **DIO1**: GPIO 14 (IRQ)
- **BUSY**: GPIO 13

### I2C Pins

- **Primary (Internal OLED)**: SDA=17, SCL=18
- **Secondary (CardKB)**: SDA=41, SCL=42

### Other Pins

- **Button**: GPIO 0
- **LED**: GPIO 35
- **VEXT**: GPIO 36 (Power control)
- **Battery**: GPIO 1 (ADC)

## Features

- ✅ Graphical UI based on device-ui library
- ✅ CardKB keyboard support
- ✅ ILI9225 TFT display (landscape mode)
- ✅ SPI3_HOST for display (separate from LoRa)
- ✅ PSRAM support
- ✅ Battery monitoring
- ✅ LoRa mesh networking

## Building

To compile this variant:

```bash
pio run -e heltec-v3-ui
```

To upload:

```bash
pio run -e heltec-v3-ui -t upload
```

## Wiring Guide

See [WIRING_GUIDE.md](WIRING_GUIDE.md) for detailed connection instructions.

## Notes

- Display uses SPI3_HOST to avoid conflicts with LoRa radio (SPI2_HOST)
- CardKB must be connected to secondary I2C bus (SDA=41, SCL=42)
- UI is adapted from T-Deck but optimized for 220x176 resolution
- This variant requires more flash than standard builds due to UI library

## Credits

Based on Heltec V3 Faktec 2 variant with UI additions inspired by LilyGo T-Deck configuration.
