#include "ST7920U8G2Display.h"

#ifdef ST7920_CLK

#include "configuration.h"

ST7920U8G2Display::ST7920U8G2Display(int8_t clk, int8_t mosi, int8_t cs, int8_t rst, int8_t v0)
    : pin_clk(clk), pin_mosi(mosi), pin_cs(cs), pin_rst(rst), pin_v0(v0), u8g2(nullptr), initialized(false) {
}

ST7920U8G2Display::~ST7920U8G2Display() {
    if (u8g2) {
        delete u8g2;
    }
}

bool ST7920U8G2Display::init() {
    if (initialized) return true;
    
    LOG_INFO("ST7920U8G2: Initializing with U8G2 library");
    LOG_INFO("ST7920U8G2: Pins CLK=%d, MOSI=%d, CS=%d, RST=%d, V0=%d", 
             pin_clk, pin_mosi, pin_cs, pin_rst, pin_v0);
    
    // Initialize contrast pin (V0) with PWM if specified
    if (pin_v0 >= 0) {
        pinMode(pin_v0, OUTPUT);
        // Setup PWM for contrast control
        ledcSetup(0, 1000, 8); // Channel 0, 1kHz, 8-bit resolution
        ledcAttachPin(pin_v0, 0);
        // Set high contrast (low voltage on V0)
        ledcWrite(0, 30); // Even lower for maximum contrast
        LOG_INFO("ST7920U8G2: Contrast PWM setup on pin %d", pin_v0);
    }
    
    // Create U8G2 object with software SPI
    u8g2 = new U8G2_ST7920_128X64_F_SW_SPI(U8G2_R0, pin_clk, pin_mosi, pin_cs, pin_rst);
    
    // Initialize U8G2
    u8g2->begin();
    
    LOG_INFO("ST7920U8G2: U8G2 initialized successfully");
    
    // Test pattern
    clear();
    u8g2->setFont(u8g2_font_ncenB08_tr);
    u8g2->drawStr(0, 15, "ST7920 Test");
    u8g2->drawStr(0, 30, "U8G2 Working!");
    
    // Draw test pattern
    for (int x = 0; x < 128; x += 10) {
        u8g2->drawPixel(x, 50);
    }
    for (int y = 0; y < 64; y += 10) {
        u8g2->drawPixel(100, y);
    }
    
    display();
    
    initialized = true;
    return true;
}

void ST7920U8G2Display::clear() {
    if (u8g2) {
        u8g2->clearBuffer();
    }
}

void ST7920U8G2Display::display() {
    if (u8g2) {
        u8g2->sendBuffer();
    }
}

void ST7920U8G2Display::setPixel(int16_t x, int16_t y, bool color) {
    if (u8g2 && x >= 0 && x < 128 && y >= 0 && y < 64) {
        if (color) {
            u8g2->drawPixel(x, y);
        } else {
            // U8G2 doesn't have a direct way to clear a pixel in buffer mode
            // This is a limitation, but for most uses it's fine
        }
    }
}

void ST7920U8G2Display::drawString(int16_t x, int16_t y, const String& text) {
    drawString(x, y, text.c_str());
}

void ST7920U8G2Display::drawString(int16_t x, int16_t y, const char* text) {
    if (u8g2) {
        u8g2->drawStr(x, y, text);
    }
}

void ST7920U8G2Display::drawText(int16_t x, int16_t y, const char* text) {
    drawString(x, y, text);
}

void ST7920U8G2Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    if (u8g2) {
        u8g2->drawLine(x0, y0, x1, y1);
    }
}

void ST7920U8G2Display::drawFrame(int16_t x, int16_t y, int16_t width, int16_t height) {
    if (u8g2) {
        u8g2->drawFrame(x, y, width, height);
    }
}

void ST7920U8G2Display::fillRect(int16_t x, int16_t y, int16_t width, int16_t height) {
    if (u8g2) {
        u8g2->drawBox(x, y, width, height);
    }
}

void ST7920U8G2Display::drawCircle(int16_t x, int16_t y, int16_t radius) {
    if (u8g2) {
        u8g2->drawCircle(x, y, radius);
    }
}

void ST7920U8G2Display::fillCircle(int16_t x, int16_t y, int16_t radius) {
    if (u8g2) {
        u8g2->drawDisc(x, y, radius);
    }
}

uint16_t ST7920U8G2Display::getTextWidth(const char* text) {
    if (u8g2) {
        return u8g2->getStrWidth(text);
    }
    return 0;
}

void ST7920U8G2Display::setContrast(uint8_t contrast) {
    if (pin_v0 >= 0) {
        // Set PWM duty cycle for contrast
        // Lower values = higher contrast for ST7920
        ledcWrite(0, 255 - contrast);
    }
    
    // Also set U8G2 contrast if supported
    if (u8g2) {
        u8g2->setContrast(contrast);
    }
}

#endif // ST7920_CLK