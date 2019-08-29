

CasterSoundboard
================
[![Snap Status](https://build.snapcraft.io/badge/JupiterBroadcasting/CasterSoundboard.svg)](https://build.snapcraft.io/user/JupiterBroadcasting/CasterSoundboard)
[![License](https://img.shields.io/github/license/JupiterBroadcasting/CasterSoundboard.svg)](https://github.com/JupiterBroadcasting/CasterSoundboard/blob/master/LICENSE)

A soundboard for hot-keying and playing back sounds. (For podcasting)

CasterSoundboard is written in C++ Qt5.
---

## Guides
+ [Control CasterSoundboard with your phone or tablet via a network](https://github.com/JupiterBroadcasting/CasterSoundboard/wiki/Guide:-How-to-Control-CasterSoundboard-Using-Your-Phone-or-Tablet "CasterSoundboard's Wiki")
---

## Libraries Used
+ [osc-cpp-qt](https://github.com/MugenSAS/osc-cpp-qt)
---

## How to Install

### Snap

Install CasterSoundboard in seconds on [Ubuntu and other snap supported Linux 
distributions](https://snapcraft.io/docs/core/install) with:

    snap install castersoundboard

Installing a snap is very quick. Snaps are secure. They are isolated with all 
of their dependencies. Snaps also auto update when a new version is released.

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
$ git clone https://github.com/JupiterBroadcasting/CasterSoundboard
```

Now we need to install the qt5 development libraries and tools, the
commands for various platforms are listed below:

*Fedora*
```
# dnf install qt5-devel make gcc gcc-c++
```

*Debian and Ubuntu*
```
# apt install qtdeclarative5-dev qt5-default qtmultimedia5-dev build-essential
```

*Mac OSX*
```
# brew install qt
```

You should then have access to `qmake`, `make`, and the qt5 development
libraries. Next `cd` into the project folder inside the repo:

```
$ cd CasterSoundboard/CasterSoundboard
```

You can specify the target directory using the `PREFIX` `qmake` variable (default: `/usr/local`).
Now run `qmake`, this will auto generate a makefile:

```
$ qmake PREFIX=/usr
```

or on Fedora:

```
$ qmake-qt5 PREFIX=/usr
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

### Troubleshooting ###
Problem: On Ubuntu you get error:
```
defaultServiceProvider::requestService(): no service found for - "org.qt-project.qt.mediaplayer"
```
Solution: Install missing qt5 multimedia plugins
```
sudo apt-get install libqt5multimedia5-plugins
```
### Screenshots ###
![Alt text](/docs/img/screenshot_1.png?raw=true "Running on OS X. Development version.")
![Alt text](/docs/img/screenshot_2.png?raw=true "Running on OS X. Development version.")
