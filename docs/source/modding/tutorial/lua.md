# 2. Lua Contexts

The Temporary Escape game uses Lua to define high level behavior and logic decisions. You can use Lua for almost
anything. The game's engine exposes its internal functions that can be used inside any Lua script.

## Server and sector context

There are two modes (a better named would be a "context") where Lua runs. **The server context and the sector context.**

These contexts run independently of each other can can not access each other. This means that there is no way to
directly access Lua script functions/variables/etc from one context to the other.

Think of Lua contexts as separate programs. You can't directly access another program's variables.

### Server context

The server context is responsible for the high level operations and events happening in the entire game's Universe.
This context is started and runs first before the sector context. There is only exactly one server context.

Some of the things that get processed in this server context are:

* An event that the player has joined a game.
* Generating galaxies, systems, planets, etc (What you see in the galaxy map).
* Creating factions and applying behavior to them.
* Processing changes on the galaxy.

### Sector context

The sector context is created each time the sector is loaded. The sector is loaded when the first player jumps to the
sector. The sector context is also destroyed when all the players leave the sector and the sector needs to be un-loaded
to free up the memory.

Some of the things that get processed in this sector context are:

* Entity behavior within the sector
* Sector specific events such as object collisions, entity deaths, and more.
* Populating the sector with entities

## Entrypoints

Each server and sector context has its specific entrypoint within the game folder. These are:

* `sector.lua` - For sectors, loaded each time for each sector.
* `server.lua` - For server, loaded exactly once.

These file names are hardcoded and can not be changed, and are purely optional.

You can import any additional script files (via the `require` function) from these entrypoint files. You can also
import other mod's script files via the folder's name. See the "Import another file" section below for more information.

If you need to add entities into your mod, you must create the `sector.lua` file, but you do not need to create
the `server.lua` file.

The game engine detects these files automatically and runs them when needed (when the server starts, or when the sector
starts).

```{warning}
**You can not share Lua variables or functions between different contexts!** This limitation also applies to
sharing Lua variables or functions between two sectors. Each sector has it's own Lua context.

This also means modifying any global variables is only visible within that context (i.e. sector).
```
