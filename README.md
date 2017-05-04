CasterSoundboard
================

A soundboard for hot-keying and playing back sounds. (For podcasting)

CasterSoundboard is written in C++ Qt5.

This is the development branch code.
=======

## How to Install

CasterSoundboard is not packaged for any platforms at this time see
Building from Source for installation instructions.

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
# dnf install qt-devel make gcc gcc-c++
```

*Debian and Ubuntu*
```
# apt install qtdeclarative5-dev qt5-default build-essentials
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

Then run make to build the project:

```
$ make
```

And finally run make install to install the app:

```
# make install
```

Now you're good to go!
