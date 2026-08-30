# Chapter 16: Cryptographic Key Exchanges

Chapter 3 explored how the Mesh uses a Pre-Shared Key (AES-256) for the Public channel, and Ed25519 for digital signatures. But what happens when you want to send a highly sensitive, private Direct Message (DM) to another user? You cannot use the Public AES key, or everyone on the network could read it. This chapter explains the mathematics of the Key Exchange.

---

## 16.1 The Key Distribution Problem

If Node A wants to securely message Node B, they need a shared secret key (a Symmetric AES key).
However, Node A and Node B are physically separated by 10 miles of wilderness. If Node A generates a new AES key and transmits it over the radio to Node B, an attacker listening with a Software Defined Radio (SDR) will intercept the key, compromising all future messages.

How do two people agree on a shared secret across a public, unencrypted channel without anyone else figuring out what the secret is?

---

## 16.2 Elliptic-Curve Diffie-Hellman (ECDH)

MeshCore solves this using **Elliptic-Curve Diffie-Hellman (ECDH)**, specifically utilizing the X25519 curve.

### 16.2.1 The Mathematics of Paint
To conceptualize ECDH, imagine cryptographic keys as colors of paint.
1. Node A and Node B agree on a public "Base Color" (e.g., Yellow). Everyone in the world knows the Base Color is Yellow.
2. Node A selects a Secret Color (e.g., Red). Node A mixes Red + Yellow = **Orange**.
3. Node B selects a Secret Color (e.g., Blue). Node B mixes Blue + Yellow = **Green**.
4. Node A and Node B transmit their mixed colors (Orange and Green) openly across the radio network. The attacker intercepts Orange and Green, but separating mixed paint is mathematically impossible (the Discrete Logarithm Problem).
5. Node A receives Green, and adds its Secret Color (Red). Red + Green = **Brown**.
6. Node B receives Orange, and adds its Secret Color (Blue). Blue + Orange = **Brown**.

Node A and Node B have independently arrived at the exact same shared secret (Brown) without ever transmitting their secret colors over the air.

### 16.2.2 The C++ Implementation
In MeshCore, every node already possesses a Public/Private keypair (their Node ID).
When Node A wants to DM Node B, it executes an ECDH function:

```cpp
// Generating the Shared Secret for Direct Messaging
uint8_t shared_secret[32];
crypto::ecdh_generate(
    shared_secret, 
    NodeA.private_key, // My Secret Color
    NodeB.public_key   // Your Mixed Color
);
```

The resulting 32-byte `shared_secret` is perfectly identical on both nodes. It is then used as the AES-256 key to encrypt the payload of the Direct Message. 

Because Node A's private key never leaves the ESP32, and the math requires both private keys to derive the shared secret, an attacker capturing the packets can never decrypt the Direct Message, even if they know both users' public Node IDs.

---

## 16.3 Forward Secrecy

If an attacker physically captures Node A, desolders the flash chip, and extracts the Private Key, they could theoretically decrypt all past intercepted Direct Messages.

Advanced implementations of MeshCore mitigate this using **Perfect Forward Secrecy (PFS)**. Instead of using the static Node IDs for the ECDH exchange, the nodes automatically generate new, ephemeral (temporary) X25519 keypairs for every single conversation session. 
Once the conversation ends, the ephemeral keys are deleted from RAM. Even if the node is later captured and physically compromised, the past messages remain mathematically locked forever because the keys used to encrypt them no longer exist.
