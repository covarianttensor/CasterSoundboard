# AI Agent Contributor Guide: Caster Soundboard

This document provides architectural context, technical guardrails, build protocols, and quality standards for AI coding agents contributing to **Caster Soundboard**.

All AI agents working on this repository **must read and adhere to this guide** to maintain system stability, cross-platform packaging integrity, and UI consistency.

---

## 1. Project Mission & Technology Stack

Caster Soundboard is a high-performance, touch-first soundboard and live audio routing tool for Linux (Wayland and X11).

* **Core Language:** C++17
* **Framework:** Qt 6.8+ (Core, Gui, Qml, Quick, QuickControls2, Multimedia, Network, Test)
* **UI Layer:** Declarative QML (Touch-first, responsive grid, zero legacy Qt Widgets in the UI)
* **Build System:** **CMake** (`CMakeLists.txt` is the single source of truth across all targets)
* **Audio Subsystem:** Direct PipeWire & PulseAudio integration via `pactl` remap-source and null-sinks
* **Primary Git Branch:** `main` (The legacy v1 Qt Widgets codebase is archived in `legacy-v1` — do **not** reintroduce widgets code)

---

## 2. Directory Layout & Architecture

```
CasterSoundboard/
├── CMakeLists.txt             # Single build definition for Native, AppImage, Flatpak & Nix
├── AGENTS.md                  # This file
├── README.md                  # User-facing project documentation
├── default.nix & flake.nix    # NixOS package derivation & flake
├── io.github.covarianttensor.CasterSoundboard.yml # Flatpak manifest
├── dist/linux/                # AppStream metainfo, .desktop file, and 256x256 & scalable SVG app icons
├── docs/ & examples/          # Sample .caster profiles and documentation
├── TouchOSC-Layouts/          # TouchOSC layout templates
├── scripts/
│   ├── build-appimage.sh      # Standalone AppImage compilation script
│   ├── build-flatpak.sh       # Flatpak bundle compilation script
│   ├── test-web-remote.js     # Headless Chromium E2E touch interaction test suite
│   └── caster-mcp.js          # Stdio-to-HTTP/SSE bridge for Model Context Protocol (MCP)
├── src/
│   ├── main.cpp               # QML engine bootstrapper, CLI parser & hotkey filter
│   ├── audio/                 # Low-level PipeWire/PulseAudio virtual routing backend
│   │   ├── PulseCompatBackend.cpp/.h      # Virtual sink/source management via pactl
│   │   ├── CasterPlaybackRouter.cpp/.h    # Sink-input monitoring & stream remapping
│   │   └── AudioDeviceController.cpp/.h   # Hardware audio endpoint discovery
│   ├── core/                  # Headless C++ business engine (CasterCore static library)
│   │   ├── SoundboardProjectController.*  # Workspace serialization & app coordinator
│   │   ├── SoundboardTabModel.*           # Multi-tab data model
│   │   ├── SoundboardGridModel.*          # Sound tile grid container
│   │   ├── SoundTileController.*          # Media playback, volume, looping & cue points
│   │   ├── PttController.*                # Push-to-Talk (PTT) & mic automation manager
│   │   ├── AudioRoutingController.*       # High-level QML-to-audio-backend bridge
│   │   ├── HotkeyManager.*                # Keyboard shortcut dispatcher & Bank Mode
│   │   ├── WebRemoteController.*          # Local HTTP/AJAX web remote control server
│   │   ├── McpServerController.*          # Model Context Protocol (MCP) AI server (HTTP/SSE & stdio)
│   │   └── OscServerController.*          # Two-way UDP Open Sound Control server
│   ├── qml/                   # Modern QML touch UI
│   │   ├── Main.qml                       # Root application shell & window geometry
│   │   ├── NavSidebar.qml                 # Primary left navigation sidebar
│   │   ├── TopActionBar.qml               # Global action buttons (Stop All, Ducking)
│   │   ├── SoundboardTabBar.qml           # Sound bank tabs with hotkey indicators
│   │   ├── SoundTile.qml                  # Media tile component with waveform sweep
│   │   ├── SoundTileDrawer.qml            # Tile settings dialog with cue waveform editor
│   │   ├── PttDialog.qml                  # Push-to-Talk (PTT) & mic automation dialog
│   │   └── views/
│   │       ├── SoundboardsView.qml        # Resizable sound tile grid view
│   │       ├── AudioRoutingView.qml       # Orthogonal schematic audio routing flow
│   │       ├── OscView.qml                # Network OSC configuration
│   │       ├── PreferencesView.qml        # Workspace settings & ducking attenuation
│   │       ├── DocumentationView.qml      # Embedded user & CLI guide
│   │       └── AboutView.qml              # App version, project metrics & system diagnostics
│   └── web/                   # Static HTML/JS for local Web Remote control surface
└── tests/                     # Automated unit test suite (C++ QtTest)
```

