# Chapter 6: Debugging and Diagnostic Theory

Writing embedded C++ is drastically different than writing Python or Java. When a Python script crashes, you get a clean stack trace. When an ESP32 crashes, the processor literally halts, the memory is dumped via UART, and the chip forcibly reboots. This chapter covers the rigorous methodologies required to debug bare-metal systems.

---

## 6.1 The Core Dump and Exception Decoding

If you allocate too much memory on the stack (a Stack Overflow), or attempt to read from a Null Pointer (e.g., `int* p = nullptr; int x = *p;`), the Xtensa LX7 processor triggers a **Hardware Exception**.

Over the Serial monitor, you will see a catastrophic crash log:
```text
Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.
Core  1 register dump:
PC      : 0x400d1f7c  PS      : 0x00060030  A0      : 0x800d2380  A1      : 0x3ffb1f90  
A2      : 0x00000000  A3      : 0x00000001  A4      : 0x00000008  A5      : 0x00000000  
```

### 6.1.1 Translating the Dump
This is a raw hexadecimal dump of the CPU's internal registers. 
- **PC (Program Counter)**: Indicates the exact memory address of the instruction that caused the crash (`0x400d1f7c`).
- **LoadProhibited**: Indicates the code tried to read memory it didn't have access to (usually dereferencing a null or invalid pointer).

To translate `0x400d1f7c` back into human-readable C++ code, we use a tool called an **Exception Decoder** (like `xtensa-esp32-elf-addr2line`). It parses the `.elf` (Executable and Linkable Format) file generated during compilation. The `.elf` file contains a mapping of every memory address to the exact line of code in your `.cpp` files. 
The decoder will translate `0x400d1f7c` to something like: `MyMesh::handleCmdFrame() at src/MyMesh.cpp:204`.

---

## 6.2 Hardware Debugging (JTAG/SWD)

While `Serial.println("Got here");` is the most common debugging technique, it changes the timing of the super-loop, often masking timing-related bugs (known as Heisenbugs).

For true debugging, engineers use **JTAG (Joint Test Action Group)**.
By connecting a hardware debugger (like an ESP-Prog) to specific pins on the ESP32 (TDI, TDO, TCK, TMS), we take direct physical control of the Xtensa CPU cores.

### 6.2.1 Hardware Breakpoints
Using a debugger (via OpenOCD and GDB), we can set a **Hardware Breakpoint** on a line of code. 
When the CPU executes that line, the silicon physically halts. The entire super-loop freezes in time. You can then use VS Code to inspect the literal contents of the RAM, check the value of every variable, and step through the C++ code line-by-line.

---

## 6.3 Memory Leak Detection

In desktop apps, memory leaks slow down the computer until you close the app. In embedded systems, a memory leak guarantees a system crash.

If `MyMesh` dynamically allocates memory for a packet `mesh::Packet* p = new mesh::Packet();` but forgets to call `delete p;`, that block of heap memory is lost forever.
Since the ESP32 only has ~320 KB of RAM, a 256-byte leak occurring every time a packet arrives will crash the node after roughly 1,200 packets.

### 6.3.1 Heap Monitoring
To detect leaks, MeshCore nodes should periodically query the OS for free memory:
```cpp
uint32_t free_ram = esp_get_free_heap_size();
uint32_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
```
If `free_ram` steadily decreases over hours of operation, a memory leak exists. 
More insidiously, if `free_ram` remains stable, but `largest_block` approaches zero, the heap is suffering from **Fragmentation** (as discussed in Chapter 1), and a crash is imminent.
