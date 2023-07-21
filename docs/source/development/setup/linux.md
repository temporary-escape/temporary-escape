# Setup - Linux

## 1. Requirements

* You must have some Arch or Debian based Linux 64-bit, preferably Manjaro or Ubuntu.
  ARM architecture is not supported at the moment.
* To run the game you must have Vulkan available. You can check if Vulkan is in your system by looking for the
  Vulkan library: `sudo find /usr -name "libvulkan.so"`. This library is installed with your graphics drivers.

## 2. Install dependencies

You will need the Clang LLVM toolchain, Git, [CMake](https://cmake.org/),
[pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/), [Ninja Build](https://ninja-build.org/),
and some additional packages needed for the dependencies.

```shell
# Debian/Ubuntu
sudo apt-get install git cmake ninja-build clang llvm curl zip unzip tar pkg-config gdb \
                     autoconf automake libtool libxinerama-dev libxcursor-dev xorg-dev \
                     libglu1-mesa-dev libc6-dev linux-libc-dev liburing-dev
                     
# Arch
sudo pacman -S git cmake ninja clang curl zip unzip tar pkg-config gdb \
               autoconf automake libtool 
               
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

Next, configure the project with CMake. Choose the `ninja-linux` preset and use the `Debug` build type. This
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
cmake --preset ninja-linux \
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

Install [Visual Studio Code](https://code.visualstudio.com/) and make sure you have the latest updated version.

Once installed, make sure to install the CMake Tools extension by looking for "cmake" and installing the
"CMake Tools" by Microsoft.

![](images/linux-vscode-extension.png)

You will also need to install "C++" extension by Microsoft.

![](images/linux-vscode-extension2.png)

Next, open the terminal and clone the game source code somewhere into your system.

```shell
git clone https://github.com/temporary-escape/temporary-escape.git

cd ./temporary-escape

git submodule update --init
```

Next, **open that cloned repository folder** and make sure to select "Trust" when asked to.

![](images/linux-vscode-trust.png)

Next, open the command panel (press keys \[Ctrl\] + \[P\]) and type "> cmake configure", select the "CMake: Configure"
option.

![](images/linux-vscode-configure.png)

When asked about the preset, select the "Ninja Clang" one.

```{warning}
This will download all of the C++ dependencies, compile them, and configure the game's source code.
**This may take up to 1 hour to complete and may take up to 3GB of disk space.** The dependencies are installed
locally within the build folder only.
```

![](images/linux-vscode-toolchain.png)

When asked about the test preset, select the "ninja-linux" one.

![](images/linux-vscode-testpreset.png)

At the end you should see something like this with "Build files have been written to: ...".

![](images/linux-vscode-configured.png)

Next, to build the game, go to the CMake options in the sidebar, and click on the build icon in the top bar.

![](images/linux-vscode-build.png)

Once built, you should see something like this:

![](images/linux-vscode-builddone.png)

To launch the game with debugger through Visual Studio, you must create a new launch configuration `launch.json`
in the `.vscode` folder. If the folder does not exist, create it. Use the following contents below for the
`launch.json` file:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "type": "cppdbg",
      "cwd": "${workspaceFolder}/build",
      "request": "launch",
      "name": "Debug TemporaryEscape",
      "program": "${workspaceFolder}/build/TemporaryEscape",
      "args": [
        "--root",
        "${workspaceFolder}"
      ],
      "MIMode": "gdb",
      "miDebuggerPath": "/usr/bin/gdb",
      "setupCommands": [
        {
          "description": "Enable pretty-printing for gdb",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        }
      ]
    }
  ]
}
```

Once the `launch.json` is created, go to the "Run and Debug" panel by clicking the play button from the side menu.
Select "Debug TemporaryEscape" and click the play icon button.

```{warning}
When running the game in a singleplayer or multiplayer mode **for the first time from the source code**,
the game will compile shader code and will compress the textures from png format into ktx2 format.
**This may take several minutes.** This is done only once.
```

![](images/linux-vscode-debugging.png)

To run the game from command line, you must do it as the following:

```shell
# Run the game with --root argument pointing to the cloned repository!
./build/TemporaryEscape --root $(pwd)
```

## 3. Option C: JetBrains CLion

Install [JetBrains CLion](https://www.jetbrains.com/clion/).

Once installed, open a terminal and clone the game source code somewhere into your system.

```shell
git clone https://github.com/temporary-escape/temporary-escape.git

cd ./temporary-escape

git submodule update --init
```

Next, open CLion and open the cloned repository as a folder.

![](images/linux-clion-open.png)

When asked about trusting the project, click "Trust Project".

![](images/linux-clion-trust.png)

Once the project is open, you should see "Open Project Wizard" window. Either modify an existing "Default" toolchain
or add a new one. For the "C Compiler" type in `clang` and for the "C++ Compiler" type in `clang++`. The "Debugger"
should be "Bundled GDB" for the best experience. Then click "Next".

![](images/linux-clion-toolchain.png)

On the next page, select the toolchain you have modified (e.g. `Default`), choose `Ninja` for the "Generator".
In the "CMake options" add in the following option:

```text
-DCMAKE_INSTALL_PREFIX=./build/install
```

![](images/linux-clion-configure.png)

And finally click Finish.

```{warning}
This will download all of the C++ dependencies, compile them, and configure the game's source code.
**This may take up to 1 hour to complete and may take up to 3GB of disk space.** The dependencies are installed
locally within the build folder only.
```

Once configured, you should see something as "Build files have been written to: bla bla bla" in the "CMake" tab at
the bottom.

![](images/linux-clion-configured.png)

To build the game select the "TemporaryEscapeMain" from the dropdown in the top menu bar, and click the build icon next
to it.

![](images/linux-clion-build.png)

Once built you should see something as the following:

![](images/linux-clion-builddone.png)

To play and debug the game, you must first edit the configuration for the "TemporaryEscapeMain" target. Click the
down icon in the dropdown in the top menu bar, and select "Edit Configurations" from the dropdown.

![](images/linux-clion-editrun.png)

You must add a program argument `--root` followed by a space followed by the exact path to the source code of the game.

![](images/linux-clion-editargs.png)

And finally click play button (left icon). To run with a debugger click the second button (right icon).

```{warning}
When running the game in a singleplayer or multiplayer mode **for the first time from the source code**,
the game will compile shader code and will compress the textures from png format into ktx2 format.
**This may take several minutes.** This is done only once.
```

![](images/linux-clion-play.png)

To run the game from command line, you must do it as the following:

```shell
# Run the game with --root argument pointing to the cloned repository!
./build/TemporaryEscape --root $(pwd)
```

## 4. Installing the game (optional)

To install the game, open a command line, go to the project folder, and type the following command below.
Make sure that you have used `-DCMAKE_INSTALL_PREFIX=/some/path` during the configuration. If you are using
a CMake preset (via `--preset`) then the install folder is going to be a directory named `install` inside the
project folder. It will be created during the install command as shown below.

```shell
# If you are using CLion the folder is most likely "cmake-build-debug" or "cmake-build-release"
cmake --build ./build --target install
```

## 5. Creating an AppImage or a tar package (optional)

To create a `tar.gz` package, open a command line, go to the project folder, and type the following command below.

```shell
# If you are using CLion the folder is most likely "cmake-build-debug" or "cmake-build-release"
cmake --build ./build --target package
```

To create an `.AppImage` you first must download `appimagetool` and put it somewhere in your system.
Use the command below to install the tool.

```shell
sudo wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage -O /usr/bin/appimagetool
sudo chmod +x /usr/bin/appimagetool
```

The following command below will create the AppImage using the tar.gz package file.
Use the command above (cmake target package) to create the tar package.

```shell
./appimage/build.sh /path/to/temporary-escape-<version>.tar.gz
```

## 6. Vulkan validation layers (optional)

The validation layers are useful for debugging Vulkan issues. This is enabled at runtime and does not need re-compiling.
To use the validation layers you must first download and install Vulkan SDK into your system.

Download Vulkan SDK version `1.3.239.0` from
the [Vulkan SDK official download page](https://vulkan.lunarg.com/sdk/home#linux).
Or you can use
an [unofficial mirror link here](https://us-east-1.linodeobjects.com/temporary-escape/vulkan-sdk/vulkansdk-linux-x86_64-1.3.239.0.tar.gz).

Extract the SDK into your system, for example into the `/opt/vulkan-sdk/1.3.239.0` folder.

When starting the game, modify the environment variable with:

```shell
# Go to the project folder
# Then make sure to set these environment variables
export VULKAN_SDK=/opt/vulkan-sdk/1.3.239.0/x86_64
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/opt/vulkan-sdk/1.3.239.0/x86_64/lib
export VK_LAYER_PATH=/opt/vulkan-sdk/1.3.239.0/x86_64/etc/vulkan/explicit_layer.d

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

![](images/linux-clion-layers.png)

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
        "VULKAN_SDK=/opt/vulkan-sdk/1.3.239.0/x86_64",
        "LD_LIBRARY_PATH=/opt/vulkan-sdk/1.3.239.0/x86_64/lib",
        "VK_LAYER_PATH=/opt/vulkan-sdk/1.3.239.0/x86_64/etc/vulkan/explicit_layer.d"
      ]
      // Other stuff here
    }
  ]
}
```
