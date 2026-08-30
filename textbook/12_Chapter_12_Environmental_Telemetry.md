# Chapter 12: Environmental Data and Telemetry

Mesh networks are ideal for remote telemetry. A node deployed in an agricultural field can monitor soil moisture and temperature, reporting the data back over miles of terrain. This chapter explains how to encode complex floating-point sensor data into dense, LoRa-optimized payloads.

---

## 12.1 The Problem with Floats

A standard BME280 sensor provides three data points:
*   Temperature: `24.53 °C`
*   Humidity: `45.2 %`
*   Pressure: `1013.25 hPa`

In C++, these are typically stored as 32-bit `float` variables. A `float` consumes 4 bytes of memory. 
If we transmit three floats, we consume 12 bytes. If we transmit them as JSON strings (`{"temp": 24.53}`), we consume over 50 bytes. 
In LoRa, every byte increases the Time-on-Air, increasing battery drain and the probability of a packet collision. 

### 12.1.1 Fixed-Point Arithmetic and Bit Packing
To save airtime, MeshCore nodes compress telemetry data by abandoning floating-point math entirely before transmission.

**1. Temperature (Range -40.0 to +80.0 °C)**
Instead of a 4-byte float, we multiply the temperature by 10 (or 100) and cast it to a signed integer.
`24.53 * 100 = 2453`
The number `2453` easily fits into a 16-bit integer (`int16_t`), which only consumes 2 bytes. We have just halved the size of the payload with zero loss of usable precision.

**2. Humidity (Range 0.0 to 100.0 %)**
Humidity never drops below 0 and never exceeds 100. If we only care about 0.5% precision, we can multiply the percentage by 2.
`45.2 * 2 = 90`
The number `90` fits perfectly into an 8-bit unsigned integer (`uint8_t`), consuming a mere 1 byte.

### 12.1.2 The Telemetry Struct
In C++, we define a packed struct to guarantee the compiler doesn't insert empty "padding" bytes between the variables (a common issue on 32-bit processors).

```cpp
#pragma pack(push, 1) // Force 1-byte alignment
struct TelemetryPayload {
    int16_t temperature; // 2 bytes
    uint8_t humidity;    // 1 byte
    uint16_t pressure;   // 2 bytes
};
#pragma pack(pop)
```
This entire struct consumes exactly 5 bytes. 

---

## 12.2 Integrating the BME280

To acquire this data, the node uses the I2C protocol (Chapter 5) to talk to the Bosch BME280 silicon.

### 12.2.1 The Sensor Polling Loop
In `main.cpp`, we saw `sensors.loop()`.
Because reading the BME280 requires the sensor's internal ADC to perform a measurement, it takes a few milliseconds. 

```cpp
void Sensors::loop() {
    if (millis() - last_telemetry_tx > 3600000) { // Every 1 hour
        TelemetryPayload payload;
        
        // Read floats from sensor
        float t = bme.readTemperature();
        float h = bme.readHumidity();
        float p = bme.readPressure() / 100.0F;
        
        // Compress to integers
        payload.temperature = (int16_t)(t * 100);
        payload.humidity = (uint8_t)(h * 2);
        payload.pressure = (uint16_t)p;
        
        // Pass to the Mesh router
        the_mesh.sendTelemetry((uint8_t*)&payload, sizeof(payload));
        
        last_telemetry_tx = millis();
    }
}
```

By heavily compressing the data before passing it to `the_mesh`, the resulting LoRa packet is incredibly short. This allows the node to utilize a higher Spreading Factor (like SF11) to achieve massive range, while still staying well below the legal duty-cycle airtime limits.
