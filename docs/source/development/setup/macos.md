# Setup - MacOS

## 1. Requirements

* You must have MacOS 11 or later.
* The game is compiled for x86_64 (Intel) and you must
  have [Rosetta installed](https://support.apple.com/en-us/HT211861) if you have M1 Mac!
* [Homebrew](https://brew.sh/) to install dependencies.
* XCode is not needed.
* It is recommended to use JetBrains CLion

## 2. Install dependencies

You will need the Clang LLVM toolchain, Git, [CMake](https://cmake.org/),
[pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/), and [Ninja Build](https://ninja-build.org/).
All of them can be installed via Homebrew.

```bash
brew install llvm git cmake pkg-config ninja
```

## 3. Option A: Command line only

```{note}
To get the best experience developing this game, **it is recommended to use an IDE**.
See the section for the Visual Studio Code below, or the section for the JetBrains CLion below.
```

First, you will need to clone the source code and fetch the git submodules.

```shell
git clone https://github.com/temporary-escape/temporary-escape.git

cd ./temporary-escape

git submodule update --init
```

Next, create a build directory. This directory will contain the temporary files for compiling the game.

```shell
mkdir ./build
```

Next, configure the project with CMake. Choose the `Ninja` generator and use the `Debug` build type. This
needs to be done only once. When you delete the `build` folder you must re-do this step.

```{warning}
This next command will download all of the C++ dependencies, compile them, and configure the game's source code.
**This may take up to 1 hour to complete and may take up to 3GB of disk space.** The dependencies are installed
locally within the build folder only.
```

```{hint}
Debug builds are useful for debugging, **but it makes the game slow**. This is especially noticable when
generating a new universe. It may take up to 10x longer. Prefer to use `Release` unless you need debugging support.
```

```shell
# This may take up to 1 hour to complete!
cmake --preset ninja-macos \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_INSTALL_PREFIX=./install
```

At the end you should see something as "Build files have been written to: bla bla bla".

Next, compile the game. You can replace the `-j X` with number of CPU cores available on your computer.

```shell
cmake --build ./build --target TemporaryEscapeMain -j 4
```

At the end you should see something as "Linking CXX executable TemporaryEscape".

To run the game, you must run it with `--root` argument pointing to the cloned repository.

```{warning}
When running the game in a singleplayer or multiplayer mode **for the first time from the source code**,
the game will compile shader code and will compress the textures from png format into ktx2 format.
**This may take several minutes.** This is done only once.
```

```shell
# Run the game with --root argument pointing to the cloned repository!
./build/TemporaryEscape --root $(pwd)
```

## 3. Option B: Visual Studio Code

To be added...

## 3. Option C: JetBrains CLion

Install [JetBrains CLion](https://www.jetbrains.com/clion/).

Once installed, open a terminal and clone the game source code somewhere into your system.

```shell
git clone https://github.com/temporary-escape/temporary-escape.git

cd ./temporary-escape

git submodule update --init
```

Next, open CLion and open the cloned repository as a folder.

![](images/macos-clion-open.png)

When asked about trusting the project, click "Trust Project".

![](images/macos-clion-trust.png)

Once the project is open, you should see "Open Project Wizard" window. Either modify an existing "Default" toolchain
or add a new one. For the "C Compiler" type in `clang` and for the "C++ Compiler" type in `clang++`. The "Debugger"
should be "Bundled GDB" for the best experience. Then click "Next".

![](images/macos-clion-toolchain.png)

On the next page, select the toolchain you have modified (e.g. `Default`), choose `Ninja` for the "Generator".
In the "CMake options" add in the following option:

```text
-DCMAKE_INSTALL_PREFIX=./install
```

![](images/macos-clion-configure.png)

And finally click Finish.

```{warning}
This will download all of the C++ dependencies, compile them, and configure the game's source code.
**This may take up to 1 hour to complete and may take up to 3GB of disk space.** The dependencies are installed
locally within the build folder only.
```

Once configured, you should see something as "Build files have been written to: bla bla bla" in the "CMake" tab at
the bottom.

![](images/macos-clion-configured.png)

To build the game select the "TemporaryEscapeMain" from the dropdown in the top menu bar, and click the build icon next
to it.

![](images/macos-clion-build.png)

Once built you should see something as the following:

![](images/macos-clion-builddone.png)

To play and debug the game, you must first edit the configuration for the "TemporaryEscapeMain" target. Click the
down icon in the dropdown in the top menu bar, and select "Edit Configurations" from the dropdown.

![](images/macos-clion-editrun.png)

You must add a program argument `--root` followed by a space followed by the exact path to the source code of the game.

![](images/macos-clion-editargs.png)

And finally click play button (left icon). To run with a debugger click the second button (right icon).

```{warning}
When running the game in a singleplayer or multiplayer mode **for the first time from the source code**,
the game will compile shader code and will compress the textures from png format into ktx2 format.
**This may take several minutes.** This is done only once.
```

![](images/macos-clion-play.png)

To run the game from command line, you must do it as the following:

```shell
# Run the game with --root argument pointing to the cloned repository!
./build/TemporaryEscape --root $(pwd)
```

## 3. Option D: XCode

To be added...

## 4. Installing the game (optional)

To install the game, open a command line, go to the project folder, and type the following command below.
Make sure that you have used `-DCMAKE_INSTALL_PREFIX=/some/path` during the configuration. If you are using
a CMake preset (via `--preset`) then the install folder is going to be a directory named `install` inside the
project folder. It will be created during the install command as shown below.

```shell
# If you are using CLion the folder is most likely "cmake-build-debug" or "cmake-build-release"
cmake --build ./build --target install
```

## 5. Creating an dmg or a tar package (optional)

To create a `.tar.gz` and `.dmg` package, open a command line, go to the project folder, and type the following command
below.

```shell
# If you are using CLion the folder is most likely "cmake-build-debug" or "cmake-build-release"
cmake --build ./build --target package
```

## 6. Vulkan validation layers (optional)

The validation layers are useful for debugging Vulkan issues. This is enabled at runtime and does not need re-compiling.
To use the validation layers you must first download and install Vulkan SDK into your system.

Download Vulkan SDK version `1.3.239.0` from
the [Vulkan SDK official download page](https://vulkan.lunarg.com/sdk/home#mac).
Or you can use
an [unofficial mirror link here](https://temporary-escape.us-east-1.linodeobjects.com/vulkan-sdk/vulkansdk-macos-1.3.239.0.dmg).

Install the SDK into your Mac, for example into the `/Users/username/VulkanSDK/1.3.239.0` folder.

When starting the game, modify the environment variable with:

```shell
# Go to the project folder
# Then make sure to set these environment variables
# Modify the VULKAN_SDK path to the correct one!!!
export VULKAN_SDK=/Users/username/VulkanSDK/1.3.239.0/macOS
export DYLD_LIBRARY_PATH=$VULKAN_SDK/lib:$DYLD_LIBRARY_PATH
export VK_LAYER_PATH=$VULKAN_SDK/share/vulkan/explicit_layer.d
export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation

# Then start the game
./build/TemporaryEscape --root $(pwd)
```

```{note}
The game will pick up the `VK_LAYER_PATH` and will enable the validation layers when the game starts.
The game will stop and exit immediately if the validation layers can not be setup. This may
happen if your Vulkan SDK folder is wrong or the environment variables are wrong. Look into
the log file in your `~/.temporary-escape/` folder.
```

If you are using JetBrains CLion, you can modify the "Run Configuration" for the "TemporaryEscapeMain" target
(the dropdown in the top menu bar). Add these environment variables as shown below.

![](images/macos-clion-layers.png)

If you are using Visual Studio Code, you can modify the `launch.json` file in the `.vscode` folder. Simply
add those variables into `"environment": []` property as shown below.

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "type": "cppdbg",
      "name": "Debug TemporaryEscape",
      "environment": [
        "VULKAN_SDK=/Users/username/VulkanSDK/1.3.239.0/macOS",
        "DYLD_LIBRARY_PATH=/Users/username/VulkanSDK/1.3.239.0/lib",
        "VK_LAYER_PATH=/Users/username/VulkanSDK/1.3.239.0/macOS/share/vulkan/explicit_layer.d",
        "VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation"
      ]
      // Other stuff here
    }
  ]
}
```
