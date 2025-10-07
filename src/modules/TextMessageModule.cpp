#include "TextMessageModule.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "buzz.h"
#include "configuration.h"
#include "graphics/Screen.h"
#include "modules/ChatHistoryStore.h" //for chat history
#include <ctime>
#include <string>

// Declaration for marquee auto-scroll function
namespace graphics
{
void resetScrollToTop(uint32_t nodeIdOrDest, bool isDM);
}
TextMessageModule *textMessageModule;

ProcessMessage TextMessageModule::handleReceived(const meshtastic_MeshPacket &mp)
{
#if defined(DEBUG_PORT) && !defined(DEBUG_MUTE)
    auto &p = mp.decoded;
    LOG_INFO("Received text msg from=0x%08x, id=0x%08x, ch=%u, to=0x%08x, msg=%.*s", mp.from, mp.id, mp.channel, mp.to,
             p.payload.size, p.payload.bytes);
#endif

    // We only store/display messages destined for us.
    // Keep a copy of the most recent text message.
    devicestate.rx_text_message = mp;
    devicestate.has_rx_text_message = true;

    // Only trigger screen wake if configuration allows it
    if (shouldWakeOnReceivedMessage()) {
        powerFSM.trigger(EVENT_RECEIVED_MSG);
    }
    notifyObservers(&mp);

    // Store in chat history
    std::string text;
    if (mp.decoded.payload.size > 0 && mp.decoded.payload.bytes) {
        text.assign(reinterpret_cast<const char *>(mp.decoded.payload.bytes), static_cast<size_t>(mp.decoded.payload.size));
    } else {
        text.clear();
    }

    // Timestamp and channel
    uint32_t ts = mp.rx_time;
    if (ts == 0) {
        ts = (uint32_t)time(nullptr);
        if (ts == 0)
            ts = millis() / 1000;
    }
    const uint8_t channelIndex = static_cast<uint8_t>(mp.channel);

    // DM if 'to' is NOT broadcast
    const bool isDirect = !isBroadcast(mp.to);

    if (isDirect) {
        chat::ChatHistoryStore::instance().addDM(static_cast<uint32_t>(mp.from),
                                                 /*outgoing=*/false, text, ts);
        // Update scroll position to stay at last read message
        graphics::resetScrollToTop(static_cast<uint32_t>(mp.from), true);
    } else {
        chat::ChatHistoryStore::instance().addCHAN(channelIndex,
                                                   static_cast<uint32_t>(mp.from), // real sender
                                                   /*outgoing=*/false, text, ts);
        // Update scroll position to stay at last read message
        graphics::resetScrollToTop(static_cast<uint32_t>(channelIndex), false);
    }

    return ProcessMessage::CONTINUE;
}

bool TextMessageModule::wantPacket(const meshtastic_MeshPacket *p)
{
    return MeshService::isTextPayload(p);
}
