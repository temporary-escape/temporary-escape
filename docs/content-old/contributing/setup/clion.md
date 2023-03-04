---
weight: 3
title: "CLion"
---

# JetBrains CLion

This setup is suitable for Windows, Linux, or Mac OSX.

## 1. Dependencies

Make sure that you have installed [dependencies]({{< ref "../dependencies.md" >}}) and installed [CLion](https://www.jetbrains.com/clion/download/#section=linux).

## 2. Cloning

Clone `https://github.com/matusnovak/temporary-escape.git` (or your own fork) via command line or through the "Get from VCS" option within the IDE. You will also need to download the git submodules.

```bash
git clone https://github.com/matusnovak/temporary-escape.git
cd temporary-escape
git submodule update --init
```

## 3. Open the project

Launch CLion and open the cloned repository.

{{< image url="contributing/setup/clion-open.png" >}}

## 4. Open Project Wizard

Once you have openned the project, CLion will ask for CMake settings. You will see a new "Open Project Wizard" window.

**Choose "Ninja" as the "Generator".**

Set the **"CMake Options"** to the following below. Your `/opt/vcpkg` may differ depending where you have installed it.

```
-G Ninja -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake
```

{{< image url="contributing/setup/clion-open-project-wizard.png" >}}

Next, select the "Manage toolchains..." option.

{{< image url="contributing/setup/clion-manage-toolchains-link.png" >}}

In the new window, select Clang as the C and C++ compiler. If you are on Linux and don't know where Clang is, open a terminal and type `which clang`. Your Clang should be located at `/usr/bin/clang` and `/usr/bin/clang++`.

{{< image url="contributing/setup/clion-manage-toolchains.png" >}}

Click "OK" and then "OK" to close the wizard. CLion should now automatically configure the project.

After the configuration is done, you should see something like this in the "CMake" window at the bottom.

{{< image url="contributing/setup/clion-cmake-output.png" >}}

## 5. Launching

To launch the game you must select `TemporaryEscapeMain` target at the top menu bar.

{{< image url="contributing/setup/clion-select-target.png" >}}

Next, in the same menu, select the "Edit Configurations" option.

{{< image url="contributing/setup/clion-edit-configurations.png" >}}

In the new window, for TemporaryEscapeMain application, set `--root $PROJECT_DIR$` as the program argument. The `--root` argument is needed because by default the `TemporaryEscape` executable expects `assets` and `shaders` folders to be located exactly 1 folder above, which is not the case during the development work. 

Click "Apply" and "Ok".

{{< image url="contributing/setup/clion-run-configurations.png" >}}

And then press the "play" button.

{{< image url="contributing/setup/clion-target-run.png" >}}

CLion will now compile and run the game. You should see log output from the game in the "Run" window at the bottom of the IDE. Example below.

{{< image url="contributing/setup/clion-run-output.png" >}}
