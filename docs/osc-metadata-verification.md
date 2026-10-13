# OSC metadata verification

Verified on 2026-10-12 with Qt 5.15.13 / GCC 13 on Linux, using Qt's offscreen
platform and real loopback UDP sockets.

## Results

- Full local application build: passed.
- `tests/metadata/metadata-tests.pro`: passed (5 QtTest results including setup
  and cleanup). Captures serialized UDP messages and checks exact addresses,
  argument counts/types and values for all 32 slots, including boundary IDs and
  comma. Checks all existing per-player messages remain present, layout-derived
  positions, accepted/unchanged/cancelled/dismissed colour edits, inactive boards,
  byte-identical profile round trips and reload back to default grey. Checks
  metadata against Qt-rendered gradient pixels, including fractional RGB values.
- `tests/metadata/application-tests.pro`: passed (3 QtTest results including
  setup and cleanup). Starts OSC through its settings dialog, sends `/glo/sync`,
  switches real tabs, checks OSC-disabled silence, ignores inbound metadata
  writes, and verifies an existing volume command. Runs the local
  `tests/osc/test_osc_packets.py` hardening sender when available, then confirms
  the application still responds correctly. That separate script is optional
  and is not a dependency of this feature.
- Existing web integration suite: passed, including loading a silent WAV,
  playback, natural completion and desktop stop feedback. No web files changed.
- Feature patch applies cleanly to locally available `origin/master`
  `1a84315b3a3d4bdca1a4b784bd19460637f44438`. The standalone metadata suite builds
  and passes on that source without the web POC, audio device manager, profile
  CLI patch or OSC hardening patch. No new runtime dependencies.
- Existing tracked working-tree changes were compared with the initial diff and
  preserved. The only addition to the already-modified README is the API link.
  Nothing was staged or committed.

## Running the new tests

From the repository root, build each suite in a separate directory:

```sh
mkdir -p /tmp/caster-metadata-test
cd /tmp/caster-metadata-test
qmake /path/to/CasterSoundboard/tests/metadata/metadata-tests.pro
make -j4
QT_QPA_PLATFORM=offscreen ./metadata-tests
```

Repeat with `application-tests.pro` and run `metadata-application-tests` for the
application suite. A Qt5 development environment with Multimedia and Test is
required. Permit local sockets. The application test increases its UDP receive
buffer to capture full sync bursts; no production transport settings change.

## Manual verification and limits

The picker, tab switching and rendering were exercised automatically offscreen,
not by a human on a visible desktop. Audible playback, physical remote OSC
clients and Windows/macOS builds were not tested here. No layout editing exists;
resizing/scrolling does not move logical slots. The reported colour represents
the background gradient, not a transient playback overlay.

For a visible desktop check:

1. Configure an OSC listener as Caster's outbound destination, start OSC and
   send `/glo/sync` with float `1.0`. Check 32 colour and 32 grid messages.
2. Compare `1`, `8`, `Z`, and `,` with the four visible corners. Load a populated
   profile and compare button background top colours with the metadata.
3. Accept a colour edit, then reopen the picker and cancel another edit. Only
   the accepted change should publish; saving/reopening should keep that colour.
4. Switch between differently coloured tabs and reload a default profile. Check
   the current board's metadata, including restored default `#DADADA`.
5. Disable OSC and exercise playback, cues, loops, ducking and hotkeys. Re-enable
   it and verify the same operations with an older OSC client.
