

CasterSoundboard
================
[![License](https://img.shields.io/github/license/covarianttensor/CasterSoundboard.svg)](https://github.com/covarianttensor/CasterSoundboard/blob/master/LICENSE)

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

### AppImage
Build a standalone `CasterSoundboard-x86_64.AppImage` locally using:
```bash
./scripts/build-appimage.sh
```

### Nix
Build and run using `default.nix`:
```bash
nix-build
./result/bin/CasterSoundboard
```

### Screenshot ###
![Alt text](/docs/img/screenshot.png?raw=true "Running on Ubuntu")
