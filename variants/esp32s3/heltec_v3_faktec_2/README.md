# Heltec V3 FAKTEC 2 - ILI9225 Display Variant

## Hardware Configuration

### Display: ILI9225 TFT

- **Size**: 2.0 inches
- **Resolution**: 176 x 220 pixels
- **Interface**: SPI
- **Driver**: LovyanGFX
- **Colors**: 65K (RGB565)

### Pin Configuration

#### SPI Display Pins

| Function | GPIO | Description             |
| -------- | ---- | ----------------------- |
| CS       | 5    | Chip Select             |
| SCK      | 19   | SPI Clock               |
| MOSI     | 20   | SPI Data Out (SDA)      |
| MISO     | 11   | SPI Data In (optional)  |
| DC (RS)  | 34   | Data/Command Select     |
| RST      | 33   | Display Reset           |
| BL       | 21   | Backlight Control (PWM) |

#### I2C Bus (CardKB & Peripherals)

| Function | GPIO | Description |
| -------- | ---- | ----------- |
| SDA      | 41   | I2C Data    |
| SCL      | 42   | I2C Clock   |

#### LoRa (SX1262)

| Function | GPIO | Description        |
| -------- | ---- | ------------------ |
| CS       | 8    | Chip Select        |
| SCK      | 19   | SPI Clock (shared) |
| MOSI     | 20   | SPI MOSI (shared)  |
| MISO     | 11   | SPI MISO (shared)  |
| RESET    | 12   | LoRa Reset         |
| DIO1     | 14   | LoRa IRQ           |
| BUSY     | 13   | LoRa Busy          |

#### Other

| Function | GPIO | Description            |
| -------- | ---- | ---------------------- |
| LED      | 35   | Status LED             |
| Button   | 0    | User Button            |
| Vext     | 36   | External Power Control |
| Battery  | 1    | Battery Voltage (ADC)  |

## Features

- ✅ 2" Color TFT Display (176x220)
- ✅ LoRa SX1262 Radio
- ✅ CardKB Support (M5Stack I2C Keyboard)
- ✅ Battery Monitoring
- ✅ GPS Support (optional UART2 on GPIO 3/46)
- ✅ 8MB Flash

## Display Configuration

- **SPI Host**: SPI3_HOST
- **Write Frequency**: 40 MHz
- **Read Frequency**: 16 MHz
- **Backlight**: Active HIGH, PWM capable
- **Orientation**: Can be rotated in software

## Firmware Build

```bash
pio run -e heltec-v3-faktec-2
```

## Firmware Upload

```bash
pio run -e heltec-v3-faktec-2 --target upload
```

## Display Specifications

- **Controller**: ILI9225
- **Interface**: 4-wire SPI
- **Supported by**: LovyanGFX library
- **Memory**: Frame buffer support
- **Fonts**: Larger fonts enabled (176x220 resolution)

## Notes

- The ILI9225 shares SPI bus with LoRa radio (different CS pins)
- Backlight can be PWM controlled for brightness adjustment
- Display supports portrait and landscape modes
- CardKB keyboard peripheral uses separate I2C bus (GPIO 41/42)
- Internal OLED (if present) is disabled to save power

## Compatibility

- **Base Board**: Heltec WiFi LoRa 32 V3 (ESP32-S3)
- **Display Module**: Generic 2" ILI9225 176x220 SPI TFT
- **Keyboard**: M5Stack CardKB (optional, I2C address 0x5F)

## Wiring Diagram

```
ESP32-S3 Heltec V3       ILI9225 Display
==================       ===============
GPIO 5 (CS)     -------> CS
GPIO 19 (SCK)   -------> SCK
GPIO 20 (MOSI)  -------> SDI/SDA
GPIO 11 (MISO)  -------> SDO (optional)
GPIO 34 (DC)    -------> RS/DC
GPIO 33 (RST)   -------> RESET
GPIO 21 (BL)    -------> LED/BL
3.3V            -------> VCC
GND             -------> GND
```

## CardKB Wiring (Optional)

```
ESP32-S3 Heltec V3       M5Stack CardKB
==================       ==============
GPIO 41 (SDA)   -------> SDA
GPIO 42 (SCL)   -------> SCL
3.3V            -------> 5V (has regulator)
GND             -------> GND
```

## Troubleshooting

### Display not working

1. Check all wiring connections
2. Verify SPI pins match configuration
3. Check backlight is powered (GPIO 21 HIGH)
4. Ensure display is powered (3.3V)

### Display shows garbage/wrong colors

1. Try adjusting TFT_INVERT setting (true/false)
2. Check SPI frequency (reduce to 20MHz if unstable)
3. Verify ground connection is solid

### CardKB not detected

1. Run I2C scanner to verify address 0x5F
2. Check SDA/SCL connections on GPIO 41/42
3. Verify CardKB has power (LED should light)

## Credits

- Display driver: LovyanGFX library
- Base configuration: Heltec V3 variant
- MeshChatstic firmware enhancements
