
/** @file ESP32Board.h
 *  @brief Native-test board implementation.
 */

#pragma once

#include <Mesh.h>

/**
 * @brief Test double standing in for the real ESP32Board.
 */
class ESP32Board : public mesh::MainBoard {
  public:
    uint16_t battMilliVolts = 4000;
    float mcuTemperature = 25.0f;
    int rebootCount = 0;

    /** @brief Initializes the mock board. @param None. @return None. */
    void begin() {}
    /** @brief Returns mock battery voltage. @param None. @return Voltage in
     * millivolts. */
    uint16_t getBattMilliVolts() override { return battMilliVolts; }
    /** @brief Returns mock MCU temperature. @param None. @return Temperature in
     * Celsius. */
    float getMCUTemperature() override { return mcuTemperature; }
    /** @brief Returns mock board name. @param None. @return Board name. */
    const char *getManufacturerName() const override { return "mock-esp32"; }
    /** @brief Records a mock reboot. @param None. @return None. */
    void reboot() override { rebootCount++; }
    /** @brief Returns the mock startup reason. @param None. @return Startup
     * reason. */
    uint8_t getStartupReason() const override { return 0; }
    /** @brief Completes mock board startup. @param None. @return None. */
    void onBootComplete() override {}
};
