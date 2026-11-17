{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/f4b140d5b253f5e2a1ff4e5506edbf8267724bde";
  };
  outputs = inputs@{
    self, nixpkgs, flake-parts,
  }:
  flake-parts.lib.mkFlake { inherit inputs; } {
    systems = [ "x86_64-linux" ];
    perSystem = { self', pkgs, system, ... }: {
      devShells.default = pkgs.mkShell.override {
        stdenv = pkgs.ccacheStdenv;
      } {
        NIX_LD = pkgs.lib.fileContents "${pkgs.stdenv.cc}/nix-support/dynamic-linker";
        NIX_LD_LIBRARY_PATH = "${pkgs.lib.makeLibraryPath (with pkgs; [
          stdenv.cc.cc.lib
          zlib
          expat
          cairo
          z3.lib
        ])}";
        # inputsFrom will add python3.13 to the environment. Using rawBuildInputs and rawNativeBuildInputs to avoid that.
        inputsFrom = (with pkgs;[ llvm circt ]);
        buildInputs = (with pkgs;[ lld  z3.lib zlib clang-tools ]);
        nativeBuildInputs = (with pkgs;[ lldb  gdb llvmPackages.mlir]);
        shellHook = ''
          export CCACHE_DIR="$PWD/.ccache"
          if [ -n "$LD_LIBRARY_PATH" ]; then
            export LD_LIBRARY_PATH="$NIX_LD_LIBRARY_PATH:$LD_LIBRARY_PATH"
          else
            export LD_LIBRARY_PATH="$NIX_LD_LIBRARY_PATH"
          fi
        '';
      };
    };
  };
}
