# Flatpak PipeWire & PulseAudio Routing Architecture

This document details the sandboxing architecture, WirePlumber permissions model, and resolution for virtual audio routing in **Caster Soundboard** when running inside a Flatpak container.

---

## 1. Background & Problem Statement

Caster Soundboard provides low-level virtual audio routing:
* Creating virtual null sinks (`Caster_Sink`, `Caster_Monitor_Sink`)
* Creating virtual remap sources (`Caster_Mic`)
* Monitoring client playback streams via `pactl list sink-inputs`
* Remapping sink-inputs to output devices via `pactl move-sink-input`
* Managing system default input/output endpoints via `pactl set-default-sink` and `pactl set-default-source`

### The Issue in Flatpak
Under Flatpak on modern Linux distributions running PipeWire (`pipewire-pulse` and `wireplumber`), running `pactl` commands from within the container produced:
```text
Failure: Access denied (or pactl set-default-sink returning code 1)
```
While direct audio playback worked, all routing operations (null-sink creation, stream moving, and default source/sink switching) failed with "Access denied".

---

## 2. Root Cause Analysis

PipeWire and WirePlumber enforce security isolation for Flatpak sandboxes through the `flatpak_pm` access rule:
1. When a client connects to the PulseAudio emulation socket (`$XDG_RUNTIME_DIR/pulse/native`), PipeWire checks whether the client is inside a container (via `/.flatpak-info` and socket credentials).
2. Sandboxed clients are tagged as `flatpak` or `restricted`. Restricted clients are forbidden from:
   - Creating or unloading server modules (`load-module module-null-sink`, `load-module module-remap-source`)
   - Moving streams of other clients (`move-sink-input`)
   - Modifying server-wide default endpoints (`set-default-sink`, `set-default-source`)
3. WirePlumber inspects the connection properties. Specifically, if the client connection declares the property:
   ```properties
   media.category = "Manager"
   ```
   WirePlumber's policy recognizes the connection as an audio manager (like a volume mixer or patchbay) rather than a simple unprivileged media player, granting permission to execute management commands.

---

## 3. Implemented Solution

The solution requires two levels of configuration:

### A. Flatpak Manifest Permissions (`io.github.covarianttensor.CasterSoundboard.yml`)
1. **PulseAudio Compatibility Socket**:
   ```yaml
   finish-args:
     - --socket=pulseaudio
   ```
2. **Direct PipeWire Socket Access**:
   ```yaml
   finish-args:
     - --filesystem=xdg-run/pipewire-0:ro
   ```
3. **Manager Category Environment Variable**:
   ```yaml
   finish-args:
     - --env=PULSE_PROP=media.category=Manager
   ```

### B. C++ Process Environment Injection
Even when the environment variable is passed via Flatpak manifest, child processes spawned via `QProcess` must inherit this environment.

In [`src/main.cpp`](../../src/main.cpp):
```cpp
// Ensure WirePlumber and PipeWire grant audio manager permissions (essential for Flatpak sandbox)
if (qEnvironmentVariableIsEmpty("PULSE_PROP")) {
    qputenv("PULSE_PROP", "media.category=Manager");
} else {
    QByteArray prop = qgetenv("PULSE_PROP");
    if (!prop.contains("media.category")) {
        prop += " media.category=Manager";
        qputenv("PULSE_PROP", prop);
    }
}
```

In [`src/audio/PulseCompatBackend.cpp`](../../src/audio/PulseCompatBackend.cpp) and [`src/audio/CasterPlaybackRouter.cpp`](../../src/audio/CasterPlaybackRouter.cpp):
Every `QProcess` instance executing `pactl` inherits `QProcessEnvironment::systemEnvironment()` with `PULSE_PROP=media.category=Manager` guaranteed.

---

## 4. Verification Protocol

To verify that virtual audio routing permissions function inside the Flatpak sandbox:

```bash
# 1. Enter the Flatpak sandbox environment:
flatpak run --command=sh io.github.covarianttensor.CasterSoundboard

# 2. Test default endpoint query:
pactl info

# 3. Test virtual sink creation:
pactl load-module module-null-sink sink_name=test_verify_sink sink_properties=device.description=TestSink

# 4. Clean up module:
pactl unload-module test_verify_sink
```
All commands must exit with code `0` and zero "Access denied" errors.
