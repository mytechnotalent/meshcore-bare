/** @file main.cpp
 *  @brief MeshCore application entry point.
 */

#include "globals.h"

/**
 * @brief Halts the firmware after an unrecoverable initialization failure.
 * @param None.
 * @return None.
 */
void halt() {
    while (1)
        ;
}

/**
 * @brief Initializes hardware, storage, interfaces, and the mesh node.
 * @param None.
 * @return None.
 */
void setup() {
    Serial.begin(115200);
    board.begin();
    if (!radio_init()) {
        Serial.println("Radio init failed.");
        halt();
    }
    fast_rng.begin(radio_driver.getRngSeed());
    SPIFFS.begin(true);
    store.begin();
    // Begin the Mesh networking (false = no display)
    the_mesh.begin(false);
    char emptyName[] = "";
#ifdef BLE_CUSTOM_NAME
    bluetooth_interface.begin(BLE_CUSTOM_NAME, emptyName, BLE_CUSTOM_PIN);
#else
    bluetooth_interface.begin("MeshCore-Bare", emptyName, 123456);
#endif
#ifdef PUBLIC_CHANNEL_NAME
    the_mesh.addChannel(PUBLIC_CHANNEL_NAME, PUBLIC_CHANNEL_PSK);
    Serial.printf("Added channel: %s\n", PUBLIC_CHANNEL_NAME);
#endif
    interface_manager.addInterface(InterfaceType::Bluetooth,
                                   &bluetooth_interface);
    the_mesh.startInterface(interface_manager);
    sensors.begin();
#if ENV_INCLUDE_GPS == 1
    the_mesh.applyGpsPrefs();
#endif
    board.onBootComplete();
    Serial.println(
        "MeshCore Node booted, config applied, and BLE advertising started!");
}

/**
 * @brief Services the radio, interfaces, sensors, and real-time clock.
 * @param None.
 * @return None.
 */
void loop() {
    the_mesh.loop();
    interface_manager.loop();
    sensors.loop();
    rtc_clock.tick();
}
