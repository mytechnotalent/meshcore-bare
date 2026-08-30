# Chapter 5: Advanced Hardware Interfacing

While MeshCore-Bare is a headless node, a true mesh ecosystem relies on physical telemetry. This chapter explains the C++ and hardware theory behind integrating external peripherals like GPS, OLED screens, and environmental sensors into the node.

---

## 5.1 Serial Communication Protocols

The ESP32 communicates with the outside world using three primary hardware protocols. We've already covered SPI (Serial Peripheral Interface) in Chapter 1, which we use for the high-speed LoRa radio. The other two are I2C and UART.

### 5.1.1 I2C (Inter-Integrated Circuit)
I2C is designed for short-distance communication on a single circuit board. It uses only two wires:
1.  **SDA (Serial Data)**
2.  **SCL (Serial Clock)**

Unlike SPI which requires a dedicated Chip Select (NSS) wire for every device, I2C uses **Hardware Addresses**. You can connect 100 sensors to the exact same two wires.
When the ESP32 wants to talk to a BME280 temperature sensor, it broadcasts the sensor's hexadecimal address (e.g., `0x76`) over the SDA line. All other sensors ignore the message.

```cpp
// Example I2C Initialization
Wire.begin(SDA_PIN, SCL_PIN);
Wire.beginTransmission(0x76);
Wire.write(0xF4); // Command register
Wire.endTransmission();
```
I2C is slow (typically 100 kHz or 400 kHz), but saves massive amounts of physical pins on the microcontroller. It is primarily used for OLED displays and environmental sensors.

### 5.1.2 UART (Universal Asynchronous Receiver-Transmitter)
UART is used for GPS modules. It uses two wires, but they are cross-connected:
*   ESP32 **TX** connects to GPS **RX**.
*   ESP32 **RX** connects to GPS **TX**.

UART is asynchronous, meaning there is no Clock wire (SCL) to keep the devices in sync. Instead, both devices must strictly agree on a **Baud Rate** (e.g., 9600 bits per second) beforehand.
Because GPS modules constantly stream massive amounts of NMEA ASCII text data (`$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47`), UART requires hardware FIFOs (First-In, First-Out buffers). 

If `sensors.loop()` does not read the `Serial1` buffer fast enough, the buffer overflows, and GPS coordinates are silently corrupted.

---

## 5.2 Power Management and Deep Sleep

A solar-powered mesh node might need to survive for weeks on a tiny lithium-ion battery. The ESP32 draws roughly 40-50 mA while idling, which will kill a 2000 mAh battery in two days.

### 5.2.1 Light Sleep vs Deep Sleep
The ESP32 offers two power-saving modes:

1.  **Light Sleep**: The CPU pauses, but the RAM remains powered. When an interrupt wakes the CPU, execution resumes on the exact line of code it left off on. Current draw: ~1-2 mA.
2.  **Deep Sleep**: The CPU, RAM, and all digital peripherals are completely powered down. Only the ULP (Ultra-Low Power) coprocessor and the RTC (Real-Time Clock) memory remain active. Current draw: ~10 µA (Microamps).

### 5.2.2 Architecting for Deep Sleep
When waking up from Deep Sleep, the ESP32 performs a full hardware reset. The code starts entirely from `setup()`. All variables in standard RAM are wiped.
To preserve state (like the routing tables) during Deep Sleep, variables must be declared in **RTC RAM**:

```cpp
// This variable survives Deep Sleep
RTC_DATA_ATTR int packet_count = 0; 
```

A battery-optimized Mesh node will use the SX1262's internal timer to wake the ESP32 via a physical interrupt pin only when a LoRa packet actually hits the antenna. The ESP32 wakes up (booting in milliseconds), processes the packet, and immediately goes back to Deep Sleep.
