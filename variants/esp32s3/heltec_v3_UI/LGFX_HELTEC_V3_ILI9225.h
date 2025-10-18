#pragma once

/**
 * @file LGFX_HELTEC_V3_ILI9225.h
 * @brief LovyanGFX configuration for Heltec WiFi LoRa 32 V3 with ILI9225 TFT
 * @version 1.0
 * @date 2025-10-16
 *
 * Hardware configuration:
 * - Board: Heltec WiFi LoRa 32 V3 (ESP32-S3)
 * - Display: ILI9225 TFT 176x220 2" SPI Display
 * - Secondary: SSD1306 OLED 128x64 I2C (status display)
 * - LoRa: SX1262 on separate SPI bus
 */

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#ifndef SPI_FREQUENCY
#define SPI_FREQUENCY 10000000
#endif

#ifndef SPI_READ_FREQUENCY
#define SPI_READ_FREQUENCY 8000000
#endif

class LGFX_HELTEC_V3_ILI9225 : public lgfx::LGFX_Device
{
    lgfx::Panel_ILI9225 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;

  public:
    const uint32_t screenWidth = 176;
    const uint32_t screenHeight = 220;

    bool hasButton(void) { return true; }  // BOOT button on GPIO 0

    LGFX_HELTEC_V3_ILI9225(void)
    {
        {
            auto cfg = _bus_instance.config();

            // SPI configuration for ILI9225
            cfg.spi_host = SPI3_HOST;           // Separate SPI bus from LoRa (SPI2_HOST)
            cfg.spi_mode = 0;                    // SPI Mode 0
            cfg.freq_write = SPI_FREQUENCY;      // 10MHz write (ILI9225 max ~40MHz, conservative)
            cfg.freq_read = SPI_READ_FREQUENCY;  // 8MHz read (safer for long wires)
            cfg.spi_3wire = false;               // 4-wire SPI (separate MOSI/MISO)
            cfg.use_lock = true;                 // Enable SPI transaction locking (critical for stability)
            cfg.dma_channel = SPI_DMA_CH_AUTO;   // Auto-select DMA channel
            cfg.pin_sclk = 19;                   // SCK
            cfg.pin_mosi = 20;                   // MOSI (SDI)
            cfg.pin_miso = -1;                   // MISO not connected (display is write-only)
            cfg.pin_dc = 7;                      // DC (Data/Command select)

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();

            cfg.pin_cs = 5;                      // Chip Select
            cfg.pin_rst = 33;                    // Reset
            cfg.pin_busy = -1;                   // Not used

            // Display configuration for ILI9225 (176x220 pixels)
            cfg.panel_width = 176;               // Physical width
            cfg.panel_height = 220;              // Physical height
            cfg.offset_x = 0;                    // No X offset
            cfg.offset_y = 0;                    // No Y offset
            cfg.offset_rotation = 1;             // Rotation offset (0-7, experiment for correct orientation)
            cfg.dummy_read_pixel = 8;            // Dummy bits before pixel read
            cfg.dummy_read_bits = 1;             // Dummy bits before data read
            cfg.readable = true;                 // Display supports reading (limited on ILI9225)
            cfg.invert = false;                  // No color inversion
            cfg.rgb_order = false;               // RGB order (false = RGB, true = BGR)
            cfg.dlen_16bit = false;              // 8-bit data length
            cfg.bus_shared = false;              // Dedicated SPI bus (not shared with SD/LoRa)

            // ILI9225 specific - memory width/height
            cfg.memory_width = 176;
            cfg.memory_height = 220;

            _panel_instance.config(cfg);
        }

        {
            auto cfg = _light_instance.config();

            cfg.pin_bl = 21;                     // Backlight control pin
            cfg.invert = false;                  // Active HIGH backlight
            cfg.freq = 44100;                    // PWM frequency for backlight
            cfg.pwm_channel = 7;                 // PWM channel (0-15)

            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }
};
