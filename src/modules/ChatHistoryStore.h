#pragma once

#include "configuration.h"

// Three chat modes:
// 1. Full mode: Unlimited history + file persistence (ESP32/devices with lots of Flash)
// 2. Memory-only mode: Limited history in RAM only (nRF52/RP2040 with sufficient RAM)
// 3. Minimal mode: Only last message per conversation (very constrained devices)

#if !defined(MESHTASTIC_EXCLUDE_CHAT_HISTORY) && !defined(CHAT_MEMORY_ONLY) && HAS_SCREEN
// === MODE 1: FULL CHAT HISTORY WITH PERSISTENCE ===

#include <deque>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

namespace chat
{

struct ChatEntry {
    uint32_t ts;      // timestamp
    bool outgoing;    // true if sent by us
    bool isChannel;   // true for channel, false for DM
    bool unread;      // true if not read yet
    uint32_t node;    // sender node (for alias display); 0 if it's us and doesn't matter
    uint8_t channel;  // channel index (only for channel messages)
    std::string text; // message content

    static std::string serialize(const ChatEntry &e);
    static ChatEntry deserialize(const std::string &line);
};

class ChatHistoryStore
{
  public:
    static ChatHistoryStore &instance();

    // Add messages
    void addDM(uint32_t peer, bool outgoing, const std::string &text, uint32_t ts, bool unread = true);
    void addCHAN(uint8_t channel, uint32_t fromNode, bool outgoing, const std::string &text, uint32_t ts, bool unread = true);

    // Read-only access to history (returns stable deque; empty if doesn't exist)
    const std::deque<ChatEntry> &getDM(uint32_t peer) const;
    const std::deque<ChatEntry> &getCHAN(uint8_t channel) const;

    // Management
    void clearDM(uint32_t peer);
    void clearCHAN(uint8_t channel);
    void removeByNode(uint32_t peer);    // delete entire DM conversation with that peer
    void removeChannel(uint8_t channel); // delete entire channel history

    // New methods to remove complete history (RAM + persistent) but maintain channel/frame
    void clearChatHistoryDM(uint32_t peer);        // Remove only DM history, maintain the peer
    void clearChatHistoryChannel(uint8_t channel); // Remove only channel history, maintain channel/frame

    // Unread message management
    int getUnreadCountDM(uint32_t peer) const;                        // Count unread messages from a specific DM
    int getUnreadCountCHAN(uint8_t channel) const;                    // Count unread messages from a specific channel
    int getTotalUnreadCount() const;                                  // Total count of unread messages
    void markAsReadDM(uint32_t peer);                                 // Mark all DM messages as read
    void markAsReadCHAN(uint8_t channel);                             // Mark all channel messages as read
    void markAllAsRead();                                             // Mark all messages as read
    void markMessageAsRead(uint32_t peer, int messageIndex);          // Mark specific DM message as read
    void markChannelMessageAsRead(uint8_t channel, int messageIndex); // Mark specific channel message as read
    void markMessageAsReadDM(uint32_t peer, uint32_t timestamp);      // Mark specific DM message as read by timestamp
    void markMessageAsReadCHAN(uint8_t channel, uint32_t timestamp);  // Mark specific channel message as read by timestamp

    // Efficient counter initialization (loads only metadata for counts)
    void initializeUnreadCounters();

    // Functions to position marquee on first unread message
    int getFirstUnreadIndexDM(uint32_t peer) const;     // Returns index of first unread message in DM (-1 if all read)
    int getFirstUnreadIndexCHAN(uint8_t channel) const; // Returns index of first unread message in channel (-1 if all read)
    int getLastReadIndexDM(uint32_t peer) const;        // Returns index of last read message in DM (-1 if none read)
    int getLastReadIndexCHAN(uint8_t channel) const;    // Returns index of last read message in channel (-1 if none read)

    // Conversation discovery
    std::vector<uint32_t> listDMPeers() const;
    std::vector<uint8_t> listChannels() const;

    // Persistence
    void saveAll();
    void loadAll();

