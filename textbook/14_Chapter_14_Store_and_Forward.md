# Chapter 14: Store-and-Forward Architectures

Standard Managed Flooding (discussed in Chapter 8) requires the sender and receiver to be online at the exact same moment. If Node B's battery dies, and Node A sends a text message, the message bounces around the mesh, fails to find Node B, and vanishes forever. This chapter explains how MeshCore solves this via the Store-and-Forward protocol.

---

## 14.1 The Role of the Router Node

To implement Store-and-Forward, the network designates specific, always-on nodes (usually solar-powered nodes mounted on roofs) as **Routers** (or Servers).

When Node A wants to send a message to Node B, but Node A suspects Node B might be sleeping or offline, Node A flags the packet header: `WANT_STORE_AND_FORWARD = 1`.

### 14.1.1 Capturing the Payload
When the Router Node receives this packet:
1. It looks at the destination ID (Node B). 
2. It does *not* know Node B's private AES-256 key, so it cannot read the message.
3. Instead, the Router takes the encrypted binary blob, generates a unique File ID, and saves it to its internal flash memory via SPIFFS (e.g., `/saf/msg_1234`).
4. The Router then sends an acknowledgment (ACK) back to Node A: *"I have safely stored this message. You can go to sleep."*

---

## 14.2 The Retrieval Handshake

Twelve hours later, Node B replaces its battery and boots up. 
How does it get its messages?

1.  **The Beacon**: When Node B boots, it broadcasts an advertisement packet to the mesh, updating its `last_seen` timestamp in everyone's routing table.
2.  **The Notification**: The Router Node hears this beacon. It checks its flash memory and sees it is holding 3 encrypted blobs destined for Node B. The Router sends a notification packet to Node B: *"I have 3 stored messages for you."*
3.  **The Pull Request**: Node B receives the notification. When its user opens the smartphone app, the app tells the ESP32 to send a "Pull" command to the Router.
4.  **The Download**: The Router opens the `/saf/msg_1234` file from SPIFFS, chunks the encrypted blob into standard LoRa frames, and transmits them directly to Node B.
5.  Node B decrypts the chunks, reassembles the Protobuf payload, and pushes the text message to the smartphone.

---

## 14.3 Time-To-Live (TTL) and Garbage Collection

Flash memory is finite. A Router Node sitting on a busy mountaintop could receive hundreds of Store-and-Forward requests per day. If nodes never wake up to retrieve their messages, the Router's SPIFFS partition will eventually fill up (100% capacity), causing the node to crash when it attempts to save the next `NodePrefs` update.

### 14.3.1 Timestamping and Expiration
To prevent this, every stored message is tagged with an absolute UTC timestamp (derived from the Router's RTC Clock, which is synced via GPS or smartphone).

The Router runs a background State Machine in its super-loop (a **Garbage Collector**):
```cpp
void StoreAndForward::loop() {
    if (millis() - last_cleanup_time > 3600000) { // Once an hour
        last_cleanup_time = millis();
        
        // Scan the /saf/ directory
        for (auto& file : getStoredMessages()) {
            if (current_utc_time - file.timestamp > (86400 * 3)) { // 3 days
                // Message expired. Delete to free flash memory.
                _fs.remove(file.filepath);
            }
        }
    }
}
```

This asynchronous garbage collection guarantees the Router remains stable and never exceeds its storage budget, even in a hostile RF environment with failing client nodes.
