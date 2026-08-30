
/** @file test_main.cpp
 *  @brief Native tests for MyMesh behavior and packet exchange.
 */

#include "FS.h"
#include <unity.h>

#define FILESYSTEM fs::FS

#include "MyMesh.h"
#include <string.h>
#include <vector>

/**
 * @brief Minimal mesh::Radio test double (no real RF I/O).
 *
 * Also supports being wired to another FakeRadio's outbound bytes, so two
 * MyMesh instances can exchange real, wire-format packets over a simulated
 * link (see pumpOnce()).
 */
class FakeRadio : public mesh::Radio {
  public:
    virtual ~FakeRadio() = default;
    uint32_t estAirtimeMs = 10;
    bool sendComplete = true;
    bool inRecvMode = true;
    std::vector<uint8_t> pendingIncoming;
    std::vector<uint8_t> lastSent;
    bool hasSent = false;

    int recvRaw(uint8_t *bytes, int sz) override {
        if (pendingIncoming.empty())
            return 0;
        int n = (int)pendingIncoming.size();
        if (n > sz)
            n = sz;
        memcpy(bytes, pendingIncoming.data(), n);
        pendingIncoming.clear();
        return n;
    }
    uint32_t getEstAirtimeFor(int /*len_bytes*/) override {
        return estAirtimeMs;
    }
    float packetScore(float snr, int /*packet_len*/) override { return snr; }
    bool startSendRaw(const uint8_t *bytes, int len) override {
        lastSent.assign(bytes, bytes + len);
        hasSent = true;
        return true;
    }
    bool isSendComplete() override { return sendComplete; }
    void onSendFinished() override {}
    bool isInRecvMode() const override { return inRecvMode; }
    float getLastSNR() const override {
        return 8.0f;
    } // keep calcRxDelay small -> immediate processing
};

/**
 * @brief Test double for BaseSerialInterface, capturing pushed frames so
 *        tests can observe MyMesh's outbound app-protocol traffic.
 */
class FakeSerialInterface : public BaseSerialInterface {
  public:
    bool connected = false;
    int writeFrameCalls = 0;
    std::vector<uint8_t> lastFrame;

    void enable() override {}
    void disable() override {}
    bool isEnabled() const override { return true; }
    bool isConnected() const override { return connected; }
    bool isWriteBusy() const override { return false; }
    size_t writeFrame(const uint8_t src[], size_t len) override {
        writeFrameCalls++;
        lastFrame.assign(src, src + len);
        return len;
    }
    size_t checkRecvFrame(uint8_t * /*dest*/) override { return 0; }
};

/**
 * @brief Delivers whatever `tx` last transmitted (if anything) to `rx`'s
 *        inbound queue, simulating a shared radio channel between two nodes.
 *
 * @param tx Radio to read a transmitted frame from.
 * @param rx Radio to deliver that frame to.
 * @return true if a frame was transferred.
 */
static bool pumpOnce(FakeRadio &tx, FakeRadio &rx) {
    if (!tx.hasSent)
        return false;
    rx.pendingIncoming = tx.lastSent;
    tx.hasSent = false;
    tx.lastSent.clear();
    return true;
}

// Definitions for the globals MyMesh.cpp references directly
// (board/sensors/radio_driver).
ESP32Board board;
SensorManager sensors;
MockRadioWrapper radio_driver;

/**
 * @brief MyMesh.cpp calls this free function (normally defined in globals.cpp,
 *        which pulls in real RadioLib hardware) to mint a new node identity.
 */
mesh::LocalIdentity radio_new_identity() {
    static StdRNG rng;
    static bool seeded = false;
    if (!seeded) {
        rng.begin(42);
        seeded = true;
    }
    return mesh::LocalIdentity(&rng);
}

/**
 * @brief Test-only subclass exposing MyMesh's protected policy methods so
 *        tests can call them directly without duplicating MyMesh's logic.
 */
