# Chapter 4: The Event-Driven Super-Loop

This chapter examines the beating heart of the MeshCore software architecture: the Super-Loop. Unlike desktop applications running on Linux or Windows, this bare-metal embedded system does not possess a multitasking operating system to manage threads for you. It relies entirely on cooperative, non-blocking C++ architecture.

---

## 4.1 The Concept of the Super-Loop

If you open `main.cpp`, you will see the `loop()` function:

```cpp
void loop() {
    the_mesh.loop();
    interface_manager.loop();
    sensors.loop();
    rtc_clock.tick();
}
```

This loop is executed by the Xtensa core as fast as physically possible—often millions of times per second. 
The absolute golden rule of embedded programming is: **Never Block the Loop**. 

If `sensors.loop()` contains a `delay(1000)` (pausing for 1 second to wait for a temperature reading), the entire CPU freezes for 1000 milliseconds. During that second:
1. The radio receives a packet, but `the_mesh.loop()` is frozen and cannot read it, causing the hardware buffer to overflow and drop the packet.
2. The smartphone sends a Bluetooth command, but `interface_manager` is frozen and the connection times out.

### 4.1.1 State Machines
To avoid blocking, every component is written as a Finite State Machine (FSM). 
Instead of waiting for a sensor, a state machine checks the time:
```cpp
// Correct non-blocking architecture
void Sensors::loop() {
    uint32_t current_time = millis();
    if (current_time - last_check > 1000) {
        last_check = current_time;
        startTemperatureRead(); // Tells hardware to start, returns instantly
    }
}
```

---

## 4.2 Hardware Interrupts (ISRs)

While the super-loop handles routine polling, some events cannot wait. If a LoRa packet arrives, the SX1262 radio chip physically raises the voltage on one of the ESP32's digital pins (DIO_1) to trigger a **Hardware Interrupt**.

### 4.2.1 The Interrupt Service Routine (ISR)
An ISR is a C++ function that the CPU executes *immediately*, halting whatever it was currently doing in the super-loop.

```cpp
// Inside the Radio driver
void IRAM_ATTR radio_interrupt_handler() {
    packet_ready_flag = true;
}
```
**`IRAM_ATTR`**: Notice this compiler directive. It forces the compiler to store this specific function in IRAM (Internal RAM) instead of flash memory. If the function was in flash, the CPU would have to access the SPI bus to read the instructions, which takes too long for an interrupt and causes crashes.

The ISR must be incredibly fast. It simply sets a boolean flag (`packet_ready_flag = true;`) and exits. 

### 4.2.2 Tying Interrupts to the Loop
Back in the super-loop, `the_mesh.loop()` is rapidly checking that flag:
```cpp
void MyMesh::loop() {
    if (packet_ready_flag) {
        packet_ready_flag = false;
        pullPacketFromRadio();
    }
}
```
This pattern—using an interrupt to set a flag, and the super-loop to process the heavy lifting—is known as **Deferred Interrupt Processing**. It ensures the CPU spends minimal time inside the high-priority ISR, keeping the system stable.

---

## 4.3 The Observer Pattern

How does the Bluetooth interface know when a radio packet arrives? They don't have direct references to each other. MeshCore uses the **Observer Pattern**.

`MyMesh` provides callback hooks:
```cpp
the_mesh.setPacketReceivedCallback([](const mesh::Packet& p) {
    interface_manager.sendToBluetooth(p);
});
```
When `the_mesh` decrypts a packet, it iterates through a list of function pointers (callbacks) and executes them. This allows completely decoupled C++ components to react to events asynchronously, maintaining a pristine, modular codebase.
