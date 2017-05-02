with import <nixpkgs> {};

stdenv.mkDerivation {
  name = "CasterSoundboard";
  src = ./CasterSoundboard;
  nativeBuildInputs = [ qt5.qmakeHook qt5.makeQtWrapper ];
  buildInputs = [ qt5.qtmultimedia ];
  enableParallelBuilding = true;
  postInstall = "wrapQtProgram \"$out/bin/$name\"";
}
