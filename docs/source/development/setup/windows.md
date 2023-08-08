# Setup - Windows

## 1. Requirements

* You must have Windows 10 64-bit or later, or Windows Server 2019 or later.
* The game is compiled for x64 architecture only.
* **Visual Studio Tools** are needed with the **Clang compiler**.
* MinGW, MSys, or Cygwin are **not supported**.
* It is recommended to use Visual Studio 2022

## 2. Install dependencies

First you need the **Visual Studio Build Tools**. This already comes with the Visual Studio IDE, but
you don't need to install the entire IDE, the Build Tools alone is enough.
Go to [Visual Studio Download page](https://visualstudio.microsoft.com/downloads/?q=build+tools) and at the bottom
in the section "All Downloads" look for "Tools for Visual Studio". Download it and install it.

![](images/windows-deps-buildtools.png)

Download it and install it. Once the window opens, check the "Desktop development with C++" and from the
right hand side "Installation details" check the "C++ Clang tools for Windows" in the "Optional" section.
That's all you need, click the Install button.

![](images/windows-deps-clangtools.png)

You will also need to install the following tools:

* [Git for Windows](https://git-scm.com/download/win)
* [Chocolately](https://chocolatey.org/install) (follow the "Install Chocolately for Individual Use")

One the Git and Chocolately is installed, open PowerShell as an Administrator, and install the following packages:

```shell
choco install cmake ninja
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
md build
```

Before you call the CMake configure command, you must call the `vcvars64.bat` file from the Visual Studio toolchain.
This is done by calling the bah script from your terminal. This must be done every time you want to call
the CMake configure command.

```shell
# MUST BE DONE VIA CMD!!!
# The script can also be located at ("Community" sub-folder):
# C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

# To verify that clang is present
# Must print some version number
clang --version
```

![](images/windows-cmd-vcvars.png)

Next, configure the project with CMake. Choose the `ninja-msvc` preset and use the `Debug` build type. This
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
cmake --preset ninja-msvc -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=./install
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
.\build\TemporaryEscape.exe --root C:\Projects\temporary-escape
```

The logs for the game can be found in the following path as listed below.
**The logs will not appear in the terminal output.**

```text
C:\Users\Username\AppData\Roaming\TemporaryEscape\TemporaryEscape.log
```

## 3. Option B: Visual Studio Code

Install [Visual Studio Code](https://code.visualstudio.com/) and make sure you have the latest updated version.

Once installed, make sure to install the CMake Tools extension by looking for "cmake" and installing the
"CMake Tools" by Microsoft.

![](images/windows-vscode-extension.png)

You will also need to install "C++" extension by Microsoft.

![](images/windows-vscode-extension2.png)

Next, open the terminal and clone the game source code somewhere into your system.

```shell
git clone https://github.com/temporary-escape/temporary-escape.git

cd ./temporary-escape

git submodule update --init
```

```{warning}
You must run `vcvars64.bat` from a command line, then start Visual Studio Code by executing command `code`.
This will make sure that the correct compiler is set! Otherwise the configuration will fail.
```

```shell
# MUST BE DONE VIA CMD!!!
# The script can also be located at ("Community" sub-folder):
# C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

# Starts a new window of Visual Studio Code with vcvars64 variables set!
code
```

![](images/windows-vscode-vcvars.png)

Next, **open that cloned repository folder** with Visual Studio Code, and make sure to select "Trust" when asked to.

![](images/windows-vscode-trust.png)

Next, open the command panel (press keys \[Ctrl\] + \[P\]) and type "> cmake configure", select the "CMake: Configure"
option.

![](images/windows-vscode-configure.png)

When asked about the preset, select the "Ninja Clang" one.

```{warning}
This will download all of the C++ dependencies, compile them, and configure the game's source code.
**This may take up to 1 hour to complete and may take up to 3GB of disk space.** The dependencies are installed
locally within the build folder only.
```

![](images/windows-vscode-toolchain.png)

When asked about the test preset, select the "ninja-linux" one.

![](images/windows-vscode-testpreset.png)

At the end you should see something like this with "Build files have been written to: ...".

![](images/windows-vscode-configured.png)

Next, open the command panel (press keys \[Ctrl\] + \[P\]) and type "> cmake build", select the "CMake: Build"
option.

![](images/windows-vscode-build.png)

When asked which target, select the `TemporaryEscapeMain` target.

![](images/windows-vscode-target.png)

Once built, you should see something like this:

![](images/windows-vscode-builddone.png)

To launch the game with debugger through Visual Studio Code, you must create a new launch configuration `launch.json`
in the `.vscode` folder. If the folder does not exist, create it. Use the following contents below for the
`launch.json` file:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "type": "cppvsdbg",
      "cwd": "${workspaceFolder}/build",
      "request": "launch",
      "name": "Debug TemporaryEscape",
      "program": "${workspaceFolder}/build/TemporaryEscape.exe",
      "args": [
        "--root",
        "${workspaceFolder}"
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

![](images/windows-vscode-debugging.png)

The logs for the game can be found in the following path as listed below.
**The logs will not appear in the terminal output.**

```text
C:\Users\Username\AppData\Roaming\TemporaryEscape\TemporaryEscape.log
```

To run the game from command line, you must do it as the following:

```shell
# Run the game with --root argument pointing to the cloned repository!
./build/TemporaryEscape.exe --root C:/Projects/temporary-escape
```

## 3. Option C: Visual Studio IDE 2022

You will need to install Visual Studio 2022 or later. To do so, open the start menu, and search
for `visual studio installer`.
This has been installed with the Build Tools that you have installed previously (see the "Dependencies" section above).

![](images/windows-vs-find.png)

Select the "Available" tab at the top and install the Community version of Visual Studio.

![](images/windows-vs-install.png)

When selecting the components, make sure to select "Desktop development with C++". You must also select "C++ Clang tools
for Windows" from the right hand side menu "Installation details".

![](images/windows-vs-install2.png)

Next, you will need to clone the source code and fetch the git submodules.

```shell
git clone https://github.com/temporary-escape/temporary-escape.git

cd ./temporary-escape

git submodule update --init
```

Next, open the cloned folder. Visual Studio should automatically choose the provided CMake profile and it should
start configuring the project.

![](images/windows-vs-open.png)

```{warning}
This will download all of the C++ dependencies, compile them, and configure the game's source code.
**This may take up to 1 hour to complete and may take up to 3GB of disk space.** The dependencies are installed
locally within the build folder only.
```

Once configured, you should see a message "Build files have been written to: bla bla bla".

![](images/windows-vs-configured.png)

To build the game, select the `TemporaryEscape.exe` Startup Item from the dropdown in the top menu bar.

![](images/windows-vs-target.png)

Next, in the top menu bar, click on "Build" and then select "Build TemporaryEscape.exe".

![](images/windows-vs-build.png)

Once built, you should see a message as shown below.

![](images/windows-vs-builddone.png)

To play the game, in the top menu bar, click "Debug" and then select "Debug and Launch Settings for
TemporaryEscapeMain".

![](images/windows-vs-launchsettings.png)

This will create a new `launch.vs.json` file. Add in the following contents after `"name":` property.
Set the correct path. This path must point to the root project folder. **Don't forget to save the file.**

```text
  "args": [
    "--root",
    "C:/projects/temporary-escape"
  ]
```

![](images/windows-vs-launchjson.png)

And finally press the play button in the top menu bar.

```{warning}
When running the game in a singleplayer or multiplayer mode **for the first time from the source code**,
the game will compile shader code and will compress the textures from png format into ktx2 format.
**This may take several minutes.** This is done only once.
```

![](images/windows-vs-play.png)

The logs for the game can be found in the following path as listed below.
**The logs will not appear in the terminal output.**

```text
C:\Users\Username\AppData\Roaming\TemporaryEscape\TemporaryEscape.log
```

To run the game from command line, you must do it as the following:

```shell
# Run the game with --root argument pointing to the cloned repository!
./build/TemporaryEscape.exe --root C:/Projects/temporary-escape
```

## 3. Option D: jetBrains CLion

First, download and install [CLion from the official JetBrains website](https://www.jetbrains.com/clion/download/).

Next, you will need to clone the source code and fetch the git submodules.

```shell
git clone https://github.com/temporary-escape/temporary-escape.git

cd ./temporary-escape

git submodule update --init
```

Next, open CLion and open the cloned repository as a folder.

![](images/windows-clion-open.png)

When asked about trusting the project, click "Trust Project".

![](images/windows-clion-trust.png)

Once the project is open, you should see "Open Project Wizard" window. Select the "Visual Studio" toolchain.
Choose the `amd64` (nor ARM!) architecture. You will have to explicitly set the compiler to `clang-cl.exe` that
is located in a `x64` folder. The location should be the following (but may be different!):

```text
C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/Llvm/x64/bin/clang-cl.exe
```

Do this for both the C and the C++ compiler.

![](images/windows-clion-toolchain.png)

On the next page, select the Visual Studio toolchain and choose `Ninja` for the "Generator".
In the "CMake options" add in the following option:

```text
-DCMAKE_INSTALL_PREFIX=./install
```

```{hint}
Debug builds are useful for debugging, **but it makes the game slow**. This is especially noticable when
generating a new universe. It may take up to 10x longer. Prefer to use `Release` unless you need debugging support.
```

![](images/windows-clion-configure.png)

And finally click Finish.

```{warning}
This will download all of the C++ dependencies, compile them, and configure the game's source code.
**This may take up to 1 hour to complete and may take up to 3GB of disk space.** The dependencies are installed
locally within the build folder only.
```

Once configured, you should see something as "Build files have been written to: bla bla bla" in the "CMake" tab at
the bottom.

![](images/windows-clion-configured.png)

To build the game select the "TemporaryEscapeMain" from the dropdown in the top menu bar, and click the build icon next
to it.

![](images/windows-clion-build.png)

Once built you should see something as the following:

![](images/windows-clion-builddone.png)

To play and debug the game, you must first edit the configuration for the "TemporaryEscapeMain" target. Click the
down icon in the dropdown in the top menu bar, and select "Edit Configurations" from the dropdown.

![](images/windows-clion-editrun.png)

You must add a program argument `--root` followed by a space followed by the exact path to the source code of the game.

![](images/windows-clion-editargs.png)

And finally click play button (left icon). To run with a debugger click the second button (right icon).

```{warning}
When running the game in a singleplayer or multiplayer mode **for the first time from the source code**,
the game will compile shader code and will compress the textures from png format into ktx2 format.
**This may take several minutes.** This is done only once.
```

![](images/windows-clion-play.png)

The logs for the game can be found in the following path as listed below.
**The logs will not appear in the terminal output.**

```text
C:\Users\Username\AppData\Roaming\TemporaryEscape\TemporaryEscape.log
```

To run the game from command line, you must do it as the following:

```shell
# Run the game with --root argument pointing to the cloned repository!
./build/TemporaryEscape.exe --root C:/Projects/temporary-escape
```

## 4. Installing the game (optional)

To install the game, open a command line, go to the project folder, and type the following command below.
Make sure that you have used `-DCMAKE_INSTALL_PREFIX=/some/path` during the configuration. If you are using
a CMake preset (via `--preset`) then the install folder is going to be a directory named `install` inside the
project folder. It will be created during the install command as shown below.

```shell
# If you are using CLion the folder is most likely "cmake-build-debug-visual-studio" 
# or "cmake-build-release-visual-studio"
cmake --build ./build --target install
```

## 5. Creating an exe installer or a zip package (optional)

To create an installer`.exe` and `.zip` package, open a command line, go to the project folder, and type the following
command below.

```shell
# If you are using CLion the folder is most likely "cmake-build-debug-visual-studio" 
# or "cmake-build-release-visual-studio"
cmake --build ./build --target package
```

## 6. Vulkan validation layers (optional)

The validation layers are useful for debugging Vulkan issues. This is enabled at runtime and does not need re-compiling.
To use the validation layers you must first download and install Vulkan SDK into your system.

Download Vulkan SDK version `1.3.239.0` from
the [Vulkan SDK official download page](https://vulkan.lunarg.com/sdk/home#windows).
Or you can use
an [unofficial mirror link here](https://temporary-escape.us-east-1.linodeobjects.com/vulkan-sdk/VulkanSDK-1.3.239.0-Installer.exe).

You will also need to install the "VulkanRT" from the official download page. Or you can use
an [unofficial mirror link here](https://temporary-escape.us-east-1.linodeobjects.com/vulkan-sdk/VulkanRT-1.3.239.0-Installer.exe).

Install the SDK into your Windows, for example into the `C:\VulkanSDK\1.3.239.0` folder.

```shell
set VK_LAYER_PATH=C:\VulkanSDK\1.3.239.0\Bin
set VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation

# Then start the game
.\build\TemporaryEscape.exe --root C:/Projects/temporary-escape
```

```{note}
The game will pick up the `VK_LAYER_PATH` and will enable the validation layers when the game starts.
The game will stop and exit immediately if the validation layers can not be setup. This may
happen if your Vulkan SDK folder is wrong or the environment variables are wrong. Look into
the log file in your `C:\Users\Username\AppData\Roaming\TemporaryEscape` folder.
```

If you are using JetBrains CLion, you can modify the "Run Configuration" for the "TemporaryEscapeMain" target
(the dropdown in the top menu bar). Add these environment variables as shown below.

![](images/windows-clion-layers.png)

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
        "VK_LAYER_PATH=C:\\VulkanSDK\\1.3.239.0\\Bin",
        "VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation"
      ]
      // Other stuff here
    }
  ]
}
```
