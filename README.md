

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
Download the pre-built `CasterSoundboard.flatpak` bundle from the [Releases page](https://github.com/covarianttensor/CasterSoundboard/releases/latest), then install and run:
```bash
flatpak install --user CasterSoundboard.flatpak
flatpak run io.github.covarianttensor.CasterSoundboard
```

Or build and install locally from source:
```bash
flatpak install flathub org.kde.Platform//6.8 org.kde.Sdk//6.8
flatpak-builder --user --install --force-clean build-flatpak io.github.covarianttensor.CasterSoundboard.yml
```

### AppImage (Standalone Portable)
Download the pre-built `CasterSoundboard-x86_64.AppImage` from the [Releases page](https://github.com/covarianttensor/CasterSoundboard/releases/latest), then make it executable and run:
```bash
chmod +x CasterSoundboard-x86_64.AppImage
./CasterSoundboard-x86_64.AppImage
```

Or build a standalone AppImage locally:
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

## Web Control

See [Web Control](docs/web-control.md) for functionality, setup and verification.
