#!/usr/bin/env python3

import os
import sys
import time
import shlex
import pickle
import subprocess
from dataclasses import dataclass
from typing import List, Optional

QINP_EXT = ".qnp"
TEST_EXT = ".blob"
TEST_DIR = "./tests/"

def runCmd(cmd: List[str], **kwargs):
	print("[ CMD ]", " ".join(map(shlex.quote, cmd)))
	return subprocess.run(cmd, **kwargs)

def makeTestCmd(testName: str, argv: List[str]) -> List[str]:
	# TODO: Pass argv to QINP
	cmd = [ "./bin/Debug/QINP", "-r", "-i=stdlib/", f"{TEST_DIR}{testName}{QINP_EXT}", f"-o=/tmp/{testName}.out" ]
	cmd.extend(map(lambda str: "-a=" + str, argv))
	return cmd

@dataclass
class TestInfo:
	argv: List[str]
	stdin: bytes
	stdout: bytes
	exitCode: int

DEF_TEST_INFO = TestInfo([], b"", b"", 0)

def runTestCmd(testName: str, testInfo: TestInfo) -> subprocess.CompletedProcess[str]:
	output = None
	try:
		output = runCmd(makeTestCmd(testName, testInfo.argv), input=testInfo.stdin, capture_output=True, timeout=5)
	except subprocess.TimeoutExpired:
		return True, output

	return False, output

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

	print("[ INF ] Enter input. Press Ctrl+D to finish...")
	testInfo.stdin = sys.stdin.buffer.read()
	testInfo.argv = argv

	saveTestInfo(testName, testInfo)

def updateOutput(testName: str) -> None:
	testInfo = loadTestInfo(testName) or DEF_TEST_INFO

	timedOut, output = runTestCmd(testName, testInfo)

	if timedOut:
		print("[ ERR ] Test timed out!")
		return

	testInfo.stdout = output.stdout
	testInfo.exitCode = output.returncode

	print("[ INF ] Exit code:", testInfo.exitCode)
	print("[ INF ] Output:\n", testInfo.stdout.decode("utf-8"))

	saveTestInfo(testName, testInfo)

def runTest(testName: str) -> bool:
	print(f"[ INF ] Running test '{testName}'...")

	testInfo = loadTestInfo(testName)
	if testInfo is None:
		print("[ ERR ] Test not found!")
		return False

	timedOut, output = runTestCmd(testName, testInfo)

	if timedOut:
		print("[ ERR ] Test timed out!")
		return False
	
	success = True

	if output.returncode != testInfo.exitCode:
		print("[ ERR ] Exit code mismatch!")
		print("[ ERR ]   Expected:", testInfo.exitCode)
		print("[ ERR ]   Actual:", output.returncode)
		success = False
	
	if output.stdout != testInfo.stdout:
		print("[ ERR ] Output mismatch!")
		print("[ ERR ]   Expected:\n", testInfo.stdout.decode("utf-8"))
		print("[ ERR ]   Actual:\n", output.stdout.decode("utf-8"))
		success = False

	if not success:
		print("[ ERR ] Test failed!")
	
	return success

def buildQINP() -> bool:
	print("[ INF ] Building QINP...")
	output = runCmd([ "./build.sh", "Debug" ])
	success = output.returncode == 0
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
			if not buildQINP():
				sys.exit(1)
			if len(argv) < 1:
				print("Usage: ./test.py update output [test-name]")
				sys.exit(1)
			testName, *argv = argv
			if testName == "all":
				answer = input("Are you sure you want to update all tests? (y/N) ").lower()
				if answer != "y":
					sys.exit(0)
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

		if not buildQINP():
			sys.exit(1)

		numFailed = 0
		numTests = 0
		failNames = []

		if testName == "all":
			for testPath in os.listdir(TEST_DIR):
				if testPath.endswith(QINP_EXT):
					testName = testPath[:-len(QINP_EXT)]
					if not runTest(testName):
						numFailed += 1
						failNames.append(testName)
					numTests += 1
		else:
			if not runTest(testName):
				numFailed += 1
				failNames.append(testName)
			numTests += 1

		if numFailed == 0:
			print(f"[ INF ] {numTests}/{numTests} tests passed!")
		else:
			print(f"[ ERR ] {numFailed}/{numTests} tests failed!")
			print("[ ERR ] Failed tests:", *failNames)
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