---

## 3. One-Command Build & Verification Protocol

When making modifications, agents must verify both compilation and unit tests:

```bash
# 1. Configure and build
cmake -B build-cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build-cmake -j$(nproc)

# 2. Run all unit tests (must pass 100%)
ctest --test-dir build-cmake --output-on-failure
```

### 🧪 Required Testing Tools & Environment

To execute all verification tiers (C++ QtTest unit tests, headless QML screenshots, PipeWire audio routing, and Web Remote touch automation), the development host requires:

| Tool | Purpose | Arch / CachyOS | Debian / Ubuntu / Mint | NixOS Flake Attribute |
| :--- | :--- | :--- | :--- | :--- |
| **CMake & Ninja** | Build configuration & compilation | `cmake ninja` | `cmake ninja-build` | `pkgs.cmake pkgs.ninja` |
| **Qt 6.8+ Development** | C++ core, QML, multimedia, and QtTest | `qt6-base qt6-declarative qt6-multimedia` | `qt6-base-dev qt6-declarative-dev qt6-multimedia-dev` | `pkgs.qt6.qtbase pkgs.qt6.qtdeclarative` |
| **pactl & PipeWire** | Virtual audio routing, null-sinks, and stream remapping | `libpulse pipewire-pulse` | `pulseaudio-utils pipewire-pulse` | `pkgs.pulseaudio pkgs.pipewire` |
| **FFmpeg** | Waveform PCM downsampling for MP3, OGG, FLAC, M4A | `ffmpeg` | `ffmpeg` | `pkgs.ffmpeg` |
| **Chromium** | Headless browser execution for Web Remote UI testing | `chromium` | `chromium-browser` | `pkgs.chromium` |
| **Node.js & npm** | Scripting runner for headless web test automation | `nodejs npm` | `nodejs npm` | `pkgs.nodejs` |
| **puppeteer-core** | Lightweight CDP client driving system Chromium | `npm install puppeteer-core` | `npm install puppeteer-core` | `npm install puppeteer-core` |

### Visual Verification (Headless Screenshots)
Caster Soundboard provides built-in CLI flags to capture screenshots without needing a display server:
```bash
# Capture main soundboard view:
./build-cmake/bin/CasterSoundboard -s /tmp/screenshot_main.png

# Capture specific view (0=Soundboard, 1=Audio, 2=OSC, 3=Settings, 4=Docs, 5=About):
./build-cmake/bin/CasterSoundboard --view 1 -s /tmp/screenshot_audio.png

# Simulate active routing state for audio diagram verification:
./build-cmake/bin/CasterSoundboard --view 1 --test-routing-active -s /tmp/screenshot_routing.png
```

### 🌐 Web Remote E2E & Touch Verification (Headless Browser)
When modifying [`src/web/index.html`](src/web/index.html) or [`WebRemoteController.cpp`](src/core/WebRemoteController.cpp), verify touch interaction, event delegation, and real-time state synchronization using headless Chromium:

```bash
# 1. Start CasterSoundboard in the background with web remote and sample project:
./build-cmake/bin/CasterSoundboard --auto-start-web examples/broadcast_show.caster &
APP_PID=$!

# 2. Run the automated touch & interaction test suite via Node.js:
# (Verifies sound tile touch taps, PTT glow badges, Panic Stop All, tab switching, and Ducking)
node scripts/test-web-remote.js

# 3. Terminate background instance:
kill $APP_PID
```

### 🤖 Model Context Protocol (MCP) Verification
When modifying [`McpServerController.cpp`](src/core/McpServerController.cpp) or [`scripts/caster-mcp.js`](scripts/caster-mcp.js), verify HTTP/SSE and stdio JSON-RPC 2.0 communication:

