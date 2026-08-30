# MeshCore: A Comprehensive Textbook on C++ Embedded Mesh Networking

This directory contains the definitive, exhaustive textbook on the MeshCore architecture, embedded C++, and RF engineering. Due to the staggering depth and volume of this text, it is divided into massive individual chapters.

## Table of Contents

1. [Chapter 1: The Silicon and the Spectrum - Foundations of ESP32 and LoRa](./textbook/01_Chapter_1_Foundations.md)
   - The ESP32-S3 Architecture (Xtensa LX7, Harvard Architecture, Pipelines)
   - The C++ Memory Model (Text, Data, BSS, Heap, Stack frames, Fragmentation)
   - The Physics of LoRa (Chirp Spread Spectrum, Shannon-Hartley theorem)
   - Link Budgets and the Friis Transmission Equation
   - C++ Pointers, References, and Abstract Polymorphism

2. [Chapter 2: The MeshCore Architecture and Configuration](./textbook/02_Chapter_2_MeshCore_Architecture.md)
   - Preprocessor Macros and `platformio.ini` RF Setup
   - Cryptographic Entropy and Hardware Bootstrapping (`globals.cpp`)
   - Binary Serialization in SPIFFS (`DataStore.cpp`)
   - Flood Routing, Collision Avoidance, and Airtime Budgets (`MyMesh.cpp`)
   - The Bluetooth Bridge (`SerialBLEInterface`)

3. [Chapter 3: Cryptography and Protocol Framing](./textbook/03_Chapter_3_Cryptography_and_Comms.md)
   - Asymmetric Ed25519 Identity and Digital Signatures
   - Symmetric AES-256 Packet Encryption and Pre-Shared Keys
   - LoRa MAC Header Optimization and C++ Bitmasking
   - The Protocol Buffer (Protobuf) Bluetooth RPC Bridge

4. [Chapter 4: The Event-Driven Super-Loop](./textbook/04_Chapter_4_Event_Architecture.md)
   - Super-Loop Architecture and Non-Blocking FSMs
   - Hardware Interrupts and the `IRAM_ATTR` Directive
   - Deferred Interrupt Processing and Observer Patterns

5. [Chapter 5: Advanced Hardware Interfacing](./textbook/05_Chapter_5_Hardware_Interfaces.md)
   - I2C (Inter-Integrated Circuit) and Hardware Addresses
   - UART (Universal Asynchronous Receiver-Transmitter) and FIFOs
   - Power Management: Deep Sleep vs Light Sleep and RTC RAM

6. [Chapter 6: Debugging and Diagnostic Theory](./textbook/06_Chapter_6_Debugging_and_Diagnostics.md)
   - Core Dumps, Registers, and Exception Decoding
   - JTAG Hardware Breakpoints and OpenOCD
   - Detecting Heap Leaks and Memory Fragmentation

7. [Chapter 7: Over-The-Air (OTA) Updates](./textbook/07_Chapter_7_OTA_Updates.md)
   - The ESP32 Flash Partition Table (OTA_0, OTA_1, Bootloader)
   - The A/B Update Mechanism and Automatic Rollbacks
   - MTU Chunking and the Bluetooth OTA State Machine

8. [Chapter 8: Mesh Routing Algorithms in Depth](./textbook/08_Chapter_8_Routing_Algorithms.md)
   - Ad-hoc On-Demand Distance Vector (AODV) vs Managed Flooding
   - The Hidden Node Problem
   - CSMA/CA and Channel Activity Detection (CAD) Jitter

9. [Chapter 9: Porting to New Hardware Platforms](./textbook/09_Chapter_9_Porting_Hardware.md)
   - The Hardware Abstraction Layer (HAL)
   - Conditional Compilation in `target.h`
   - Filesystem Abstractions: SPIFFS vs LittleFS (NRF52 / RP2040)

10. [Chapter 10: Optimizing Battery Life and Solar Charging](./textbook/10_Chapter_10_Battery_and_Solar.md)
    - Calculating the RF Energy Budget
    - Maximum Power Point Tracking (MPPT) for Solar Panels
    - High-Precision Telemetry with the INA219

11. [Chapter 11: The MeshCore API and Serial Automation](./textbook/11_Chapter_11_API_and_Automation.md)
    - Virtual COM Ports and the `MultiSerialInterface`
    - Consistent Overhead Byte Stuffing (COBS)
    - Building Python Gateway Bots over USB

12. [Chapter 12: Environmental Data and Telemetry](./textbook/12_Chapter_12_Environmental_Telemetry.md)
    - The Overhead of Floating-Point Math
    - Fixed-Point Arithmetic and Bit Packing
    - Struct Alignment and BME280 Integration

13. [Chapter 13: Antenna Theory and RF Propagation](./textbook/13_Chapter_13_Antenna_Theory.md)
    - Voltage Standing Wave Ratio (VSWR) and Reflected Power
    - Antenna Gain (dBi vs dBd) and Radiation Patterns
    - Line-of-Sight and the Fresnel Zone

14. [Chapter 14: Store-and-Forward Architectures](./textbook/14_Chapter_14_Store_and_Forward.md)
    - Asynchronous Messaging and Router Nodes
    - The Retrieval Handshake and Beacons
    - Time-To-Live (TTL) and Flash Memory Garbage Collection

15. [Chapter 15: Extending the Protocol (Custom Plugins)](./textbook/15_Chapter_15_Custom_Plugins.md)
    - PortNums and Application Layer Routing
    - Defining Custom Protobuf Schemas (Nanopb)
    - Creating and Registering C++ Event Handlers

16. [Chapter 16: Cryptographic Key Exchanges](./textbook/16_Chapter_16_Cryptographic_Key_Exchange.md)
    - The Key Distribution Problem
    - Elliptic-Curve Diffie-Hellman (ECDH over X25519)
    - Perfect Forward Secrecy (PFS) and Ephemeral Keys

17. [Chapter 17: GPS and Location Encoding](./textbook/17_Chapter_17_GPS_and_Location.md)
    - Parsing NMEA Sentences with `TinyGPS++`
    - Horizontal Dilution of Precision (HDOP)
    - Fixed-Point Scalar Compression of Latitude/Longitude

18. [Chapter 18: Advanced Network Topologies](./textbook/18_Chapter_18_Network_Topologies.md)
    - Star (LoRaWAN) vs Mesh Architectures
    - Yagi-Uda Directional Antennas and RF Backhauls
    - MQTT Gateways (Bridging across the Internet)

19. [Chapter 19: Building Custom Frontends](./textbook/19_Chapter_19_Custom_Frontends.md)
    - The Web Bluetooth API (WebBLE) and JavaScript
    - The Serial Protobuf Handshake for Desktop Apps
    - Compiling Native Mobile Apps (Kotlin and Swift)

20. [Chapter 20: The Future of the Mesh](./textbook/20_Chapter_20_Future_of_the_Mesh.md)
    - The Scaling Limits of Managed Flooding
    - Frequency Agility and TDMA
    - Autonomous Drone Swarms and M2M AI Coordination

*(End of Textbook)*
