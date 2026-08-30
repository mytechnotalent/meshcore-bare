
/** @file test_main.cpp
 *  @brief Native tests for DataStore persistence.
 */

#include "FS.h"
#include <string.h>
#include <unity.h>
#include <vector>

#define FILESYSTEM fs::FS

#include "DataStore.h"

/**
 * @brief Minimal mesh::RTCClock stub (DataStore only stores the pointer).
 */
class FakeRTCClock : public mesh::RTCClock {
  public:
    uint32_t now = 1715770351;
    virtual ~FakeRTCClock() = default;
    uint32_t getCurrentTime() override { return now; }
    void setCurrentTime(uint32_t time) override { now = time; }
};

static fs::FS *g_fs;
static FakeRTCClock *g_clock;
static DataStore *g_store;

/** Resets per-test storage before each Unity test. */
void setUp(void) {
    g_fs = new fs::FS();
    g_clock = new FakeRTCClock();
    g_store = new DataStore(*g_fs, *g_clock);
    g_store->begin();
}

/** Releases per-test storage after each Unity test. */
void tearDown(void) {
    delete g_store;
    delete g_clock;
    delete g_fs;
}

void test_blob_put_get_delete_round_trip(void) {
    const uint8_t key[] = {1, 2, 3, 4, 5, 6, 7};
    const uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};

    TEST_ASSERT_TRUE(
        g_store->putBlobByKey(key, sizeof(key), data, sizeof(data)));

    uint8_t out[32];
    uint8_t len = g_store->getBlobByKey(key, sizeof(key), out);
    TEST_ASSERT_EQUAL_UINT8(sizeof(data), len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, out, sizeof(data));

    TEST_ASSERT_TRUE(g_store->deleteBlobByKey(key, sizeof(key)));
    TEST_ASSERT_EQUAL_UINT8(0, g_store->getBlobByKey(key, sizeof(key), out));
}

/** Verifies missing blobs return an empty result. */
void test_blob_get_missing_key_returns_zero(void) {
    const uint8_t key[] = {9, 9, 9, 9, 9, 9, 9};
    uint8_t out[32];
    TEST_ASSERT_EQUAL_UINT8(0, g_store->getBlobByKey(key, sizeof(key), out));
}

/** Verifies preference persistence through DataStore. */
void test_prefs_save_then_load_round_trip(void) {
    NodePrefs src;
    strcpy(src.node_name, "UnitTestNode");
    src.freq = 869.525f;
    src.sf = 9;
    src.tx_power_dbm = 14;

    TEST_ASSERT_TRUE(g_store->savePrefs(src));

    NodePrefs dest;
    g_store->loadPrefs(dest);

    TEST_ASSERT_EQUAL_STRING(src.node_name, dest.node_name);
    TEST_ASSERT_EQUAL_FLOAT(src.freq, dest.freq);
    TEST_ASSERT_EQUAL_UINT8(src.sf, dest.sf);
    TEST_ASSERT_EQUAL_INT8(src.tx_power_dbm, dest.tx_power_dbm);
}

/**
 * @brief Test double capturing contacts fed to/from DataStore for save/load.
 */
class RecordingHost : public DataStoreHost {
  public:
    std::vector<ContactInfo> contacts;
    std::vector<ChannelDetails> channels;

    bool onContactLoaded(const ContactInfo &contact) override {
        contacts.push_back(contact);
        return true; // keep loading
    }
    bool getContactForSave(uint32_t idx, ContactInfo &contact) override {
        if (idx >= contacts.size())
            return false;
        contact = contacts[idx];
        return true;
    }
    bool onChannelLoaded(uint8_t /*channel_idx*/,
                         const ChannelDetails &ch) override {
        channels.push_back(ch);
        return true;
    }
    bool getChannelForSave(uint8_t channel_idx, ChannelDetails &ch) override {
        if (channel_idx >= channels.size())
            return false;
        ch = channels[channel_idx];
        return true;
    }
};

/** Verifies contact persistence through DataStore. */
void test_contacts_save_then_load_round_trip(void) {
    RecordingHost saveHost;
    ContactInfo c1{};
    uint8_t pub1[32];
    memset(pub1, 0x11, sizeof(pub1));
    c1.id = mesh::Identity(pub1);
    strcpy(c1.name, "Alice");
    c1.type = 1;
    c1.flags = 2;
    c1.lastmod = 1234;
    saveHost.contacts.push_back(c1);

    g_store->saveContacts(&saveHost, nullptr);

    RecordingHost loadHost;
    g_store->loadContacts(&loadHost);

    TEST_ASSERT_EQUAL(1, loadHost.contacts.size());
    TEST_ASSERT_EQUAL_STRING("Alice", loadHost.contacts[0].name);
    TEST_ASSERT_EQUAL_UINT8(1, loadHost.contacts[0].type);
    TEST_ASSERT_EQUAL_UINT8(2, loadHost.contacts[0].flags);
    TEST_ASSERT_EQUAL_UINT32(1234, loadHost.contacts[0].lastmod);
    TEST_ASSERT_TRUE(loadHost.contacts[0].id.matches(c1.id));
}

/** Verifies channel persistence through DataStore. */
void test_channels_save_then_load_round_trip(void) {
    RecordingHost saveHost;
    ChannelDetails ch{};
    memset(&ch, 0, sizeof(ch));
    strcpy(ch.name, "Public");
    memset(ch.channel.secret, 0x42, sizeof(ch.channel.secret));
    saveHost.channels.push_back(ch);

    g_store->saveChannels(&saveHost);

    RecordingHost loadHost;
    g_store->loadChannels(&loadHost);

    TEST_ASSERT_EQUAL(1, loadHost.channels.size());
    TEST_ASSERT_EQUAL_STRING("Public", loadHost.channels[0].name);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(ch.channel.secret,
                                  loadHost.channels[0].channel.secret,
                                  sizeof(ch.channel.secret));
}

/** Verifies formatting clears stored data. */
void test_formatFileSystem_clears_storage(void) {
    const uint8_t key[] = {1, 2, 3, 4, 5, 6, 7};
    const uint8_t data[] = {1, 2, 3, 4};
    TEST_ASSERT_TRUE(
        g_store->putBlobByKey(key, sizeof(key), data, sizeof(data)));
    TEST_ASSERT_TRUE(g_store->formatFileSystem());

    uint8_t out[32];
    TEST_ASSERT_EQUAL_UINT8(0, g_store->getBlobByKey(key, sizeof(key), out));
}

/** Verifies file removal through DataStore. */
void test_removeFile_deletes_written_file(void) {
    File f = g_fs->open("/foo.txt", "w", true);
    TEST_ASSERT_TRUE((bool)f);
    f.print("hello");
    f.close();

    TEST_ASSERT_TRUE(g_fs->exists("/foo.txt"));
    TEST_ASSERT_TRUE(g_store->removeFile("/foo.txt"));
    TEST_ASSERT_FALSE(g_fs->exists("/foo.txt"));
}

/** Runs the DataStore Unity test suite. */
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_blob_put_get_delete_round_trip);
    RUN_TEST(test_blob_get_missing_key_returns_zero);
    RUN_TEST(test_prefs_save_then_load_round_trip);
    RUN_TEST(test_contacts_save_then_load_round_trip);
    RUN_TEST(test_channels_save_then_load_round_trip);
    RUN_TEST(test_formatFileSystem_clears_storage);
    RUN_TEST(test_removeFile_deletes_written_file);
    return UNITY_END();
}

#include "../../src/DataStore.cpp"
