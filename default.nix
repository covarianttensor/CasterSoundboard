{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation {
  pname = "CasterSoundboard";
  version = "1.0";

  src = ./.;
  preConfigure = ''
    cd CasterSoundboard
  '';

  nativeBuildInputs = [
    pkgs.qt6.qmake
    pkgs.qt6.wrapQtAppsHook
  ];

  buildInputs = [
    pkgs.qt6.qtbase
    pkgs.qt6.qtmultimedia
    pkgs.qt6.qtwayland
    pkgs.ffmpeg
  ];

  qmakeFlags = [
    "PREFIX=${placeholder "out"}"
  ];

  enableParallelBuilding = true;
}
