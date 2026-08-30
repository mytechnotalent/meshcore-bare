# Chapter 7: Over-The-Air (OTA) Updates

Deploying a mesh node often involves climbing a tree, mounting it to a roof, or sealing it inside a waterproof enclosure. In these scenarios, plugging in a USB cable to flash a firmware update is physically impossible. This chapter explores how the ESP32 performs Over-The-Air (OTA) updates over a wireless link without risking a "bricked" (unbootable) device.

---

## 7.1 The ESP32 Partition Table

To understand OTA, you must understand how the ESP32's flash memory is divided. In a standard Arduino project, the flash is divided into an App partition (where the code lives) and a SPIFFS partition (where files live).

In an OTA-enabled project, the flash memory is heavily partitioned:
1.  **Bootloader**: The immutable code that runs first and decides which partition to boot.
2.  **Factory Partition**: (Optional) The original firmware flashed at the factory.
3.  **OTA_0 Partition**: The primary app partition.
4.  **OTA_1 Partition**: The secondary app partition.
5.  **OTA Data Partition**: A tiny sector that simply tells the Bootloader which partition (OTA_0 or OTA_1) is the "active" one.
6.  **SPIFFS Partition**: For `DataStore.cpp`.

### 7.1.1 The A/B Update Mechanism
If the node is currently running the code in **OTA_0**:
1. When you push a firmware update over Bluetooth, the ESP32 writes the incoming bytes into the inactive **OTA_1** partition. 
2. It does not overwrite OTA_0! If the connection drops at 99%, the device is completely safe because OTA_0 is untouched.
3. Once 100% of the firmware is downloaded and the cryptographic checksum (MD5/SHA256) is verified, the ESP32 updates the **OTA Data Partition** to point to OTA_1.
4. The ESP32 reboots. The bootloader reads the OTA Data Partition, sees that OTA_1 is the active slot, and boots the new code.

### 7.1.2 Automatic Rollback
What if the new code in OTA_1 compiles perfectly, but contains a bug in `setup()` that causes a catastrophic crash? 
The ESP32 possesses a hardware watchdog timer. If the new firmware crashes repeatedly during boot, the bootloader will detect the panic, automatically mark OTA_1 as corrupted, flip the OTA Data Partition back to OTA_0, and reboot into the old, stable firmware. This guarantees you never have to climb the tree to rescue a bricked node.

---

## 7.2 Implementing Bluetooth OTA in MeshCore

In MeshCore, firmware files (`firmware.bin`) are often over 1 Megabyte in size. Sending 1MB over a low-bandwidth Bluetooth connection requires careful protocol design.

### 7.2.1 Chunking and the MTU
Bluetooth Low Energy (BLE) limits packet sizes to the negotiated MTU (Maximum Transmission Unit). Usually, this is around 256 bytes.
To send a 1MB firmware file, the smartphone app splits the binary into roughly 4,000 "chunks".

### 7.2.2 The OTA State Machine
The ESP32 `interface_manager` implements a dedicated OTA State Machine:
1.  **`OTA_BEGIN`**: The phone sends the total file size and MD5 hash. The ESP32 prepares the inactive partition (erasing the flash sectors takes several milliseconds).
2.  **`OTA_WRITE`**: The phone blasts chunks of data. The ESP32 writes them to flash memory. Flash writes block the CPU, so `the_mesh.loop()` is paused during this write.
3.  **`OTA_END`**: The ESP32 verifies the final hash and commands a reboot.

By understanding the A/B partition layout and the MTU chunking mechanics, you can securely manage a fleet of remote nodes entirely from your smartphone.
