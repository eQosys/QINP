#!/usr/bin/env python3

import os
import json

if __name__ == "__main__":
	print("Compiling QINP...")
	if os.system("./build.sh Release"):
		print("Failed to compile QINP")
		exit(1)

	print("Generating import-all code file...")
	with open("/tmp/stdlib-import-all.qnp", "w") as f:
		for root, dirs, files in os.walk("stdlib/"):
			if root.startswith("stdlib/platform"):
				continue
			for file in files:
				if file.endswith(".qnp"):
					f.write(f"import \"{os.sep.join(os.path.join(root, file).split(os.sep)[1:])}\"\n")

	print("Exporting symbols...")
	if os.system("./bin/Release/QINP -e=/tmp/stdlib-symbols.json -i=stdlib/ /tmp/stdlib-import-all.qnp"):
		print("Failed to export symbols")
		exit(1)

	print("Loading symbols...")
	with open("/tmp/stdlib-symbols.json", "r") as f:
		symbols = json.load(f)["subSymbols"]["std"]["subSymbols"]

	# TODO: Sort the symbols and generate a reference file for each stdlib file