---
weight: 4
title: "Visual Studio"
---

# Microsoft Visual Studio

This setup is available only for Windows.

## 1. Dependencies

Make sure that you have installed [dependencies]({{< ref "../dependencies.md" >}}) and installed [Visual Studio](https://visualstudio.microsoft.com/vs/community/).

## 2. Cloning

Clone `https://github.com/matusnovak/temporary-escape.git` (or your own fork) via command line or through the Visual Studio. You will also need to download the git submodules.

```bash
git clone https://github.com/matusnovak/temporary-escape.git
cd temporary-escape
git submodule update --init
```

## 3. Open the project

Open Visual Studio and open the cloned repository by choosing "Open a local folder" option.

{{< image url="contributing/setup/vs-open-local.png" >}}

Then **wait** until the CMake configuration has finished.

{{< image url="contributing/setup/vs-generation-finished.png" >}}

## 4. CMake settings

Once the configuration has completed, go to the Main menu bar -> Project -> **CMake Settings for TemporaryEscape**.

{{< image url="contributing/setup/vs-configure.png" >}}

In the CMake settings select `clang_cl_x64_x64` as the toolset.

{{< hint info >}}
**MSVC or Clang?**  
Both will work fine but prefer to use Clang. We use Clang to compile this game for Linux and Mac OSX so we should use it for Windows as well.
{{< /hint >}}

{{< image url="contributing/setup/vs-cmake-toolset.png" >}}

In the options below, scroll down to `TEMPORARY_ESCAPE_BUILD_TESTS` and set it to enabled.

{{< image url="contributing/setup/vs-cmake-options.png" >}}

Once you are done don't forget to save the settings.

{{< image url="contributing/setup/vs-cmake-save.png" >}}

The project should now reconfigure and you should see CMake successful output yet again.

## 6. Select build target

In the top menu bar select the `TemporaryEscape.exe` target, not the install one. **Don not compile or run/debug the game yet.**

{{< image url="contributing/setup/vs-target-dropdown.png" >}}
{{< image url="contributing/setup/vs-target-select.png" >}}

## 7. Debug options

Next, go to the Main menu bar -> Debug -> **Debug and Launch Settings for TemporaryEscapeMain.**

{{< image url="contributing/setup/vs-debug-settings.png" >}}

A new file named `launch.vs.json` should open with some default configuration. You will need to a add command line argument `--root ${workspaceRoot}` into the configuration. It should look like this:

```json
{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
      "type": "default",
      "project": "CMakeLists.txt",
      "projectTarget": "TemporaryEscape.exe (src\\TemporaryEscape\\TemporaryEscape.exe)",
      "name": "TemporaryEscape.exe (src\\TemporaryEscape\\TemporaryEscape.exe)",
      "args": ["--root", "${workspaceRoot}"]
    }
  ]
}
```

Save the file.

## 8. Compile, run, and debug

Finally, simply press the play button at the top menu bar.

{{< image url="contributing/setup/vs-target-play.png" >}}
