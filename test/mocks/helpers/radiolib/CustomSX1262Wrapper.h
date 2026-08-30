
/** @file CustomSX1262Wrapper.h
 *  @brief Native-test radio module and wrapper implementations.
 */

#pragma once

#include <stdint.h>

/**
 * @brief Stand-in for the real RadioLib SX1262 module type.
 */
class MockRadioModule {};

/**
 * @brief Stand-in for the real CustomSX1262Wrapper radio driver type.
 *
 * Implements the small slice of the real wrapper's API that MyMesh.cpp
 * calls directly on the global `radio_driver` (radio parameter tuning and
 * stats reporting) so those call sites compile and behave deterministically.
 */
class MockRadioWrapper {
  public:
    float freq = 0, bw = 0;
    uint8_t sf = 0, cr = 0;
    int8_t txPowerDbm = 0;
    bool rxBoostedGain = false;
    float lastRSSI = -80.0f, lastSNR = 5.0f;
    uint32_t packetsRecv = 0, packetsSent = 0, packetsRecvErrors = 0;

    void setParams(float _freq, float _bw, uint8_t _sf, uint8_t _cr) {
        freq = _freq;
        bw = _bw;
        sf = _sf;
        cr = _cr;
    }
    void setTxPower(int8_t dbm) { txPowerDbm = dbm; }
    void setRxBoostedGainMode(bool enable) { rxBoostedGain = enable; }
    bool getRxBoostedGainMode() const { return rxBoostedGain; }
    float getLastRSSI() const { return lastRSSI; }
    float getLastSNR() const { return lastSNR; }
    uint32_t getPacketsRecv() const { return packetsRecv; }
    uint32_t getPacketsSent() const { return packetsSent; }
    uint32_t getPacketsRecvErrors() const { return packetsRecvErrors; }
};
