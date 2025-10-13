#pragma once

#include "Arduino.h"
#include "SSD1306Wire.h"
#include "Observer.h"
#include "Status.h"
#include "PowerStatus.h"
#include "GPSStatus.h"
#include "NodeStatus.h"
#include "ScreenFonts.h"

namespace graphics
{

/**
 * @brief StatusDisplay manages a secondary OLED display to show system status information
 * 
 * This class provides a dedicated status display for showing system information like:
 * - Battery level and charging status
 * - GPS status (satellites, fix)
 * - Node count and mesh status
 * - WiFi/MQTT connection status
 * - Unread message counts
 * - Current time/uptime
 */
class StatusDisplay
{
  private:
    SSD1306Wire *display = nullptr;
    bool enabled = false;
    uint32_t lastUpdate = 0;
    uint32_t updateInterval = 1000; // Update every second

    // Status info cache
    struct StatusInfo {
        float batteryPercent = 0;
        bool isCharging = false;
        bool hasUSB = false;
        
        int numSatellites = 0;
        bool hasGPSLock = false;
        
        int totalNodes = 0;
        int onlineNodes = 0;
        
        bool wifiConnected = false;
        bool mqttConnected = false;
        
        int unreadMessages = 0;
        
        uint32_t uptime = 0;
        
        bool needsUpdate = true;
    } status;

    // Message banner state
    bool showingBanner = false;
    uint32_t bannerEndTime = 0;

    // Observer callbacks for system status updates
    CallbackObserver<StatusDisplay, const meshtastic::Status *> powerObserver = 
        CallbackObserver<StatusDisplay, const meshtastic::Status *>(this, &StatusDisplay::onStatusUpdate);
    
    CallbackObserver<StatusDisplay, const meshtastic::Status *> gpsObserver = 
        CallbackObserver<StatusDisplay, const meshtastic::Status *>(this, &StatusDisplay::onStatusUpdate);
    
    CallbackObserver<StatusDisplay, const meshtastic::Status *> nodeObserver = 
        CallbackObserver<StatusDisplay, const meshtastic::Status *>(this, &StatusDisplay::onStatusUpdate);

    // Internal methods
    void drawBatteryIcon(int x, int y, float percent, bool isCharging);
    void drawGPSIcon(int x, int y, bool hasLock, int satellites);
    void drawWiFiIcon(int x, int y, bool connected);
    void drawStatusLine(int line, const String& text);
    void updateDisplay();

  public:
    StatusDisplay();
    ~StatusDisplay();

    /**
     * Initialize the status display with specified I2C pins and address
     * @param sda SDA pin number
     * @param scl SCL pin number  
     * @param address I2C address (typically 0x3C for SSD1306)
     * @param geometry Display geometry (default GEOMETRY_128_64)
     * @return true if initialization successful
     */
    bool init(int sda, int scl, int address = 0x3C, OLEDDISPLAY_GEOMETRY geometry = GEOMETRY_128_64);

    /**
     * Enable or disable the status display
     */
    void setEnabled(bool enable);
    bool isEnabled() const { return enabled; }

    /**
     * Set update interval in milliseconds
     */
    void setUpdateInterval(uint32_t interval) { updateInterval = interval; }

    /**
     * Manual update trigger - call from main loop
     */
    void loop();

    /**
     * Force immediate display update
     */
    void forceUpdate();

    /**
     * Set custom status information
     */
    void setUnreadMessages(int count);
    void setWiFiStatus(bool connected);
    void setMQTTStatus(bool connected);

    /**
     * Clear the display
     */
    void clear();

    /**
     * Turn display on/off (hardware power save)
     */
    void setPowerSave(bool enable);

    /**
     * Show new message banner on secondary display only
     * This wakes up the display if needed and shows a notification
     */
    void showMessageBanner(const char *from, const char *preview, uint32_t durationMs = 5000);

    // Observer callback - called when system status changes
    int onStatusUpdate(const meshtastic::Status *newStatus);

    /**
     * Subscribe to system status observers
     */
    void subscribeToStatus();

    /**
     * Update unread message count from ChatHistoryStore
     */
    void updateUnreadCount();
};

} // namespace graphics