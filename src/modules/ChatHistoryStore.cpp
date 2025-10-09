#include "modules/ChatHistoryStore.h"

// Full implementation for devices with sufficient memory
#if !defined(MESHTASTIC_EXCLUDE_CHAT_HISTORY) && !defined(CHAT_MEMORY_ONLY) && HAS_SCREEN

#include "FSCommon.h"
#include <algorithm>
#include <sstream>

namespace chat
{

static const std::deque<ChatEntry> kEmptyDeque;

// --- Simple CSV serialization ---
std::string ChatEntry::serialize(const ChatEntry &e)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%u,%d,%d,%d,%u,%u,", e.ts, e.outgoing, e.isChannel, e.unread, e.node, e.channel);
    std::string s(buf);
    // Escapar comas en el texto si es necesario (simple)
    for (char c : e.text) {
        if (c == ',')
            s += "<c>";
        else
            s += c;
    }
    return s;
}

ChatEntry ChatEntry::deserialize(const std::string &line)
{
    ChatEntry e;
#if defined(__EXCEPTIONS) || defined(ARCH_ESP32)
    try {
#endif
        std::stringstream ss(line);
        std::string item;
        if (!std::getline(ss, item, ','))
            return e;
        e.ts = std::stoul(item);
        if (!std::getline(ss, item, ','))
            return e;
        e.outgoing = std::stoi(item);
        if (!std::getline(ss, item, ','))
            return e;
        e.isChannel = std::stoi(item);
        if (!std::getline(ss, item, ','))
            return e;
        e.unread = std::stoi(item);
        if (!std::getline(ss, item, ','))
            return e;
        e.node = std::stoul(item);
        if (!std::getline(ss, item, ','))
            return e;
        e.channel = std::stoul(item);
        if (!std::getline(ss, item))
            return e; // resto es texto

        // Desescapar comas
        size_t pos = 0, last = 0;
        std::string txt;
        while ((pos = item.find("<c>", last)) != std::string::npos) {
            txt += item.substr(last, pos - last) + ',';
            last = pos + 3;
        }
        txt += item.substr(last);
        e.text = txt;
#if defined(__EXCEPTIONS) || defined(ARCH_ESP32)
    } catch (...) {
        // If there's an error in parsing, return empty entry
        e = ChatEntry{};
    }
#endif
    return e;
}

ChatHistoryStore &ChatHistoryStore::instance()
{
    static ChatHistoryStore inst;
    return inst;
}

ChatHistoryStore::ChatHistoryStore()
{
    // Don't load data synchronously in constructor to avoid restart loops
    // Loading will be done on demand
}

void ChatHistoryStore::pushBounded(std::deque<ChatEntry> &q, ChatEntry e)
{
    // Insert in chronological order (ascending timestamp)
    if (q.empty() || q.back().ts <= e.ts) {
        q.push_back(std::move(e));
    } else {
        auto it = std::upper_bound(q.begin(), q.end(), e.ts, [](uint32_t t, const ChatEntry &ce) { return t < ce.ts; });
        q.insert(it, std::move(e));
    }
    while (q.size() > kMaxPerGroup)
        q.pop_front();
}

void ChatHistoryStore::addDM(uint32_t peer, bool outgoing, const std::string &text, uint32_t ts, bool unread)
{
    ChatEntry e;
    e.ts = ts;
    e.outgoing = outgoing;
    e.isChannel = false;
    e.unread = unread && !outgoing; // Only incoming messages can be unread
    e.node = peer;                  // peer of the conversation
    e.channel = 0;
    e.text = text;
    pushBounded(dm_[peer], std::move(e));
    saveDM(peer);
}

void ChatHistoryStore::addCHAN(uint8_t channel, uint32_t fromNode, bool outgoing, const std::string &text, uint32_t ts,
                               bool unread)
{
    ChatEntry e;
    e.ts = ts;
    e.outgoing = outgoing;
    e.isChannel = true;
    e.unread = unread && !outgoing; // Only incoming messages can be unread
    e.node = fromNode;              // sender (for alias display); 0 if it's us and doesn't matter
    e.channel = channel;
    e.text = text;
    pushBounded(ch_[channel], std::move(e));
    saveCHAN(channel);
}
// --- Persistence ---
void ChatHistoryStore::saveDM(uint32_t peer)
{
    std::string filename = "/prefs/chat_dm_" + std::to_string(peer) + ".csv";
    auto f = FSCom.open(filename.c_str(), FILE_O_WRITE);
    if (!f)
        return;
    for (const auto &e : dm_[peer]) {
        f.println(ChatEntry::serialize(e).c_str());
    }
    f.close();
}

