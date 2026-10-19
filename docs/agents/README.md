# AI Agent Architecture & Knowledge Base Index

Welcome to the internal engineering and architectural knowledge base for **Caster Soundboard**. This directory houses detailed technical investigations, subsystem post-mortems, and architectural blueprints for AI coding agents and human contributors.

---

## 📚 Knowledge Base Index

| Document | Topic & Focus Area | Key Architectural Findings |
| :--- | :--- | :--- |
| [**Flatpak PipeWire & PulseAudio Routing**](flatpak-pipewire-routing.md) | Linux sandboxing, WirePlumber permissions, and virtual routing | Resolves `Failure: Access denied` on `pactl set-default-sink` and module creation inside Flatpak by declaring `media.category = "Manager"` via `PULSE_PROP` and sandbox socket permissions. |
| [**QML ComboBox Dropdowns & Desktop Theme Isolation**](qml-combobox-and-desktop-styling.md) | Touch-first QML UI, KDE/Plasma Breeze overrides, and popup lifecycle | Solves missing/blank items in ComboBox dropdowns under KDE. Enforces `QQuickStyle::setStyle("Basic")` and documents the pure `Rectangle` + `HoverHandler` + `TapHandler` delegate pattern. |
| [**Audio Device Naming & Fallbacks**](audio-device-fallbacks.md) | Audio hardware quirks, ALSA/Udev descriptors, and PipeWire states | Explains whitespace-only `device.description` handling, fallback hierarchy (`vendor.name`, `alsa.card_name`), and why PipeWire `SUSPENDED` states are healthy idle conditions. |

---

## 🧭 How to Use This Knowledge Base

1. **Before Modifying Audio Code**:
   - Review [Flatpak PipeWire & PulseAudio Routing](flatpak-pipewire-routing.md) before altering `PulseCompatBackend.cpp`, `CasterPlaybackRouter.cpp`, or `io.github.covarianttensor.CasterSoundboard.yml`.
   - Review [Audio Device Naming & Fallbacks](audio-device-fallbacks.md) before altering device discovery or label construction.

2. **Before Modifying UI & QML Controls**:
   - Review [QML ComboBox Dropdowns & Desktop Theme Isolation](qml-combobox-and-desktop-styling.md) before modifying any `ComboBox`, `ItemDelegate`, or popup components.

3. **Contributing New Knowledge**:
   - When solving complex cross-platform edge cases (Flatpak, NixOS, AppImage, Wayland, PipeWire, QQC2), author a focused markdown guide in this directory and register it in `INDEX.md`.

---

## 🔗 Related Reference Documents

* [**AGENTS.md**](../../AGENTS.md) — Primary contributor guide, build commands, and testing guardrails.
* [**README.md**](../../README.md) — User-facing application overview and feature set.
* [**CONTRIBUTING.md**](../../CONTRIBUTING.md) — Open source development and pull request guidelines.
