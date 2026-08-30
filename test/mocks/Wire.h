
/** @file Wire.h
 *  @brief Native-test I2C compatibility shim.
 */

#pragma once

#include <stdint.h>

/**
 * @brief Mocked Arduino TwoWire.
 */
class TwoWire {
  public:
    /** @brief Starts mock I2C. @param None. @return None. */
    void begin() {}
    /** @brief Starts a mock I2C transmission. @param address Device address.
     * @return None. */
    void beginTransmission(uint8_t address) {}
    /** @brief Ends a mock I2C transmission. @param None. @return Status code.
     */
    uint8_t endTransmission() { return 0; }
    /** @brief Requests mock I2C data. @param address Device address. @param
     * quantity Requested bytes. @return Bytes available. */
    uint8_t requestFrom(uint8_t address, uint8_t quantity) { return 0; }
    /** @brief Returns mock I2C bytes available. @param None. @return Bytes
     * available. */
    int available() { return 0; }
    /** @brief Reads a mock I2C byte. @param None. @return Byte, or -1. */
    int read() { return -1; }
    /** @brief Writes a mock I2C byte. @param value Byte to write. @return Bytes
     * written. */
    size_t write(uint8_t value) { return 1; }
};

/**
 * @brief Mocked global Wire instance.
 */
inline TwoWire Wire;