class TestableMyMesh : public MyMesh {
  public:
    using MyMesh::MyMesh;
    virtual ~TestableMyMesh() = default;
    using MyMesh::calcDirectTimeoutMillisFor;
    using MyMesh::calcFloodTimeoutMillisFor;
    using MyMesh::getExtraAckTransmitCount;
    using MyMesh::shouldOverwriteWhenFull;
};

/**
 * @brief Thin subclass adding a virtual destructor so heap-owned instances
 *        can be deleted through their own (most-derived) pointer type
 *        without a -Wdelete-non-virtual-dtor warning.
 */
class TestSimpleMeshTables : public SimpleMeshTables {
  public:
    virtual ~TestSimpleMeshTables() = default;
};

/**
 * @brief Thin subclass adding a virtual destructor (see TestSimpleMeshTables).
 */
class TestVolatileRTCClock : public VolatileRTCClock {
  public:
    virtual ~TestVolatileRTCClock() = default;
};

/**
 * @brief Thin subclass adding a virtual destructor (see TestSimpleMeshTables).
 */
class TestStdRNG : public StdRNG {
  public:
    virtual ~TestStdRNG() = default;
};

static FakeRadio *g_radio;
static TestStdRNG *g_rng;
static TestVolatileRTCClock *g_rtc;
static TestSimpleMeshTables *g_tables;
static ArduinoMillis *g_millis;
static StaticPoolPacketManager *g_packet_manager;
static fs::FS *g_fs;
static DataStore *g_store;
static TestableMyMesh *g_mesh;

/** Resets per-test mesh state before each Unity test. */
void setUp(void) {
    g_radio = new FakeRadio();
    g_rng = new TestStdRNG();
    g_rng->begin(12345);
    g_rtc = new TestVolatileRTCClock();
    g_tables = new TestSimpleMeshTables();
    g_millis = new ArduinoMillis();
    g_packet_manager = new StaticPoolPacketManager(16);
    g_fs = new fs::FS();
    g_store = new DataStore(*g_fs, *g_rtc);
    g_store->begin();
    g_mesh = new TestableMyMesh(*g_radio, *g_millis, *g_packet_manager, *g_rng,
                                *g_rtc, *g_tables, *g_store, nullptr);
}

/** Releases per-test mesh state after each Unity test. */
void tearDown(void) {
    delete g_mesh;
    delete g_store;
    delete g_fs;
    delete g_tables;
    delete g_packet_manager;
    delete g_millis;
    delete g_rtc;
    delete g_rng;
    delete g_radio;
}

void test_constructor_sets_default_node_name_and_radio_prefs(void) {
    TEST_ASSERT_EQUAL_STRING("NONAME", g_mesh->getNodeName());
    NodePrefs *prefs = g_mesh->getNodePrefs();
    TEST_ASSERT_EQUAL_FLOAT(LORA_FREQ, prefs->freq);
    TEST_ASSERT_EQUAL_UINT8(LORA_SF, prefs->sf);
    TEST_ASSERT_EQUAL_INT8(LORA_TX_POWER, prefs->tx_power_dbm);
    TEST_ASSERT_FALSE(prefs->isRepeatEn()); // setRepeatEn(false) in ctor
}

/** Verifies the configured Bluetooth PIN. */
void test_getBLEPin_returns_default_pin(void) {
    TEST_ASSERT_EQUAL_UINT32(g_mesh->getNodePrefs()->ble_pin,
                             g_mesh->getBLEPin());
}

/** Verifies the extra acknowledgement count remains bounded. */
void test_getExtraAckTransmitCount_is_bounded(void) {
    uint8_t count = g_mesh->getExtraAckTransmitCount();
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(8, count);
}

/** Verifies flood timeout scaling. */
void test_calcFloodTimeoutMillisFor_scales_with_airtime(void) {
    uint32_t small = g_mesh->calcFloodTimeoutMillisFor(50);
    uint32_t large = g_mesh->calcFloodTimeoutMillisFor(5000);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(small, large);
}

