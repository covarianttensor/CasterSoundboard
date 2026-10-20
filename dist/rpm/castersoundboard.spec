Name:           castersoundboard
Version:        2.1.2
Release:        1%{?dist}
Summary:        Touch-first live soundboard and audio routing tool for Linux

License:        LGPL-3.0-or-later
URL:            https://github.com/covarianttensor/CasterSoundboard
Source0:        https://github.com/covarianttensor/CasterSoundboard/archive/refs/tags/v%{version}.tar.gz#/CasterSoundboard-%{version}.tar.gz

BuildRequires:  cmake >= 3.16
BuildRequires:  gcc-c++
BuildRequires:  ninja-build
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

# Qt6 build dependencies
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qtmultimedia-devel
BuildRequires:  qt6-qtwayland-devel
BuildRequires:  mesa-libGL-devel
BuildRequires:  libxkbcommon-devel

# Runtime dependencies
Requires:       qt6-qtdeclarative%{?_isa}
Requires:       qt6-qtmultimedia%{?_isa}
Requires:       ffmpeg
Requires:       (pulseaudio-utils or pipewire-pulseaudio)
Requires:       hicolor-icon-theme

%description
Caster Soundboard is a high-performance, touch-first soundboard and live
audio routing application designed for podcasters, streamers, and live
broadcasters on Linux (Wayland and X11).

Key features include:
* Responsive touch-first soundboard tile grid with multi-tab bank switching
* Seamless virtual audio routing with zero configuration via PipeWire & PulseAudio
* Push-to-Talk (PTT) automation with custom hotkeys and microphone mute integration
* Interactive audio waveform visualizer with trim handles, looping, and cue points
* Local Web Remote control surface accessible from smartphones and tablets
* Multi-track audio ducking with configurable attenuation and smooth release timing
* Open Sound Control (OSC) and Model Context Protocol (MCP) AI remote integration

%prep
%autosetup -n CasterSoundboard-%{version} -p1

%build
%cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCASTER_VERSION=%{version}
%cmake_build

%install
%cmake_install

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/io.github.covarianttensor.CasterSoundboard.desktop
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/io.github.covarianttensor.CasterSoundboard.metainfo.xml

%files
%license LICENSE
%doc README.md
%{_bindir}/CasterSoundboard
%{_datadir}/applications/io.github.covarianttensor.CasterSoundboard.desktop
%{_datadir}/metainfo/io.github.covarianttensor.CasterSoundboard.metainfo.xml
%{_datadir}/icons/hicolor/256x256/apps/io.github.covarianttensor.CasterSoundboard.png
%{_datadir}/icons/hicolor/scalable/apps/io.github.covarianttensor.CasterSoundboard.svg

%changelog
* Mon Oct 19 2026 Oscar Cerna <covarianttensor@gmail.com> - 2.1.2-1
- Release v2.1.2: Maintenance release with Flatpak routing, dropdown fixes, and window size adjustment
