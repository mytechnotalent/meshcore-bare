# Chapter 2: The MeshCore Architecture and Configuration

In Chapter 1, we established the physical and theoretical foundations of the ESP32-S3 and LoRa modulation. In this chapter, we will dissect the MeshCore-Bare codebase line-by-line, exploring exactly how the software configures the hardware, manages non-volatile data, and executes the flood routing protocol.

---

## 2.1 The Preprocessor Configuration (`platformio.ini`)

Before the C++ compiler touches a single line of your code, the PlatformIO build system reads `platformio.ini` and passes crucial configuration macros (using `-D`) to the compiler. This is how we statically configure the node for your specific deployment without wasting RAM on dynamic configuration variables.

### 2.1.1 Radio Power and Antenna Switching
```ini
-D LORA_TX_POWER=17
-D SX126X_CURRENT_LIMIT=140
-D SX126X_DIO2_AS_RF_SWITCH=1
```
*   **`LORA_TX_POWER=17`**: This sets the output power of the SX1262 amplifier to 17 dBm (decibels relative to a milliwatt). 17 dBm is approximately 50 milliwatts. While the SX1262 can push up to 22 dBm (158 mW), higher transmit power rapidly drains battery and can violate local ISM band regulations. 17 dBm offers an optimal balance of range and efficiency.
*   **`SX126X_CURRENT_LIMIT=140`**: The SX1262 has an internal DC-DC converter. By setting a hard current limit of 140 mA, we prevent the radio from pulling too much current and browning-out the ESP32 (which can happen if the antenna is disconnected, causing massive impedance mismatch).
*   **`SX126X_DIO2_AS_RF_SWITCH=1`**: The SX1262 has two antenna paths (one for transmitting, one for receiving). Instead of using external logic gates to switch between them, this macro tells the radio chip to use its internal `DIO2` pin to automatically throw the RF switch when it moves from RX to TX mode.

### 2.1.2 Cryptography and Bluetooth Identity
```ini
-D BLE_CUSTOM_NAME="\"meshcore-bare\""
-D BLE_CUSTOM_PIN=123456
-D PUBLIC_CHANNEL_NAME="\"Public\""
-D PUBLIC_CHANNEL_PSK="\"izOH6cXN6mrJ5e26oRXNcg==\""
```
*   **`BLE_CUSTOM_NAME`**: The C++ compiler requires string literals to be wrapped in quotes. Because PlatformIO passes this as a command-line argument, we must escape the quotes (`"\"name\""`). This sets the GATT server name broadcasted to your phone.
*   **`PUBLIC_CHANNEL_PSK`**: This is a Base64 encoded Pre-Shared Key (PSK). MeshCore utilizes symmetric AES-256 encryption. For a node to decrypt a packet on the "Public" channel, it must possess this exact 32-byte key. If the PSK does not match, the packet is treated as cryptographic noise and discarded by `MyMesh.cpp`.

---

## 2.2 Bootstrapping the Node (`globals.cpp` & `main.cpp`)

When the ESP32 receives power, `main.cpp` executes the `setup()` function to orchestrate the hardware initialization.

### 2.2.1 Radio Initialization
In `globals.cpp`, we define:
```cpp
bool radio_init() {
#if defined(P_LORA_SCLK)
    spi.begin(P_LORA_SCLK, P_LORA_MISO, P_LORA_MOSI, P_LORA_NSS);
    return radio.std_init(&spi);
#else
    return radio.std_init();
#endif
}
```
This function maps the SPI bus to the specific physical pins defined in `target.h`. `radio.std_init()` sends a sequence of initialization commands over the MOSI (Master Out Slave In) wire to the SX1262's internal configuration registers, setting the base frequency, bandwidth, and spreading factor defined in our macros.

### 2.2.2 Entropy and Cryptographic Identity
For a mesh node to securely communicate, it needs a cryptographically secure public/private keypair. 
```cpp
mesh::LocalIdentity radio_new_identity() {
    RadioNoiseListener rng(radio);
    return mesh::LocalIdentity(&rng);
}
```
Cryptographic keys require high-quality random numbers (entropy). If an attacker can guess the random number generator's seed, they can recreate your private key. 
Microcontrollers struggle to generate true randomness because they are deterministic machines. However, the `RadioNoiseListener` solves this by turning on the SX1262 radio and reading the analog thermal noise (static) from the surrounding electromagnetic spectrum. This static is digitized and used to seed the `fast_rng` (Random Number Generator), guaranteeing a perfectly unique, mathematically un-guessable identity for your node.

---

## 2.3 Non-Volatile Storage (`DataStore.cpp`)

To remember the contacts and channels you configure from your smartphone, the node utilizes SPIFFS to write serialized C++ structs to the flash drive.

