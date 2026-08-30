# Chapter 19: Building Custom Frontends

While MeshCore provides a native smartphone application, the architecture is specifically designed to allow developers to build their own custom user interfaces. Because the communication protocol between the smartphone and the ESP32 is standardized using Protocol Buffers, you can build a frontend on almost any platform. This chapter explores how to interface with the node from custom software.

---

## 19.1 The Web Bluetooth API (WebBLE)

The fastest way to build a custom dashboard for a MeshCore node is a web browser. Modern browsers (Chrome, Edge) support the **Web Bluetooth API**, allowing a static HTML/JavaScript webpage to connect directly to the ESP32 over Bluetooth without installing any native apps.

### 19.1.1 Connecting via JavaScript
In your JavaScript code, you request access to the specific Bluetooth Service UUID that MeshCore advertises:

```javascript
navigator.bluetooth.requestDevice({
    filters: [{ namePrefix: 'MeshCore' }],
    optionalServices: ['12345678-1234-5678-1234-56789abcdef0']
})
.then(device => {
    return device.gatt.connect();
})
.then(server => {
    // Get the TX and RX characteristics
    return server.getPrimaryService('12345678...');
});
```

### 19.1.2 Web-Based Protobuf
To decode the payloads in the browser, you use `protobuf.js`. You load the exact same `meshcore.proto` file that the ESP32 firmware was compiled with.

When the ESP32 sends a packet over BLE:
1.  The JavaScript `characteristicvaluechanged` event fires.
2.  You extract the raw `Uint8Array`.
3.  You pass it to `protobuf.js`: `const message = MeshPacket.decode(raw_bytes);`
4.  You render `message.text` to the DOM.

This allows you to build a cross-platform (iOS, Android, Windows, Mac) mapping and messaging dashboard that runs entirely in the browser.

---

## 19.2 The Serial Protobuf Handshake

If you are building a native desktop application (e.g., using Python/Tkinter or C#/WPF) and connecting to the node via a USB cable, you must handle the physical serial connection.

### 19.2.1 The Initialization Sequence
When you plug the ESP32 into your computer, it does not immediately start dumping Protobuf packets over the Serial wire. It waits for a handshake.

1.  **The Wakeup Call**: Your desktop app opens the COM port (115200 baud) and sends a specific wakeup byte sequence (e.g., `0xAA 0xBB 0xCC`).
2.  **The Node Info Response**: The ESP32 responds with a `MyNodeInfo` Protobuf packet. This packet contains the node's Public Key, its battery level, and its current firmware version.
3.  **The Config Sync**: The desktop app sends a `GetConfig` request. The ESP32 responds by dumping the serialized `NodePrefs` struct.

Only after this handshake is complete will the ESP32 begin forwarding live LoRa packets over the USB wire. 

---

## 19.3 Compiling Native Mobile Apps

If you are forking the official MeshCore smartphone app, you must understand the native compilation toolchains.

### 19.3.1 Kotlin (Android) and Swift (iOS)
The official apps do not parse Bluetooth bytes manually. 
During the build process (using Gradle for Android, or SPM for iOS), the build scripts invoke the Google `protoc` compiler. 

If you add a new field to `meshcore.proto` in the C++ firmware (e.g., `float wind_speed = 5;`), you simply copy that updated `.proto` file into your Android Studio project. 
When you hit "Build", Gradle automatically generates the new Kotlin data classes, and `wind_speed` becomes instantly available in your Android UI code with zero manual parsing required. This seamless, unified schema is the secret to MeshCore's rapid cross-platform development.
