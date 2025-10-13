#pragma once

#ifdef ST7920_CLK

#include <Arduino.h>
#include <OLEDDisplay.h>
#include <U8g2lib.h>

/**
 * ST7920 Display Driver for Meshtastic
 * Extends OLEDDisplay to integrate ST7920 128x64 LCD as primary display
 */
class ST7920Display : public OLEDDisplay {
private:
    int8_t pin_clk;
    int8_t pin_mosi; 
    int8_t pin_cs;
    int8_t pin_rst;
    int8_t pin_v0;
    U8G2_ST7920_128X64_F_SW_SPI* u8g2;
    bool initialized;

public:
    ST7920Display(int8_t clk, int8_t mosi, int8_t cs, int8_t rst = -1, int8_t v0 = -1);
    virtual ~ST7920Display();
    
    // Métodos virtuales puros que DEBO implementar
    virtual void display(void) override;
    virtual int getBufferOffset(void) override;
    
    // Métodos virtuales opcionales
    virtual void setBrightness(uint8_t brightness) override;
    virtual bool connect() override;
    virtual void sendInitCommands() override;
    
    // Métodos específicos del ST7920
    bool init();
    void showTestPattern();
    void setupPWMContrast();
    
    // Métodos de compatibilidad con uso anterior
    void drawString(int16_t x, int16_t y, const char* text);
    void clear();
};

#endif // ST7920_CLK