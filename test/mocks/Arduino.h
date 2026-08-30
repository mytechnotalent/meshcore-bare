
/** @file Arduino.h
 *  @brief Minimal Arduino API used by native tests.
 */

#pragma once

#include "Stream.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGMEM
#define PSTR(s) (s)
#define F(s) (s)
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))

typedef uint8_t byte;
typedef bool boolean;

#define HIGH 0x1
#define LOW 0x0
#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

/**
 * @brief Mock system uptime tracker (advance manually in tests as needed).
 */
inline uint32_t g_mock_millis = 0;

/** @brief Returns simulated uptime.
 *  @param None.
 *  @return Simulated milliseconds.
 */
inline unsigned long millis() { return g_mock_millis; }

/** @brief Returns simulated microseconds.
 *  @param None.
 *  @return Simulated microseconds.
 */
inline unsigned long micros() { return g_mock_millis * 1000UL; }

/** @brief Advances simulated uptime.
 *  @param ms Milliseconds to advance.
 *  @return None.
 */
inline void delay(uint32_t ms) { g_mock_millis += ms; }

/** @brief Seeds the mock random generator.
 *  @param seed Random seed.
 *  @return None.
 */
inline void randomSeed(unsigned long seed) { srand((unsigned int)seed); }

/**
 * @brief Mock random(min, max) matching Arduino's [min, max) semantics.
 *
 * @param min Inclusive lower bound.
 * @param max Exclusive upper bound.
 * @return Pseudo-random value in [min, max).
 */
inline long random(long min, long max) {
    if (max <= min)
        return min;
    return min + (rand() % (max - min));
}

/**
 * @brief Mock random(max) matching Arduino's [0, max) semantics.
 *
 * @param max Exclusive upper bound.
 * @return Pseudo-random value in [0, max).
 */
/** @brief Returns a mock random value below a limit.
 *  @param max Exclusive upper bound.
 *  @return Random value.
 */
inline long random(long max) { return random(0L, max); }

inline char *ltoa(long value, char *buffer, int base) {
    if (base == 10)
        snprintf(buffer, 32, "%ld", value);
    else
        buffer[0] = 0;
    return buffer;
}

/** @brief Configures a mock GPIO pin.
 *  @param pin Pin number.
 *  @param mode Pin mode.
 *  @return None.
 */
inline void pinMode(uint8_t pin, uint8_t mode) {}
/** @brief Writes a mock GPIO pin.
 *  @param pin Pin number.
 *  @param value Output value.
 *  @return None.
 */
inline void digitalWrite(uint8_t pin, uint8_t value) {}
/** @brief Reads a mock GPIO pin.
 *  @param pin Pin number.
 *  @return Mock pin state.
 */
inline int digitalRead(uint8_t pin) { return LOW; }

#define constrain(amt, low, high)                                              \
    ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// Plain template functions (not macros) so they don't clobber unrelated
// qualified calls like std::numeric_limits<T>::max() elsewhere in the TU.
template <class T, class L>
inline auto min(const T &a, const L &b) -> decltype((b < a) ? b : a) {
    return (b < a) ? b : a;
}
template <class T, class L>
inline auto max(const T &a, const L &b) -> decltype((b > a) ? b : a) {
    return (b > a) ? b : a;
}

/**
 * @brief Mocked Serial stream (no-op sink / never-ready source by default).
 */
inline Stream Serial;