void ChatHistoryStore::loadDM(uint32_t peer)
{
    std::string filename = "/prefs/chat_dm_" + std::to_string(peer) + ".csv";
    auto f = FSCom.open(filename.c_str(), FILE_O_READ);
    if (!f)
        return; // Archivo no existe, sin error

    std::deque<ChatEntry> q;
#if defined(__EXCEPTIONS) || defined(ARCH_ESP32)
    try {
#endif
        while (f.available()) {
            std::string line = f.readStringUntil('\n').c_str();
            if (!line.empty() && line.length() < 512) { // Basic size validation
                ChatEntry entry = ChatEntry::deserialize(line);
                // Basic data validation
                if (entry.ts > 0 && entry.ts < 4000000000U && entry.text.length() < 256) {
                    q.push_back(std::move(entry));
                }
            }
        }
        // Apply kMaxPerGroup limit after loading from file
        while (q.size() > kMaxPerGroup)
            q.pop_front();
        dm_[peer] = std::move(q);
#if defined(__EXCEPTIONS) || defined(ARCH_ESP32)
    } catch (...) {
        // If there's an error in deserialization, ignore the file
        dm_[peer] = std::deque<ChatEntry>();
    }
#endif
    f.close();
}

void ChatHistoryStore::saveCHAN(uint8_t channel)
{
    std::string filename = "/prefs/chat_ch_" + std::to_string(channel) + ".csv";
    auto f = FSCom.open(filename.c_str(), FILE_O_WRITE);
    if (!f)
        return;
    for (const auto &e : ch_[channel]) {
        f.println(ChatEntry::serialize(e).c_str());
    }
    f.close();
}

void ChatHistoryStore::loadCHAN(uint8_t channel)
{
    std::string filename = "/prefs/chat_ch_" + std::to_string(channel) + ".csv";
    auto f = FSCom.open(filename.c_str(), FILE_O_READ);
    if (!f)
        return; // Archivo no existe, sin error

    std::deque<ChatEntry> q;
#if defined(__EXCEPTIONS) || defined(ARCH_ESP32)
    try {
#endif
        while (f.available()) {
            std::string line = f.readStringUntil('\n').c_str();
            if (!line.empty() && line.length() < 512) { // Basic size validation
                ChatEntry entry = ChatEntry::deserialize(line);
                // Basic data validation
                if (entry.ts > 0 && entry.ts < 4000000000U && entry.text.length() < 256) {
                    q.push_back(std::move(entry));
                }
            }
        }
        // Apply kMaxPerGroup limit after loading from file
        while (q.size() > kMaxPerGroup)
            q.pop_front();
        ch_[channel] = std::move(q);
#if defined(__EXCEPTIONS) || defined(ARCH_ESP32)
    } catch (...) {
        // If there's an error in deserialization, ignore the file
        ch_[channel] = std::deque<ChatEntry>();
    }
#endif
    f.close();
}

void ChatHistoryStore::saveAll()
{
    for (const auto &kv : dm_)
        saveDM(kv.first);
    for (const auto &kv : ch_)
        saveCHAN(kv.first);
}

void ChatHistoryStore::loadAll()
{
    // DON'T load aggressively at startup to avoid restart loops
    // Loading is done on demand when each conversation is needed
    // This function remains for compatibility but doesn't do anything critical
}

void ChatHistoryStore::initializeUnreadCounters()
{
    // This function loads only metadata (unread counts) from saved files
    // without loading full message content to initialize counters efficiently

    // Simple approach: just mark as initialized and rely on on-demand loading
    // This avoids filesystem complexity and potential restart loops
    // The getTotalUnreadCount() will trigger loading when conversations are accessed
    countersInitialized = true;
}