  private:
    ChatHistoryStore();
    static void pushBounded(std::deque<ChatEntry> &q, ChatEntry e);

    void saveDM(uint32_t peer);
    void loadDM(uint32_t peer);
    void saveCHAN(uint8_t channel);
    void loadCHAN(uint8_t channel);

    static constexpr size_t kMaxPerGroup = 50; // Max messages per conversation
    bool countersInitialized = false;

    std::map<uint32_t, std::deque<ChatEntry>> dm_; // DM conversations
    std::map<uint8_t, std::deque<ChatEntry>> ch_;  // Channel conversations
};

} // namespace chat

#elif defined(CHAT_MEMORY_ONLY) && HAS_SCREEN
// === MODE 2: MEMORY-ONLY CHAT HISTORY (LIMITED) ===
// Keeps limited history in RAM only - perfect for nRF52/RP2040
// Provides carousel functionality without Flash usage

#include <deque>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

namespace chat
{

struct ChatEntry {
    uint32_t ts;      // timestamp
    bool outgoing;    // true if sent by us
    bool isChannel;   // true for channel, false for DM
    bool unread;      // true if not read yet
    uint32_t node;    // sender node (for alias display); 0 if it's us and doesn't matter
    uint8_t channel;  // channel index (only for channel messages)
    std::string text; // message content

    // No serialization needed for memory-only mode
    static std::string serialize(const ChatEntry &) { return ""; }
    static ChatEntry deserialize(const std::string &) { return ChatEntry(); }
};

class ChatHistoryStore
{
  public:
    static ChatHistoryStore &instance()
    {
        static ChatHistoryStore inst;
        return inst;
    }

    // Add messages (limited to kMaxPerGroup per conversation)
    void addDM(uint32_t peer, bool outgoing, const std::string &text, uint32_t ts, bool unread = true);
    void addCHAN(uint8_t channel, uint32_t fromNode, bool outgoing, const std::string &text, uint32_t ts, bool unread = true);

    // Full access to history (limited but functional)
    const std::deque<ChatEntry> &getDM(uint32_t peer) const;
    const std::deque<ChatEntry> &getCHAN(uint8_t channel) const;

    // Management
    void clearDM(uint32_t peer);
    void clearCHAN(uint8_t channel);
    void removeByNode(uint32_t peer) { clearDM(peer); }
    void removeChannel(uint8_t channel) { clearCHAN(channel); }
    void clearChatHistoryDM(uint32_t peer) { clearDM(peer); }
    void clearChatHistoryChannel(uint8_t channel) { clearCHAN(channel); }

    // Unread management
    int getUnreadCountDM(uint32_t peer) const;
    int getUnreadCountCHAN(uint8_t channel) const;
    void markAsReadDM(uint32_t peer);
    void markAsReadCHAN(uint8_t channel);
    void markMessageAsRead(uint32_t peer, int index);
    void markChannelMessageAsRead(uint8_t channel, int index);
    void markMessageAsReadDM(uint32_t peer, uint32_t timestamp);
    void markMessageAsReadCHAN(uint8_t channel, uint32_t timestamp);
    void initializeUnreadCounters() {}
    int getFirstUnreadIndexDM(uint32_t peer) const;
    int getFirstUnreadIndexCHAN(uint8_t channel) const;
    int getLastReadIndexDM(uint32_t peer) const { return -1; }
    int getLastReadIndexCHAN(uint8_t channel) const { return -1; }

    // Discovery
    std::vector<uint32_t> listDMPeers() const;
    std::vector<uint8_t> listChannels() const;

    // No-op persistence (memory-only mode)
    void saveAll() {}
    void loadAll() {}

  private:
    ChatHistoryStore() = default;
    static void pushBounded(std::deque<ChatEntry> &q, ChatEntry e);

    static constexpr size_t kMaxPerGroup = 30; // Limited messages per conversation for RAM efficiency
    
    std::map<uint32_t, std::deque<ChatEntry>> dm_; // DM conversations (limited)
    std::map<uint8_t, std::deque<ChatEntry>> ch_;  // Channel conversations (limited)
    
