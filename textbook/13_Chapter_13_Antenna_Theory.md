# Chapter 13: Antenna Theory and RF Propagation

No matter how optimized your C++ codebase is, a LoRa node is fundamentally useless if its physical antenna is poorly tuned. The physical layer (PHY) dictates the ultimate range of the network. This chapter introduces the physics of RF antennas and how they dictate the performance of your MeshCore node.

---

## 13.1 Voltage Standing Wave Ratio (VSWR)

When the SX1262 LoRa chip transmits a packet at 17 dBm, it sends an electrical current down the copper trace to the antenna.

If the antenna is perfectly tuned to the exact frequency (e.g., 915 MHz in North America), 100% of that electrical energy is converted into electromagnetic waves and radiated into the air.

However, if the antenna is tuned to 868 MHz (Europe) but the code is transmitting at 915 MHz, an **Impedance Mismatch** occurs. 
When the electrical current hits the mismatched antenna, some of it "bounces" back down the wire and hits the radio chip. This is known as **Reflected Power**.

### 13.1.1 Measuring Efficiency
The ratio of Forward Power to Reflected Power is the **VSWR (Voltage Standing Wave Ratio)**.
*   **VSWR 1.0**: Perfect. 0% power is reflected.
*   **VSWR 2.0**: Acceptable. ~11% of power is reflected and lost as heat.
*   **VSWR 5.0+**: Catastrophic. Over 40% of the power bounces back. This will generate massive heat inside the SX1262 and can physically destroy the silicon amplifier.

Never power on a MeshCore node without an antenna attached. Doing so creates an infinite VSWR (an open circuit), reflecting 100% of the 17 dBm power back into the chip, instantly burning it out.

---

## 13.2 Antenna Gain (dBi vs dBd)

Antennas do not "amplify" power. An antenna cannot create energy out of nothing. When you buy a "high-gain" antenna, you are buying an antenna that *focuses* the energy.

Imagine a lightbulb floating in space. It radiates light equally in a perfect sphere. This is an **Isotropic Radiator**.
*   **dBi (Decibels relative to Isotropic)** measures how much an antenna focuses energy compared to that perfect sphere.
*   A standard rubber-duck dipole antenna has a gain of **2.15 dBi**. It takes the perfect sphere and squishes it down slightly like a donut, pushing more RF energy out to the horizon and less straight up into the sky.
*   A fiberglass collinear antenna might have an **8 dBi** gain. It squishes the donut into a flat pancake. 

### 13.2.1 The Tradeoff
If you mount an 8 dBi antenna on a mountain peak, the "pancake" of RF energy will shoot straight out to the horizon, completely passing over the heads of the nodes in the valley directly below the mountain. 
For high-elevation router nodes, a lower-gain antenna (3 dBi) is often vastly superior because it radiates energy downwards as well as outwards.

---

## 13.3 The Fresnel Zone

Radio waves do not travel in a perfectly straight, laser-like line. They travel in an elliptical, football-shaped envelope known as the **Fresnel Zone**.

If Node A and Node B have perfect Line-of-Sight (you can literally see Node B with binoculars), but they are sitting on the ground, the connection will fail.
Why? Because the bottom half of the "football" (the Fresnel Zone) is striking the earth. When radio waves hit the ground, they reflect back up and collide with the primary signal, causing destructive interference.

To achieve maximum LoRa range (10+ miles), the antenna must be elevated high enough that at least 60% of the First Fresnel Zone is clear of obstacles (trees, buildings, and the curvature of the Earth). 
This physical reality is why software optimizations can never compensate for poor physical deployment.
