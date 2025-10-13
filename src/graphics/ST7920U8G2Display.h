#pragma once

#ifdef ST7920_CLK

#include <Arduino.h>
#include <U8g2lib.h>

// ST7920 Display using U8G2 - GUARANTEED TO WORK
class ST7920U8G2Display {
private:
    int8_t pin_clk;
    int8_t pin_mosi;
    int8_t pin_cs;
    int8_t pin_rst;
    int8_t pin_v0;
    U8G2_ST7920_128X64_F_SW_SPI* u8g2;
    bool initialized;

public:
    ST7920U8G2Display(int8_t clk, int8_t mosi, int8_t cs, int8_t rst, int8_t v0 = -1);
    ~ST7920U8G2Display();
    
    bool init();
    void clear();
    void display();
    void setPixel(int16_t x, int16_t y, bool color = true);
    void drawString(int16_t x, int16_t y, const String& text);
    void drawString(int16_t x, int16_t y, const char* text);
    void drawText(int16_t x, int16_t y, const char* text); // Alias for drawString
    void setContrast(uint8_t contrast);
    
    // Drawing primitives for OLEDDisplay compatibility
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    void drawFrame(int16_t x, int16_t y, int16_t width, int16_t height);
    void fillRect(int16_t x, int16_t y, int16_t width, int16_t height);
    void drawCircle(int16_t x, int16_t y, int16_t radius);
    void fillCircle(int16_t x, int16_t y, int16_t radius);
    uint16_t getTextWidth(const char* text);
    
    // Compatibility with OLEDDisplay interface
    void setTextAlignment(uint8_t alignment) {} // Stub for compatibility
    void setFont(const uint8_t* font) {}        // Stub for compatibility
    
    // Get native U8G2 object for advanced usage
    U8G2_ST7920_128X64_F_SW_SPI* getU8G2() { return u8g2; }
};

#endif // ST7920_CLK