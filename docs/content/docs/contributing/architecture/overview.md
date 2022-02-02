---
weight: 2
title: "Overview"
---

# Architecture Overview

Here is a rough diagram of architecture and component ownership. For example, The Application is responsible for the AssetManager. More details of each component is described further down in this page. To have more in-depth information, it is highly recommended to start with the [networking]({{< ref "networking.md" >}}) page.

{{< mermaid >}}
flowchart LR
    Main --> Application
    Application --> Renderer
    Application --> AssetManager
    Application --> ModManager
    Application --> Server
    Server --> Sector
    Sector --> Scene
    Server --> Database
    Server --> Services
    Server --> TCP-Server
    Application --> Client
    Client --> TCP-Client
    Client --> Scene-Proxy
    Client --> Store/Cache
    Application --> Views
    TCP-Server --> *TCP*
    TCP-Client --> *TCP*
{{< /mermaid >}}

## Example of an asteroid

The following is an example of data flow, and what roughly happens, when we want to display a simple scene with an entity (such as an steroid) to the player once they connect to the server.

First, we need to know the definition of the asteroid we want to have in the sector. Before the client or the server is bootstraped the assets are loaded. The [asset manager]({{< ref "asset-manager.md" >}}) is responsible for loading all assets (models, textures, blocks, asteroids, items, etc.). In this case, the asteroid is defined in an XML file.

Once the assets are loaded the [database]({{< ref "database.md" >}}) is bootstraped. The database holds all information about the universe. If no database is present an empty database is created.

Afterwards the server is started. If this is the first time we are running the universe, it is procedurally generated, and may take a while to generate.

When the server is ready to accept connections, the client will be created. Even if you are playing in a sigleplayer mode, the game is still running in a multiplayer mode, but accepts no outside connections (just you). The client will connect to the server.

Once the client is connected the server will have to decide where to put the player. Using procedural generation, the server will spawn a player at a given sector within a system in the starting galaxy. The sector is spawned and loaded. The sector will use the [database]({{< ref "database.md" >}}) to spawn entities. If the sector has not been spawned before it is populated based on the procedural generation.

All simulation happens in two places. Anything you see in your game (as a player) happens in the [scene]({{< ref "scene.md" >}}) that is owned by the sector instance. A scene is just a class that is responsible for entity logic and physics. Anything outside (research, etc.) happens in the services that is owned by the server. A service is simply a wrapper around the database that has a specific usage (regions, systems, sectors, players, inventory, etc.).

Now we have a sector that is updated periodically (tick) with a scene that has an asteroid. Because the player was added to this sector the entity is replicated to the client.

The server sends a message, data about the asteroid, over a [network]({{< ref "networking.md" >}}) framework to the client. The entity and all of its [components]({{< ref "ecs.md" >}}) will be put into a message. In this case, we will send over an entity position and the "model component". The [model component]({{< ref "ecs.md" >}}) describes what kind of a 3D model the entity should have. The model is not send over, only the name of the asset that the model is loaded from is.

The client receives the message and uses the asset name from the message to find the real 3D model asset. The client adds the entity to its own scene. Both the server and client own their own scenes. The server's scene is replicated to the client one.

Finally, the rendering will take the client's scene and render everything to the screen.

## Next

See [Networking]({{< ref "networking.md" >}}) page.

