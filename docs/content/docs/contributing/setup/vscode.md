---
weight: 2
title: "VS Code"
---

# Visual Studio Code

This setup is suitable for Windows, Linux, or Mac OSX.

## 1. Dependencies

Make sure that you have installed [dependencies]({{< ref "../dependencies.md" >}}) and installed [VSCode](https://code.visualstudio.com/).

## 2. Cloning

Clone `https://github.com/matusnovak/temporary-escape.git` (or your own fork) via command line or through the GitHub VSCode extension. Open the `temporary-folder` through VS Code. You will also need to download the git submodules.

```bash
git clone https://github.com/matusnovak/temporary-escape.git
cd temporary-escape
git submodule update --init
```

## 3. Settings

You will need to set some CMake/C++ related settings in VSCode for this project. Create a `.vscode` folder within the project root folder and add add a file `settings.json` with the following contents below. Your `/opt/vcpkg` may differ, depending on where you have installed your vcpkg.

```json
{
    "cmake.configureOnOpen": false,
    "cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake"
    },
    "[cpp]": {
        "editor.defaultFormatter": "ms-vscode.cpptools",
        "editor.formatOnSave": true
    }
}
```

## 4. Selecting kit

Press \[Ctrl\] + \[P\] and type `> cmake kit`. It should bring up the following options. Choose `CMake: Select a kit`.

{{< image url="contributing/setup/vscode-cmake-kit-select.png" >}}

And then select Clang

{{< image url="contributing/setup/vscode-cmake-kit-clang.png" >}}

## 5. Configure

Press \[Ctrl\] + \[P\] and type `> cmake configure` and select the first option. This may take few minutes to complete. CMake and vcpkg will download and install all the required C++ libraries.

{{< image url="contributing/setup/vscode-cmake-configure.png" >}}

After a successful configuration you should see the following in the "Output" window:

```
[cmake] -- Configuring done
[cmake] -- Generating done
[cmake] -- Build files have been written to: /home/mnovak/Projects/temporary-escape/build
```

## 6. Build

Press \[Ctrl\] + \[P\] and type `> cmake set build target`.

{{< image url="contributing/setup/vscode-cmake-build-target-select.png" >}}

Choose `TemporaryEscapeMain` from the selection.

{{< image url="contributing/setup/vscode-cmake-build-target.png" >}}

And finally do the same thing for `> cmake build`.

{{< image url="contributing/setup/vscode-cmake-build.png" >}}

You should see the following in the "Output" window:

```
[build] [...] Linking CXX executable src/TemporaryEscape/TemporaryEscape
[build] Build finished with exit code 0
```

## 7. Launch settings

Before you can run and debug this game, you will need to create launch configuration. In the `.vscode` folder (in project's root folder) create a file `launch.json` with the following contents below. This is a minimal configuration that you will need. The `--root` argument is needed because by default the `TemporaryEscape` executable expects `assets` and `shaders` folders to be located exactly 1 folder above, which is not the case during the development work. Using `--root ${workspaceFolder}` fixes any issues.

```json
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(gdb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "program": "${command:cmake.launchTargetPath}",
            "args": [
                "--root",
                "${workspaceFolder}"
            ],
            "stopAtEntry": false,
            "cwd": "${command:cmake.launchTargetDirectory}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "CMake Build"
        }
    ]
}
```

Additionally, create `tasks.json` within the same `.vscode` folder with the following contents below.

```json
{
    // See https://go.microsoft.com/fwlink/?LinkId=733558
    // for the documentation about the tasks.json format
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake Build",
            "type": "cmake",
            "command": "build"
        }
    ]
}
```

## 8. Debugging

Now you can simply run it through the GUI by selecting Run -> Start Debugging from the top main bar, or by pressing \[Ctrl\] + \[P\] and selecting `> Debug: Start Debugging`.

{{< image url="contributing/setup/vscode-start-debugging.png" >}}

This will automatically compile any changes you have made. After the compilation is done, the game should open and you should see a new window. Once you close the game, you should see "has exited with code 0" in the "Debug Console".

Here is an example of "Debug Console" where you would find stacktraces if the game would crash.

{{< image url="contributing/setup/vscode-debug-console.png" >}}

Switch to the "Terminal" to see the logging output.

{{< image url="contributing/setup/vscode-terminal-logs.png" >}}

That's all.
