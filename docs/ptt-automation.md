# Push-to-Talk (PTT) & Mic Automation Guide

Caster Soundboard provides built-in hardware-level Push-to-Talk (PTT) and Microphone Mute automation for live broadcasts, Discord voice channels, streaming, and gaming.

---

## 1. Overview & Architectural Role

When playing sound clips through a virtual microphone into voice chat applications like Discord, TeamSpeak, OBS Studio, or multiplayer games, you typically need to:
1. Hold down your voice chat Push-to-Talk hotkey (e.g. `V` or `F24`) so listeners can hear the audio, OR
2. Unmute your microphone before the sound plays and re-mute it after it finishes.

Caster Soundboard automates this entire lifecycle using the Linux kernel **`uinput`** subsystem, eliminating manual key holding and preventing clipped sound starts or endings.

*(Note: In legacy v1 profiles and early development, this capability was briefly referred to as "Actions". It is now standardized throughout the engine, UI, and project serialization as **Push-to-Talk (PTT)**).*

---

## 2. Configuration & Modes

### Opening the PTT Dialog
- Click the **`🎙️ PTT`** button in the top toolbar.
- Right-click any sound bank tab in the tab bar and select **"Configure Tab PTT..."**.
- Launch with CLI testing flag: `./CasterSoundboard --open-ptt-prompt`.

### Operation Modes
Each PTT profile supports three distinct operational modes:

| Mode | Behavior | Best Used For |
| :--- | :--- | :--- |
| **Push-to-Talk (Hold)** | Holds down the specified key when audio begins and releases it when audio terminates. | Standard Discord or in-game PTT setups. |
| **Mute Toggle (Tap)** | Sends a keypress before playback begins (to unmute), and sends another keypress after playback completes (to re-mute). | Open-mic users who stay muted until a sound effect plays. |
| **Disabled (None)** | No keystrokes are injected. Sounds play through audio devices without triggering keyboard events. | Background music, local monitoring, or tabs that don't need PTT. |

### Lead-In & Tail Delay Buffers (Anti-Clipping)
- **Audio Lead-In Buffer (0–500 ms)**: Delays sound playback slightly after keying up PTT. This gives Discord and game voice engines time to open the VoIP channel before the first audio frame plays, eliminating chopped intros.
- **PTT Release Delay (0–1000 ms)**: Keeps the PTT key held down for a short period after the audio stream finishes to ensure natural room reverb and speech tails are not prematurely cut off.

---

## 3. Tab Assignment & Real-Time Indicators

- **Tab Association**: PTT profiles can be pinned to specific tabs or shared across tabs.
- **Soundboard Tab Indicators**:
  - **Idle**: Displays a microphone badge with the assigned key (e.g., `🎙️ PTT: KEY_V`).
  - **Transmitting**: Glows with an emerald pulsing halo and badge (`🟢 TRANSMITTING (KEY_V)`) while audio is playing.
- **Top Bar PTT Button**: Shows active transmission status in real-time.
- **Web Remote Control**: Mobile devices connected via Wi-Fi display synchronized PTT indicators (`🎙️ PTT` / `🟢 PTT`) on every tab.

---

## 4. Linux `/dev/uinput` Driver Requirements

Caster Soundboard uses the Linux kernel `uinput` virtual device driver (`UInputChannel.cpp`) to create a virtual hardware keyboard. This ensures keys register reliably inside full-screen games and sandboxed Flatpak apps (like Discord Flatpak) even when Caster Soundboard does not have window focus.

### Enabling Access to `/dev/uinput`
Most modern Linux distributions (Arch, Fedora, Ubuntu, Mint, Debian) restrict `/dev/uinput` to the `input` group or root. To enable access for your user account without requiring root:

1. Add your user to the `input` group:
   ```bash
   sudo usermod -aG input $USER
   ```
2. Verify or add a udev rule (typically preconfigured by `systemd` or `libinput`):
   ```bash
   echo 'KERNEL=="uinput", MODE="0660", GROUP="input", OPTIONS+="static_node=uinput"' | sudo tee /etc/udev/rules.d/99-uinput.rules
   sudo udevadm control --reload-rules && sudo udevadm trigger
   ```
3. Log out and back into your desktop session for the group membership to take effect.
4. Verify accessibility in Caster Soundboard's **About & System Information** view (view index 5):
   - **PTT Automation Driver:** `✓ Ready (/dev/uinput virtual keyboard)`.
