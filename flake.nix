{
	inputs = {
		nixpkgs.url = "github:NixOS/nixpkgs";
		flake-utils.url = "github:numtide/flake-utils";
	};

	outputs = { nixpkgs, flake-utils, ... }:
		flake-utils.lib.eachDefaultSystem (system:
			let pkgs = import nixpkgs { inherit system; };
			in {
				devShell = pkgs.mkShell {
					buildInputs = with pkgs; [ libgccjit flex bison yasm gnumake libxml2 doxygen graphviz-nox python39 gdb python39Packages.graphviz ];
					shellHook = "";
				};
			});
}