### 2.3.1 Binary Serialization
Look closely at how `DataStore.cpp` saves the `NodePrefs` struct:
```cpp
bool DataStore::savePrefs(NodePrefs &_prefs) {
    File file = openWrite(_fs, "/prefs.json");
    if (file) {
        bool success = _prefs.saveSerial(file);
        file.close();
        return success;
    }
    return false;
}
```
The `saveSerial()` function takes the `_prefs` struct (which contains variables like `float airtime_factor`, `uint32_t freq`, `uint8_t sf`) and writes them out byte-for-byte. 
Unlike JSON or XML which convert the number `100` into three characters (`'1'`, `'0'`, `'0'`), binary serialization writes the literal 32-bit integer `0x00000064` to the flash. This means the 80+ settings in `NodePrefs` consume less than 150 bytes of flash memory, and reading them back takes mere microseconds because no string parsing is required.

### 2.3.2 Contact Management
In `DataStore::saveContacts()`, we iterate through the known `ContactInfo` structs:
```cpp
bool success = (file.write(c.id.pub_key, 32) == 32);
success = success && (file.write((uint8_t *)&c.name, 32) == 32);
success = success && (file.write(&c.type, 1) == 1);
```
Every contact consists of a 32-byte Ed25519 public key, a 32-byte name string, and various metadata bytes (like `last_advert_timestamp`). If the node loses power, the `loadContacts()` function reads these exact bytes back into the `ContactInfo` structures in RAM, restoring your network topology instantly.

---

## 2.4 The Routing Engine (`MyMesh.cpp`)

`MyMesh.cpp` is the crown jewel of the codebase. It orchestrates the flow of packets between the radio antenna and the Bluetooth interface.

### 2.4.1 The Mesh Constructor
```cpp
MyMesh the_mesh(radio_driver, fast_rng, rtc_clock, tables, store, NULL);
```
The `MyMesh` object holds references to everything. It needs the `radio_driver` to transmit, the `fast_rng` to generate packet IDs, the `rtc_clock` to timestamp messages, the `tables` to track routing, and the `store` to save contacts. 

### 2.4.2 Flood Routing and Deduplication
When the radio receives a packet, it triggers an interrupt. `the_mesh.loop()` eventually processes it. Because MeshCore uses a **Flood Routing** topology, every node blindly rebroadcasts packets it hears. 

To prevent a single message from bouncing around the network infinitely (a broadcast storm), `MyMesh` tracks the unique signature of every recently received packet in its routing tables.
If a packet arrives, `MyMesh` checks the cache:
- **If the packet is known**: It is silently dropped.
- **If the packet is new**: The node decrypts the outer header. If the Hop Limit hasn't been reached, the node decrements the Hop Limit, recalculates the header checksum, and schedules it for retransmission.

### 2.4.3 Anti-Collision: The Retransmit Delay
If 5 nodes hear a packet simultaneously, and all 5 immediately rebroadcast it, their radio waves will collide in mid-air, destroying the packets (constructive/destructive interference). 
To prevent this, `MyMesh` overrides the base class delay logic:
```cpp
int MyMesh::calcRxDelay(float score, uint32_t air_time) const {
    // ... complex delay logic
}
```
When a node decides to relay a packet, it calculates a randomized backoff timer (e.g., 50ms to 2000ms). The node with the strongest signal (best SNR/RSSI) typically calculates a shorter delay, allowing it to transmit first. When the other 4 nodes hear that transmission, they cancel their own queued retransmissions, drastically reducing network congestion.

### 2.4.4 The Airtime Budget
LoRa is severely constrained by duty-cycle regulations. If you transmit continuously, you violate federal laws (FCC/ETSI) and drain your battery.
```cpp
float MyMesh::getAirtimeBudgetFactor() const {
    return _prefs.airtime_factor; // Derived from NodePrefs
}
```
`MyMesh` tracks exactly how many milliseconds it spends transmitting. If the node approaches its legal airtime budget, `MyMesh` begins dynamically throttling. It will drop low-priority relay packets (like environmental telemetry) to save airtime for high-priority traffic (like direct text messages or SOS beacons).

---

## 2.5 The Bluetooth Bridge

The ultimate goal of this bare node is to serve as an invisible bridge between your smartphone and the LoRa network.

In `main.cpp`, we initialize the BLE server:
```cpp
bluetooth_interface.begin(BLE_CUSTOM_NAME, emptyName, BLE_CUSTOM_PIN);
interface_manager.addInterface(InterfaceType::Bluetooth, &bluetooth_interface);
the_mesh.startInterface(interface_manager);
```

When you hit "Send" on your phone, the MeshCore app sends a Protobuf-encoded payload to a specific BLE Characteristic.
1. `SerialBLEInterface` catches the BLE write event and moves the bytes into a buffer.
2. `interface_manager` polls the buffer and hands the bytes to `the_mesh`.
3. `the_mesh` passes it to `handleCmdFrame()`.
4. `handleCmdFrame()` identifies the payload as a direct message, fetches the recipient's public key from the `DataStore`, encrypts the message, packages it into a LoRa frame, and commands the `RADIO_CLASS` to transmit it.

This complex, multi-layered architecture is what allows your node to operate autonomously, routing traffic for others while seamlessly serving as a high-speed gateway for your personal device.
