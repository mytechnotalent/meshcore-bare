# Chapter 18: Advanced Network Topologies

While MeshCore defaults to a decentralized Managed Flood, real-world deployments often require structured topologies to bridge vast geographical distances or interface with the traditional Internet. This chapter explores how to architect hybrid networks using Gateways and directional antennas.

---

## 18.1 Star vs. Mesh Topologies

### 18.1.1 The Star Topology (LoRaWAN)
In a traditional LoRaWAN network (like Helium or The Things Network), the architecture is a **Star**.
Thousands of "Client Nodes" (sensors) transmit packets. They cannot hear each other. They only transmit to a massive, centralized "Gateway" mounted on a tower. 
*   **Pros**: Client nodes can sleep 99.9% of the time, achieving 10-year battery life.
*   **Cons**: Single point of failure. If the Gateway tower loses power, the entire network goes dark. Nodes cannot talk directly to each other.

### 18.1.2 The Mesh Topology (MeshCore)
In MeshCore, every node is a router. 
*   **Pros**: Infinitely resilient. If a node dies, traffic seamlessly routes through a different neighbor. Nodes can communicate off-grid in a deep valley with zero infrastructure.
*   **Cons**: Nodes must stay awake to listen for incoming traffic, drawing 46 mA continuously and requiring solar panels or large batteries (as discussed in Chapter 10).

---

## 18.2 The Hybrid Architecture (Backhauls)

To get the best of both worlds, engineers design Hybrid architectures using a mix of Omni-directional and Directional antennas.

### 18.2.1 The Yagi-Uda Antenna
Imagine two distinct mesh networks: One in City A, and one in City B, separated by 30 miles of wilderness. Standard Omni-directional antennas (which radiate in a circle) cannot bridge this gap.

To connect the cities, you install a **Yagi** directional antenna on a tall building in City A, and point it directly at a matching Yagi in City B. A Yagi antenna has immense gain (e.g., 14 dBi), but all of its RF energy is focused into a tight, 30-degree laser beam.

### 18.2.2 The Dedicated Backhaul Node
You configure the ESP32 connected to the Yagi as a **Dedicated Router**. 
1. It is configured to operate on a different LoRa channel (e.g., Channel 2) to prevent its massive transmissions from drowning out the local mesh (Channel 1).
2. It uses a hardware UART link to connect to a local node. 
3. When the local node hears a packet destined for City B, it passes the bytes over the wire to the Backhaul Node.
4. The Backhaul Node fires the packet across the 30-mile physical RF laser beam to City B.

---

## 18.3 MQTT Gateways (The Internet Bridge)

What if City A and City B are separated by a mountain range, making an RF backhaul physically impossible?
You bridge the networks using the Internet via **MQTT (Message Queuing Telemetry Transport)**.

### 18.3.1 The Wi-Fi Gateway
You configure a specific MeshCore node in City A to connect to a local Wi-Fi router. 
In `main.cpp`, you initialize the `WiFiClient` and an MQTT library.

1. When the Gateway Node hears a LoRa packet from the local mesh, it wraps the encrypted binary blob in a JSON wrapper: `{"node": "1234", "payload": "0x4F2A..."}`
2. It publishes this JSON to an MQTT Broker (a server on the Internet) via Wi-Fi.
3. The Gateway Node in City B is subscribed to the exact same MQTT topic.
4. The City B Gateway downloads the JSON, unwraps the binary blob, and transmits it via its LoRa radio.

To the local nodes in City A and City B, it appears as though they are sitting right next to each other, completely unaware that their packets were securely tunnelled through the global Internet.
