# Issue #8 investigation and verification

Issue: https://github.com/covarianttensor/CasterSoundboard/issues/8

The report describes 100% CPU and rapid memory growth, later narrowed to one
saved tab even without playback. Recreating that tab avoided the problem.
The original profile is unavailable, so this is a **best-guess fix based on the
original fault report**, not confirmation of its exact root cause.

Two real defects were identified: an idle cue feedback loop and an outgoing OSC
memory leak. The CPU loop was reproduced; the OSC leak was noticed along the way
by tracing ownership. The original reported 32 GB exhaustion was not reproduced.

## What looked problematic

The queued position handler enforced start/stop points even while stopped. With
a saved-style cue where start = stop = 500 ms, stopping resets the position and
seeking back produces further notifications. A silent two-second WAV reproduced
39,334 position notifications in a 200 ms event-processing window before the fix.
Invalid ranges and obsolete queued positions could keep this cycle running.

Separately, `OscMessageComposer::getBytes()` returns a newly allocated QByteArray.
`MainWindow::sendOSCMessageToClient()` deleted the composer but never deleted that
packet buffer, leaking memory on every outgoing OSC packet. A position event
storm can amplify this leak when OSC is enabled.

## Fix

- Reset invalid cue ranges to the full track and clamp oversized stop points.
  Preserve valid saved cues while media duration is temporarily unknown.
- Ignore obsolete queued position notifications and avoid enforcing playback
  boundaries while stopped or paused. Preserve natural EndOfMedia looping.
- Guard cue-preview processing against inactive, obsolete, or empty ranges.
- Use scoped ownership for outgoing OSC messages and packet buffers, including
  exception paths.

## Verification

Based directly on upstream `origin/master` at
`1a84315b3a3d4bdca1a4b784bd19460637f44438` (the upstream default branch).

- Full application qmake/make build on Qt 5.15.13: passed.
- Cue regression suite: 6 passed, 0 failed, including setup/cleanup. Four cases
  cover equal, reversed, negative, and past-end cues. Each also exercises valid
  cue preservation, stopping, partial-track looping, and full-track looping.
- `git diff --check`: passed.

Build and run from the repository root:

```sh
repo="$PWD"
mkdir -p /tmp/caster-cue-build
cd /tmp/caster-cue-build
qmake "$repo/tests/cues/cue-tests.pro"
make -j4
QT_QPA_PLATFORM=offscreen ./cue-tests
```

Tests use a silent WAV and require a working Qt multimedia backend with access
to the audio server. Longer-term observation is still needed to assess whether
this resolves all symptoms described in the original report.

Implementation written by OpenAI Codex.
Built, tested and submitted by mofftech.
