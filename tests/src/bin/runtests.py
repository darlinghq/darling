#!/usr/bin/env python
import os
import glob
import subprocess
import time
from xml.dom.minidom import Document

TEST_ARCHITECTURES = [ "i386", "x86-64" ]
TEST_PLATFORMS = tuple([ "10.2", "10.6", "10.8" ])

successful_tests = {}
failed_tests = {}
dyld_command = "dyld"

def main():
	global dyld_command
	orig_cwd = os.getcwd()

	if 'DYLD' in os.environ:
		dyld_command = os.environ['DYLD']

	if dyld_command.endswith('32'):
		TEST_ARCHITECTURES.remove('x86-64')
	elif dyld_command.endswith('64'):
		TEST_ARCHITECTURES.remove('i386')

	os.chdir(os.path.dirname(os.path.realpath(__file__)))
	tests = glob.glob('i386/*.stdout');

	for arch in TEST_ARCHITECTURES:
		successful_tests[arch] = {}
		failed_tests[arch] = {}

		for plat in TEST_PLATFORMS:
			successful_tests[arch][plat] = []
			failed_tests[arch][plat] = []

	for fileName in tests:
		runTest(fileName[5:-7])
	
	src_dir = os.path.realpath(os.getcwd() + '/..')
	os.chdir(orig_cwd)
	writeTestResults("report.xml", src_dir)

def runTest(fileName):
	print "************"
	print "Running test " + fileName
	print "************"


	for arch in TEST_ARCHITECTURES:
		expectedOutput = open(arch + "/" + fileName + ".stdout", "r").read()

		for plat in TEST_PLATFORMS:
			print "* Testing " + arch + "/" + plat
			path = arch + "/" + plat + "/" + fileName

			if not os.path.exists(path + ".bin"):
				print "Skipping " + path
				continue

			try:
				time_start = time.time()
				output = subprocess.check_output([dyld_command, path + ".bin"])
				elapsed_time = (time.time()-time_start)

				if output != expectedOutput:
					description = "Expected output:\n" + expectedOutput + "\n\nActual output:\n" + output

					print "*** FAILED!"
					print description

					failed_tests[arch][plat].append({ 'test': fileName, 'output': description, 'time': elapsed_time })
				else:
					successful_tests[arch][plat].append({ 'test': fileName, 'output': output, 'time': elapsed_time })

			except subprocess.CalledProcessError:
				elapsed_time = (time.time()-time_start)
				failed_tests[arch][plat].append({ 'test': fileName, 'output': "Non-zero exit code", 'time': elapsed_time })

def writeTestResults(outFile, sourcesDir):
	doc = Document()
	file = open(outFile, "w")

	testLog = doc.createElement("testsuites");

	for arch in TEST_ARCHITECTURES:

		for plat in TEST_PLATFORMS:

			platNode = doc.createElement("testsuite")
			platNode.setAttribute("name", arch+"/"+plat)
			platNode.setAttribute("tests", str(len(failed_tests[arch][plat]) + len(successful_tests[arch][plat])))

			for test in failed_tests[arch][plat]:
				testNode = doc.createElement("testcase")
				testNode.setAttribute("name", test['test'])

				errorNode = doc.createElement("failure")
				#errorNode.setAttribute("file", sourcesDir + '/' + test['test'])
				#errorNode.setAttribute("line", "0")
				errorNode.appendChild(doc.createTextNode(test['output']))

				#timeNode = doc.createElement("TestingTime")
				testNode.setAttribute("time", str(test['time']) + "s")
				#testNode.appendChild(timeNode)

				testNode.appendChild(errorNode)
				platNode.appendChild(testNode)

			for test in successful_tests[arch][plat]:
				testNode = doc.createElement("testcase")
 				testNode.setAttribute("name", test['test'])

				#timeNode = doc.createElement("TestingTime")
				testNode.setAttribute("time", str(test['time']) + "s")
				#testNode.appendChild(timeNode)
				platNode.appendChild(testNode)

			testLog.appendChild(platNode)

	doc.appendChild(testLog)
	doc.writexml(file, encoding="utf-8")
	file.close()


if __name__ == "__main__":
    main()

