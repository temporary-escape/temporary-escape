---
weight: 31
title: "C++ Dependencies"
---

# C++ Dependencies

In order to build this game from the source you will need the following tools:

* Clang compiler with C++17 support or MSVC
* [vcpkg](https://vcpkg.io/en/index.html) latest version
* [cmake](https://cmake.org/) version 3.10 or higher
* Some additional OS dependencies

{{< hint warning >}}
**MinGW or Cygwin**  
Neither MinGW or Cygwin are officially supported. They might work but it is recommended to use Clang or MSVC.
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

# Install vcpkg from https://aur.archlinux.org/packages/vcpkg-git/
sudo pamac build vcpkg-git

# Or use a manuall setup:
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

TBA

## Mac OSX

TBA
