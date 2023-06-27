#!/usr/bin/env python3

import os
import json
import shutil
import hashlib

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
	details: dict[str, str]

@dataclass
class Detail:
	detailStr: str
	declLink: str
	defLink: str

FILES = {}

def commentsFromJSON(comments_json):
	comments = {}

	for file in comments_json["comments"]:
		if not file in comments:
			comments[file] = {}
		for line in comments_json["comments"][file]:
			comments[file][int(line)] = comments_json["comments"][file][line]

	return comments

def getDeclPos(symbol):
	if "decl" in symbol["pos"]:
		return symbol["pos"]["decl"]
	return symbol["pos"]["def"]

def commentExists(comments, symbol):
	pos = getDeclPos(symbol)
	return pos["file"] in comments and (pos["line"] - 1) in comments[pos["file"]]

def getPreExtendedComment(comments, symbol):
	pos = getDeclPos(symbol)
	file = pos["file"]
	line = pos["line"] - 1
	comment = comments[file][line]

	while line - 1 in comments[file]:
		comment = comments[file][line - 1] + "\n" + comment
		line -= 1
	
	comment = comment.replace("\n\\\\ ", "\n")
	if comment.startswith("\\\\ "):
		comment = comment[3:]
	return comment

def codeLink(file, line):
	return "/" + file + "?plain=1#L" + str(line)

def autoAddDetail(file, lineStr, comments, symbol):
	declFile = symbol["pos"]["decl"]["file"].replace("bin/Release/", "")
	defFile = symbol["pos"]["def"]["file"].replace("bin/Release/", "")
	if commentExists(comments, symbol):
		FILES[file].details[lineStr] = Detail(
			getPreExtendedComment(comments, symbol),
			codeLink(declFile, symbol["pos"]["decl"]["line"]) if declFile != "" else "",
			codeLink(defFile, symbol["pos"]["def"]["line"]) if defFile != "" else ""
			)

def lineLink(line):
	return "ref_" + str(hashlib.md5(line.encode()).hexdigest())

def wrapLine(line, symbol, hasDetail):
	result = " - "
	if hasDetail:
		result += "[" + line + "](#" + lineLink(line) + ")"
	else:
		result += line

	return result

def genLineVariable(file, symbol, comments, addVarPrefix = False, useFullName = False, doWrapLine = True):
	line = ""
	if addVarPrefix:
		line += "var\<"
	
	line += symbol["datatype"].replace("<", "\<").replace(">", "\>")

	if addVarPrefix:
		line += "\> "
	else:
		line += " "

	if useFullName:
		line += symbol["fullName"]
	else:
		line += symbol["name"]

	autoAddDetail(file, line, comments, symbol)
	return wrapLine(line, symbol, commentExists(comments, symbol)) if doWrapLine else line

def genLineFunction(file, symbol, comments, funcName, isDefine, doWrapLine = True):
	isExtern = funcName is None
	line = ""
	if isExtern:
		funcName = symbol["fullName"]
		line = "extern " + line
	if symbol["isBlueprint"]:
		pass #line = "blueprint " + line

	isVoidFunc = (symbol["retType"] == "void")

	line += "fn\<"

	if not isVoidFunc:
		line += symbol["retType"].replace("<", "\<").replace(">", "\>")

	line += "\> "

	line += funcName

	line += "("

	for param in symbol["params"]:
		line += genLineVariable(file, param, comments, doWrapLine=False) + ", "

	if len(symbol["params"]) > 0:
		line = line[:-2]

	if symbol["isVariadic"]:
		line += ", ..."

	line += ")"

	if symbol["isNoDiscard"]:
		line += " nodiscard"

	if not isDefine:
		line += " ..."

	autoAddDetail(file, line, comments, symbol)
	return wrapLine(line, symbol, commentExists(comments, symbol)) if doWrapLine else line

def genLinePack(file, symbol, comments, isDefine, doWrapLine = True):
	line = ""

	if symbol["isUnion"]:
		line += "union"
	else:
		line += "pack"

	line += " " + symbol["fullName"]

	if not isDefine:
		line += " ..."

	autoAddDetail(file, line, comments, symbol)
	return wrapLine(line, symbol, commentExists(comments, symbol)) if doWrapLine else line

