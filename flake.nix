{
  description = "sequence-diagram-cli development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    let
      seqdia-overlay = final: prev: {
        seqdia = final.stdenv.mkDerivation {
          pname = "seqdia";
          version = "2.0.0";

          src = ./.;

          nativeBuildInputs = with final; [
            gnumake
            gcc
          ];
          buildInputs = [ ];

          installPhase = ''
            mkdir -p $out/bin
            cp seqdia $out/bin/
          '';

          meta = with final.lib; {
            description = "CLI tool for generating ASCII sequence diagrams";
            license = licenses.asl20;
            platforms = platforms.unix;
          };
        };
      };
    in
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ seqdia-overlay ];
        };
      in
      {
        packages = rec {
          default = seqdia;
          seqdia = pkgs.seqdia;
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            clang-tools
            gcc
            pkg-config
            gnumake
          ];
        };
      }
    )
    // {
      overlays.default = seqdia-overlay;
    };
}
