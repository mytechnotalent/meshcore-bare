/** @file globals.cpp
 *  @brief Global MeshCore and hardware instances.
 */

#include "globals.h"

ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);
ESP32Board board;
SensorManager sensors;

#if defined(P_LORA_SCLK)
SPIClass spi;
Module radio_module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, spi);
#else
Module radio_module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY);
#endif
RADIO_CLASS radio(&radio_module);

WRAPPER_CLASS radio_driver(radio, board);

StdRNG fast_rng;
SimpleMeshTables tables;
ArduinoMillis mesh_millis;
StaticPoolPacketManager packet_manager(16);
DataStore store(SPIFFS, rtc_clock);

MyMesh the_mesh(radio_driver, mesh_millis, packet_manager, fast_rng, rtc_clock,
                tables, store, nullptr);

MultiSerialInterface interface_manager;
SerialBLEInterface bluetooth_interface;

bool radio_init() {
#if defined(P_LORA_SCLK)
    spi.begin(P_LORA_SCLK, P_LORA_MISO, P_LORA_MOSI, P_LORA_NSS);
    return radio.std_init(&spi);
#else
    return radio.std_init();
#endif
}

mesh::LocalIdentity radio_new_identity() {
    RadioNoiseListener rng(radio);
    return mesh::LocalIdentity(&rng);
}