const std::deque<ChatEntry> &ChatHistoryStore::getDM(uint32_t peer) const
{
    auto it = dm_.find(peer);
    if (it != dm_.end())
        return it->second;

        // On-demand loading with error handling
#if defined(__EXCEPTIONS) || defined(ARCH_ESP32)
    try {
#endif
        const_cast<ChatHistoryStore *>(this)->loadDM(peer);
        it = dm_.find(peer);
        if (it != dm_.end())
            return it->second;
#if defined(__EXCEPTIONS) || defined(ARCH_ESP32)
    } catch (...) {
        // If loading fails, return empty deque silently
    }
#endif

    return kEmptyDeque;
}

const std::deque<ChatEntry> &ChatHistoryStore::getCHAN(uint8_t channel) const
{
    auto it = ch_.find(channel);
    if (it != ch_.end())
        return it->second;

        // On-demand loading with error handling
#if defined(__EXCEPTIONS) || defined(ARCH_ESP32)
    try {
#endif
        const_cast<ChatHistoryStore *>(this)->loadCHAN(channel);
        it = ch_.find(channel);
        if (it != ch_.end())
            return it->second;
#if defined(__EXCEPTIONS) || defined(ARCH_ESP32)
    } catch (...) {
        // If loading fails, return empty deque silently
    }
#endif

    return kEmptyDeque;
}

void ChatHistoryStore::clearDM(uint32_t peer)
{
    dm_.erase(peer);
}

void ChatHistoryStore::clearCHAN(uint8_t channel)
{
    ch_.erase(channel);
}

void ChatHistoryStore::removeByNode(uint32_t peer)
{
    dm_.erase(peer);
}

void ChatHistoryStore::removeChannel(uint8_t channel)
{
    ch_.erase(channel);
}

// Nuevas funciones para eliminar solo el historial pero mantener canal/frame
void ChatHistoryStore::clearChatHistoryDM(uint32_t peer)
{
    // Eliminar de RAM
    dm_.erase(peer);

    // Eliminar archivo persistente
    std::string filename = "/chat_dm_" + std::to_string(peer) + ".txt";
    FSCom.remove(filename.c_str());
}

void ChatHistoryStore::clearChatHistoryChannel(uint8_t channel)
{
    // Eliminar de RAM
    ch_.erase(channel);

    // Eliminar archivo persistente
    std::string filename = "/chat_ch_" + std::to_string(channel) + ".txt";
    FSCom.remove(filename.c_str());
}

std::vector<uint32_t> ChatHistoryStore::listDMPeers() const
{
    std::vector<uint32_t> v;
    v.reserve(dm_.size());
    for (auto &kv : dm_)
        v.push_back(kv.first); // Collect all DM peer IDs
    std::sort(v.begin(), v.end());
    return v;
}

std::vector<uint8_t> ChatHistoryStore::listChannels() const
{
    std::vector<uint8_t> v;
    v.reserve(ch_.size());
    for (auto &kv : ch_)
        v.push_back(kv.first); // Collect all channel indices
    std::sort(v.begin(), v.end());
    return v;
}

// --- Unread message management ---
int ChatHistoryStore::getUnreadCountDM(uint32_t peer) const
{
    auto it = dm_.find(peer);
    if (it == dm_.end())
        return 0;

    int count = 0;
    for (const auto &entry : it->second) {
        if (entry.unread && !entry.outgoing)
            count++;
    }
    return count;
}

int ChatHistoryStore::getUnreadCountCHAN(uint8_t channel) const
{
    auto it = ch_.find(channel);
    if (it == ch_.end())
        return 0;

    int count = 0;
    for (const auto &entry : it->second) {
        if (entry.unread && !entry.outgoing)
            count++;
    }
    return count;
}

int ChatHistoryStore::getTotalUnreadCount() const
{
    // Initialize counters efficiently on first call
    if (!countersInitialized) {
        const_cast<ChatHistoryStore *>(this)->initializeUnreadCounters();
        const_cast<ChatHistoryStore *>(this)->countersInitialized = true;
    }

    int total = 0;

    // Count unread DMs
    for (const auto &kv : dm_) {
        for (const auto &entry : kv.second) {
            if (entry.unread && !entry.outgoing)
                total++;
        }
    }

    // Count unread channels
    for (const auto &kv : ch_) {
        for (const auto &entry : kv.second) {
            if (entry.unread && !entry.outgoing)
                total++;
        }
    }

    return total;
}

