# Audio Devices

Open **Audio Devices** on the toolbar to choose the physical microphone, system
playback output and soundboard monitoring output independently. Review the change
preview and confirm Apply. Startup and Refresh only discover devices; saved
preferences are never applied automatically.

The Linux backend uses `pactl` with PulseAudio or PipeWire's PulseAudio-compatible
server. It can mix the microphone and Caster playback into a managed virtual
microphone, monitor soundboard audio without microphone feedback, and optionally
make the mix the system default input. Select Default/System Default in the
receiving application's input settings; an existing recording session may need
reconnecting. Caster does not move another application's recording streams.

Settings live in `QSettings("CasterSoundboard", "AudioDevices")`, independently
of soundboard profiles. The existing Qt output selector remains available when
the management backend is unavailable. While managed routing is active it is
disabled to avoid conflicting output choices.

Stop and clean exit remove Caster-owned modules. The previous default input is
restored only if it still points to Caster's virtual input. Physical card profiles
and the chosen system playback output remain in place. External changes and
unrelated modules are preserved; cleanup failures are reported.

**Recover selected device** previews a targeted card profile reset. **Restart
desktop audio** is a separate, confirmed PipeWire recovery operation that can
interrupt all desktop audio and other applications' virtual devices. Neither is
automatic. The dialog's Help explains setup and recovery.

Stream routing follows stream discovery, including ALSA streams identified through
their client PID. It cannot guarantee delivery of the first samples before the
stream has been moved. Hardware-specific behaviour still requires manual testing.

## Verification

Build with `qmake6 CasterSoundboard/CasterSoundboard.pro` in a separate build
directory, then `make`. The same procedure with `tests/audio/audio-tests.pro`
builds fake-server regressions for discovery, routing, rollback, ownership,
recovery, service restart and subscription shutdown. Run the resulting
`audio-tests`; the private-server case is skipped without its explicit opt-in.

`python3 tests/audio/isolated-server.py /absolute/path/to/audio-tests` exercises
real modules and cleanup on a private PipeWire server, with hardware monitors
disabled. It requires PipeWire, pipewire-pulse, pactl and WirePlumber 0.4.
