/** @file globals.h
 *  @brief Declarations for global MeshCore and hardware instances.
 */

#pragma once

#include "MyMesh.h"
#include "target.h"
#include <Arduino.h>
#include <helpers/MultiSerialInterface.h>
#include <helpers/esp32/SerialBLEInterface.h>

/** Provides the fallback RTC used by the auto-discovering clock. */
extern ESP32RTCClock fallback_clock;
/** Provides the RTC used by MeshCore and persistent storage. */
extern AutoDiscoverRTCClock rtc_clock;
/** Provides board-level hardware operations. */
extern ESP32Board board;
/** Provides environmental sensor access. */
extern SensorManager sensors;

#if defined(P_LORA_SCLK)
/** Provides the configured SPI bus for the radio. */
extern SPIClass spi;
#endif

/** Provides the configured SX126x radio module. */
extern RADIO_CLASS radio;
/** Provides the MeshCore radio wrapper. */
extern WRAPPER_CLASS radio_driver;
/** Provides the random-number generator used by MeshCore. */
extern StdRNG fast_rng;
/** Provides the shared mesh tables. */
extern SimpleMeshTables tables;
/** Provides the millisecond clock used by MeshCore. */
extern ArduinoMillis mesh_millis;
/** Owns the fixed packet pool used by MeshCore. */
extern StaticPoolPacketManager packet_manager;
/** Provides persistent storage for the node. */
extern DataStore store;
/** Provides the application mesh instance. */
extern MyMesh the_mesh;
/** Routes serial traffic to the active interface. */
extern MultiSerialInterface interface_manager;
/** Provides the Bluetooth serial interface. */
extern SerialBLEInterface bluetooth_interface;

/**
 * @brief Initializes the radio.
 * @param None.
 * @return true when initialization succeeds; otherwise false.
 */
bool radio_init();

/**
 * @brief Creates a new local identity using radio entropy.
 * @param None.
 * @return A newly generated local identity.
 */
mesh::LocalIdentity radio_new_identity();
