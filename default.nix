with import <nixpkgs> {};

stdenv.mkDerivation {
  name = "CasterSoundboard";
  src = ./CasterSoundboard;
  nativeBuildInputs = [ qt5.qmakeHook qt5.makeQtWrapper ];
  buildInputs = [
    qt5.qtmultimedia
    gst_all_1.gst-plugins-base
    gst_all_1.gst-plugins-good
    gst_all_1.gst-plugins-bad
    gst_all_1.gst-plugins-ugly
  ];
  enableParallelBuilding = true;
  postInstall = ''
    wrapQtProgram "$out/bin/$name" \
      --prefix GST_PLUGIN_SYSTEM_PATH_1_0 : "$GST_PLUGIN_SYSTEM_PATH_1_0"
  '';
}
