# Chapter 8: Mesh Routing Algorithms in Depth

Chapter 2 briefly introduced Managed Flooding. This chapter delves deep into the theoretical models of mesh routing, comparing MeshCore's architecture to other standard protocols like AODV and exploring the physics of RF collisions.

---

## 8.1 Distance Vector vs. Flooding

In classical networking (like the Internet), routers use highly complex protocols (BGP, OSPF) to build exact maps of the network. When a packet arrives, the router looks at the map and forwards the packet down the optimal physical wire.

In a mobile wireless mesh, nodes move. A perfectly calculated map becomes obsolete in seconds.

### 8.1.1 AODV (Ad-hoc On-Demand Distance Vector)
Many mesh networks use AODV. If Node A wants to reach Node Z, it broadcasts a "Route Request" (RREQ). This request ripples through the network. When Node Z hears it, it sends a "Route Reply" (RREP) back along the exact same path. Node A now knows the exact sequence of hops (e.g., A -> B -> D -> Z) and embeds this path in the packet.
*   **Pros**: Highly efficient once the route is established. No wasted bandwidth on unnecessary nodes.
*   **Cons**: Massive overhead to discover routes. If Node D drives away in a car, the route breaks, and the entire discovery process must restart.

### 8.1.2 MeshCore's Managed Flooding
Because LoRa is extremely low bandwidth, AODV route discovery consumes too much airtime. Instead, MeshCore uses **Managed Flooding**.
Node A simply broadcasts the packet. Every node that hears it rebroadcasts it. The packet washes over the physical landscape like a wave in a pond.
*   **Pros**: Zero overhead for route discovery. Completely immune to nodes moving. If Node D drives away, the packet just routes through Node E instead.
*   **Cons**: Horrendously inefficient in dense networks. A 100-node network might result in 100 retransmissions of a single text message.

To mitigate the cons, MeshCore relies on Deduplication (dropping packets it has already relayed) and Hop Limits (killing packets after 3 hops).

---

## 8.2 The Hidden Node Problem and CSMA

In wireless networks, if two nodes transmit on the same frequency at the exact same millisecond, their radio waves collide. The receiver hears garbage.

### 8.2.1 The Hidden Node Problem
Imagine Node A, Node B, and Node C in a line. 
*   A can hear B.
*   C can hear B.
*   A **cannot** hear C (they are hidden from each other due to distance).

If A wants to talk to B, it listens to the air, hears silence, and begins transmitting.
Simultaneously, C wants to talk to B. It listens to the air, hears silence (because it cannot hear A's transmission), and begins transmitting.
Both signals hit Node B simultaneously. The packets collide and are destroyed.

### 8.2.2 CSMA/CA (Carrier-Sense Multiple Access with Collision Avoidance)
To combat this, the SX1262 utilizes CAD (Channel Activity Detection). Before `MyMesh` pulls the trigger to transmit, it commands the radio to perform a microscopic scan of the frequency.
If the radio detects LoRa preambles (indicating someone else is transmitting), `MyMesh` aborts the transmission and recalculates a random backoff timer (e.g., waiting 500ms before trying again). 

This randomized jitter, combined with CAD, drastically reduces the probability of catastrophic air collisions, even when dozens of nodes attempt to flood a single packet simultaneously.
