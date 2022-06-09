#!/usr/bin/env python3

import os
import json

from typing import Dict
from dataclasses import dataclass

@dataclass
class PageContent:
	globals: list[str]
	functions: list[str]
	packs: list[str]
	enums: list[str]
	macros: list[str]

def genLineVariable(symbol):
	return symbol["datatype"] + " " + symbol["name"]

def genLineFunction(symbol, funcName, isDefine):
	isExtern = funcName is None
	if isExtern:
		funcName = symbol["name"]
	return ("", "extern ")[isExtern] + symbol["retType"] + " " + funcName + "(" + ", ".join(map(genLineVariable, symbol["params"])) + ")" + (" ...", "")[isDefine]

def genLinePack(symbol, isDefine):
	return ("pack", "union")[symbol["isUnion"]] + " " + symbol["name"] + (" ...", "")[isDefine]

def genLineEnum(symbol):
	return symbol["name"]

def genMacroLine(symbol):
	return symbol["name"]

def generateLines(base, files, funcName = None):
	for name, symbol in base.items():
		declFile = symbol["pos"]["decl"]["file"]
		defFile = symbol["pos"]["def"]["file"]

		if declFile not in files:
			files[declFile] = PageContent([], [], [], [], [])
		if defFile not in files:
			files[defFile] = PageContent([], [], [], [], [])

		match symbol["type"]:
			case "None" | "Namespace" | "Global":
				pass
			case "Variable":
				files[declFile].globals.append(genLineVariable(symbol))
			case "FuncName":
				generateLines(symbol["subSymbols"], files, name)
			case "FuncSpec":
				if defFile == "<unknown>" or defFile != declFile:
					files[declFile].functions.append(genLineFunction(symbol, funcName, False))
				if defFile != "<unknown>":
					files[defFile].functions.append(genLineFunction(symbol, funcName, True))
			case "ExtFunc":
				files[declFile].functions.append(genLineFunction(symbol, None, True))
			case "Pack":
				if defFile == "<unknown>" or defFile != declFile:
					files[declFile].packs.append(genLinePack(symbol, False))

				if defFile != "<unknown>":
					files[defFile].packs.append(genLinePack(symbol, True))
			case "Enum":
				files[declFile].enums.append(genLineEnum(symbol))
			case "EnumMember":
				pass
			case "Macro":
				files[declFile].macros.append(genMacroLine(symbol))
			case _:
				print(f"Unknown symbol type: {symbol['type']}")

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

	files = {}

	generateLines(symbols, files)

	for file, content in files.items():
		print(file)
		print(" ", content)

	pass