# Web Control

Open **Web Server Settings** on the toolbar, set a listening IP, port, user and
password, then Start. The default address `0.0.0.0` listens on all IPv4 interfaces
and the default port is 5052. Open `http://<computer-address>:5052` on another
device and authenticate with those credentials. Empty credentials are rejected.

The page displays loaded sounds from every tab in their actual grid positions,
with labels and colours. Press a sound to toggle playback. Server-sent events
update playback states, errors, colours, layouts and tab changes. Empty slots
remain visually empty. Stable tab identities survive tab reordering/removal.

The Web Server and OSC status buttons start or stop their respective servers.
They appear when a tab is open. Server settings remain accessible without tabs.
Preferences can enable automatic web startup; it is off by default. Settings and
credentials are stored in local QSettings. The password field displays its value.

Authentication uses HTTP Basic. This built-in server does not encrypt network
traffic or local saved credentials; use it on a trusted network. All page, state,
event and playback routes require authentication. Playback requests also require
a custom header and reject foreign browser origins. Requests and connected clients
are bounded, and slow event-stream consumers are disconnected.

This feature uses ordinary Caster playback and has no dependency on Actions,
Audio Devices or window-size persistence.

## Verification

Build the application with qmake6 and make in a separate directory. Build
`tests/web/web-tests.pro` the same way and run `web-tests` with
`QT_QPA_PLATFORM=offscreen` for automated checks. Tests cover authentication,
fragmented requests, request limits, origin checks, real silent WAV playback,
event updates, tab identity, start/stop/restart, saved settings and startup.
Tests use TCP port 15052 and UDP port 15053 and need local socket access.
