
/** @file test_main.cpp
 *  @brief Native tests for NodePrefs serialization.
 */

#include "NodePrefs.h"
#include <string.h>
#include <string>
#include <unity.h>

/**
 * @brief In-memory Stream that reads back a fixed text buffer.
 */
class MockInputStream : public Stream {
    const char *_text;
    int pos, len;

  public:
    MockInputStream(const char *text) : _text(text) {
        pos = 0;
        len = strlen(text);
    }
    int available() override { return len - pos; }
    int read() override {
        if (pos < len)
            return _text[pos++];
        return -1;
    }
    int peek() override {
        if (pos < len)
            return _text[pos];
        return -1;
    }
};

/**
 * @brief In-memory Stream that captures everything printed to it.
 */
class MockPrintStream : public Stream {
    int len = 0;
    uint8_t _buf[2048];

  public:
    size_t write(uint8_t b) override {
        if ((size_t)len < sizeof(_buf)) {
            _buf[len++] = b;
            return 1;
        }
        return 0;
    }
    int getLength() const { return len; }
    const uint8_t *getBytes() const { return _buf; }
    std::string str() const { return std::string((const char *)_buf, len); }
};

/** Resets per-test state before each Unity test. */
void setUp(void) {}

/** Releases per-test state after each Unity test. */
void tearDown(void) {}

/** Verifies default preference values. */
void test_default_constructor_sets_expected_defaults(void) {
    NodePrefs prefs;
    TEST_ASSERT_EQUAL_STRING("", prefs.node_name);
    TEST_ASSERT_EQUAL_STRING("", prefs.default_scope_name);
    TEST_ASSERT_FALSE(
        prefs.isRepeatEn()); // repeat forwarding disabled by default
    for (size_t i = 0; i < sizeof(prefs.default_scope_key); i++) {
        TEST_ASSERT_EQUAL_UINT8(0, prefs.default_scope_key[i]);
    }
}

/** Verifies the repeat-forwarding accessor pair. */
void test_setRepeatEn_toggles_isRepeatEn(void) {
    NodePrefs prefs;
    prefs.setRepeatEn(false);
    TEST_ASSERT_FALSE(prefs.isRepeatEn());
    prefs.setRepeatEn(true);
    TEST_ASSERT_TRUE(prefs.isRepeatEn());
}

/** Verifies complete preference serialization round trips. */
void test_saveSerial_then_loadSerial_round_trips_all_fields(void) {
    NodePrefs src;
    strcpy(src.node_name, "TestNode");
    src.node_lat = 12.5;
    src.node_lon = -71.25;
    src.freq = 910.525f;
    src.bw = 62.5f;
    src.sf = 7;
    src.cr = 5;
    src.tx_power_dbm = 17;
    src.airtime_factor = 1.5f;
    src.rx_delay_base = 3.0f;
    src.path_hash_mode = 2;
    src.multi_acks = 1;
    src.ble_pin = 654321;
    src.buzzer_quiet = 1;
    src.vibe_quiet = 0;
    src.gps_enabled = 1;
    src.gps_interval = 60;
    src.advert_loc_policy = ADVERT_LOC_SHARE;
    src.autoadd_config = 3;
    src.manual_add_contacts = 1;
    src.telemetry_mode_base = TELEM_MODE_ALLOW_ALL;
    src.telemetry_mode_loc = TELEM_MODE_ALLOW_FLAGS;
    src.telemetry_mode_env = TELEM_MODE_DENY;
    src.autoadd_max_hops = 4;
    strcpy(src.default_scope_name, "scope-1");
    memset(src.default_scope_key, 0xAB, sizeof(src.default_scope_key));
    src.setRepeatEn(false);

    MockPrintStream out;
    TEST_ASSERT_TRUE(src.saveSerial(out));

    NodePrefs dest;
    std::string json =
        out.str(); // keep alive: MockInputStream just borrows the pointer
    MockInputStream in(json.c_str());
    TEST_ASSERT_TRUE(dest.loadSerial(in));

    TEST_ASSERT_EQUAL_STRING(src.node_name, dest.node_name);
    TEST_ASSERT_EQUAL_DOUBLE(src.node_lat, dest.node_lat);
    TEST_ASSERT_EQUAL_DOUBLE(src.node_lon, dest.node_lon);
    TEST_ASSERT_EQUAL_FLOAT(src.freq, dest.freq);
    TEST_ASSERT_EQUAL_FLOAT(src.bw, dest.bw);
    TEST_ASSERT_EQUAL_UINT8(src.sf, dest.sf);
    TEST_ASSERT_EQUAL_UINT8(src.cr, dest.cr);
    TEST_ASSERT_EQUAL_INT8(src.tx_power_dbm, dest.tx_power_dbm);
    TEST_ASSERT_EQUAL_FLOAT(src.airtime_factor, dest.airtime_factor);
    TEST_ASSERT_EQUAL_UINT8(src.path_hash_mode, dest.path_hash_mode);
    TEST_ASSERT_EQUAL_UINT8(src.multi_acks, dest.multi_acks);
    TEST_ASSERT_EQUAL_UINT32(src.ble_pin, dest.ble_pin);
    TEST_ASSERT_EQUAL_UINT8(src.buzzer_quiet, dest.buzzer_quiet);
    TEST_ASSERT_EQUAL_UINT8(src.gps_enabled, dest.gps_enabled);
    TEST_ASSERT_EQUAL_UINT32(src.gps_interval, dest.gps_interval);
    TEST_ASSERT_EQUAL_UINT8(src.advert_loc_policy, dest.advert_loc_policy);
    TEST_ASSERT_EQUAL_UINT8(src.autoadd_config, dest.autoadd_config);
    TEST_ASSERT_EQUAL_UINT8(src.manual_add_contacts, dest.manual_add_contacts);
    TEST_ASSERT_EQUAL_UINT8(src.telemetry_mode_base, dest.telemetry_mode_base);
    TEST_ASSERT_EQUAL_UINT8(src.telemetry_mode_loc, dest.telemetry_mode_loc);
    TEST_ASSERT_EQUAL_UINT8(src.telemetry_mode_env, dest.telemetry_mode_env);
    TEST_ASSERT_EQUAL_UINT8(src.autoadd_max_hops, dest.autoadd_max_hops);
    TEST_ASSERT_EQUAL_STRING(src.default_scope_name, dest.default_scope_name);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(src.default_scope_key, dest.default_scope_key,
                                  sizeof(src.default_scope_key));
    TEST_ASSERT_EQUAL(src.isRepeatEn(), dest.isRepeatEn());
}

/** Verifies malformed serialized input is rejected. */
void test_loadSerial_rejects_malformed_json(void) {
    NodePrefs prefs;
    MockInputStream in("{name:\"oops\""); // missing closing brace
    TEST_ASSERT_FALSE(prefs.loadSerial(in));
}

/** Runs the NodePrefs Unity test suite. */
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_default_constructor_sets_expected_defaults);
    RUN_TEST(test_setRepeatEn_toggles_isRepeatEn);
    RUN_TEST(test_saveSerial_then_loadSerial_round_trips_all_fields);
    RUN_TEST(test_loadSerial_rejects_malformed_json);
    return UNITY_END();
}
