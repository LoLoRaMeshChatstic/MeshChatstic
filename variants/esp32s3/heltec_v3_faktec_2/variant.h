#define LED_PIN LED

// Secondary SSD1306 Status Display (original Heltec OLED as status monitor)
#define USE_STATUS_DISPLAY     // Enable secondary status display
#define STATUS_SDA SDA_OLED    // Use original OLED I2C pins for status display
#define STATUS_SCL SCL_OLED
#define STATUS_ADDR 0x3C       // Standard SSD1306 I2C address

#define RESET_OLED RST_OLED
#define I2C_SDA SDA_OLED // I2C pins for this board (OLED bus)
#define I2C_SCL SCL_OLED

// Enable secondary bus for external peripherals (CardKB)
#define I2C_SDA1 SDA     // GPIO 41 - CardKB and sensors
#define I2C_SCL1 SCL     // GPIO 42

#define VEXT_ENABLE Vext // active low, powers the lora antenna boost
#define BUTTON_PIN 0

#define ADC_CTRL 37
#define ADC_CTRL_ENABLED LOW
#define BATTERY_PIN 1           // Battery voltage measurement pin
#define ADC_CHANNEL ADC1_GPIO1_CHANNEL
#define ADC_ATTENUATION ADC_ATTEN_DB_2_5
#define ADC_MULTIPLIER 4.9 * 1.045

#define USE_SX1262

// ======================================
// LoRa SX1262 Configuration (SPI2_HOST default bus)
// ======================================
// Estos pines están en el bus SPI por defecto del ESP32-S3
// IMPORTANTE: NO compartir estos pines con otras funciones
#define LORA_DIO0 -1            // No connect on SX1262
#define LORA_RESET 12           // Pin de reset del módulo LoRa
#define LORA_DIO1 14            // SX1262 IRQ - Interrupt pin
#define LORA_DIO2 13            // SX1262 BUSY - Status pin
#define LORA_DIO3               // TXCO enable (internal)

// Pines SPI para LoRa (SPI2_HOST - bus por defecto)
#define LORA_SCK 9              // SPI Clock
#define LORA_MISO 11            // SPI MISO (Master In Slave Out)
#define LORA_MOSI 10            // SPI MOSI (Master Out Slave In)
#define LORA_CS 8               // Chip Select

#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_DIO2
#define SX126X_RESET LORA_RESET

// Heltec V3 usa DIO2 para controlar el RF switch internamente
#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_DIO3_TCXO_VOLTAGE 1.8

// Desactivado temporalmente por problemas de inicialización
// #define HAS_32768HZ 1

#define HAS_SCREEN 1
#define HAS_GPS 0
