# Chapter 15: Extending the Protocol (Custom Plugins)

The MeshCore architecture is designed to be modular. If you want to add a Geiger counter to your node to measure radiation, you do not need to modify the core `MyMesh.cpp` routing engine. This chapter explains how to use **PortNums** (Packet Handlers) and the Protobuf schema to build custom C++ plugins.

---

## 15.1 Understanding PortNums

In standard internet protocols (TCP/UDP), packets have "Ports" (e.g., Port 80 for HTTP, Port 443 for HTTPS). 
When a packet arrives at your computer, the OS looks at the Port and hands the packet to the correct application (your web browser).

MeshCore uses the exact same concept, called **PortNums**. 
The MAC Header (Chapter 3) contains a specific byte indicating the PortNum:
*   `PortNum = 1`: Text Message
*   `PortNum = 2`: Routing / Node Info
*   `PortNum = 3`: GPS Position
*   `PortNum = 65`: Custom User Plugin

---

## 15.2 Modifying the Protobuf Schema

To send radiation data, you must first define the data structure. You edit the `meshcore.proto` file (which is shared between the ESP32 and the smartphone app):

```protobuf
message GeigerPayload {
    float counts_per_minute = 1;
    float microsieverts = 2;
    uint32 timestamp = 3;
}
```
You run the `protoc` compiler (with the Nanopb plugin for C), which generates `GeigerPayload` C structs that the ESP32 can natively understand.

---

## 15.3 Writing the C++ Plugin

You create a new file, `GeigerPlugin.cpp`. 

### 15.3.1 Registering the Handler
In your plugin's `begin()` function, you subscribe to the specific PortNum (e.g., Port 65) using `MyMesh`'s observer pattern:

```cpp
void GeigerPlugin::begin() {
    // Tell the Mesh core: "If you receive a packet on Port 65, hand it to me."
    the_mesh.registerPacketHandler(65, [](const mesh::Packet& pkt) {
        handleIncomingRadiationData(pkt);
    });
}
```

### 15.3.2 Decoding the Payload
When a packet arrives, your handler decrypts it and unpacks the Protobuf:

```cpp
void handleIncomingRadiationData(const mesh::Packet& pkt) {
    GeigerPayload data;
    
    // Using Nanopb to decode the raw bytes into the C++ struct
    pb_istream_t stream = pb_istream_from_buffer(pkt.payload, pkt.payload_len);
    if (pb_decode(&stream, GeigerPayload_fields, &data)) {
        
        Serial.printf("ALERT: Received radiation reading! CPM: %f\n", data.counts_per_minute);
        
        // If radiation is high, trigger an alarm pin
        if (data.microsieverts > 10.0) {
            digitalWrite(ALARM_PIN, HIGH);
        }
    }
}
```

### 15.3.3 Transmitting from the Plugin
Conversely, if *your* node is the one with the Geiger counter attached, your plugin uses the super-loop to periodically broadcast data:

```cpp
void GeigerPlugin::loop() {
    if (millis() - last_tx > 60000) { // Every 60 seconds
        GeigerPayload data = readSensorHardware();
        
        // Encode the struct into a binary buffer
        uint8_t buffer[64];
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        pb_encode(&stream, GeigerPayload_fields, &data);
        
        // Command the mesh to transmit it on Port 65
        the_mesh.sendPacket(buffer, stream.bytes_written, 65, DEST_BROADCAST);
        
        last_tx = millis();
    }
}
```

By isolating your code into a Plugin tied to a specific PortNum, you ensure that even if your Geiger code contains a bug, it will never break the core routing engine or crash the standard text messaging system.
