# Actions

Open **Actions…**, add or select a named profile, then choose **Pin to tab**.
Each tab can contain multiple profiles; only its selected profile runs. Saving
in this dialog applies edits and selections to the tabs you changed. **Save Tab**
or **Save As** then writes each tab's `.caster` file. Unsaved Actions prompt on closing the tab or application. Cancel discards the dialog's drafts.

**Pin to tab → None** keeps the profile saved but unattached: it does not run or
prevent the tab from closing. It remains in its owning `.caster` file and is
available when that file is reopened. The dialog shows where it will be stored.
Choosing another tab moves the profile there; save both changed Caster files.

Talk modes:

- **PTT**: hold the key, wait the buffer, play, wait the buffer, release.
- **Mute**: tap the toggle key, wait, play, wait, tap it again. This assumes the
  game starts muted; there is no game-state feedback.
- **None**: ordinary playback without input setup or buffers.

For PTT/Mute, click the key field and press a single keyboard key. The field
shows its Linux physical key name. PTT and Mute remember separate keybinds;
switching Talk Mode restores that mode's key, including after saving/reloading.
The buffer defaults to 150 ms and applies at
both ends. Keep the receiving game focused and trigger playback through OSC.
Caster does not find, activate, address, or monitor game windows.

A tab with any attached profile (including Talk Mode None) cannot close. Detach
its profiles using Pin to tab None, or remove them, to unlock closing.
Exiting the application remains possible;
unsaved Actions prompt for Save/Discard/Cancel.

## Runtime behaviour

The desktop, keyboard and OSC playback methods share the same engine. Cue
preview does not run Actions. A second toggle during preparation cancels it.
Stop All cancels playback, preparation and tail buffers and closes input helpers.

Sounds sharing a key and talk mode share one input session. The final sound
ending starts the tail buffer; a new sound arriving during that buffer reuses the
session. Conflicting modes on the same key are rejected. Each sound snapshots its
settings, so editing profiles cannot change the cleanup key of a running sound.
Pause releases its claim; resume reacquires input and waits the lead-in. Loops
have a five-minute session limit. Cancellation and failures clean up immediately.

Input failure prevents playback, or stops affected playback if a session fails.
Errors appear in an Actions warning dialog. An acknowledgement means input
was emitted, not that a game accepted it.

## Linux backend

The Qt engine runs an embedded Python 3 helper for each used key. The helper
uses the installed `libevdev.so.2` runtime through ctypes. No development headers
or extra Python packages are needed. The desktop user needs access to
`/dev/uinput`; Caster does not change permissions or install system services.
Helper code comes from the application resource bundle, not from saved profiles.
Profiles cannot supply shell commands or executable paths.

A virtual keyboard persists between actions. The first action for a key has an
additional 1.5-second device settling delay, separate from the audio buffer.
Stop All closes the helpers, so the next action needs setup again. Linux XKB scan
codes preserve physical key positions, including German Y/Z and right modifiers.
Custom per-device layouts still need validation.

Heartbeats run every second. The helper releases on pipe EOF, SIGINT/SIGTERM, or
five seconds without a heartbeat. The engine limits a session to 300 seconds;
the helper independently limits it to 305 seconds. Mute cleanup pairs its opening
tap, but cannot determine actual game mute state. Killing/freezing the helper or
system failure cannot guarantee game-level key state.

## Profiles and compatibility

Action definitions and selection live inside `.caster` files, not application-wide
settings. Legacy files load with no Actions. A versioned extension follows the
original board/player data. Unknown or malformed extensions are rejected. Saves
use checked atomic replacement through `QSaveFile`.

Extension version 2 stores attachment and separate PTT/Mute keys. Version 1 files
still load: both keys inherit the formerly shared key, preserving their behaviour
until edited.

Older Caster builds can discard the Actions extension when saving; keep a backup
when switching between builds.

## Build

```sh
mkdir -p build-actions
cd build-actions
qmake6 ../CasterSoundboard/CasterSoundboard.pro
make -j4
./CasterSoundboard --profile /path/to/board.caster
```

## Verification

`tests/actions/actions-tests.pro` covers profile round-tripping, legacy loading,
explicit tab assignment, selection, close protection, saving, overlapping claims,
cancellation, failures, settings snapshots, silent WAV completion and pause/resume.
Its fake input channel never sends real keyboard events.

`python3 -B -m unittest discover -s tests/actions -p test_helper.py -v` tests the
helper with a fake keyboard, including PTT, mute pairing, disconnect, heartbeat
expiry and cancellation during the opening tap. Existing cue and metadata suites
also exercise playback and OSC behaviour with Actions present.

Real keyboard injection and game acceptance require manual Linux validation with
access to `/dev/uinput`; automated tests do not claim that validation.
