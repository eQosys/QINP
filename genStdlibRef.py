#!/usr/bin/env python3

import os
import json
import shutil

from typing import Dict
from dataclasses import dataclass

DOCS_DIR = "docs/"

MAIN_PAGE_MD_TEMPLATE = """
# Standard Library Reference

The Standard Library is a collection of functions, packs and definitions that are available to all QINP programs and are essential to the language.

All files building the Standard Library can be found in the [stdlib](../../stdlib/) directory:
<filelist>

Every Function/pack defined in the standard library exists in the `std` space.

Identifiers with a leading double underscore are implementation details and should not be used directly.
"""

PAGE_MD_TEMPLATE = """
# Stdlib - <filename>

## Overview
<overview>
<content>
"""

@dataclass
class PageContent:
	globals: list[str]
	functions: list[str]
	packs: list[str]
	enums: list[str]
	macros: list[str]

def genLineVariable(symbol, addVarPrefix = False, addStdPrefix = False):
	return ("", "var<")[addVarPrefix] + symbol["datatype"] + ("", ">")[addVarPrefix] + (" ", " std.")[addStdPrefix] + symbol["name"]

def genLineFunction(symbol, funcName, isDefine):
	isExtern = funcName is None
	line = ""
	if isExtern:
		funcName = symbol["name"]
		line = "extern " + line
	if symbol["isBlueprint"]:
		line = "blueprint " + line

	isVoidFunc = symbol["retType"] == "void"

	return line + "fn<" + (symbol["retType"], "")[isVoidFunc] + "> std." + funcName + "(" + ", ".join(map(genLineVariable, symbol["params"])) + ("", ", ...")[symbol["isVariadic"]] + ")" + (" ...", "")[isDefine]

def genLinePack(symbol, isDefine):
	return ("pack", "union")[symbol["isUnion"]] + " std." + symbol["name"] + (" ...", "")[isDefine]

def genLineEnum(symbol):
	return "std." + symbol["name"]

def genMacroLine(symbol):
	return "std." + symbol["name"]

def generateContent(name, lines):
	content = "\n## " + name + "\n"
	for line in lines:
		content += " - [" + line + "]()\n"
	return content

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
				continue
			case "Variable":
				files[declFile].globals.append(genLineVariable(symbol, True, True))
			case "FuncName":
				if name == "&_BLUEPRINTS_&":
					generateLines(symbol["subSymbols"], files, funcName)
				else:
					generateLines(symbol["subSymbols"], files, name)
			case "FuncSpec":
				if symbol["genFromBlueprint"]:
					continue
				if declFile != defFile:
					files[declFile].functions.append(genLineFunction(symbol, funcName, False))
				if symbol["state"] == "Defined":
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
				continue
			case "Macro":
				files[declFile].macros.append(genMacroLine(symbol))
			case _:
				print(f"Unknown symbol type: {symbol['type']}")

if __name__ == "__main__":
	print("Compiling QINP...")
	if os.system("./build.sh Release"):
		print("Failed to compile QINP")
		exit(1)

	sortedFileList = []

	print("Generating import-all code file...")
	with open("/tmp/stdlib-import-all.qnp", "w") as f:
		for root, dirs, files in os.walk("stdlib/"):
			for file in files:
				if file.endswith(".qnp"):
					sortedFileList.append(os.path.join(root, file))
					if not root.startswith("stdlib/platform"):
						path = os.sep.join(os.path.join(root, file).split(os.sep)[1:])
						f.write(f"import \"{path}\"\n")

	sortedFileList.sort()

	print("Exporting symbols...")
	if os.system("./bin/Release/QINP -e=/tmp/stdlib-symbols.json -i=stdlib/ /tmp/stdlib-import-all.qnp"):
		print("Failed to export symbols")
		exit(1)

	print("Loading symbols...")
	with open("/tmp/stdlib-symbols.json", "r") as f:
		symbols = json.load(f)["subSymbols"]["std"]["subSymbols"]

	files = {}

	generateLines(symbols, files)

	shutil.rmtree(DOCS_DIR + "stdlib", ignore_errors=True)

	for file, pageContent in files.items():
		if not file.startswith("stdlib/"):
			print("Skipping file: " + file)
			continue

		print("Generating file: " + file)

		overview = ""
		content = ""

		if len(pageContent.globals) > 0:
			overview += " - [Globals](#globals)\n"
			content += generateContent("Globals", pageContent.globals)
		if len(pageContent.functions) > 0:
			overview += " - [Functions](#functions)\n"
			content += generateContent("Functions", pageContent.functions)
		if len(pageContent.packs) > 0:
			overview += " - [Packs/Unions](#packs-unions)\n"
			content += generateContent("Packs/Unions", pageContent.packs)
		if len(pageContent.enums) > 0:
			overview += " - [Enums](#enums)\n"
			content += generateContent("Enums", pageContent.enums)
		if len(pageContent.macros) > 0:
			overview += " - [Macros](#macros)\n"
			content += generateContent("Macros", pageContent.macros)

		page = PAGE_MD_TEMPLATE
		page = page.replace("<filename>", file)
		page = page.replace("<overview>", overview)
		page = page.replace("<content>", content)

		os.makedirs(DOCS_DIR + os.path.dirname(file), exist_ok = True)
		with open(DOCS_DIR + file + ".md", "w") as f:
			f.write(page)

	with open (DOCS_DIR + "stdlib/stdlib.md", "w") as f:
		page = MAIN_PAGE_MD_TEMPLATE
		filelist = ""
		for file in sortedFileList:
			if not file.startswith("stdlib/"):
				continue
			if file not in files:
				continue
			fileShort = os.sep.join(file.split(os.sep)[1:])
			filelist += " - [./" + fileShort + "](" + fileShort + ".md)\n"
		page = page.replace("<filelist>", filelist)
		f.write(page)