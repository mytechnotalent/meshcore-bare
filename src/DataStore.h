/** @file DataStore.h
 *  @brief Filesystem persistence interface.
 */

#pragma once

#include "NodePrefs.h"
#include <helpers/ChannelDetails.h>
#include <helpers/ContactInfo.h>
#include <helpers/IdentityStore.h>

/** Supplies application callbacks used while loading and saving data. */
class DataStoreHost {
  public:
    /**
     * @brief Accepts a loaded contact.
     * @param contact Loaded contact.
     * @return true to continue loading; otherwise false.
     */
    virtual bool onContactLoaded(const ContactInfo &contact) = 0;
    /**
     * @brief Supplies a contact for persistence.
     * @param idx Contact index.
     * @param contact Destination contact.
     * @return true when a contact exists; otherwise false.
     */
    virtual bool getContactForSave(uint32_t idx, ContactInfo &contact) = 0;
    /**
     * @brief Accepts a loaded channel.
     * @param channel_idx Channel index.
     * @param ch Loaded channel.
     * @return true to continue loading; otherwise false.
     */
    virtual bool onChannelLoaded(uint8_t channel_idx,
                                 const ChannelDetails &ch) = 0;
    /**
     * @brief Supplies a channel for persistence.
     * @param channel_idx Channel index.
     * @param ch Destination channel.
     * @return true when a channel exists; otherwise false.
     */
    virtual bool getChannelForSave(uint8_t channel_idx, ChannelDetails &ch) = 0;
};

/** Provides persistent identity, preference, contact, channel, and blob data.
 */
class DataStore {
    FILESYSTEM *_fs;
    FILESYSTEM *_fsExtra;
    mesh::RTCClock *_clock;
    IdentityStore identity_store;
    /** Loads preferences from the legacy binary format. */
    void loadPrefsInt(const char *filename, NodePrefs &prefs);
#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
    /**
     * @brief Checks and repairs the advertisement blob file.
     * @param None.
     * @return None.
     */
    void checkAdvBlobFile();
#endif
  public:
    /**
     * @brief Creates a store using one filesystem and an RTC clock.
     * @param fs Primary filesystem.
     * @param clock Real-time clock.
     * @return None.
     */
    DataStore(FILESYSTEM &fs, mesh::RTCClock &clock);
    /**
     * @brief Creates a store using primary and secondary filesystems.
     * @param fs Primary filesystem.
     * @param fsExtra Secondary filesystem.
     * @param clock Real-time clock.
     * @return None.
     */
    DataStore(FILESYSTEM &fs, FILESYSTEM &fsExtra, mesh::RTCClock &clock);
    /**
     * @brief Initializes filesystem-backed storage.
     * @param None.
     * @return None.
     */
    void begin();
    /**
     * @brief Formats storage.
     * @param None.
     * @return true when formatting succeeds; otherwise false.
     */
    bool formatFileSystem();
    /**
     * @brief Returns the primary filesystem.
     * @param None.
     * @return Primary filesystem pointer.
     */
    FILESYSTEM *getPrimaryFS() const { return _fs; }
    /**
     * @brief Returns the optional secondary filesystem.
     * @param None.
     * @return Secondary filesystem pointer, or nullptr.
     */
    FILESYSTEM *getSecondaryFS() const { return _fsExtra; }
    /**
     * @brief Loads the main identity.
     * @param identity Destination identity.
     * @return true when a valid identity was loaded; otherwise false.
     */
    bool loadMainIdentity(mesh::LocalIdentity &identity);
    /**
     * @brief Saves the main identity.
     * @param identity Identity to save.
     * @return true when the write succeeds; otherwise false.
     */
    bool saveMainIdentity(const mesh::LocalIdentity &identity);
    /**
     * @brief Loads current or legacy node preferences.
     * @param prefs Destination preferences.
     * @return None.
     */
    void loadPrefs(NodePrefs &prefs);
    /**
     * @brief Saves node preferences.
     * @param prefs Preferences to save.
     * @return true when the write succeeds; otherwise false.
     */
    bool savePrefs(NodePrefs &prefs);
    /**
     * @brief Loads contacts into the supplied host.
     * @param host Destination callback host.
     * @return None.
     */
    void loadContacts(DataStoreHost *host);
    /**
     * @brief Saves contacts accepted by the optional filter.
     * @param host Source callback host.
     * @param filter Optional contact filter.
     * @return None.
     */
    void saveContacts(DataStoreHost *host,
                      bool (*filter)(const ContactInfo &c) = nullptr);
    /**
     * @brief Loads channels into the supplied host.
     * @param host Destination callback host.
     * @return None.
     */
    void loadChannels(DataStoreHost *host);
    /**
     * @brief Saves channels supplied by the host.
     * @param host Source callback host.
     * @return None.
     */
    void saveChannels(DataStoreHost *host);
    /**
     * @brief Migrates data to the secondary filesystem when configured.
     * @param None.
     * @return None.
     */
    void migrateToSecondaryFS();
    /**
     * @brief Reads a blob by key.
     * @param key Blob key.
     * @param key_len Key length.
     * @param dest_buf Destination buffer.
     * @return Stored blob length, or zero when unavailable.
     */
    uint8_t getBlobByKey(const uint8_t key[], int key_len, uint8_t dest_buf[]);
    /**
     * @brief Writes a blob by key.
     * @param key Blob key.
     * @param key_len Key length.
     * @param src_buf Source data.
     * @param len Data length.
     * @return true when the write succeeds; otherwise false.
     */
    bool putBlobByKey(const uint8_t key[], int key_len, const uint8_t src_buf[],
                      uint8_t len);
    /**
     * @brief Deletes a blob by key.
     * @param key Blob key.
     * @param key_len Key length.
     * @return true when deletion succeeds; otherwise false.
     */
    bool deleteBlobByKey(const uint8_t key[], int key_len);
    /**
     * @brief Opens a file for reading on the primary filesystem.
     * @param filename File path.
     * @return Open file handle.
     */
    File openRead(const char *filename);
    /**
     * @brief Opens a file for reading on a selected filesystem.
     * @param fs Filesystem to use.
     * @param filename File path.
     * @return Open file handle.
     */
    File openRead(FILESYSTEM *fs, const char *filename);
    /**
     * @brief Removes a file from the primary filesystem.
     * @param filename File path.
     * @return true when removal succeeds; otherwise false.
     */
    bool removeFile(const char *filename);
    /**
     * @brief Removes a file from a selected filesystem.
     * @param fs Filesystem to use.
     * @param filename File path.
     * @return true when removal succeeds; otherwise false.
     */
    bool removeFile(FILESYSTEM *fs, const char *filename);
    /**
     * @brief Returns used storage in kilobytes.
     * @param None.
     * @return Used storage in kilobytes.
     */
    uint32_t getStorageUsedKb() const;
    /**
     * @brief Returns total storage in kilobytes.
     * @param None.
     * @return Total storage in kilobytes.
     */
    uint32_t getStorageTotalKb() const;

  private:
    /**
     * @brief Selects the filesystem used for contacts and channels.
     * @param None.
     * @return Selected filesystem pointer.
     */
    FILESYSTEM *_getContactsChannelsFS() const {
        if (_fsExtra)
            return _fsExtra;
        return _fs;
    };
};
