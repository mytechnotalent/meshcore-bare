/** @file MyMesh.h
 *  @brief MeshCore application and protocol interface.
 */

#pragma once

#include "AbstractUITask.h"
#include <Arduino.h>
#include <Mesh.h>

/*------------ Frame Protocol --------------*/
#define FIRMWARE_VER_CODE 13

#ifndef FIRMWARE_BUILD_DATE
#define FIRMWARE_BUILD_DATE "9 Aug 2026"
#endif

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "v1.17.0"
#endif

#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
#include <InternalFileSystem.h>
#elif defined(RP2040_PLATFORM)
#include <LittleFS.h>
#elif defined(ESP32)
#include <SPIFFS.h>
#endif

#include "DataStore.h"
#include "NodePrefs.h"

#include "target.h"
#include <RTClib.h>
#include <helpers/ArduinoHelpers.h>
#include <helpers/BaseSerialInterface.h>
#include <helpers/IdentityStore.h>
#include <helpers/SimpleMeshTables.h>
#include <helpers/StaticPoolPacketManager.h>

/* ---------------------------------- CONFIGURATION
 * ------------------------------------- */

#ifndef LORA_FREQ
#define LORA_FREQ 915.0
#endif
#ifndef LORA_BW
#define LORA_BW 250
#endif
#ifndef LORA_SF
#define LORA_SF 10
#endif
#ifndef LORA_CR
#define LORA_CR 5
#endif
#ifndef LORA_TX_POWER
#define LORA_TX_POWER 20
#endif
#ifndef MAX_LORA_TX_POWER
#define MAX_LORA_TX_POWER LORA_TX_POWER
#endif

#ifndef MAX_CONTACTS
#define MAX_CONTACTS 100
#endif

#ifndef OFFLINE_QUEUE_SIZE
#define OFFLINE_QUEUE_SIZE 16
#endif

#ifndef BLE_NAME_PREFIX
#define BLE_NAME_PREFIX "MeshCore-"
#endif

#include <helpers/BaseChatMesh.h>
#include <helpers/TransportKeyStore.h>

/* --------------------------------------------------------------------------------------
 */

#define REQ_TYPE_GET_STATUS 0x01 // same as _GET_STATS
#define REQ_TYPE_KEEP_ALIVE 0x02
#define REQ_TYPE_GET_TELEMETRY_DATA 0x03

struct AdvertPath {
    uint8_t pubkey_prefix[7];
    uint8_t path_len;
    char name[32];
    uint32_t recv_timestamp;
    uint8_t path[MAX_PATH_SIZE];
};

/** Implements the application-facing MeshCore node behavior. */
class MyMesh : public BaseChatMesh, public DataStoreHost {
  public:
    /**
     * @brief Creates a mesh node using caller-owned runtime services.
     * @param radio Radio wrapper.
     * @param millis Millisecond clock.
     * @param packet_manager Packet manager.
     * @param rng Random-number generator.
     * @param rtc Real-time clock.
     * @param tables Mesh tables.
     * @param store Persistent data store.
     * @param ui Optional UI task.
     * @return None.
     */
    MyMesh(mesh::Radio &radio, mesh::MillisecondClock &millis,
           mesh::PacketManager &packet_manager, mesh::RNG &rng,
           mesh::RTCClock &rtc, SimpleMeshTables &tables, DataStore &store,
           AbstractUITask *ui = nullptr);
    /**
     * @brief Loads persisted state and initializes the node.
     * @param has_display Whether a display is present.
     * @return None.
     */
    void begin(bool has_display);
    /**
     * @brief Attaches and enables the application serial interface.
     * @param serial Serial interface.
     * @return None.
     */
    void startInterface(BaseSerialInterface &serial);
    /**
     * @brief Returns the current advertised node name.
     * @param None.
     * @return Null-terminated node name.
     */
    const char *getNodeName();
    /**
     * @brief Returns the mutable node preferences.
     * @param None.
     * @return Pointer to node preferences.
     */
    NodePrefs *getNodePrefs();
    /**
     * @brief Returns the active Bluetooth PIN.
     * @param None.
     * @return Bluetooth PIN.
     */
    uint32_t getBLEPin();
    /**
     * @brief Advances radio, interface, and persistence processing.
     * @param None.
     * @return None.
     */
    void loop();
    /**
     * @brief Handles one application protocol frame.
     * @param len Frame length.
     * @return None.
     */
    void handleCmdFrame(size_t len);
    /**
     * @brief Sends a self advertisement.
     * @param None.
     * @return true when queued successfully; otherwise false.
     */
    bool advert();
    /**
     * @brief Enters serial CLI rescue mode.
     * @param None.
     * @return None.
     */
    void enterCLIRescue();
    /**
     * @brief Copies recently heard advertisements into a destination array.
     * @param dest Destination array.
     * @param max_num Destination capacity.
     * @return Number of advertisements copied.
     */
    int getRecentlyHeard(AdvertPath dest[], int max_num);

