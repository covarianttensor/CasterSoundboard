{ lib
, stdenv
, cmake
, ninja
, pkg-config
, qt6
, ffmpeg
, pulseaudio
, systemd
, version ? "2.1.1"
}:

stdenv.mkDerivation rec {
  pname = "caster-soundboard";
  inherit version;

  src = ./.;

  nativeBuildInputs = [
    cmake
    ninja
    pkg-config
    qt6.wrapQtAppsHook
  ];

  buildInputs = [
    qt6.qtbase
    qt6.qtdeclarative
    qt6.qtquickcontrols2
    qt6.qtmultimedia
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    "-DCASTER_VERSION=${version}"
  ];

  # Wrap binary so runtime dependencies are automatically available on PATH
  qtWrapperArgs = [
    "--prefix PATH : ${lib.makeBinPath [ ffmpeg pulseaudio systemd ]}"
  ];

  meta = with lib; {
    description = "A touch-first soundboard for hot-keying and playing audio sound effects and music tracks";
    homepage = "https://github.com/covarianttensor/CasterSoundboard";
    license = licenses.lgpl3Plus;
    platforms = platforms.linux;
    mainProgram = "CasterSoundboard";
  };
}
