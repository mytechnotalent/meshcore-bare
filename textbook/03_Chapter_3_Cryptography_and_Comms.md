# Chapter 3: Cryptography and Protocol Framing

In Chapters 1 and 2, we covered the hardware, RF physics, and the core routing architecture of MeshCore. This chapter dives into the mathematics and computer science behind how the network secures its traffic and how the Bluetooth interface communicates with a smartphone app.

---

## 3.1 The Mesh Cryptographic Model

In a decentralized mesh network, there is no central server to authenticate users. If Node A receives a packet from Node B, how does it know Node B is actually Node B and not a malicious actor? Furthermore, since every node in the mesh acts as a relay, how do we prevent the relays from reading the contents of the messages?

MeshCore solves this using a hybrid cryptographic architecture involving both Symmetric and Asymmetric cryptography.

### 3.1.1 Asymmetric Cryptography (Ed25519)
When your node boots up for the first time and gathers analog entropy (as discussed in Chapter 2), it generates an **Ed25519 Keypair**.
*   **The Private Key**: A mathematically random 32-byte number. This never leaves the ESP32.
*   **The Public Key**: A 32-byte number mathematically derived from the private key. This is your "Node ID".

Ed25519 is an elliptic curve signature scheme. When your node transmits an advertisement packet (saying "Hi, I am Node 1234"), it hashes the packet contents and uses the Private Key to generate a digital signature. 
When receiving nodes hear this packet, they take the attached Public Key and the signature and perform a mathematical verification. If even a single bit of the packet was altered in transit (or if someone spoofed the Public Key without owning the Private Key), the signature verification fails, and the packet is immediately dropped by `MyMesh.cpp`.

### 3.1.2 Symmetric Cryptography (AES-256)
While asymmetric cryptography is perfect for proving identity, it is computationally expensive and slow to use for encrypting bulk data (like text messages).
For this, MeshCore uses **AES-256 (Advanced Encryption Standard)** in CTR (Counter) mode.

#### Channel PSKs
In `platformio.ini`, you saw the macro `-D PUBLIC_CHANNEL_PSK="\"izOH6cXN6mrJ5e26oRXNcg==\""`.
This is a Pre-Shared Key (PSK). Every node on the "Public" channel possesses this exact same AES-256 key.

When `MyMesh` wants to transmit a packet to the channel:
1.  It takes the raw text payload.
2.  It generates a cryptographic Nonce (a Number Used Once, usually derived from the packet ID).
3.  It encrypts the payload using the AES-256 PSK and the Nonce.
4.  It attaches the unencrypted header (Source ID, Destination ID, Hop Limit) so that relays can route the packet.

When a relay node receives the packet, it looks at the outer header. It does not possess the AES-256 key for your private channel, so the payload looks like pure cryptographic noise. The relay blindly retransmits the noise. Only a node possessing the correct PSK can decrypt the payload.

---

## 3.2 The Packet Framing Protocol

To send data over LoRa, we cannot just send a string of text. The SX1262 expects a highly structured array of raw bytes. This structure is known as the **Frame**.

### 3.2.1 The LoRa MAC Header
Every packet broadcast into the air begins with a MAC (Media Access Control) header. In MeshCore, this header is heavily optimized to save precious Airtime.

A typical MeshCore packet header contains:
1.  **Packet ID (4 bytes)**: A unique identifier for deduplication (preventing broadcast storms).
2.  **Destination ID (4 bytes)**: A truncated hash of the recipient's public key (or a channel ID).
3.  **Source ID (4 bytes)**: A truncated hash of the sender's public key.
4.  **Flags (1 byte)**: Bitmasks indicating packet type (Advert, Text, Telemetry), Hop Limit, and Hop Count.

```cpp
// Example C++ Bitmasking for Flags
uint8_t flags = packet[12];
uint8_t hop_limit = flags & 0b00000111; // Extracts the bottom 3 bits
bool is_advert = (flags & 0b10000000) != 0; // Extracts the top bit
```
Because C++ allows direct bit-wise operations (`&`, `|`, `<<`, `>>`), the MeshCore protocol packs multiple pieces of metadata into single bytes, drastically reducing the size of the packet compared to a desktop protocol like HTTP.

---

## 3.3 The Protocol Buffer (Protobuf) Bridge

While the LoRa air-interface uses a custom, hyper-optimized binary frame, the Bluetooth link between the ESP32 and the smartphone app utilizes **Protocol Buffers (Protobuf)**.

### 3.3.1 What is Protobuf?
Protocol Buffers are a language-neutral, platform-neutral data serialization format created by Google. 
Instead of writing raw bytes to the Bluetooth port, you define a `.proto` file:
```protobuf
message TextPacket {
    string sender = 1;
    string text = 2;
    uint32 timestamp = 3;
}
```
A compiler then generates C++ code for the ESP32, and Swift/Kotlin code for the smartphone app.

### 3.3.2 Why Protobuf for BLE?
Bluetooth Low Energy (BLE) limits the MTU (Maximum Transmission Unit) size, often to just 20 or 256 bytes per packet.
If we sent JSON data (`{"sender": "Kevin", "text": "Hello"}`), we would waste dozens of bytes on brackets, quotes, and keys. 

Protobuf compiles that message down into a dense binary structure (e.g., `0A 05 4B 65 76 69 6E 12 05 48 65 6C 6C 6F 18 92 C3 00`), saving immense amounts of bandwidth over the Bluetooth link.

### 3.3.3 The `SerialBLEInterface` Implementation
When the smartphone app connects to the ESP32, it subscribes to two GATT (Generic Attribute Profile) characteristics: `TX` (Transmit) and `RX` (Receive).

In `main.cpp`, we initialize the interface. When the phone writes a Protobuf payload to the `RX` characteristic:
1.  The ESP32 triggers a hardware interrupt.
2.  `SerialBLEInterface` reads the raw bytes from the Bluetooth stack.
3.  It uses the generated Protobuf C++ classes to decode the payload: `pb_decode(&stream, TextPacket_fields, &my_packet);`
4.  If decoding succeeds, the data is pushed to `MyMesh::handleCmdFrame()`.
5.  `MyMesh` then translates the Protobuf command into a dense LoRa binary frame and blasts it into the air.

This seamless translation between Google's high-level Protobuf RPC architecture and MeshCore's low-level RF binary framing is what enables complex smartphone apps to interact with a bare-metal microcontroller over a decentralized radio network.

---
*End of Chapter 3. Proceed to Chapter 4 for an exploration of C++ Event-Driven Architecture and writing custom hardware plugins.*
