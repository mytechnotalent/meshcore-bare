# Chapter 17: GPS and Location Encoding

A mesh network often acts as a decentralized location tracker. By equipping nodes with GPS receivers, users can track each other across massive, off-grid environments without relying on cellular infrastructure. This chapter explores how to interface with GPS hardware and mathematically compress the coordinates for LoRa transmission.

---

## 17.1 Parsing NMEA Sentences

GPS modules (like the U-blox NEO-6M) communicate with the ESP32 over a UART serial connection (Chapter 5).
The module outputs data in standard ASCII **NMEA (National Marine Electronics Association)** sentences at a rate of 1 Hz (once per second).

An example NMEA sentence (`$GPGGA`) looks like this:
`$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47`

### 17.1.1 The TinyGPS++ Library
Parsing this text string manually in C++ is tedious and error-prone (due to `char` arrays and floating-point conversions). MeshCore nodes typically utilize a parsing library like `TinyGPS++`.

```cpp
// Inside the GPS State Machine
while (Serial1.available() > 0) {
    if (gps.encode(Serial1.read())) {
        if (gps.location.isValid()) {
            float lat = gps.location.lat();
            float lng = gps.location.lng();
            // We have a valid 3D fix!
        }
    }
}
```
The parser consumes bytes one by one. Only when a full, valid sentence with a matching checksum (`*47`) is received does the `gps.location.isValid()` return true.

---

## 17.2 HDOP and Signal Degradation

GPS satellites orbit 12,000 miles above the Earth. The signals are incredibly weak. If a node is placed under dense tree cover or inside a canyon, it will lose line-of-sight to several satellites.

When the GPS module drops from 9 satellites to 4 satellites, the location becomes wildly inaccurate.
MeshCore evaluates the **HDOP (Horizontal Dilution of Precision)** to determine if the location is worth broadcasting.

*   **HDOP < 1.0**: Ideal. The node is perfectly tracked within a few meters.
*   **HDOP > 5.0**: Poor. The node's position could be inaccurate by hundreds of meters.

If the HDOP is too high, the C++ firmware will suppress the transmission of the GPS packet to prevent flooding the mesh with useless, jumping coordinates.

---

## 17.3 Compressing Latitude and Longitude

If a node transmits its raw `float` coordinates (`48.1173`, `-1.8500`), it consumes 8 bytes. However, standard 32-bit floats suffer from precision loss at extreme coordinates, causing a node to "snap" to a grid in the smartphone app.

To maintain sub-meter precision while saving airtime, MeshCore nodes compress the coordinates into 32-bit signed integers (`int32_t`) using a fixed scalar multiplier of **10,000,000 (1e7)**.

```cpp
// Floating point (error prone, 8 bytes)
float lat = 48.1173; 

// Fixed-Point Integer (perfect precision, 4 bytes)
int32_t compressed_lat = (int32_t)(lat * 1e7); // 481173000
```

### 17.3.1 The Position Payload
The final Protobuf payload is packed tightly:

```protobuf
message Position {
    sfixed32 latitude_i = 1;  // 4 bytes
    sfixed32 longitude_i = 2; // 4 bytes
    int32 altitude = 3;       // 4 bytes
    uint32 time = 4;          // 4 bytes
}
```
When this packet routes through the network, relay nodes can extract the integer coordinates, compare them to their own GPS integers, and instantly calculate the physical distance (using the Haversine formula) to decide if they should reply with a localized ping.
