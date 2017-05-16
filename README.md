CasterSoundboard
================

A soundboard for hot-keying and playing back sounds. (For podcasting)

CasterSoundboard is written in C++ Qt5.
=======

## Guides
### Control CasterSoundboard with your phone or tablet via a network:
[Click Here For Guide](https://github.com/JupiterBroadcasting/CasterSoundboard/wiki/Guide:-How-to-Control-CasterSoundboard-Using-Your-Phone-or-Tablet "CasterSoundboard's Wiki")

=======

## Libraries Used
### osc-cpp-qt
https://github.com/MugenSAS/osc-cpp-qt
---

## How to Install

CasterSoundboard is not packaged for any platforms at this time see
Building from Source for installation instructions.

## Packages
### Arch AUR
Thanks to git user [@j605](https://github.com/j605 "Jagannathan Tiruvallur Eachambadi") for creating a PKGBUILD for people on Arch who want to try this software:
https://aur.archlinux.org/packages/castersoundboard-git

### Building From Source

First clone the repository using git, if you don't have git installed
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

You should then have access to qmake, make, and the qt5 development
libraries. Next cd into the project folder inside the repo:

```
$ cd CasterSoundboard/CasterSoundboard
```

Now run qmake, this will auto generate a makefile:

```
$ qmake
```

or on Fedora:

```
$ qmake-qt5
```

Then run make to build the project:

```
$ make
```

And finally run make install to install the app:

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
### Screenshot ###
![Alt text](/docs/img/screenshot.png?raw=true "Running on Ubuntu")
