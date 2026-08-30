![image](https://github.com/mytechnotalent/meshcore-bare/blob/main/meshcore-bare.png?raw=true)

## FREE Reverse Engineering Self-Study Course [HERE](https://github.com/mytechnotalent/Reverse-Engineering-Tutorial)

<br>

# MeshCore-Bare

Welcome to **MeshCore-Bare**, a highly-optimized, headless C++ mesh networking node designed for the ESP32-S3 and the SX1262 LoRa radio. 

If you are looking to understand the deep, architectural theory of how this codebase works, please read the [TUTORIAL.md](./TUTORIAL.md) and the accompanying 20-chapter textbook in the `/textbook/` directory.

If you just want to get your hardware wired up and start sending text messages immediately, follow the guide below!

---

## 1. Hardware Required

To build a single MeshCore node from scratch, you will need the following components:
*   **Microcontroller**: An ESP32-S3 development board (e.g., ESP32-S3-DevKitC-1). 
*   **Radio Module**: An SX1262 LoRa module (ensure you purchase one tuned to your country's frequency band, e.g., 915 MHz for North America, 868 MHz for Europe).
*   **Antenna**: A LoRa antenna (e.g., 915 MHz dipole). **Never power the radio without the antenna attached!**
*   **Wiring**: Breadboard and jumper wires (or a soldering iron for a permanent rig).

---

## 2. Wiring the Hardware (Pinout)

The ESP32-S3 communicates with the SX1262 LoRa radio via the SPI bus. You must wire the pins *exactly* as defined in our `platformio.ini` environment.

Connect the pins from the SX1262 module to your ESP32-S3 as follows:

| SX1262 Pin | ESP32-S3 Pin | Function |
| :--- | :--- | :--- |
| **VCC** / **3.3V** | `3V3` | Power |
| **GND** | `GND` | Ground |
| **SCK** | `GPIO 5` | SPI Clock |
| **MISO** | `GPIO 3` | SPI Master In, Slave Out |
| **MOSI** | `GPIO 6` | SPI Master Out, Slave In |
| **NSS** / **CS** | `GPIO 7` | SPI Chip Select |
| **DIO1** | `GPIO 16` | Hardware Interrupt (Tx/Rx Done) |
| **BUSY** | `GPIO 15` | Radio Busy Signal |
| **RST** | `GPIO 8` | Radio Hardware Reset |
| **RXEN** | `GPIO 4` | Receive Enable (RF Switch) |
| **TXEN** | `GPIO 9` | Transmit Enable (RF Switch) |

*(Note: Depending on your specific SX1262 breakout board, you may not have RXEN/TXEN pins if the RF switch is tied to DIO2 internally. Check your board's datasheet.)*

---

## 3. Flashing the Firmware

1. Install **Visual Studio Code** and the **PlatformIO** extension.
2. Clone this repository and open the folder in VSCode.
3. Plug your ESP32-S3 into your computer via a USB-C data cable.
4. Click the **PlatformIO: Upload** button (the right-pointing arrow in the bottom blue status bar).
5. PlatformIO will automatically download the required dependencies (RadioLib, Nanopb), compile the C++ firmware, and flash it to your ESP32-S3.

---

## 4. Connecting to the Smartphone App

MeshCore-Bare does not have an OLED screen or physical keyboard. You interact with it entirely via Bluetooth using the official MeshCore smartphone app.

1. **Download the App**: Install the "MeshCore" app from the iOS App Store or the Google Play Store.
2. **Power On**: Plug your ESP32-S3 node into a USB power bank. 
3. **Pair Bluetooth**: 
    * Open the MeshCore app and go to the **Bluetooth** tab.
    * You will see a device advertising as `MeshCore-Bare`. Tap to connect.
    * When prompted for a pairing PIN by your phone's OS, enter the default PIN: **`123456`**.
4. **Start Messaging**: Once connected, the app will instantly sync the Protobuf configuration. Navigate to the **Messages** tab in the app to start sending encrypted texts over the LoRa radio!

---

## Next Steps

To change your LoRa frequencies, Bluetooth PIN, or Public Channel cryptography keys, open `platformio.ini` and modify the `-D` build flags. After making changes, click Upload in PlatformIO to flash the new configuration.

---

## 5. Running the Unit Tests

This project has a native (host-machine) unit test suite that runs without any ESP32/LoRa hardware attached. It exercises the real `NodePrefs`, `DataStore`, and `MyMesh` logic against lightweight mocks of Arduino/filesystem/radio hardware.

**First-time setup:** the native tests reuse MeshCore/Crypto sources already fetched for the main hardware build, so build (or at least fetch deps for) the `meshcore_bare` environment once first:

```
pio run -e meshcore_bare
```

**Run all native tests:**

```
pio test -e native
```

**Run a single test suite** (`test_nodeprefs`, `test_datastore`, or `test_mymesh`):

```
pio test -e native -f test_nodeprefs
```

If `pio` isn't on your PATH, use the full path to the PlatformIO CLI installed by the VS Code extension, e.g. on Windows:

```
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" test -e native
```

All 24 tests (4 NodePrefs + 7 DataStore + 13 MyMesh) currently pass. See `test/mocks/` and `test/native_glue/` for how the hardware-free harness is put together.

<br>

# License
[MIT License](https://github.com/mytechnotalent/meshcore-bare/blob/main/LICENSE)
