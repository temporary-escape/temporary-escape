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

* Make sure that you have Clang version 14 or later, CMake, Ninja Build tool, pkg-config, and `xorg-dev` on Linux.
* On Windows use **ONLY** x64 Clang-CL provided by the Visual Studio Build Tools. Anything else will fail. Default
  MSVC (i.e. `cl.exe`) will fail.
* Apple Mac Intel is supported and M1/M2 are also supported with Apple Rosetta.
* No special Linux distribution is needed.
* Prefer to use the provided CMake preset named `ninja-macos`, `ninja-linux`, or `ninja-msvc`.
* You need a GPU that supports Vulkan 1.1 or higher (anything from year 2019 or later should be ok). On Apple Mac you
  need Metal, which is supported out of the box.
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