void ChatHistoryStore::markAsReadDM(uint32_t peer)
{
    auto it = dm_.find(peer);
    if (it == dm_.end())
        return;

    bool changed = false;
    for (auto &entry : it->second) {
        if (entry.unread) {
            entry.unread = false;
            changed = true;
        }
    }

    if (changed)
        saveDM(peer);
}

void ChatHistoryStore::markAsReadCHAN(uint8_t channel)
{
    auto it = ch_.find(channel);
    if (it == ch_.end())
        return;

    bool changed = false;
    for (auto &entry : it->second) {
        if (entry.unread) {
            entry.unread = false;
            changed = true;
        }
    }

    if (changed)
        saveCHAN(channel);
}

void ChatHistoryStore::markAllAsRead()
{
    // Mark all DMs as read
    for (auto &kv : dm_) {
        bool changed = false;
        for (auto &entry : kv.second) {
            if (entry.unread) {
                entry.unread = false;
                changed = true;
            }
        }
        if (changed)
            saveDM(kv.first);
    }

    // Mark all channels as read
    for (auto &kv : ch_) {
        bool changed = false;
        for (auto &entry : kv.second) {
            if (entry.unread) {
                entry.unread = false;
                changed = true;
            }
        }
        if (changed)
            saveCHAN(kv.first);
    }
}

void ChatHistoryStore::markMessageAsRead(uint32_t peer, int messageIndex)
{
    auto it = dm_.find(peer);
    if (it == dm_.end())
        return;

    if (messageIndex >= 0 && messageIndex < (int)it->second.size()) {
        if (it->second[messageIndex].unread) {
            it->second[messageIndex].unread = false;
            saveDM(peer);
        }
    }
}

void ChatHistoryStore::markChannelMessageAsRead(uint8_t channel, int messageIndex)
{
    auto it = ch_.find(channel);
    if (it == ch_.end())
        return;

    if (messageIndex >= 0 && messageIndex < (int)it->second.size()) {
        if (it->second[messageIndex].unread) {
            it->second[messageIndex].unread = false;
            saveCHAN(channel);
        }
    }
}

int ChatHistoryStore::getFirstUnreadIndexDM(uint32_t peer) const
{
    auto it = dm_.find(peer);
    if (it == dm_.end())
        return -1;

    // Buscar el primer mensaje no leído desde el más antiguo (final del deque)
    const auto &history = it->second;
    for (int i = (int)history.size() - 1; i >= 0; --i) {
        if (history[i].unread && !history[i].outgoing) {
            return i;
        }
    }
    return -1; // Todos los mensajes están leídos
}

int ChatHistoryStore::getFirstUnreadIndexCHAN(uint8_t channel) const
{
    auto it = ch_.find(channel);
    if (it == ch_.end())
        return -1;

    // Buscar el primer mensaje no leído desde el más antiguo (final del deque)
    const auto &history = it->second;
    for (int i = (int)history.size() - 1; i >= 0; --i) {
        if (history[i].unread && !history[i].outgoing) {
            return i;
        }
    }
    return -1; // Todos los mensajes están leídos
}

int ChatHistoryStore::getLastReadIndexDM(uint32_t peer) const
{
    auto it = dm_.find(peer);
    if (it == dm_.end())
        return -1;

    // Buscar el último mensaje leído desde el final del deque (más viejo) hacia el inicio (más nuevo)
    // Para ser consistente con la lógica de display: itemIndex = total - 1 - (scrollIndex + row)
    const auto &history = it->second;
    for (int i = (int)history.size() - 1; i >= 0; --i) {
        if (!history[i].unread) {
            return i;
        }
    }
    return -1; // Ningún mensaje está leído
}

int ChatHistoryStore::getLastReadIndexCHAN(uint8_t channel) const
{
    auto it = ch_.find(channel);
    if (it == ch_.end())
        return -1;

    // Buscar el último mensaje leído desde el final del deque (más viejo) hacia el inicio (más nuevo)
    // Para ser consistente con la lógica de display: itemIndex = total - 1 - (scrollIndex + row)
    const auto &history = it->second;
    for (int i = (int)history.size() - 1; i >= 0; --i) {
        if (!history[i].unread) {
            return i;
        }
    }
    return -1; // Ningún mensaje está leído
}