def genLineEnum(file, symbol, comments, doWrapLine = True):
	line = "enum "

	line += symbol["fullName"]

	autoAddDetail(file, line, comments, symbol)
	return wrapLine(line, symbol, commentExists(comments, symbol)) if doWrapLine else line

def genMacroLine(file, symbol, comments, doWrapLine = True):
	line = ""

	line += symbol["fullName"]

	if "params" in symbol:
		line += "("
		for param in symbol["params"]:
			line += param + ", "
		if len(symbol["params"]) > 0:
			line = line[:-2]
		line += ")"

	autoAddDetail(file, line, comments, symbol)
	return wrapLine(line, symbol, commentExists(comments, symbol)) if doWrapLine else line

def generateContent(name, lines):
	content = "\n## " + name + "\n"
	for line in lines:
		content += line + "\n"
	return content

def generateDetails(details):
	content = "\n## Details\n"

	for name, detail in details.items():
		content += "#### <a id=\"" + lineLink(name) + "\"/>" + name + "\n"

		addedLink = False

		if detail.declLink != "" and detail.declLink != detail.defLink:
			content += "> [Declaration](" + detail.declLink + ")"
			addedLink = True

		if detail.defLink != "":
			content += " | " if addedLink else "> "
			addedLink = True
			content += "[Definition](" + detail.defLink + ")"

		if addedLink:
			content += "\n"
		
		content += "```qinp\n" + detail.detailStr + "\n```\n"

	return content

def generateLines(base, comments, files, funcName = None):
	for name, symbol in base.items():
		declFile = symbol["pos"]["decl"]["file"].replace("bin/Release/", "")
		defFile = symbol["pos"]["def"]["file"].replace("bin/Release/", "")

		if declFile not in files:
			files[declFile] = PageContent([], [], [], [], [], {})
		if defFile not in files:
			files[defFile] = PageContent([], [], [], [], [], {})

		match symbol["type"]:
			case "None" | "Global":
				continue
			case "Namespace":
				generateLines(symbol["subSymbols"], comments, files, funcName)
			case "Variable":
				files[declFile].globals.append(genLineVariable(declFile, symbol, comments, True, True))
			case "FuncName":
				if name == "&_BLUEPRINTS_&":
					generateLines(symbol["subSymbols"], comments, files, funcName)
				else:
					generateLines(symbol["subSymbols"], comments, files, symbol["fullName"])
			case "FuncSpec":
				if symbol["genFromBlueprint"]:
					continue
				if declFile != defFile:
					files[declFile].functions.append(genLineFunction(declFile, symbol, comments, funcName, False))
				if symbol["state"] == "Defined":
					files[defFile].functions.append(genLineFunction(defFile, symbol, comments, funcName, True))
			case "ExtFunc":
				files[declFile].functions.append(genLineFunction(declFile, symbol, comments, None, True))
			case "Pack":
				if defFile == "<unknown>" or defFile != declFile:
					files[declFile].packs.append(genLinePack(declFile, symbol, comments, False))

				if defFile != "<unknown>":
					files[defFile].packs.append(genLinePack(defFile, symbol, comments, True))
			case "Enum":
				files[declFile].enums.append(genLineEnum(declFile, symbol, comments))
			case "EnumMember":
				continue
			case "Macro":
				files[declFile].macros.append(genMacroLine(declFile, symbol, comments))
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
	if os.system("./bin/Release/QINP -e=/tmp/stdlib-symbols.json -c=/tmp/stdlib-comments.json /tmp/stdlib-import-all.qnp"):
		print("Failed to export symbols")
		exit(1)

	print("Loading symbols...")
	with open("/tmp/stdlib-symbols.json", "r") as f:
		symbols = json.load(f)["subSymbols"]["std"]["subSymbols"]

	print("Loading comments...")
	with open("/tmp/stdlib-comments.json", "r") as f:
		comments = commentsFromJSON(json.load(f))

	generateLines(symbols, comments, FILES)

	shutil.rmtree(DOCS_DIR + "stdlib", ignore_errors=True)

	for file, pageContent in FILES.items():
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
		if (len(pageContent.details) > 0):
			overview += " - [Details](#details)\n"
			content += generateDetails(pageContent.details)

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
			if file not in FILES:
				continue
			fileShort = os.sep.join(file.split(os.sep)[1:])
			filelist += " - [./" + fileShort + "](" + fileShort + ".md)\n"
		page = page.replace("<filelist>", filelist)
		f.write(page)