/** Verifies direct timeout scaling by path length. */
void test_calcDirectTimeoutMillisFor_scales_with_path_len(void) {
    uint32_t direct = g_mesh->calcDirectTimeoutMillisFor(50, 0);
    uint32_t via_3_hops = g_mesh->calcDirectTimeoutMillisFor(50, 3);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(direct, via_3_hops);
}

/** Verifies overwrite policy stability. */
void test_shouldOverwriteWhenFull_returns_bool(void) {
    bool v1 = g_mesh->shouldOverwriteWhenFull();
    bool v2 = g_mesh->shouldOverwriteWhenFull();
    TEST_ASSERT_EQUAL(v1, v2); // deterministic given unchanged prefs
}

/** Verifies channel creation and lookup. */
void test_addChannel_then_getChannel_round_trips(void) {
    ChannelDetails *ch =
        g_mesh->addChannel("Public", "izOH6cXN6mrJ5e26oRXNcg==");
    TEST_ASSERT_NOT_NULL(ch);
    TEST_ASSERT_EQUAL_STRING("Public", ch->name);

    ChannelDetails dest{};
    TEST_ASSERT_TRUE(g_mesh->getChannel(0, dest));
    TEST_ASSERT_EQUAL_STRING("Public", dest.name);
}

/**
 * @brief Builds a minimal, valid ContactInfo for contact-management tests.
 *
 * @param pubKeyByte Byte value to fill the contact's public key with (kept
 *        distinct per-caller so contacts don't collide).
 * @param name Contact display name.
 * @return A populated ContactInfo.
 */
static ContactInfo makeContact(uint8_t pubKeyByte, const char *name) {
    ContactInfo c{};
    uint8_t pub[32];
    memset(pub, pubKeyByte, sizeof(pub));
    c.id = mesh::Identity(pub);
    strncpy(c.name, name, sizeof(c.name) - 1);
    c.type = 1; // ADV_TYPE_CHAT
    return c;
}

/** Verifies contact creation, lookup, and removal. */
void test_addContact_then_lookup_and_remove(void) {
    ContactInfo c = makeContact(0x22, "Bob");
    TEST_ASSERT_TRUE(g_mesh->addContact(c));

    ContactInfo *found = g_mesh->lookupContactByPubKey(c.id.pub_key, 32);
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_STRING("Bob", found->name);

    TEST_ASSERT_TRUE(g_mesh->removeContact(*found));
    TEST_ASSERT_NULL(g_mesh->lookupContactByPubKey(c.id.pub_key, 32));
}