```bash
# 1. Start CasterSoundboard in the background with MCP auto-started:
./build-cmake/bin/CasterSoundboard --auto-start-mcp examples/broadcast_show.caster &
APP_PID=$!

# 2. Verify HTTP health / status:
curl -s http://127.0.0.1:5055/status

# 3. Verify JSON-RPC tool listing:
curl -s -X POST http://127.0.0.1:5055/message -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":1,"method":"tools/list","params":{}}'

# 4. Verify stdio bridge execution:
echo '{"jsonrpc":"2.0","id":1,"method":"tools/list","params":{}}' | node scripts/caster-mcp.js

# 5. Terminate background instance:
kill $APP_PID
```

---

## 4. Technical Guardrails & Coding Standards

### 🛡️ Audio & Container Awareness (Flatpak & NixOS)
1. **Never Hardcode Fixed Binary Paths**:
   - Never call `/usr/bin/pactl`, `/usr/bin/ffmpeg`, or `/usr/bin/systemctl`.
   - Always search `$PATH` via `QStandardPaths::findExecutable("tool_name")`. This guarantees compatibility on **NixOS** where binaries live in `/nix/store/...`.
2. **Transparent Flatpak Host-Spawning**:
   - When running inside Flatpak, check `QFile::exists("/.flatpak-info")`.
   - If true, route host management commands (`pactl`, `systemctl`) via `flatpak-spawn --host <command> <args>`.
3. **Multi-Codec Waveforms**:
   - In [`SoundTileController.cpp`](src/core/SoundTileController.cpp), WAV files are parsed directly for speed. All other formats (MP3, OGG, FLAC, M4A) decode downsampled PCM via `ffmpeg`. Always support Flatpak host-spawn fallback if `ffmpeg` is not in the sandbox.

### 🎨 QML UI & Ergonomics
1. **Never Intercept Pointer Events with Container `MouseArea`**:
   - If a card or panel needs hover feedback or cursor highlighting, **always use `HoverHandler`** (`HoverHandler { id: cardHover }`).
   - Never stretch an invisible `MouseArea` across a card, as it will swallow clicks intended for child `ComboBox`, `TextField`, or `Button` elements.
2. **Dimmed Elements Must Light Up on Hover and Focus**:
   - Inactive audio pathways or controls that are dimmed (`opacity: 0.35`–`0.45`) must smoothly illuminate to full brightness (`opacity: 1.0`, accented colored borders, active connector dashes) when hovered or when child inputs are focused (`activeFocus` or `popup.visible`).
3. **Never Steal Hotkeys While Typing**:
   - Input controls (`TextField`, `DarkTextField`) must isolate keystrokes. In [`src/main.cpp`](src/main.cpp), `GlobalHotkeyFilter` checks `isTextInputObject(obj)` to prevent hotkeys (like `Space` for Stop All or letters for tiles) from triggering while typing.
4. **Theme Consistency**:
   - Canvas / View Background: `#181A1B`
   - Card Background: `#222528` / `#1E2023` (border `#33373B`)
   - Inputs Background: `#282C32` (hover `#2D3238`, activeFocus `#30353C`, focus border `#8E2DE2`)
   - Violet Primary Accent: `#8E2DE2`
   - Active / Success Emerald: `#10B981` / `#2ECC71`
   - Warning Amber: `#F59E0B`
   - Panic / Stop Red: `#E74C3C` / `#A93226`
   - Hardware Microphone Blue: `#3B82F6`

### 📦 Multi-Target Packaging Guardrails
Any modification must preserve functionality across all four distribution formats:
* **Native**: Handled by standard `CMakeLists.txt` `install()` rules.
* **AppImage**: Built via `scripts/build-appimage.sh` using `linuxdeploy` and `linuxdeploy-plugin-qt`.
* **Flatpak**: Manifest `io.github.covarianttensor.CasterSoundboard.yml` must use `buildsystem: cmake-ninja`.
* **NixOS**: Maintained in `default.nix` and `flake.nix` using `qt6.wrapQtAppsHook`.

---

## 5. Agent Verification Checklist

Before concluding your task, verify the following:
* [ ] The project builds with zero compilation errors (`cmake --build build-cmake`).
* [ ] All 10 unit tests pass (`ctest --test-dir build-cmake`).
* [ ] Web Remote changes verified with headless browser touch automation (`node scripts/test-web-remote.js`).
* [ ] No existing comments or docstrings were unnecessarily removed.
* [ ] Changes are committed to `main` (or a task branch off `main`), never `legacy-v1`.
* [ ] Commit messages follow the conventional commit standard (`feat:`, `fix:`, `refactor:`, `test:`, `docs:`).
