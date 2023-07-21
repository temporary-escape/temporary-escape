# Setup

To work with the C++ code of the game you must first set up the environment for your system.
You can do so by following the guides listed below.
The game can run on all three platforms and you can develop the game on all of those platforms.

* [Setup - Windows](<windows>)
* [Setup - Linux](<linux>)
* [Setup - MacOS](<macos>)

**You do not need to do this setup if you are interested only in modding the game with Lua or editing the
asset files.** See the [Modding Tutorial](<../../modding/tutorial/index>) for more information.

## TL;DR

In short:

* Make sure that you have Clang version 14 or later, CMake, Ninja Build tool, pkg-config, and `xorg-dev` on Linux.
* On Windows use **only** Clang-CL provided by the Visual Studio Build Tools. Anything else will fail.
* Prefer to use the provided CMake preset named `ninja-macos`, `ninja-linux`, or `ninja-msvc`.
* You don't need to install Vulkan SDK.
* You don't need to install vcpkg into your system.
* You don't need any additional C++ dependencies. Everything is handled by the CMake, and by the vcpkg that is a git
  submodule within the source code.

```{toctree}
:hidden:

Linux <linux>
Windows <windows>
MacOS <macos>
```