void ChatHistoryStore::markMessageAsReadDM(uint32_t peer, uint32_t timestamp)
{
    auto it = dm_.find(peer);
    if (it == dm_.end())
        return;

    bool changed = false;
    for (auto &entry : it->second) {
        if (entry.ts == timestamp && entry.unread) {
            entry.unread = false;
            changed = true;
            break; // Found the message, exit loop
        }
    }

    if (changed)
        saveDM(peer);
}

void ChatHistoryStore::markMessageAsReadCHAN(uint8_t channel, uint32_t timestamp)
{
    auto it = ch_.find(channel);
    if (it == ch_.end())
        return;

    bool changed = false;
    for (auto &entry : it->second) {
        if (entry.ts == timestamp && entry.unread) {
            entry.unread = false;
            changed = true;
            break; // Found the message, exit loop
        }
    }

    if (changed)
        saveCHAN(channel);
}

} // namespace chat

#elif defined(CHAT_MEMORY_ONLY) && HAS_SCREEN
// === MODE 2: MEMORY-ONLY IMPLEMENTATION ===
// Limited history in RAM only - no file persistence
#include <algorithm>

namespace chat
{

const std::deque<ChatEntry> ChatHistoryStore::kEmptyDeque;

ChatHistoryStore &ChatHistoryStore::instance()
{
    static ChatHistoryStore inst;
    return inst;
}

void ChatHistoryStore::pushBounded(std::deque<ChatEntry> &q, ChatEntry e)
{
    // Insert in chronological order (ascending timestamp)
    if (q.empty() || q.back().ts <= e.ts) {
        q.push_back(std::move(e));
    } else {
        auto it = std::upper_bound(q.begin(), q.end(), e.ts, [](uint32_t t, const ChatEntry &ce) { return t < ce.ts; });
        q.insert(it, std::move(e));
    }
    // Enforce limit for memory efficiency
    while (q.size() > kMaxPerGroup)
        q.pop_front();
}

void ChatHistoryStore::addDM(uint32_t peer, bool outgoing, const std::string &text, uint32_t ts, bool unread)
{
    ChatEntry e;
    e.ts = ts;
    e.outgoing = outgoing;
    e.isChannel = false;
    e.unread = unread && !outgoing; // Only incoming messages can be unread
    e.node = peer;                  // sender (for alias display); 0 if it's us and doesn't matter
    e.channel = 0;
    e.text = text;
    pushBounded(dm_[peer], std::move(e));
    // No persistence in memory-only mode
}

void ChatHistoryStore::addCHAN(uint8_t channel, uint32_t fromNode, bool outgoing, const std::string &text, uint32_t ts,
                               bool unread)
{
    ChatEntry e;
    e.ts = ts;
    e.outgoing = outgoing;
    e.isChannel = true;
    e.unread = unread && !outgoing; // Only incoming messages can be unread
    e.node = fromNode;              // sender (for alias display); 0 if it's us and doesn't matter
    e.channel = channel;
    e.text = text;
    pushBounded(ch_[channel], std::move(e));
    // No persistence in memory-only mode
}

const std::deque<ChatEntry> &ChatHistoryStore::getDM(uint32_t peer) const
{
    auto it = dm_.find(peer);
    return (it != dm_.end()) ? it->second : kEmptyDeque;
}

const std::deque<ChatEntry> &ChatHistoryStore::getCHAN(uint8_t channel) const
{
    auto it = ch_.find(channel);
    return (it != ch_.end()) ? it->second : kEmptyDeque;
}

void ChatHistoryStore::clearDM(uint32_t peer)
{
    dm_.erase(peer);
}

void ChatHistoryStore::clearCHAN(uint8_t channel)
{
    ch_.erase(channel);
}

std::vector<uint32_t> ChatHistoryStore::listDMPeers() const
{
    std::vector<uint32_t> v;
    v.reserve(dm_.size());
    for (auto &kv : dm_)
        v.push_back(kv.first);
    std::sort(v.begin(), v.end());
    return v;
}

std::vector<uint8_t> ChatHistoryStore::listChannels() const
{
    std::vector<uint8_t> v;
    v.reserve(ch_.size());
    for (auto &kv : ch_)
        v.push_back(kv.first);
    std::sort(v.begin(), v.end());
    return v;
}

int ChatHistoryStore::getUnreadCountDM(uint32_t peer) const
{
    auto it = dm_.find(peer);
    if (it == dm_.end())
        return 0;
    
    int count = 0;
    for (const auto &entry : it->second) {
        if (entry.unread)
            count++;
    }
    return count;
}

int ChatHistoryStore::getUnreadCountCHAN(uint8_t channel) const
{
    auto it = ch_.find(channel);
    if (it == ch_.end())
        return 0;
    
    int count = 0;
    for (const auto &entry : it->second) {
        if (entry.unread)
            count++;
    }
    return count;
}

void ChatHistoryStore::markAsReadDM(uint32_t peer)
{
    auto it = dm_.find(peer);
    if (it == dm_.end())
        return;
    
    for (auto &entry : it->second) {
        entry.unread = false;
    }
}

void ChatHistoryStore::markAsReadCHAN(uint8_t channel)
{
    auto it = ch_.find(channel);
    if (it == ch_.end())
        return;
    
    for (auto &entry : it->second) {
        entry.unread = false;
    }
}

void ChatHistoryStore::markMessageAsRead(uint32_t peer, int index)
{
    auto it = dm_.find(peer);
    if (it == dm_.end() || index < 0 || index >= (int)it->second.size())
        return;
    
    it->second[index].unread = false;
}

void ChatHistoryStore::markChannelMessageAsRead(uint8_t channel, int index)
{
    auto it = ch_.find(channel);
    if (it == ch_.end() || index < 0 || index >= (int)it->second.size())
        return;
    
    it->second[index].unread = false;
}

void ChatHistoryStore::markMessageAsReadDM(uint32_t peer, uint32_t timestamp)
{
    auto it = dm_.find(peer);
    if (it == dm_.end())
        return;
    
    for (auto &entry : it->second) {
        if (entry.ts == timestamp) {
            entry.unread = false;
            break;
        }
    }
}

void ChatHistoryStore::markMessageAsReadCHAN(uint8_t channel, uint32_t timestamp)
{
    auto it = ch_.find(channel);
    if (it == ch_.end())
        return;
    
    for (auto &entry : it->second) {
        if (entry.ts == timestamp) {
            entry.unread = false;
            break;
        }
    }
}

int ChatHistoryStore::getFirstUnreadIndexDM(uint32_t peer) const
{
    auto it = dm_.find(peer);
    if (it == dm_.end())
        return -1;
    
    for (int i = 0; i < (int)it->second.size(); i++) {
        if (it->second[i].unread)
            return i;
    }
    return -1;
}

int ChatHistoryStore::getFirstUnreadIndexCHAN(uint8_t channel) const
{
    auto it = ch_.find(channel);
    if (it == ch_.end())
        return -1;
    
    for (int i = 0; i < (int)it->second.size(); i++) {
        if (it->second[i].unread)
            return i;
    }
    return -1;
}

} // namespace chat

