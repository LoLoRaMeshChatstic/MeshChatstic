#include "ST7920Display.h"

#ifdef ST7920_CLK

#include "configuration.h"

ST7920Display::ST7920Display(int8_t clk, int8_t mosi, int8_t cs, int8_t rst, int8_t v0)
    : OLEDDisplay(), pin_clk(clk), pin_mosi(mosi), pin_cs(cs), pin_rst(rst), pin_v0(v0), 
      u8g2(nullptr), initialized(false) {
    
    // Set display dimensions para OLEDDisplay
    displayWidth = 128;
    displayHeight = 64;
}

ST7920Display::~ST7920Display() {
    if (u8g2) {
        delete u8g2;
    }
}

bool ST7920Display::connect() {
    if (initialized) return true;
    
    LOG_INFO("ST7920: Initializing display with pins CLK=%d, MOSI=%d, CS=%d, RST=%d, V0=%d", 
             pin_clk, pin_mosi, pin_cs, pin_rst, pin_v0);
    
    // Setup contrast pin (V0) with PWM if specified
    if (pin_v0 >= 0) {
        setupPWMContrast();
    }
    
    // Create U8G2 instance
    u8g2 = new U8G2_ST7920_128X64_F_SW_SPI(U8G2_R0, pin_clk, pin_mosi, pin_cs, pin_rst);
    
    if (!u8g2) {
        LOG_ERROR("ST7920: Failed to create U8G2 instance");
        return false;
    }
    
    // Initialize U8G2
    if (!u8g2->begin()) {
        LOG_ERROR("ST7920: U8G2 begin() failed");
        delete u8g2;
        u8g2 = nullptr;
        return false;
    }
    
    // Clear display
    u8g2->clearBuffer();
    
    // Setup PWM contrast if V0 pin specified (non-blocking)
    setupPWMContrast();
    
    // Quick initial display clear (no fancy test pattern during boot)
    u8g2->sendBuffer();
    
    initialized = true;
    LOG_INFO("ST7920: Successfully connected and initialized");
    return true;
}

void ST7920Display::sendInitCommands() {
    // U8G2 handles initialization internally
    if (u8g2) {
        u8g2->begin();
    }
}

void ST7920Display::display(void) {
    if (!u8g2 || !initialized || !buffer) return;
    
    // Optimización: copiar buffer completo de OLEDDisplay a U8G2 eficientemente
    // Similar al patrón de TFTDisplay pero adaptado para U8G2
    
    u8g2->clearBuffer();
    
    // Acceder directamente al buffer interno de U8G2 para máxima eficiencia
    uint8_t* u8g2_buffer = u8g2->getBufferPtr();
    if (!u8g2_buffer) {
        // Fallback: usar método pixel por pixel solo si es necesario
        LOG_DEBUG("ST7920: Using pixel fallback method");
        for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 128; x++) {
                int page = y / 8;
                int bit = y % 8;
                int bufferPos = page * 128 + x;
                
                if (buffer[bufferPos] & (1 << bit)) {
                    u8g2->setDrawColor(1);
                    u8g2->drawPixel(x, y);
                }
            }
        }
    } else {
        // Método eficiente: copiar buffer directamente
        // U8G2 ST7920 también usa formato página similar a OLEDDisplay
        memcpy(u8g2_buffer, buffer, displayBufferSize);
    }
    
    // Enviar buffer actualizado
    u8g2->sendBuffer();
}

int ST7920Display::getBufferOffset(void) {
    // Return offset for OLEDDisplay buffer compatibility
    return 0;
}

void ST7920Display::setBrightness(uint8_t brightness) {
    // Map brightness (0-255) to contrast PWM if V0 pin available
    if (pin_v0 >= 0) {
        // Invert brightness for V0 (lower voltage = higher contrast)
        uint8_t contrast = 255 - brightness;
        ledcWrite(0, contrast);
        LOG_DEBUG("ST7920: Set brightness=%d, contrast PWM=%d", brightness, contrast);
    }
}

void ST7920Display::setupPWMContrast() {
    if (pin_v0 >= 0) {
        pinMode(pin_v0, OUTPUT);
        // Setup PWM: Channel 0, 1kHz, 8-bit resolution
        ledcSetup(0, 1000, 8);
        ledcAttachPin(pin_v0, 0);
        // Set medium contrast (brightness ~128)
        ledcWrite(0, 127);
        LOG_INFO("ST7920: PWM contrast configured on pin %d", pin_v0);
    }
}

void ST7920Display::showTestPattern() {
    if (!u8g2 || !initialized) return;
    
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_6x10_tr); // Smaller font to avoid delays
    
    // Simple test pattern
    u8g2->drawStr(5, 15, "ST7920 Ready");
    u8g2->drawStr(5, 30, "Meshtastic");
    u8g2->drawFrame(0, 0, 128, 64);
    
    u8g2->sendBuffer();
    LOG_DEBUG("ST7920: Test pattern displayed");
}

bool ST7920Display::init() {
    return connect();
}

void ST7920Display::drawString(int16_t x, int16_t y, const char* text) {
    if (!u8g2 || !text) return;
    
    u8g2->setFont(u8g2_font_6x10_tr);
    u8g2->drawStr(x, y, text);
}

void ST7920Display::clear() {
    if (!u8g2) return;
    
    u8g2->clearBuffer();
    
    // También limpiar el buffer de OLEDDisplay si está disponible
    if (buffer && displayBufferSize > 0) {
        memset(buffer, 0, displayBufferSize);
    }
}

#endif // ST7920_CLK