  protected:
    /** @brief Returns the configured airtime budget factor.
     *  @param None.
     *  @return Airtime budget factor.
     */
    float getAirtimeBudgetFactor() const override;
    /** @brief Returns the interference threshold.
     *  @param None.
     *  @return Interference threshold.
     */
    int getInterferenceThreshold() const override;
    /** @brief Reports whether CAD is enabled.
     *  @param None.
     *  @return true when enabled; otherwise false.
     */
    bool getCADEnabled() const override;
    /** @brief Calculates receive delay.
     *  @param score Packet score.
     *  @param air_time Packet airtime in milliseconds.
     *  @return Delay in milliseconds.
     */
    int calcRxDelay(float score, uint32_t air_time) const override;
    /** @brief Calculates flood retransmission delay.
     *  @param packet Packet being retransmitted.
     *  @return Delay in milliseconds.
     */
    uint32_t getRetransmitDelay(const mesh::Packet *packet) override;
    /** @brief Calculates direct retransmission delay.
     *  @param packet Packet being retransmitted.
     *  @return Delay in milliseconds.
     */
    uint32_t getDirectRetransmitDelay(const mesh::Packet *packet) override;
    /** @brief Returns the extra acknowledgement count.
     *  @param None.
     *  @return Extra acknowledgement count.
     */
    uint8_t getExtraAckTransmitCount() const override;
    /** @brief Filters an incoming flood packet.
     *  @param packet Incoming packet.
     *  @return true when accepted; otherwise false.
     */
    bool filterRecvFloodPacket(mesh::Packet *packet) override;
    /** @brief Determines whether a packet may be forwarded.
     *  @param packet Packet under consideration.
     *  @return true when forwarding is allowed; otherwise false.
     */
    bool allowPacketForward(const mesh::Packet *packet) override;
    /** @brief Sends a packet through a transport-key scope.
     *  @param scope Transport-key scope.
     *  @param pkt Packet to send.
     *  @param delay_millis Delay before sending.
     *  @return None.
     */
    void sendFloodScoped(const TransportKey &scope, mesh::Packet *pkt,
                         uint32_t delay_millis);
    /** @brief Sends a packet to a contact through its scope.
     *  @param recipient Destination contact.
     *  @param pkt Packet to send.
     *  @param delay_millis Delay before sending.
     *  @return None.
     */
    void sendFloodScoped(const ContactInfo &recipient, mesh::Packet *pkt,
                         uint32_t delay_millis = 0) override;
    /** @brief Sends a packet through a group-channel scope.
     *  @param channel Destination channel.
     *  @param pkt Packet to send.
     *  @param delay_millis Delay before sending.
     *  @return None.
     */
    void sendFloodScoped(const mesh::GroupChannel &channel, mesh::Packet *pkt,
                         uint32_t delay_millis = 0) override;
    /** @brief Forwards raw radio data to the serial interface.
     *  @param snr Received signal-to-noise ratio.
     *  @param rssi Received signal strength.
     *  @param raw Raw packet bytes.
     *  @param len Number of raw bytes.
     *  @return None.
     */
    void logRxRaw(float snr, float rssi, const uint8_t raw[], int len) override;
    /** @brief Reports whether automatic contact addition is enabled.
     *  @param None.
     *  @return true when enabled; otherwise false.
     */
    bool isAutoAddEnabled() const override;
    /** @brief Checks whether a contact type may be auto-added.
     *  @param type Contact advertisement type.
     *  @return true when allowed; otherwise false.
     */
    bool shouldAutoAddContactType(uint8_t type) const override;
    /** @brief Reports whether the oldest contact may be overwritten.
     *  @param None.
     *  @return true when allowed; otherwise false.
     */
    bool shouldOverwriteWhenFull() const override;
    /** @brief Returns the automatic contact hop limit.
     *  @param None.
     *  @return Automatic contact hop limit.
     */
    uint8_t getAutoAddMaxHops() const override;
    /** @brief Handles a full contact table.
     *  @param None.
     *  @return None.
     */
    void onContactsFull() override;
    /** @brief Removes an overwritten contact from storage.
     *  @param pub_key Public key of the contact.
     *  @return None.
     */
    void onContactOverwrite(const uint8_t *pub_key) override;
    /** @brief Handles a received contact path.
     *  @param from Contact that supplied the path.
     *  @param in_path Received path bytes.
     *  @param in_path_len Received path length.
     *  @param out_path Destination path buffer.
     *  @param out_path_len Destination buffer length.
     *  @param extra_type Additional data type.
     *  @param extra Additional data buffer.
     *  @param extra_len Additional data length.
     *  @return true when the path is accepted; otherwise false.
     */
    bool onContactPathRecv(ContactInfo &from, uint8_t *in_path,
                           uint8_t in_path_len, uint8_t *out_path,
                           uint8_t out_path_len, uint8_t extra_type,
                           uint8_t *extra, uint8_t extra_len) override;
    /** @brief Handles a newly discovered or updated contact.
     *  @param contact Discovered contact.
     *  @param is_new Whether the contact is new.
     *  @param path_len Advertised path length.
     *  @param path Advertised path bytes.
     *  @return None.
     */
    void onDiscoveredContact(ContactInfo &contact, bool is_new,
                             uint8_t path_len, const uint8_t *path) override;
    /** @brief Handles a contact path update.
     *  @param contact Contact whose path changed.
     *  @return None.
     */
    void onContactPathUpdated(const ContactInfo &contact) override;
    /** @brief Processes an acknowledgement packet.
     *  @param data Acknowledgement data.
     *  @return Matching contact, or nullptr when unmatched.
     */
    ContactInfo *processAck(const uint8_t *data) override;
    /** @brief Queues a received message and notifies the application.
     *  @param from Message sender.
     *  @param txt_type Message type.
     *  @param pkt Source packet.
     *  @param sender_timestamp Sender timestamp.
     *  @param extra Optional extra data.
     *  @param extra_len Extra data length.
     *  @param text Message text.
     *  @return None.
     */
    void queueMessage(const ContactInfo &from, uint8_t txt_type,
                      mesh::Packet *pkt, uint32_t sender_timestamp,
                      const uint8_t *extra, int extra_len, const char *text);
    /** @brief Handles a received contact message.
     *  @param from Message sender.
     *  @param pkt Source packet.
     *  @param sender_timestamp Sender timestamp.
     *  @param text Message text.
     *  @return None.
     */
    void onMessageRecv(const ContactInfo &from, mesh::Packet *pkt,
                       uint32_t sender_timestamp, const char *text) override;
    /** @brief Handles received command data.
     *  @param from Command sender.
     *  @param pkt Source packet.
     *  @param sender_timestamp Sender timestamp.
     *  @param text Command text.
     *  @return None.
     */
    void onCommandDataRecv(const ContactInfo &from, mesh::Packet *pkt,
                           uint32_t sender_timestamp,
                           const char *text) override;
    /** @brief Handles a received signed message.
     *  @param from Message sender.
     *  @param pkt Source packet.
     *  @param sender_timestamp Sender timestamp.
     *  @param sender_prefix Sender public-key prefix.
     *  @param text Message text.
     *  @return None.
     */
    void onSignedMessageRecv(const ContactInfo &from, mesh::Packet *pkt,
                             uint32_t sender_timestamp,
                             const uint8_t *sender_prefix,
                             const char *text) override;
    /** @brief Handles a received channel message.
     *  @param channel Message channel.
     *  @param pkt Source packet.
     *  @param timestamp Message timestamp.
     *  @param text Message text.
     *  @return None.
     */
    void onChannelMessageRecv(const mesh::GroupChannel &channel,
                              mesh::Packet *pkt, uint32_t timestamp,
                              const char *text) override;
    /** @brief Handles received channel data.
     *  @param channel Data channel.
     *  @param pkt Source packet.
     *  @param data_type Application data type.
     *  @param data Data bytes.
     *  @param data_len Data length.
     *  @return None.
     */
    void onChannelDataRecv(const mesh::GroupChannel &channel, mesh::Packet *pkt,
                           uint16_t data_type, const uint8_t *data,
                           size_t data_len) override;
    /** @brief Handles a contact request.
     *  @param contact Requesting contact.
     *  @param sender_timestamp Request timestamp.
     *  @param data Request data.
     *  @param len Request data length.
     *  @param reply Reply buffer.
     *  @return Reply length.
     */
    uint8_t onContactRequest(const ContactInfo &contact,
                             uint32_t sender_timestamp, const uint8_t *data,
                             uint8_t len, uint8_t *reply) override;
    /** @brief Handles a contact response.
     *  @param contact Responding contact.
     *  @param data Response data.
     *  @param len Response data length.
     *  @return None.
     */
    void onContactResponse(const ContactInfo &contact, const uint8_t *data,
                           uint8_t len) override;
    /** @brief Handles received control data.
     *  @param packet Source packet.
     *  @return None.
     */
    void onControlDataRecv(mesh::Packet *packet) override;
    /** @brief Handles received raw data.
     *  @param packet Source packet.
     *  @return None.
     */
    void onRawDataRecv(mesh::Packet *packet) override;
    /** @brief Handles a received trace response.
     *  @param packet Source packet.
     *  @param tag Trace tag.
     *  @param auth_code Trace authorization code.
     *  @param flags Trace flags.
     *  @param path_snrs Per-hop signal-to-noise ratios.
     *  @param path_hashes Per-hop path hashes.
     *  @param path_len Path length.
     *  @return None.
     */
    void onTraceRecv(mesh::Packet *packet, uint32_t tag, uint32_t auth_code,
                     uint8_t flags, const uint8_t *path_snrs,
                     const uint8_t *path_hashes, uint8_t path_len) override;
    /** @brief Calculates a flood acknowledgement timeout.
     *  @param pkt_airtime_millis Packet airtime in milliseconds.
     *  @return Timeout in milliseconds.
     */
    uint32_t
    calcFloodTimeoutMillisFor(uint32_t pkt_airtime_millis) const override;
    /** @brief Calculates a direct acknowledgement timeout.
     *  @param pkt_airtime_millis Packet airtime in milliseconds.
     *  @param path_len Encoded path length.
     *  @return Timeout in milliseconds.
     */
    uint32_t calcDirectTimeoutMillisFor(uint32_t pkt_airtime_millis,
                                        uint8_t path_len) const override;
    /** @brief Handles a send timeout.
     *  @param None.
     *  @return None.
     */
    void onSendTimeout() override;
    // DataStoreHost methods
    /**
     * @brief Loads a contact into the mesh table.
     * @param contact Loaded contact.
     * @return true to continue loading; otherwise false.
     */
    bool onContactLoaded(const ContactInfo &contact) override {
        return addContact(contact);
    }
    /**
     * @brief Supplies a contact for persistence.
     * @param idx Contact index.
     * @param contact Destination contact.
     * @return true when found; otherwise false.
     */
    bool getContactForSave(uint32_t idx, ContactInfo &contact) override {
        return getContactByIdx(idx, contact);
    }
    /**
     * @brief Loads a channel into the mesh table.
     * @param channel_idx Channel index.
     * @param ch Loaded channel.
     * @return true to continue loading; otherwise false.
     */
    bool onChannelLoaded(uint8_t channel_idx,
                         const ChannelDetails &ch) override {
        return setChannel(channel_idx, ch);
    }
    /**
     * @brief Supplies a channel for persistence.
     * @param channel_idx Channel index.
     * @param ch Destination channel.
     * @return true when found; otherwise false.
     */
    bool getChannelForSave(uint8_t channel_idx, ChannelDetails &ch) override {
        return getChannel(channel_idx, ch);
    }
    /** Clears all pending application requests.
     *  @param None.
     *  @return None.
     */
    void clearPendingReqs() {
        pending_login = pending_status = pending_telemetry = pending_discovery =
            pending_req = 0;
    }

