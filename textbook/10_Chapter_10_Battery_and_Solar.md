# Chapter 10: Optimizing Battery Life and Solar Charging

A mesh node deployed in the wilderness without access to a wall outlet must survive indefinitely. This chapter explores the hardware and software strategies required to create a self-sustaining, solar-powered MeshCore node.

---

## 10.1 The Energy Budget

Before designing a solar system, you must calculate the node's **Energy Budget**. The ESP32 and SX1262 operate at 3.3V, but draw drastically different amounts of current depending on their state:

*   **Deep Sleep**: ~10 µA (Microamps)
*   **Idle / CPU Active**: ~40 mA (Milliamps)
*   **LoRa Receive (RX)**: ~6 mA (SX1262) + 40 mA (ESP32) = 46 mA
*   **LoRa Transmit (TX) at 17 dBm**: ~90 mA (SX1262) + 40 mA (ESP32) = 130 mA

If a node is placed on top of a mountain to act as a pure relay, it must leave its radio in RX mode 100% of the time to hear incoming packets. 
Therefore, the constant power draw is roughly 46 mA. Over 24 hours, this consumes **1,104 mAh** (Milliamp-hours) of battery capacity. A standard 18650 Lithium-Ion cell contains roughly 3,000 mAh. Without solar charging, the node will die in less than three days.

---

## 10.2 Maximum Power Point Tracking (MPPT)

To keep the battery charged, you attach a solar panel. However, connecting a solar panel directly to a lithium battery is highly inefficient and dangerous.

### 10.2.1 The Problem with Solar Voltage
A "6V" solar panel actually produces up to 7.5V in direct sunlight, and drops to 4V in the shade. A lithium battery operates strictly between 3.2V (dead) and 4.2V (fully charged).

If you use a cheap linear regulator (LDO) to drop the 7.5V solar output to 4.2V for the battery, the excess voltage is burned off as pure heat. You waste over 40% of the sun's energy.

### 10.2.2 The MPPT Solution
Solar panels possess a specific curve of voltage vs. current. There is a singular mathematical point on this curve—the **Maximum Power Point**—where Volts * Amps equals the absolute highest wattage.
Advanced MeshCore nodes utilize an MPPT Charge Controller (like the BQ24210 or CN3791). This IC contains a tiny DC-DC switching buck converter. It actively monitors the solar panel's output and dynamically adjusts the resistance to keep the panel operating at its exact Maximum Power Point, translating the excess voltage into extra current for the battery with 95% efficiency.

---

## 10.3 Software Telemetry: The INA219

To know if a remote node is dying, it must report its battery voltage. However, the ESP32's internal Analog-to-Digital Converter (ADC) is notoriously noisy and non-linear.

### 10.3.1 High-Precision Hardware Monitoring
Instead of using the ESP32's ADC, professional nodes connect an **INA219** or **INA3221** IC via the I2C bus (as discussed in Chapter 5).
The INA219 measures both the Voltage and the Current (via a shunt resistor) with 1% precision.

```cpp
// Pseudocode for Battery Telemetry
float bus_voltage = ina219.getBusVoltage_V();
float current_mA = ina219.getCurrent_mA();
```

### 10.3.2 Deep Sleep Check-ins
If the node is a "Client Node" (not a router), it does not need to leave its radio in RX mode all day. 
It can enter Deep Sleep (drawing 10 µA). The RTC (Real-Time Clock) will wake the ESP32 every 60 minutes.
Upon waking, the C++ code reads the INA219:
1. If `bus_voltage < 3.3V`, the battery is critically low. The node immediately goes back to sleep without transmitting to save itself.
2. If `bus_voltage > 3.6V`, the node transmits a Telemetry packet to the mesh, reporting its health, and goes back to sleep.
