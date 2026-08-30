/** @file target.h
 *  @brief Target hardware declarations and platform includes.
 */

#pragma once

#include <Arduino.h>
#include <Mesh.h>
#include <helpers/AutoDiscoverRTCClock.h>
#include <helpers/ESP32Board.h>
#include <helpers/SensorManager.h>
#include <helpers/radiolib/CustomSX1262Wrapper.h>

extern ESP32Board board;
extern RADIO_CLASS radio;
extern WRAPPER_CLASS radio_driver;
extern AutoDiscoverRTCClock rtc_clock;
extern SensorManager sensors;

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
