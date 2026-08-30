
/** @file AbstractUITask.h
 *  @brief UI and board abstraction used by the mesh application.
 */

#pragma once

#include <Arduino.h>
#include <MeshCore.h>
#include <helpers/MultiSerialInterface.h>
#include <helpers/SensorManager.h>
#include <helpers/ui/DisplayDriver.h>
#include <helpers/ui/UIScreen.h>

#ifdef PIN_BUZZER
#include <helpers/ui/buzzer.h>
#endif

#include "NodePrefs.h"

enum class UIEventType {
    none,
    contactMessage,
    channelMessage,
    roomMessage,
    newContactMessage,
    ack
};

/** Defines the UI callbacks and board state exposed to the mesh application. */
class AbstractUITask {
  protected:
    mesh::MainBoard *_board;
    MultiSerialInterface *_interfaceManager;
    bool _connected;
    /**
     * @brief Creates a UI task bound to board and interface services.
     * @param board Board service.
     * @param interfaceManager Serial interface manager.
     * @return None.
     */
    AbstractUITask(mesh::MainBoard *board,
                   MultiSerialInterface *interfaceManager)
        : _board(board), _interfaceManager(interfaceManager) {
        _connected = false;
    }

  public:
    /**
     * @brief Updates the connection state shown by the UI.
     * @param connected New connection state.
     * @return None.
     */
    void setHasConnection(bool connected) { _connected = connected; }
    /**
     * @brief Returns whether the UI considers the application connected.
     * @param None.
     * @return true when connected; otherwise false.
     */
    bool hasConnection() const { return _connected; }
    /**
     * @brief Returns the board battery voltage in millivolts.
     * @param None.
     * @return Battery voltage in millivolts.
     */
    uint16_t getBattMilliVolts() const { return _board->getBattMilliVolts(); }
    /**
     * @brief Returns whether Bluetooth is currently enabled.
     * @param None.
     * @return true when enabled; otherwise false.
     */
    bool isBluetoothEnabled() const {
        return _interfaceManager->isBluetoothEnabled();
    }
    /**
     * @brief Enables Bluetooth through the configured interface manager.
     * @param None.
     * @return None.
     */
    void enableBluetooth() { _interfaceManager->enableBluetooth(); }
    /**
     * @brief Disables Bluetooth through the configured interface manager.
     * @param None.
     * @return None.
     */
    void disableBluetooth() { _interfaceManager->disableBluetooth(); }
    /**
     * @brief Notifies the UI that a message was read.
     * @param msgcount Number of unread messages remaining.
     * @return None.
     */
    virtual void msgRead(int msgcount) = 0;
    /**
     * @brief Displays a received message and its route metadata.
     * @param path_len Received route length.
     * @param from_name Sender display name.
     * @param text Message text.
     * @param msgcount Number of queued messages.
     * @return None.
     */
    virtual void newMsg(uint8_t path_len, const char *from_name,
                        const char *text, int msgcount) = 0;
    /**
     * @brief Delivers a UI event.
     * @param t Event type.
     * @return None.
     */
    virtual void notify(UIEventType t = UIEventType::none) = 0;
    /**
     * @brief Advances the UI task.
     * @param None.
     * @return None.
     */
    virtual void loop() = 0;
};
