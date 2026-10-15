# Profile window-size persistence

The main window size is stored only in saved `.caster` profiles. Saving a
profile, closing its tab, or closing the visible app updates that size;
loading the profile restores it. When several saved tabs are open, closing
the app stores the final window size in each profile. Automatic size updates
preserve saved soundboard content without saving other pending edits.
Unsaved tabs and headless runs do not automatically store a window size.
Maximized, minimized, and fullscreen windows store their normal window size.
Older profiles without size metadata retain the existing window size on load.

Window size is optional metadata appended after the existing player states:
`quint32(0x4357535a)`, `quint32(1)`, and a `QSize`, using the profile's QDataStream
version. Profiles without metadata remain readable. Older upstream readers
ignore the trailing metadata; saving with those versions removes it.

Automatic updates read the saved profile and write it atomically with QSaveFile,
so pending soundboard edits are not saved implicitly. A read/write failure is
reported and leaves the window or tab open. Size is not stored in QSettings.

Build the application and run the relevant Qt 6 tests in separate directories:

```sh
mkdir -p /tmp/caster-size-app /tmp/caster-size-tests
(cd /tmp/caster-size-app && qmake6 /path/to/CasterSoundboard/CasterSoundboard/CasterSoundboard.pro && make -j4)
(cd /tmp/caster-size-tests && qmake6 /path/to/CasterSoundboard/tests/metadata/application-tests.pro && make -j4)
QT_QPA_PLATFORM=offscreen XDG_CONFIG_HOME="$(mktemp -d)" /tmp/caster-size-tests/metadata-application-tests profileWindowSize profileWindowSizeTabCloseAndFailure profileWindowSizeMalformedMetadata
```

The tests cover old profiles, close/reopen persistence, preservation of unsaved
edits, explicit save, closing a hidden/headless window, tab closure, malformed
metadata, and protection of a damaged on-disk profile during automatic updates.