    // Empty deque for non-existent conversations
    static const std::deque<ChatEntry> kEmptyDeque;
};

} // namespace chat

#else
// === MODE 3: MINIMAL CHAT (LAST MESSAGE ONLY) ===
// Lightweight chat stub for very memory-constrained devices
// Provides basic chat functionality without persistent history or carousel
#include <deque>
#include <map>
#include <string>
#include <vector>

namespace chat
{

// Minimal chat entry for last message only
struct ChatEntry {
    uint32_t ts = 0;
    bool outgoing = false;
    bool isChannel = false;
    bool unread = false;
    uint32_t node = 0;
    uint8_t channel = 0;
    std::string text;

    // Stub methods for compatibility
    static std::string serialize(const ChatEntry &) { return ""; }
    static ChatEntry deserialize(const std::string &) { return ChatEntry(); }
};

// Lightweight chat store - only keeps last message per conversation
class ChatHistoryStore
{
  public:
    static ChatHistoryStore &instance()
    {
        static ChatHistoryStore inst;
        return inst;
    }

    // Add messages (only keeps the latest one)
    void addDM(uint32_t peer, bool outgoing, const std::string &text, uint32_t ts, bool unread = true);
    void addCHAN(uint8_t channel, uint32_t fromNode, bool outgoing, const std::string &text, uint32_t ts, bool unread = true);

    // Get last message only
    const ChatEntry &getLastDM(uint32_t peer) const;
    const ChatEntry &getLastCHAN(uint8_t channel) const;

    // Legacy compatibility - returns deque with at most 1 message
    const std::deque<ChatEntry> &getDM(uint32_t peer) const;
    const std::deque<ChatEntry> &getCHAN(uint8_t channel) const;

    // Simple management
    void clearDM(uint32_t peer);
    void clearCHAN(uint8_t channel);
    void removeByNode(uint32_t peer) { clearDM(peer); }
    void removeChannel(uint8_t channel) { clearCHAN(channel); }
    void clearChatHistoryDM(uint32_t peer) { clearDM(peer); }
    void clearChatHistoryChannel(uint8_t channel) { clearCHAN(channel); }

    // Unread management (simplified)
    int getUnreadCountDM(uint32_t peer) const;
    int getUnreadCountCHAN(uint8_t channel) const;
    int getTotalUnreadCount() const;
    void markAsReadDM(uint32_t peer);
    void markAsReadCHAN(uint8_t channel);
    void markAllAsRead();

    // Stub methods for compatibility
    void markMessageAsRead(uint32_t, int) {}
    void markChannelMessageAsRead(uint8_t, int) {}
    void markMessageAsReadDM(uint32_t peer, uint32_t) { markAsReadDM(peer); }
    void markMessageAsReadCHAN(uint8_t channel, uint32_t) { markAsReadCHAN(channel); }
    void initializeUnreadCounters() {}
    int getFirstUnreadIndexDM(uint32_t) const { return -1; }
    int getFirstUnreadIndexCHAN(uint8_t) const { return -1; }
    int getLastReadIndexDM(uint32_t) const { return -1; }
    int getLastReadIndexCHAN(uint8_t) const { return -1; }

    // Discovery
    std::vector<uint32_t> listDMPeers() const;
    std::vector<uint8_t> listChannels() const;

    // No-op persistence for compatibility
    void saveAll() {}
    void loadAll() {}

  private:
    ChatHistoryStore() = default;

    // Store only last message per conversation
    std::map<uint32_t, ChatEntry> lastDM_;  // Last DM per peer
    std::map<uint8_t, ChatEntry> lastCHAN_; // Last channel message per channel

    // Compatibility deques (rebuilt on demand)
    mutable std::map<uint32_t, std::deque<ChatEntry>> dm_cache_;
    mutable std::map<uint8_t, std::deque<ChatEntry>> ch_cache_;
};

} // namespace chat

#endif // MESHTASTIC_EXCLUDE_CHAT_HISTORY