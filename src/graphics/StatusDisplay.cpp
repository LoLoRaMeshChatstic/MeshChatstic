#include "StatusDisplay.h"
#include "main.h"
#include "configuration.h"
#include "modules/ChatHistoryStore.h"
#include <time.h>

// Import global status objects
extern meshtastic::PowerStatus *powerStatus;
extern meshtastic::GPSStatus *gpsStatus; 
extern meshtastic::NodeStatus *nodeStatus;

namespace graphics
{

StatusDisplay::StatusDisplay()
{
    // Constructor - objects initialized in header
}

StatusDisplay::~StatusDisplay()
{
    if (display) {
        delete display;
        display = nullptr;
    }
}

bool StatusDisplay::init(int sda, int scl, int address, OLEDDISPLAY_GEOMETRY geometry)
{
    if (display) {
        delete display;
    }

    // Create SSD1306 display instance for secondary OLED
    display = new SSD1306Wire(address, sda, scl, geometry);
    
    if (!display) {
        return false;
    }

    // Initialize display
    display->init();
    display->setI2cAutoInit(true); // Allow multiple displays on different pins
    display->flipScreenVertically(); // Match Heltec orientation
    display->setFont(FONT_SMALL); // Use smallest available font for more lines
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    
    // Clear and show init message
    display->clear();
    display->drawString(0, 0, "Status Display");
    display->drawString(0, 16, "Initializing...");
    display->display();
    
    enabled = true;
    lastUpdate = millis();
    
    return true;
}

void StatusDisplay::setEnabled(bool enable)
{
    enabled = enable;
    if (display) {
        if (enable) {
            display->displayOn();
        } else {
            display->displayOff();
        }
    }
}

void StatusDisplay::subscribeToStatus()
{
    // Subscribe to system status updates
    if (powerStatus) {
        powerObserver.observe(&powerStatus->onNewStatus);
    }
    if (gpsStatus) {
        gpsObserver.observe(&gpsStatus->onNewStatus);  
    }
    if (nodeStatus) {
        nodeObserver.observe(&nodeStatus->onNewStatus);
    }
}

void StatusDisplay::loop()
{
    if (!enabled || !display) {
        return;
    }

    uint32_t now = millis();

    // Check if banner should be cleared
    if (showingBanner && now >= bannerEndTime) {
        showingBanner = false;
        status.needsUpdate = true; // Force refresh to normal display
    }

    // Check if it's time to update (and not showing banner)
    if (!showingBanner && (now - lastUpdate >= updateInterval || status.needsUpdate)) {
        // Update unread message count
        updateUnreadCount();
        
        updateDisplay();
        lastUpdate = now;
        status.needsUpdate = false;
    }
}

void StatusDisplay::forceUpdate()
{
    status.needsUpdate = true;
    loop();
}

void StatusDisplay::updateDisplay()
{
    if (!display) return;

    display->clear();
    display->setFont(ArialMT_Plain_10); // Fuente pequeña y legible

    // Line 0: Battery status
    String batteryLine = "";
    if (status.hasUSB) {
        batteryLine = "Pwr: USB";
        if (status.batteryPercent > 0) {
            batteryLine += " " + String((int)status.batteryPercent) + "%";
        }
    } else if (status.batteryPercent > 0) {
        batteryLine = "Bat: " + String((int)status.batteryPercent) + "%";
        if (status.isCharging) {
            batteryLine += " CHG";
        }
    } else {
        batteryLine = "Bat: N/A";
    }
    drawStatusLine(0, batteryLine);

    // Line 1: Node status
    String nodeLine = "";
    if (status.totalNodes > 0) {
        nodeLine = "Mesh: " + String(status.onlineNodes) + "/" + String(status.totalNodes);
    } else {
        nodeLine = "Mesh: 0 nodes";
    }
    drawStatusLine(1, nodeLine);

    // Line 2: WiFi Status (línea independiente)
    String wifiLine = status.wifiConnected ? "WiFi: Connected" : "WiFi: Disconnected";
    drawStatusLine(2, wifiLine);

    // Line 3: MQTT Status (línea independiente)
    String mqttLine = status.mqttConnected ? "MQTT: Connected" : "MQTT: Disconnected";
    drawStatusLine(3, mqttLine);

    // Line 4: Messages
    String msgLine = "";
    if (status.unreadMessages > 0) {
        msgLine = "Msgs: " + String(status.unreadMessages) + " new";
    } else {
        msgLine = "Msgs: None";
    }
    drawStatusLine(4, msgLine);

    // Line 5: Uptime
    status.uptime = millis() / 1000;
    uint32_t hours = status.uptime / 3600;
    uint32_t minutes = (status.uptime % 3600) / 60;
    String uptimeLine = "Up: " + String(hours) + "h " + String(minutes) + "m";
    drawStatusLine(5, uptimeLine);

    display->display();
}

void StatusDisplay::drawStatusLine(int line, const String& text)
{
    if (!display) return;
    
    // Fuente ArialMT_Plain_10 tiene altura de ~10 pixels
    int lineHeight = 10;
    int y = line * lineHeight + lineHeight; // +lineHeight para baseline
    
    if (y > 64) return; // Don't draw beyond display height (64px)
    
    display->drawString(0, y, text);
}

void StatusDisplay::setUnreadMessages(int count)
{
    if (status.unreadMessages != count) {
        status.unreadMessages = count;
        status.needsUpdate = true;
    }
}

void StatusDisplay::setWiFiStatus(bool connected)
{
    if (status.wifiConnected != connected) {
        status.wifiConnected = connected;
        status.needsUpdate = true;
    }
}

void StatusDisplay::setMQTTStatus(bool connected)
{
    if (status.mqttConnected != connected) {
        status.mqttConnected = connected;
        status.needsUpdate = true;
    }
}

void StatusDisplay::clear()
{
    if (display) {
        display->clear();
        display->display();
    }
}

void StatusDisplay::setPowerSave(bool enable)
{
    if (display) {
        if (enable) {
            display->displayOff();
        } else {
            display->displayOn();
        }
    }
}

// Observer callback implementation
int StatusDisplay::onStatusUpdate(const meshtastic::Status *newStatus)
{
    if (!newStatus) return 0;
    
    bool changed = false;
    
    // Cast to specific status type based on the status type
    if (newStatus->getStatusType() == STATUS_TYPE_POWER) {
        const meshtastic::PowerStatus *powerStatus = static_cast<const meshtastic::PowerStatus *>(newStatus);
        
        float newPercent = powerStatus->getBatteryChargePercent();
        if (status.batteryPercent != newPercent) {
            status.batteryPercent = newPercent;
            changed = true;
        }
        
        bool newCharging = powerStatus->getIsCharging();
        if (status.isCharging != newCharging) {
            status.isCharging = newCharging;
            changed = true;
        }
        
        bool newUSB = powerStatus->getHasUSB();
        if (status.hasUSB != newUSB) {
            status.hasUSB = newUSB;
            changed = true;
        }
    }
    else if (newStatus->getStatusType() == STATUS_TYPE_GPS) {
        const meshtastic::GPSStatus *gpsStatus = static_cast<const meshtastic::GPSStatus *>(newStatus);
        
        int newSats = gpsStatus->getNumSatellites();
        if (status.numSatellites != newSats) {
            status.numSatellites = newSats;
            changed = true;
        }
        
        bool newLock = gpsStatus->getHasLock();
        if (status.hasGPSLock != newLock) {
            status.hasGPSLock = newLock;
            changed = true;
        }
    }
    else if (newStatus->getStatusType() == STATUS_TYPE_NODE) {
        const meshtastic::NodeStatus *nodeStatus = static_cast<const meshtastic::NodeStatus *>(newStatus);
        
        int newTotal = nodeStatus->getNumTotal();
        if (status.totalNodes != newTotal) {
            status.totalNodes = newTotal;
            changed = true;
        }
        
        int newOnline = nodeStatus->getNumOnline();
        if (status.onlineNodes != newOnline) {
            status.onlineNodes = newOnline;
            changed = true;
        }
    }

    if (changed) {
        status.needsUpdate = true;
    }
    
    return 0;
}

void StatusDisplay::showMessageBanner(const char *from, const char *preview, uint32_t durationMs)
{
    if (!display) return;
    
    // Wake up the display if it's sleeping
    if (!enabled) {
        setEnabled(true);
    }
    display->displayOn();
    
    // Clear display and show banner
    display->clear();
    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    
    // Draw filled background box for better visibility
    display->fillRect(2, 2, 124, 60);
    display->setColor(BLACK); // Text in black over white background
    
    // Draw border frame
    display->drawRect(0, 0, 128, 64);
    display->drawRect(1, 1, 126, 62);
    
    // Show notification icon (simplified envelope)
    display->drawRect(4, 6, 12, 8);
    display->drawLine(4, 6, 10, 10);
    display->drawLine(16, 6, 10, 10);
    
    // Show "NEW MESSAGE" title
    display->drawString(20, 6, "NEW MESSAGE");
    
    // Show sender with label
    display->drawString(4, 18, "From:");
    String fromStr = String(from);
    if (fromStr.length() > 15) {
        fromStr = fromStr.substring(0, 12) + "...";
    }
    display->drawString(4, 28, fromStr);
    
    // Show message preview
    String previewStr = String(preview);
    if (previewStr.length() > 20) {
        previewStr = previewStr.substring(0, 17) + "...";
    }
    display->drawString(4, 40, previewStr);
    
    // Show timer at bottom right
    display->drawString(90, 52, String(durationMs/1000) + "s");
    
    display->display();
    
    // Reset color for normal display
    display->setColor(WHITE);
    
    // Set banner state and timer
    showingBanner = true;
    bannerEndTime = millis() + durationMs;
}

void StatusDisplay::updateUnreadCount()
{
    // Get total unread count from ChatHistoryStore
    int totalUnread = chat::ChatHistoryStore::instance().getTotalUnreadCount();
    
    if (status.unreadMessages != totalUnread) {
        status.unreadMessages = totalUnread;
        status.needsUpdate = true;
    }
}

} // namespace graphics