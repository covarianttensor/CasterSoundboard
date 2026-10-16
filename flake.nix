{
  description = "Caster Soundboard - Touch-first soundboard for live broadcasts and podcasts";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        packages.default = pkgs.callPackage ./default.nix { };
        packages.caster-soundboard = self.packages.${system}.default;

        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/CasterSoundboard";
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ self.packages.${system}.default ];
          packages = with pkgs; [
            qt6.qttools
          ];
        };
      }
    );
}
