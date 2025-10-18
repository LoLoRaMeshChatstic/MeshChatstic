# Wiring Guide - Heltec V3 with ILI9225 Display and CardKB

## Required Components

1. **Heltec WiFi LoRa 32 V3** - Base board
2. **ILI9225 2" TFT Display** - 176x220 pixels, SPI interface
3. **M5Stack CardKB** - I2C keyboard
4. Jumper wires for connections

## Display Wiring (ILI9225)

Connect the ILI9225 display to the Heltec V3 board:

| ILI9225 Pin | Heltec V3 GPIO | Function        |
| ----------- | -------------- | --------------- |
| VCC         | 3.3V           | Power           |
| GND         | GND            | Ground          |
| CS          | GPIO 5         | Chip Select     |
| RESET       | GPIO 33        | Reset           |
| DC/RS       | GPIO 7         | Data/Command    |
| SDI/MOSI    | GPIO 20        | SPI Data Out    |
| SCK         | GPIO 19        | SPI Clock       |
| LED         | GPIO 21        | Backlight (PWM) |
| SDO/MISO    | Not connected  | Not used        |

**Important Notes:**

- Display uses **SPI3_HOST** (separate from LoRa which uses SPI2_HOST)
- Both share physical pins but different CS lines prevent conflicts
- Backlight is PWM controlled for brightness adjustment

## CardKB Wiring

Connect the M5Stack CardKB to the Heltec V3 board:

| CardKB Pin | Heltec V3 GPIO | Function  |
| ---------- | -------------- | --------- |
| VCC (5V)   | 5V             | Power     |
| GND        | GND            | Ground    |
| SDA        | GPIO 41        | I2C Data  |
| SCL        | GPIO 42        | I2C Clock |

**Important Notes:**

- CardKB uses **secondary I2C bus** (not the internal OLED bus)
- Default I2C address: **0x5F**
- CardKB can work with 3.3V but 5V is recommended for better stability

## Power Considerations

### Option 1: USB Power

- Connect Heltec V3 to USB
- Both display and CardKB powered from Heltec board
- Simple and recommended for testing

### Option 2: Battery Power

- Use Heltec V3 battery connector (JST 1.25mm)
- Recommended: 3.7V LiPo 500-1000mAh
- Battery monitoring enabled (GPIO 1)

## Physical Layout Suggestion

```
┌─────────────────┐
│   CardKB        │ <- On top
└─────────────────┘
        ││
┌─────────────────┐
│   Heltec V3     │ <- Middle layer
│   with LoRa     │
└─────────────────┘
        ││
┌─────────────────┐
│   ILI9225       │ <- Display on bottom
│   TFT Display   │    (or face forward)
└─────────────────┘
```

## Testing Steps

### 1. Test Display Only

Before connecting CardKB, test display first:

1. Connect only display pins
2. Flash firmware: `pio run -e heltec-v3-ui -t upload`
3. Display should show Meshtastic boot screen
4. Check backlight is working

### 2. Test CardKB Separately

After display works:

1. Disconnect power
2. Connect CardKB to I2C secondary bus
3. Power on and monitor serial output
4. CardKB should be detected at address 0x5F
5. Test typing - characters should appear on screen

### 3. Full System Test

With everything connected:

1. Navigate UI using CardKB
2. Check LoRa functionality
3. Verify battery monitoring
4. Test all UI screens

## Troubleshooting

### Display not working

- ✅ Check all connections (especially CS, DC, RST)
- ✅ Verify backlight pin (GPIO 21) is connected
- ✅ Check SPI pins: SCK=19, MOSI=20
- ✅ Ensure display is powered (3.3V)

### CardKB not detected

- ✅ Check I2C connections: SDA=41, SCL=42
- ✅ Verify power (5V recommended)
- ✅ Scan I2C bus (should find device at 0x5F)
- ✅ Check you're using SECONDARY I2C bus, not primary

### Display shows but rotated wrong

- ✅ Firmware uses TFT_OFFSET_ROTATION=3 for landscape
- ✅ Can be changed in platformio.ini if needed

### LoRa not working

- ✅ Check antenna is connected
- ✅ Verify VEXT is enabled (GPIO 36)
- ✅ Ensure SPI pins are correct for LoRa

### System crashes

- ✅ Check for short circuits
- ✅ Verify power supply is adequate
- ✅ Monitor serial output for specific error messages

## Pin Summary Table

| Function     | GPIO | Notes                        |
| ------------ | ---- | ---------------------------- |
| **Display**  |
| Display CS   | 5    | ILI9225 Chip Select          |
| Display SCK  | 19   | SPI Clock (SPI3_HOST)        |
| Display MOSI | 20   | SPI Data Out                 |
| Display DC   | 7    | Data/Command                 |
| Display RST  | 33   | Reset                        |
| Display BL   | 21   | Backlight PWM                |
| **CardKB**   |
| CardKB SDA   | 41   | I2C Data (Secondary)         |
| CardKB SCL   | 42   | I2C Clock (Secondary)        |
| **LoRa**     |
| LoRa CS      | 8    | SX1262 Chip Select           |
| LoRa SCK     | 9    | SPI Clock (SPI2_HOST)        |
| LoRa MISO    | 11   | SPI Data In                  |
| LoRa MOSI    | 10   | SPI Data Out                 |
| LoRa RST     | 12   | Reset                        |
| LoRa DIO1    | 14   | IRQ                          |
| LoRa BUSY    | 13   | Busy Signal                  |
| **Other**    |
| Button       | 0    | Boot Button                  |
| LED          | 35   | Status LED                   |
| VEXT         | 36   | Power Control                |
| Battery      | 1    | ADC for monitoring           |
| OLED SDA     | 17   | Internal (not used with TFT) |
| OLED SCL     | 18   | Internal (not used with TFT) |

## Safety Notes

⚠️ **Important:**

- Never connect/disconnect while powered
- Double-check pin connections before powering on
- Use proper gauge wires for all connections
- Ensure proper ventilation if using battery
- Don't exceed 3.3V on GPIO pins

## Support

For issues or questions, check:

- Meshtastic Discord
- GitHub Issues
- Project README.md
