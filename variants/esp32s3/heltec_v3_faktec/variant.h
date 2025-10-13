#define LED_PIN LED

// Use ST7920 instead of SSD1306 for primary display
// Note: USE_ST7920 is already defined in platformio.ini, don't redefine here

// Secondary SSD1306 Status Display (original Heltec OLED as status monitor)
#define USE_STATUS_DISPLAY     // Enable secondary status display
#define STATUS_SDA SDA_OLED    // Use original OLED I2C pins for status display
#define STATUS_SCL SCL_OLED
#define STATUS_ADDR 0x3C       // Standard SSD1306 I2C address

#define RESET_OLED RST_OLED
#define I2C_SDA SDA_OLED // I2C pins for this board
#define I2C_SCL SCL_OLED

// ST7920 LCD display pins (SPI)
#define ST7920_CLK 33   // SCLK/E
#define ST7920_MOSI 34  // SID/RW
#define ST7920_CS 5     // CS/RS
#define ST7920_RST 21   // RST
#define ST7920_V0 3     // Contrast control (PWM)

// Enable secondary bus for external periherals
#define I2C_SDA1 SDA
#define I2C_SCL1 SCL

#define VEXT_ENABLE Vext // active low, powers the oled display and the lora antenna boost
#define VEXT_ON_VALUE LOW // Heltec V3 uses active low for VEXT
#define BUTTON_PIN 0

#define ADC_CTRL 37
#define ADC_CTRL_ENABLED LOW
#define BATTERY_PIN 1 // A battery voltage measurement pin, voltage divider connected here to measure battery voltage
#define ADC_CHANNEL ADC1_GPIO1_CHANNEL
#define ADC_ATTENUATION ADC_ATTEN_DB_2_5 // lower dB for high resistance voltage divider
#define ADC_MULTIPLIER 4.9 * 1.045

#define USE_SX1262

#define LORA_DIO0 -1 // a No connect on the SX1262 module
#define LORA_RESET 12
#define LORA_DIO1 14 // SX1262 IRQ
#define LORA_DIO2 13 // SX1262 BUSY
#define LORA_DIO3    // Not connected on PCB, but internally on the TTGO SX1262, if DIO3 is high the TXCO is enabled

#define LORA_SCK 9
#define LORA_MISO 11
#define LORA_MOSI 10
#define LORA_CS 8

#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_DIO2
#define SX126X_RESET LORA_RESET

#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_DIO3_TCXO_VOLTAGE 1.8

#define HAS_32768HZ 1