/** Verifies contact indexing after insertion. */
void test_getNumContacts_and_getContactByIdx(void) {
    int before = g_mesh->getNumContacts();
    ContactInfo c = makeContact(0x33, "Carol");
    TEST_ASSERT_TRUE(g_mesh->addContact(c));
    TEST_ASSERT_EQUAL(before + 1, g_mesh->getNumContacts());

    ContactInfo out{};
    bool foundByIdx = false;
    for (uint32_t i = 0; i < (uint32_t)g_mesh->getTotalContactSlots(); i++) {
        if (g_mesh->getContactByIdx(i, out) && out.id.matches(c.id)) {
            foundByIdx = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(foundByIdx);
}

/** Verifies contact search by name prefix. */
void test_searchContactsByPrefix_finds_by_name_prefix(void) {
    ContactInfo c = makeContact(0x44, "Dave");
    TEST_ASSERT_TRUE(g_mesh->addContact(c));

    ContactInfo *found = g_mesh->searchContactsByPrefix("Dav");
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_STRING("Dave", found->name);
}

/** Verifies self-advertisement packet construction. */
void test_createSelfAdvert_produces_advert_packet(void) {
    g_mesh->begin(false);
    mesh::Packet *pkt = g_mesh->createSelfAdvert(g_mesh->getNodeName());
    TEST_ASSERT_NOT_NULL(pkt);
    TEST_ASSERT_EQUAL_UINT8(0x04 /*PAYLOAD_TYPE_ADVERT*/,
                            pkt->getPayloadType());
    TEST_ASSERT_GREATER_THAN_UINT16(0, pkt->payload_len);
}

/** Verifies advertisement exchange between two mesh nodes. */
void test_two_nodes_exchange_advert_over_simulated_radio(void) {
    FakeRadio radioA, radioB;
    StdRNG rngA, rngB;
    rngA.begin(1);
    rngB.begin(2);
    VolatileRTCClock rtcA, rtcB;
    SimpleMeshTables tablesA, tablesB;
    fs::FS fsA, fsB;
    DataStore storeA(fsA, rtcA), storeB(fsB, rtcB);
    ArduinoMillis millisA, millisB;
    StaticPoolPacketManager packetManagerA(16), packetManagerB(16);
    storeA.begin();
    storeB.begin();
    TestableMyMesh nodeA(radioA, millisA, packetManagerA, rngA, rtcA, tablesA,
                         storeA, nullptr);
    TestableMyMesh nodeB(radioB, millisB, packetManagerB, rngB, rtcB, tablesB,
                         storeB, nullptr);
    nodeA.begin(false);
    nodeB.begin(false);

    FakeSerialInterface serialA, serialB;
    nodeA.startInterface(serialA);
    nodeB.startInterface(serialB);

    int contactsBefore = nodeB.getNumContacts();

    mesh::Packet *advert = nodeA.createSelfAdvert(nodeA.getNodeName());
    TEST_ASSERT_NOT_NULL(advert);
    nodeA.sendFlood(advert);
    delay(10); // advance the simulated clock so Dispatcher's next_tx_time has
               // passed
    nodeA.loop(); // Dispatcher::checkSend() -> radioA.startSendRaw()
    TEST_ASSERT_TRUE(radioA.hasSent);

    TEST_ASSERT_TRUE(pumpOnce(radioA, radioB));
    nodeB.loop(); // Dispatcher::checkRecv() -> parses + dispatches the advert

    TEST_ASSERT_EQUAL(contactsBefore + 1, nodeB.getNumContacts());
    ContactInfo *discovered =
        nodeB.lookupContactByPubKey(nodeA.self_id.pub_key, 32);
    TEST_ASSERT_NOT_NULL(discovered);
    TEST_ASSERT_EQUAL_STRING(nodeA.getNodeName(), discovered->name);
}

/** Verifies storage statistics remain available without hardware. */
void test_datastore_storage_stats_reachable(void) {
    // Native build has no real filesystem size concept -> reports 0.
    TEST_ASSERT_EQUAL_UINT32(0, g_store->getStorageUsedKb());
    TEST_ASSERT_EQUAL_UINT32(0, g_store->getStorageTotalKb());
}

/** Runs the MyMesh Unity test suite. */
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_constructor_sets_default_node_name_and_radio_prefs);
    RUN_TEST(test_getBLEPin_returns_default_pin);
    RUN_TEST(test_getExtraAckTransmitCount_is_bounded);
    RUN_TEST(test_calcFloodTimeoutMillisFor_scales_with_airtime);
    RUN_TEST(test_calcDirectTimeoutMillisFor_scales_with_path_len);
    RUN_TEST(test_shouldOverwriteWhenFull_returns_bool);
    RUN_TEST(test_addChannel_then_getChannel_round_trips);
    RUN_TEST(test_addContact_then_lookup_and_remove);
    RUN_TEST(test_getNumContacts_and_getContactByIdx);
    RUN_TEST(test_searchContactsByPrefix_finds_by_name_prefix);
    RUN_TEST(test_createSelfAdvert_produces_advert_packet);
    RUN_TEST(test_two_nodes_exchange_advert_over_simulated_radio);
    RUN_TEST(test_datastore_storage_stats_reachable);
    return UNITY_END();
}

#include "../../src/DataStore.cpp"
#include "../../src/MyMesh.cpp"
