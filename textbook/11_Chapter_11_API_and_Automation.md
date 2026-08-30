# Chapter 11: The MeshCore API and Serial Automation

While a smartphone app over Bluetooth is the most common way to interact with a node, it is not the only way. This chapter explores how to bypass the Bluetooth interface entirely and automate the node using Python scripts over a physical USB/Serial connection.

---

## 11.1 The MultiSerialInterface

In `main.cpp`, alongside the `bluetooth_interface`, you will see references to a `MultiSerialInterface` or a generic `StreamAPI`. 

When you plug the ESP32 into a Raspberry Pi via USB, the ESP32 enumerates as a Virtual COM port (e.g., `/dev/ttyUSB0` on Linux). 
The `interface_manager` treats this physical wire exactly the same way it treats the Bluetooth connection. It listens for incoming bytes, parses them as Protobuf payloads, and hands them to `the_mesh.loop()`.

### 11.1.1 The Protobuf Delimiter Problem
Over Bluetooth, every packet is cleanly separated because BLE has distinct "Write" events.
Over a physical Serial wire, the data is just a continuous, endless stream of bytes. If the PC sends a Protobuf packet, how does the ESP32 know where the packet ends?

MeshCore solves this using **COBS (Consistent Overhead Byte Stuffing)** or simple Length-Prefixed framing.
Before the Python script sends the 50-byte Protobuf payload, it sends a 2-byte header indicating the length (`0x00 0x32`). The ESP32 reads the length, allocates a buffer, and waits until exactly 50 bytes have arrived before decoding the Protobuf.

---

## 11.2 Automating with Python

Because Protobuf is language-agnostic, you can compile the `.proto` files into Python classes using the `protoc` compiler. This allows you to write autonomous scripts (Bots) that run on a Raspberry Pi and control the Mesh node.

### 11.2.1 Building a Weather Bot
Imagine you want to broadcast the local weather to the entire mesh network every morning at 8:00 AM.
You write a Python script on a Raspberry Pi connected to the ESP32 via USB:

```python
import serial
from meshcore_pb2 import TextPacket, MeshPacket

# 1. Open the Serial Port to the ESP32
ser = serial.Serial('/dev/ttyUSB0', 115200)

# 2. Fetch data from an Internet API
temperature = get_weather_api() 

# 3. Construct the Protobuf Packet
txt = TextPacket()
txt.text = f"Good morning! It is {temperature} degrees today."

# 4. Wrap it in the Mesh Routing Header
mesh_pkt = MeshPacket()
mesh_pkt.payload = txt.SerializeToString()
mesh_pkt.destination = "PUBLIC_CHANNEL"

# 5. Send over the wire (with length prefix)
data = mesh_pkt.SerializeToString()
ser.write(len(data).to_bytes(2, 'big'))
ser.write(data)
```

### 11.2.2 The Gateway Node
This architecture is how **Gateways** are built. If a node is plugged into a PC with an internet connection, a Python script can listen to the Serial port. When the ESP32 receives a LoRa packet from the woods, it sends the Protobuf payload over the USB wire. The Python script reads it and forwards the message to a Discord server or an MQTT broker. 

By abstracting the command protocol to Protobuf, MeshCore allows infinite extensibility without modifying a single line of the C++ firmware.