  public:
    /** Saves current sensor coordinates and node preferences.
     *  @param None.
     *  @return None.
     */
    void savePrefs() {
        _prefs.node_lat = sensors.node_lat;
        _prefs.node_lon = sensors.node_lon;
        _store->savePrefs(_prefs);
    }
#if ENV_INCLUDE_GPS == 1
    /** Applies persisted GPS preferences to the sensor manager.
     *  @param None.
     *  @return None.
     */
    void applyGpsPrefs() {
        sensors.setSettingValue("gps", _prefs.gps_enabled ? "1" : "0");
        if (_prefs.gps_interval > 0) {
            char interval_str[12]; // Max: 24 hours = 86400 seconds (5 digits +
                                   // null)
            sprintf(interval_str, "%u", _prefs.gps_interval);
            sensors.setSettingValue("gps_interval", interval_str);
        }
    }
#endif
    /** Reports whether the node has pending application work.
     *  @param None.
     *  @return true when work is pending; otherwise false.
     */
    bool hasPendingWork() const;

  private:
    /** @brief Writes a successful protocol response.
     *  @param None.
     *  @return None.
     */
    void writeOKFrame();
    /** @brief Writes an error protocol response.
     *  @param err_code Error code to write.
     *  @return None.
     */
    void writeErrFrame(uint8_t err_code);
    /** @brief Writes a disabled protocol response.
     *  @param None.
     *  @return None.
     */
    void writeDisabledFrame();
    /** @brief Writes a contact protocol response.
     *  @param code Response code.
     *  @param contact Contact to serialize.
     *  @return None.
     */
    void writeContactRespFrame(uint8_t code, const ContactInfo &contact);
    /** @brief Decodes a contact from a protocol frame.
     *  @param contact Destination contact.
     *  @param last_mod Destination modification timestamp.
     *  @param frame Encoded frame bytes.
     *  @param len Frame length.
     *  @return None.
     */
    void updateContactFromFrame(ContactInfo &contact, uint32_t &last_mod,
                                const uint8_t *frame, int len);
    /** @brief Adds a frame to the bounded offline queue.
     *  @param frame Frame bytes.
     *  @param len Frame length.
     *  @return None.
     */
    void addToOfflineQueue(const uint8_t frame[], int len);
    /** @brief Removes the oldest frame from the offline queue.
     *  @param frame Destination frame buffer.
     *  @return Frame length, or zero when empty.
     */
    int getFromOfflineQueue(uint8_t frame[]);
    int getBlobByKey(const uint8_t key[], int key_len,
                     uint8_t dest_buf[]) override {
        return _store->getBlobByKey(key, key_len, dest_buf);
    }
    bool putBlobByKey(const uint8_t key[], int key_len, const uint8_t src_buf[],
                      int len) override {
        return _store->putBlobByKey(key, key_len, src_buf, len);
    }
    /** @brief Processes serial input in CLI rescue mode.
     *  @param None.
     *  @return None.
     */
    void checkCLIRescueCmd();
    /** @brief Processes frames received from the serial interface.
     *  @param None.
     *  @return None.
     */
    void checkSerialInterface();
    /** @brief Checks whether a repeat frequency is supported.
     *  @param f Frequency in hertz.
     *  @return true when supported; otherwise false.
     */
    bool isValidClientRepeatFreq(uint32_t f) const;
    // helpers, short-cuts
    /** Saves all channels through the configured DataStore.
     *  @param None.
     *  @return None.
     */
    void saveChannels() { _store->saveChannels(this); }
    /** Saves contacts through the configured DataStore.
     *  @param None.
     *  @return None.
     */
    void saveContacts();
    DataStore *_store;
    NodePrefs _prefs;
    uint32_t pending_login;
    uint32_t pending_status;
    uint32_t pending_telemetry, pending_discovery; // pending _TELEMETRY_REQ
    uint32_t pending_req;                          // pending _BINARY_REQ
    BaseSerialInterface *_serial;
    AbstractUITask *_ui;
    ContactsIterator _iter;
    uint32_t _iter_filter_since;
    uint32_t _most_recent_lastmod;
    uint32_t _active_ble_pin;
    bool _iter_started;
    bool _cli_rescue;
    bool send_unscoped; // force un-scoped flood (instead of using send_scope)
    char cli_command[80];
    uint8_t app_target_ver;
    uint8_t *sign_data;
    uint32_t sign_data_len;
    unsigned long dirty_contacts_expiry;
    TransportKey send_scope;
    uint8_t cmd_frame[MAX_FRAME_SIZE + 1];
    uint8_t out_frame[MAX_FRAME_SIZE + 1];
    CayenneLPP telemetry;
    struct Frame {
        uint8_t len;
        uint8_t buf[MAX_FRAME_SIZE];
        /**
         * @brief Reports whether the frame contains a channel message.
         * @param None.
         * @return true for a channel message; otherwise false.
         */
        bool isChannelMsg() const;
    };
    int offline_queue_len;
    Frame offline_queue[OFFLINE_QUEUE_SIZE];
    struct AckTableEntry {
        unsigned long msg_sent;
        uint32_t ack;
        ContactInfo *contact;
    };
#define EXPECTED_ACK_TABLE_SIZE 8
    AckTableEntry expected_ack_table[EXPECTED_ACK_TABLE_SIZE]; // circular table
    int next_ack_idx;
#define ADVERT_PATH_TABLE_SIZE 16
    AdvertPath advert_paths[ADVERT_PATH_TABLE_SIZE]; // circular table
};

extern MyMesh the_mesh;
