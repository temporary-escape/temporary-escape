---
weight: 30
title: "C++ Dependencies"
---

# C++ Dependencies

In order to build this game from the source you will need the following tools:

* [git](https://git-scm.com/)
* Clang compiler with C++17 support or MSVC
* [vcpkg](https://vcpkg.io/en/index.html) latest version
* [cmake](https://cmake.org/) version 3.10 or higher
* Some additional OS dependencies

{{< hint warning >}}
**MinGW or Cygwin**  
Neither MinGW or Cygwin are officially supported. They might work but it is recommended to use Clang or MSVC.
{{< /hint >}}

{{< hint warning >}}
**Vcpkg must be installed through git**  
Vcpkg root folder must have a .git folder. Make sure that you have installed the tool via git clone. This is required because this project is using [vcpkg's versioning support](https://devblogs.microsoft.com/cppblog/take-control-of-your-vcpkg-dependencies-with-versioning-support/) which requires that the vcpkg has access to git's commits in its root directory.
{{< /hint >}}

## Linux (debian)

(Tested on Ubuntu 21.04)

You will need to install the following dependencies:

```bash
sudo apt-get update

# Development packages
sudo apt-get install -y \
    build-essential git cmake ninja-build curl \
    zip unzip tar clang pkg-config \
    libxinerama-dev libxcursor-dev \
    xorg-dev libglu1-mesa-dev liburing-dev

# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
sudo mv ./vcpkg /opt/vcpkg
sudo groupadd vcpkg
sudo usermod -aG vcpkg $USER # Adds the current user to the vcpkg group
sudo chown root:vcpkg -R /opt/vcpkg
sudo chmod 775 -R /opt/vcpkg
cd /opt/vcpkg && sudo ./bootstrap-vcpkg.sh
sudo ln -s /opt/vcpkg/vcpkg /usr/bin/vcpkg
```

## Linux (Arch)

(Tested on Manjaro 21.2.2)

You will need to install the following dependencies:

```bash
# Necessary system tools
sudo pacman -Syu

# Development packages
sudo pacman -Sy \
    git base-devel git cmake ninja \
    curl zip unzip tar clang

# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
sudo mv ./vcpkg /opt/vcpkg
sudo groupadd vcpkg
sudo usermod -aG vcpkg $USER # Adds the current user to the vcpkg group
sudo chown root:vcpkg -R /opt/vcpkg
sudo chmod 775 -R /opt/vcpkg
cd /opt/vcpkg && sudo ./bootstrap-vcpkg.sh
sudo ln -s /opt/vcpkg/vcpkg /usr/bin/vcpkg
```

## Windows 

(Tested on Windows 10 and Windows Server 2019)

**First download and install [Visual Studio](https://visualstudio.microsoft.com/vs/community/).**

Once installed, close it. Go to Start Menu and search for `Visual Studio Installer`.

{{< image url="contributing/setup/vs-installer-start.png" >}}

Once it opens, click "Modify" on the Visual Studio you have installed.

{{< image url="contributing/setup/vs-installer-modify.png" >}}

Once in the modify windows enable the Desktop development with C++. 

{{< image url="contributing/setup/vs-installer-modify-cpp.png" >}}

Next go to the "Individual Components" and search for word "Clang" and enable the "C++ Clang Compiler for Windows".

{{< image url="contributing/setup/vs-installer-modify-clang.png" >}}

Wait for it to download and install. Once it is done you can close the installer. Do not open Visual Studio yet.

Lastly you will have to download and install vcpkg. Use the following commands below.

```
cd C:\
mkdir tools
cd C:\tools\
git clone https://github.com/Microsoft/vcpkg.git
cd C:\tools\vcpkg\
bootstrap-vcpkg.bat
.\vcpkg integrate install
```

The `vcpkg integrate install` command will integrate vcpkg with Visual Studio.

Next, we will have to make sure that the `vcpkg.exe` executable is available to the entire system. Go to the Start Menu and search for "environment" and choose "Edit the system environment variables".

{{< image url="contributing/setup/vs-windows-environment.png" >}}

In the new windows click the "Environment Variables..." button.

{{< image url="contributing/setup/vs-windows-system-properties.png" >}}

Find the "PATH" variable in the "System variables" and edit it.

{{< image url="contributing/setup/vs-windows-environment-variables.png" >}}

Add the `C:\tools\vcpkg` path to the variable. This path may be different if you have chosen to install vcpkg to some other location.

{{< image url="contributing/setup/vs-windows-path.png" >}}

No additional dependencies are needed. Now you can [open the project with Visual Studio and configure CMake settings]({{< ref "setup/visual-studio.md" >}}).

## Mac OSX

TBA

```bash
brew install llvm cmake git ninja
```
