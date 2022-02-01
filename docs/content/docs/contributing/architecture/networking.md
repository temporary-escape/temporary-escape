---
weight: 3
title: "Networking"
---

# Networking

## Client Server model

The game works in a classic [client-server model](https://en.wikipedia.org/wiki/Client%E2%80%93server_model). Server is responsible for simulating the world and persisting the data. The client is responsible for reading the information provided by the server and render it.

There is no [lockstep protocol](https://en.wikipedia.org/wiki/Lockstep_protocol) or any similar mechanism that you may find in an FPS game. The client sends "commands" to the server. One of such command can be "approach this enemy NPC" while you are in your own player ship. The server receives the command and proceeds to start moving your ship to the enemy. Think of it as a strategy game. When an entity is moved, or changed, it is marked as "needs to sync", which eventually is picked up by the scene manager and sends an event to the client about its changes. The client receives a stream of updates.

Additionally, the server provides a way for the client to ask for certain information. For example, when you open a galaxy map, the client will request information about all systems within the galaxy.

Not only the client is a passive listener for events (entity changes and similar), but also an RPC-like client.

## TCP and AES-256

The game networking is done on top of TCP with help of [asio](https://think-async.com/Asio/) which is responsible for handling the sockets and a work queue. Moreover the [openssl](https://www.openssl.org/) is used for establishing a secure encrypted connection between the client and server. It uses [Diffie-Hellman key exchange](https://en.wikipedia.org/wiki/Diffie%E2%80%93Hellman_key_exchange) and uses the shared secret to establish [AES-256](https://en.wikipedia.org/wiki/Advanced_Encryption_Standard) which is used to encrypt or decrypt messages.

Note that [digital certificates](https://en.wikipedia.org/wiki/Transport_Layer_Security#Digital_certificates) are not used.

Here is a basic sequence of establishing connection from the client to the server.

{{< mermaid >}}
sequenceDiagram
    participant Client
    participant Server
    participant Database
    Client->>Server: Send public key
    Server->>Client: Send public key
    Client-->Client: Calculate shared key
    Server-->Server: Calculate shared key
    Server-->Server: Add player to "lobby"
    Client->>Server: Login
    Server->>Database: Put or update player
    Database->>Server: OK
    Server->>Client: Connect or error
{{< /mermaid >}}

## Messaging

The game uses a custom made RPC-like framework to send and receive messages. These messages are C++ structures that are serialized and deserialized via [msgpack-c](https://msgpack.org/index.html). These messages are then encrypted or decrypted via [openssl](https://www.openssl.org/) using a shared key. The raw data is stored in a "Packet", which is just an another C++ struct serialized via [msgpack-c](https://msgpack.org/index.html). This Packet also contains a unique message type ID, so that the client knows which type of a message (i.e. which C++ struct) has been received. We can use this type ID to assign a dispatch method called "handler" to the server or client. When a message is received, the corresponding "handle" method is called. 

The type ID is generated via a platform independent hash algorithm, so that when you use different OS or a different C++ compiler between the server and client, the message is always routed correctly.

Here is a sequence diagram showing the data flow of a message:

{{< mermaid >}}
sequenceDiagram
    participant Client
    participant TCPClient
    participant TCPServer
    participant Server
    Client->>TCPClient: Message fetch galaxy systems
    TCPClient->>TCPClient: Serialize C++ struct
    TCPClient->>TCPClient: Encrypt raw data
    TCPClient->>TCPClient: Put to asio work queue
    TCPClient->>TCPServer: Send
    TCPServer->>TCPServer: Decrypt raw data
    TCPServer->>TCPServer: Deserialize into C++ struct
    TCPServer->>Server: Dispatch based on message type ID
    Server->>Server: Use handler function to deal with the message
    Server->>TCPServer: Message galaxy systems result
    TCPServer->>TCPServer: Serialize C++ struct
    TCPServer->>TCPServer: Encrypt raw data
    TCPServer->>TCPServer: Put to asio work queue
    TCPServer->>TCPClient: Send
    TCPClient->>TCPClient: Decrypt raw data
    TCPClient->>TCPClient: Deserialize into C++ struct
    TCPClient->>Client: Dispatch based on message type ID
{{< /mermaid >}}

## Requests and large data

How does it deal with large data? For example, we need to fetch thousands of system info for a galaxy when the player open the galaxy map view.

The framework uses something called "requests" (see `Request` C++ struct). A request is just a type of a message that has a unique sequence ID, response type (C++ type), request data, and continuation token. When a request is made, a unique ID is assigned to the request. The request is sent over to the server. Once in the server the request is processed and data is sent back to the client.

If the server has set the "token" string, the client will re-send the message with the token. The server will then process the message yet again, but using this token. This token is used by the [database]({{< ref "database.md" >}}) to seek next range of keys. Think of the "token" as a continuation token. Once an empty token is received to the client, the request is completed.

When the request is completed, the callback handler is called. Note that this callback is executed on the same thread as the rendering thread. This is by design. We want to handle data retrieval notification/callback functions on the same thread where OpenGL is running on, so that we can allocate OpenGL resources (such as meshes) within these callback functions.

## Next

See [server]({{< ref "server.md" >}}) page.
