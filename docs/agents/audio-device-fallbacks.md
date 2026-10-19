# Audio Device Naming, Enumeration & Fallbacks

This document outlines the audio hardware enumeration quirks encountered across Linux PulseAudio and PipeWire subsystems, and the fallback logic implemented in **Caster Soundboard**.

---

## 1. Background

Caster Soundboard parses audio endpoints by querying `pactl -f json list cards`, `sinks`, and `sources` in [`src/audio/PulseCompatBackend.cpp`](../../src/audio/PulseCompatBackend.cpp).

Endpoints are paired between:
* Card properties (`device.description`, `device.vendor.name`, `alsa.card_name`)
* Port definitions (`description`, `direction`, `availability`)
* Profile membership (`sources`, `sinks`, active profiles)

---

## 2. Real-World Hardware Quirks

### A. Whitespace-Only `device.description` Strings
Certain USB audio interfaces (such as OEM DACs and microphones from manufacturers like Beibinghe Technology) report a non-empty string consisting purely of whitespace characters (e.g. `' '`) for `device.description` in ALSA/Udev properties:

```json
"properties": {
    "device.description": " ",
    "device.vendor.name": "Beibinghe Technology",
    "alsa.card_name": ""
}
```

If an application performs a simple `!value.isEmpty()` check, the whitespace string evaluates to true, resulting in UI labels with awkward leading spaces (e.g., `"  — Microphone"`).

### B. Fallback Hierarchy
In [`src/audio/PulseCompatBackend.cpp`](../../src/audio/PulseCompatBackend.cpp), the card descriptor is now evaluated using `.trimmed()` across a strict priority waterfall:

```cpp
auto cProps = c.value("properties").toObject();
QString cardDesc = cProps.value("device.description").toString().trimmed();
if (cardDesc.isEmpty())
    cardDesc = cProps.value("device.vendor.name").toString().trimmed();
if (cardDesc.isEmpty())
    cardDesc = cProps.value("alsa.card_name").toString().trimmed();
if (cardDesc.isEmpty())
    cardDesc = card;

QString portDesc = port.value("description").toString(p.key()).trimmed();
if (portDesc.isEmpty())
    portDesc = p.key();

e.label = cardDesc + " — " + portDesc;
```

### C. Idle vs. Broken Endpoints
PipeWire marks audio sinks and sources as `SUSPENDED` when no active audio streams are playing or recording. 
* **`SUSPENDED` is normal behavior**: It is a power-saving idle state, not a device failure.
* An endpoint is only flagged as unhealthy (`healthWarning`) if:
  1. The card profile is set to `"off"`.
  2. The active profile advertises the endpoint but no matching PipeWire node is realized in the audio graph.

---

## 3. Best Practices for Agents

1. **Always `.trimmed()` device and port descriptors**: Never assume hardware or driver metadata is trimmed.
2. **Never treat `SUSPENDED` as an error**: Only check card profile state and node presence to determine device health.
3. **Keep endpoint label IDs unique and stable**: Endpoints rely on `card|direction|port` for persistent profile assignment across restarts.
