# 1. Introduction

This page describes the basics of modding structure and what can be found where.

## Basic mod structure

A "mod" is simply a folder with a manifest xml file. This folder is detected automatically when the
game starts in the single-player or multi-player mode.

The mods folder "assets" simply contains all of the mods as separate directories.

```text
assets/    <- Folder which contains mods
    base/     <-- The base game mod
    my-mod/   <-- A custom mod
        manifest.xml   <-- The manifest file
```

The manifest file must be present and must be named as `manifest.xml`. The following is an example manifest file,
and all of the fields are required.

```xml
<?xml version="1.0" encoding="utf-8"?>
<manifest>
  <name>Name of the mod</name>
  <description>Some description of the mod</description>
  <author>Jane Doe</author>
  <version>1.0.0</version>
</manifest>
```

## Mod directory contents

Each directory inside the mod folder has a specific use case. For example, the textures are locaded only from
the `textures` folder. The folder names are hardcoded and can not be changed. This means that if you want
to add textures (or overwrite textures from the base mod) you must add them to your mod's `textures` directory. The game
engine detects these directories and loads the content from them automatically.

**These directories are optional!**

The following directory names are hardcoded:

| Directory (case-sensitive) | Used for                                     |
|----------------------------|----------------------------------------------|
| `blocks`                   | Ship building blocks definitions (XML files) |
| `images`                   | Icons and images as part of an image atlas   |
| `models`                   | Any 3D models (GLTF+BIN files)               |
| `particles`                | Particle type definitions (XML files)        |
| `planets`                  | Planet types definitions (XML files)         |
| `ships`                    | Ships created by the Editor used by entities |
| `sounds`                   | Sound effects and music files (OGG files)    |
| `textures`                 | Model and block textures                     |

## Scripting

Mods allow adding custom scripted behavior to events, entities, sectors, and more. **Scripting is done
in the [Lua](https://www.lua.org/) language.** More on Lua and its syntax can be found on the <project:lua.md> page.

**Scripting does not follow a strict folder structure!**

However, it is recommended that you use the following folders in your mod:

| Directory (case-sensitive) | Used for                           |
|----------------------------|------------------------------------|
| `entities`                 | Entity scripts                     |
| `events`                   | Server event handler functions     |
| `factions`                 | Faction definitions and behavior   |
| `sectors`                  | Sector types and sector generation |
| `utils`                    | Any utilities                      |

In order for the scripting to work, you must define an entrypoint. There are two entrypoint files: `server.lua`
and `sector.lua`. More on Lua and its entrypoints can be found on the <project:lua.md> page.