#else
// === MODE 3: MINIMAL IMPLEMENTATION ===
// Lightweight implementation for very memory-constrained devices (nRF52, RP2040)
#include <algorithm>

namespace chat
{

static const ChatEntry kEmptyEntry;

void ChatHistoryStore::addDM(uint32_t peer, bool outgoing, const std::string &text, uint32_t ts, bool unread)
{
    ChatEntry &entry = lastDM_[peer];
    entry.ts = ts;
    entry.outgoing = outgoing;
    entry.isChannel = false;
    entry.unread = unread && !outgoing; // Only incoming messages can be unread
    entry.node = outgoing ? 0 : peer;
    entry.channel = 0;
    entry.text = text;

    // Clear cache to force rebuild
    dm_cache_.erase(peer);
}

void ChatHistoryStore::addCHAN(uint8_t channel, uint32_t fromNode, bool outgoing, const std::string &text, uint32_t ts,
                               bool unread)
{
    ChatEntry &entry = lastCHAN_[channel];
    entry.ts = ts;
    entry.outgoing = outgoing;
    entry.isChannel = true;
    entry.unread = unread && !outgoing; // Only incoming messages can be unread
    entry.node = fromNode;
    entry.channel = channel;
    entry.text = text;

    // Clear cache to force rebuild
    ch_cache_.erase(channel);
}

const ChatEntry &ChatHistoryStore::getLastDM(uint32_t peer) const
{
    auto it = lastDM_.find(peer);
    return (it != lastDM_.end()) ? it->second : kEmptyEntry;
}

const ChatEntry &ChatHistoryStore::getLastCHAN(uint8_t channel) const
{
    auto it = lastCHAN_.find(channel);
    return (it != lastCHAN_.end()) ? it->second : kEmptyEntry;
}

const std::deque<ChatEntry> &ChatHistoryStore::getDM(uint32_t peer) const
{
    // Build single-message deque on demand for compatibility
    auto &deque = dm_cache_[peer];
    deque.clear();

    const auto &entry = getLastDM(peer);
    if (entry.ts > 0) { // Check if entry exists
        deque.push_back(entry);
    }

    return deque;
}

const std::deque<ChatEntry> &ChatHistoryStore::getCHAN(uint8_t channel) const
{
    // Build single-message deque on demand for compatibility
    auto &deque = ch_cache_[channel];
    deque.clear();

    const auto &entry = getLastCHAN(channel);
    if (entry.ts > 0) { // Check if entry exists
        deque.push_back(entry);
    }

    return deque;
}

void ChatHistoryStore::clearDM(uint32_t peer)
{
    lastDM_.erase(peer);
    dm_cache_.erase(peer);
}

void ChatHistoryStore::clearCHAN(uint8_t channel)
{
    lastCHAN_.erase(channel);
    ch_cache_.erase(channel);
}

int ChatHistoryStore::getUnreadCountDM(uint32_t peer) const
{
    const auto &entry = getLastDM(peer);
    return (entry.ts > 0 && entry.unread && !entry.outgoing) ? 1 : 0;
}

int ChatHistoryStore::getUnreadCountCHAN(uint8_t channel) const
{
    const auto &entry = getLastCHAN(channel);
    return (entry.ts > 0 && entry.unread && !entry.outgoing) ? 1 : 0;
}

int ChatHistoryStore::getTotalUnreadCount() const
{
    int total = 0;

    // Count unread DMs
    for (const auto &pair : lastDM_) {
        const auto &entry = pair.second;
        if (entry.unread && !entry.outgoing) {
            total++;
        }
    }

    // Count unread channels
    for (const auto &pair : lastCHAN_) {
        const auto &entry = pair.second;
        if (entry.unread && !entry.outgoing) {
            total++;
        }
    }

    return total;
}

void ChatHistoryStore::markAsReadDM(uint32_t peer)
{
    auto it = lastDM_.find(peer);
    if (it != lastDM_.end()) {
        it->second.unread = false;
        // Clear cache to force rebuild
        dm_cache_.erase(peer);
    }
}

void ChatHistoryStore::markAsReadCHAN(uint8_t channel)
{
    auto it = lastCHAN_.find(channel);
    if (it != lastCHAN_.end()) {
        it->second.unread = false;
        // Clear cache to force rebuild
        ch_cache_.erase(channel);
    }
}

void ChatHistoryStore::markAllAsRead()
{
    // Mark all DM messages as read
    for (auto &pair : lastDM_) {
        pair.second.unread = false;
    }

    // Mark all channel messages as read
    for (auto &pair : lastCHAN_) {
        pair.second.unread = false;
    }

    // Clear all caches
    dm_cache_.clear();
    ch_cache_.clear();
}

std::vector<uint32_t> ChatHistoryStore::listDMPeers() const
{
    std::vector<uint32_t> peers;
    for (const auto &pair : lastDM_) {
        if (pair.second.ts > 0) { // Check if entry exists
            peers.push_back(pair.first);
        }
    }
    return peers;
}

std::vector<uint8_t> ChatHistoryStore::listChannels() const
{
    std::vector<uint8_t> channels;
    for (const auto &pair : lastCHAN_) {
        if (pair.second.ts > 0) { // Check if entry exists
            channels.push_back(pair.first);
        }
    }
    return channels;
}

} // namespace chat

#endif // Chat history modes
