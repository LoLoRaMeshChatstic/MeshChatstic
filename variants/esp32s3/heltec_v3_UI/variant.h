#define LED_PIN LED

// Secondary SSD1306 Status Display (original Heltec OLED as status monitor)
#define USE_STATUS_DISPLAY     // Enable secondary status display
#define STATUS_SDA SDA_OLED    // Use original OLED I2C pins for status display
#define STATUS_SCL SCL_OLED
#define STATUS_ADDR 0x3C       // Standard SSD1306 I2C address

// I2C del OLED interno (necesario para que VEXT funcione correctamente)
#define RESET_OLED RST_OLED
#define I2C_SDA SDA_OLED // I2C pins for this board
#define I2C_SCL SCL_OLED

// Enable secondary bus for external peripherals (CardKB)
#define I2C_SDA1 SDA
#define I2C_SCL1 SCL

#define VEXT_ENABLE Vext // active low, powers the lora antenna boost
#define BUTTON_PIN 0

#define ADC_CTRL 37
#define ADC_CTRL_ENABLED LOW
#define BATTERY_PIN 1           // Battery voltage measurement pin
#define ADC_CHANNEL ADC1_GPIO1_CHANNEL
#define ADC_ATTENUATION ADC_ATTEN_DB_2_5
#define ADC_MULTIPLIER 4.9 * 1.045

#define USE_SX1262

// LoRa SX1262 pins (igual que Heltec V3 estándar)
#define LORA_DIO0 -1            // No connect on SX1262
#define LORA_RESET 12
#define LORA_DIO1 14            // SX1262 IRQ
#define LORA_DIO2 13            // SX1262 BUSY
#define LORA_DIO3               // TXCO enable (internal)

#define LORA_SCK 9
#define LORA_MISO 11
#define LORA_MOSI 10
#define LORA_CS 8

#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_DIO2
#define SX126X_RESET LORA_RESET

// Heltec V3 usa DIO2 para controlar el RF switch internamente
#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_DIO3_TCXO_VOLTAGE 1.8

#define HAS_SCREEN 1
#define HAS_TFT 1
#define HAS_GPS 0

// ILI9225 TFT LCD - 176x220 2" Display
#define TFT_CS 5
#define TFT_DC 7
#define TFT_RST 33
#define TFT_BL 21
#define TFT_SCK 19
#define TFT_MOSI 20
#define TFT_MISO -1
#define TFT_SPI_HOST SPI3_HOST
#define TFT_WIDTH 176
#define TFT_HEIGHT 220
#define TFT_OFFSET_X 0
#define TFT_OFFSET_Y 0
#define TFT_OFFSET_ROTATION 1
// SCREEN_ROTATE is defined in platformio.ini
#define SPI_FREQUENCY 10000000
#define SPI_READ_FREQUENCY 8000000
#define TFT_BACKLIGHT_ON HIGH

