#!/usr/bin/env python3

import os
import sys
import shlex
import pickle
import subprocess
from dataclasses import dataclass
from typing import List, Optional

QINP_EXT = ".qnp"
TEST_EXT = ".blob"
TEST_DIR = "./tests/"

def runCmd(cmd: List[str], **kwargs):
	print("[CMD]:", " ".join(map(shlex.quote, cmd)))
	return subprocess.run(cmd, **kwargs)

def makeTestCmd(testName: str, argv: List[str]) -> List[str]:
	# TODO: Pass argv to QINP
	cmd = ["./bin/Debug/QINP", "-r", "-p=linux", "-i=stdlib/", f"{TEST_DIR}{testName}{QINP_EXT}"]
	cmd.extend(map(lambda str: "-a=" + str, argv))
	return cmd

@dataclass
class TestInfo:
	argv: List[str]
	stdin: bytes
	stdout: bytes
	exitCode: int

DEF_TEST_INFO = TestInfo([], b"", b"", 0)

def loadTestInfo(testName: str) -> Optional[TestInfo]:
	try:
		with open(f"{TEST_DIR}{testName}{TEST_EXT}", "rb") as f:
			return pickle.load(f)
	except FileNotFoundError:
		return None

def saveTestInfo(testName: str, testInfo: TestInfo) -> None:
	with open(f"{TEST_DIR}{testName}{TEST_EXT}", "wb") as f:
		pickle.dump(testInfo, f)

def updateInput(testName: str, argv: List[str]) -> None:
	testInfo = loadTestInfo(testName) or DEF_TEST_INFO

	print("[INF] Enter input. Press Ctrl+D to finish...")
	testInfo.stdin = sys.stdin.buffer.read()
	testInfo.argv = argv

	saveTestInfo(testName, testInfo)

def updateOutput(testName: str) -> None:
	testInfo = loadTestInfo(testName) or DEF_TEST_INFO

	output = runCmd(makeTestCmd(testName, testInfo.argv), input=testInfo.stdin, capture_output=True)
	testInfo.stdout = output.stdout
	testInfo.exitCode = output.returncode

	print("[INF] Exit code:", testInfo.exitCode)
	print("[INF] Output:\n", testInfo.stdout.decode("utf-8"))

	saveTestInfo(testName, testInfo)

def runTest(testName: str) -> bool:
	print(f"[INF] Running test '{testName}'...")

	testInfo = loadTestInfo(testName)
	if testInfo is None:
		print("[ERR] Test not found!")
		return

	output = runCmd(makeTestCmd(testName, testInfo.argv), input=testInfo.stdin, capture_output=True)
	
	success = True

	if output.returncode != testInfo.exitCode:
		print("[ERR] Exit code mismatch!")
		print("[ERR]   Expected:", testInfo.exitCode)
		print("[ERR]   Actual:", output.returncode)
		success = False
	
	if output.stdout != testInfo.stdout:
		print("[ERR] Output mismatch!")
		print("[ERR]   Expected:\n", testInfo.stdout.decode("utf-8"))
		print("[ERR]   Actual:\n", output.stdout.decode("utf-8"))
		success = False

	if not success:
		print("[ERR] Test failed!")
	
	return success

if __name__ == "__main__":
	argv = sys.argv
	if len(argv) == 1:
		print("Usage: ./test.py [command]")
		sys.exit(1)
	
	command, *argv = argv[1:]

	if command == "update":
		if len(argv) < 1:
			print("Usage: ./test.py update [mode]")
			sys.exit(1)
		mode, *argv = argv
		if mode == "input":
			if len(argv) < 1:
				print("Usage: ./test.py update input [test-name]")
				sys.exit(1)
			testName, *argv = argv
			updateInput(testName, argv)
		elif mode == "output":
			if len(argv) < 1:
				print("Usage: ./test.py update output [test-name]")
				sys.exit(1)
			testName, *argv = argv
			if testName == "all":
				for testName in os.listdir(TEST_DIR):
					if testName.endswith(QINP_EXT):
						updateOutput(testName[:-len(QINP_EXT)])
			else:
				updateOutput(testName)
		else:
			print("Unknown mode:", mode)
			sys.exit(1)

	elif command == "run":
		if len(argv) < 1:
			print("Usage: ./test.py run [test-name]")
			sys.exit(1)
		testName, *argv = argv

		numFailed = 0
		numTests = 0

		if testName == "all":
			for testName in os.listdir(TEST_DIR):
				if testName.endswith(QINP_EXT):
					if not runTest(testName[:-len(QINP_EXT)]):
						numFailed += 1
					numTests += 1

		else:
			if not runTest(testName):
				numFailed += 1
			numTests += 1

		if numFailed == 0:
			print("[INF] All tests passed!")
		else:
			print(f"[ERR] {numFailed}/{numTests} tests failed!")
	elif command == "help":
		print("Usage: ./test.py [command] [args...]")
		print("Commands:")
		print("  update            Update one or more tests.")
		print("    input           Update the input of a test.")
		print("      [test-name]   Name of the test to update.")
		print("    output")
		print("      all           Update the output of all tests.")
		print("      [test-name]   Name of the test to update.")
		print("  run")
		print("    all             Run all tests.")
		print("    [test-name]     Run a single test.")
		print("  help")
	else:
		print("Unknown command:", command)
		sys.exit(1)