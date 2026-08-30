# Chapter 9: Porting to New Hardware Platforms

MeshCore is inherently platform-agnostic. While this tutorial focuses heavily on the ESP32-S3, the C++ architecture is designed to compile and run on vastly different silicon, such as the Nordic NRF52 or the Raspberry Pi RP2040. This chapter explains the Abstraction Layer that makes this possible.

---

## 9.1 The Hardware Abstraction Layer (HAL)

If `MyMesh.cpp` contained direct calls to ESP32-specific registers (e.g., `WRITE_PERI_REG(UART_FIFO_REG, byte);`), the codebase would be permanently locked to Espressif silicon. 

To port the code to an NRF52 (which has a completely different CPU architecture and memory map), we use a **Hardware Abstraction Layer**.

### 9.1.1 The `ESP32Board` Class
In Chapter 2, we instantiated `ESP32Board board;`. 
This class implements a generic `Board` interface. When `main.cpp` calls `board.begin();`, it has no idea what hardware it is running on.
*   If compiled for ESP32, `ESP32Board::begin()` configures the Xtensa sleep modes and Watchdog timer.
*   If compiled for NRF52, `NRF52Board::begin()` configures the ARM Cortex-M4 sleep modes.

### 9.1.2 Conditional Compilation in `target.h`
Open `target.h`. You will see blocks like this:
```cpp
#if defined(TARGET_TBEAM_S3)
    #define P_LORA_NSS  10
    #define P_LORA_DIO_1 11
#elif defined(TARGET_RAK4631)
    #define P_LORA_NSS  42
    #define P_LORA_DIO_1 47
#endif
```
When you run `platformio run --environment rak4631`, PlatformIO injects `-D TARGET_RAK4631` into the compiler. 
The C++ Preprocessor mathematically strips out all the ESP32 pin definitions, leaving only the Nordic NRF52 pins. The rest of the codebase (`MyMesh.cpp`, `DataStore.cpp`) remains 100% untouched.

---

## 9.2 Filesystem Abstractions (SPIFFS vs LittleFS)

Different silicon vendors provide different flash memory drivers.
*   The **ESP32** natively supports SPIFFS (SPI Flash File System).
*   The **NRF52** (using the Adafruit BSP) natively supports LittleFS.

Look at `DataStore.cpp`:
```cpp
#if defined(NRF52_PLATFORM)
    #define FILESYSTEM LittleFS
#else
    #define FILESYSTEM SPIFFS
#endif

File DataStore::openRead(const char *filename) {
    return FILESYSTEM.open(filename, "r");
}
```
By abstracting the specific filesystem class into a macro, `DataStore` can serialize contacts and channels identically across radically different microcontrollers. The C++ code never changes, only the underlying hardware driver swapped out by the compiler.

This modular, abstracted design is what allows the MeshCore-Bare codebase to support dozens of different physical boards across multiple CPU architectures without fracturing the source code into a messy, unmaintainable state.
