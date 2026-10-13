

CasterSoundboard
================
[![License](https://img.shields.io/github/license/JupiterBroadcasting/CasterSoundboard.svg)](https://github.com/JupiterBroadcasting/CasterSoundboard/blob/master/LICENSE)

A soundboard for hot-keying and playing back sounds. (For podcasting)

CasterSoundboard is written in C++ Qt6.
---

## Guides
+ [OSC colour and grid metadata API](docs/osc-metadata.md)
+ [Control CasterSoundboard with your phone or tablet via a network](https://github.com/covarianttensor/CasterSoundboard/wiki/Guide:-How-to-Control-CasterSoundboard-Using-Your-Phone-or-Tablet "CasterSoundboard's Wiki")
---

## Libraries Used
+ [osc-cpp-qt](https://github.com/MugenSAS/osc-cpp-qt)
---

## How to Install

### Flatpak (Universal Linux)
You can build and install the Flatpak on any Linux distribution with `flatpak` and `flatpak-builder`:
```bash
flatpak install flathub org.kde.Platform//6.8 org.kde.Sdk//6.8
flatpak-builder --user --install --force-clean build-flatpak io.github.covarianttensor.CasterSoundboard.yml
flatpak run io.github.covarianttensor.CasterSoundboard
```

### Nix
Build and run using `default.nix`:
```bash
nix-build
./result/bin/CasterSoundboard
```

### Arch AUR
Thanks to [@j605](https://github.com/j605 "Jagannathan Tiruvallur Eachambadi") for creating a `PKGBUILD` for people on Arch who want to try this software:
+ [CasterSoundboard-git](https://aur.archlinux.org/packages/castersoundboard-git)

### Fedora 25 and newer
RPM package can be found in the [Copr](https://copr.fedorainfracloud.org/coprs/fszymanski/castersoundboard/) repository:
```
# dnf copr enable fszymanski/castersoundboard
# dnf install castersoundboard
```

### Building From Source

First clone the repository using `git`, if you don't have `git` installed
you can find installation instructions [here](https://git-scm.com)

```
$ git clone https://github.com/covarianttensor/CasterSoundboard
```

Now we need to install the Qt 6 development libraries and tools, the
commands for various platforms are listed below:

*Arch / CachyOS*
```
# pacman -S qt6-base qt6-multimedia qt6-wayland ffmpeg base-devel
```

*Fedora*
```
# dnf install qt6-qtbase-devel qt6-qtmultimedia-devel make gcc gcc-c++
```

*Debian and Ubuntu*
```
# apt install qt6-base-dev qt6-multimedia-dev qmake6 build-essential
```

*Mac OSX*
```
# brew install qt
```

You should then have access to `qmake6` (or `qmake`), `make`, and the Qt 6 development
libraries. Next `cd` into the project folder inside the repo:

```
$ cd CasterSoundboard/CasterSoundboard
```

You can specify the target directory using the `PREFIX` `qmake` variable (default: `/usr/local`).
Now run `qmake6`, this will auto generate a makefile:

```
$ qmake6 PREFIX=/usr
```

Then run `make` to build the project:

```
$ make
```

And finally run `make install` to install the app:

```
# make install
```

Now you're good to go!

### Screenshot ###
![Alt text](/docs/img/screenshot.png?raw=true "Running on Ubuntu")
