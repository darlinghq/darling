#!/usr/bin/env python
import os
import glob
import subprocess
from xml.dom.minidom import Document

TEST_ARCHITECTURES = tuple([ "i386", "x86-64" ])
TEST_PLATFORMS = tuple([ "10.2", "10.6", "10.8" ])

successful_tests = {}
failed_tests = {}

def main():
	orig_cwd = os.getcwd()

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
				output = subprocess.check_output(["dyld", path + ".bin"])

				if output != expectedOutput:
					description = "Expected output:\n" + expectedOutput + "\n\nActual output:\n" + output

					print "*** FAILED!"
					print description

					failed_tests[arch][plat].append({ 'test': fileName, 'output': description })
				else:
					successful_tests[arch][plat].append({ 'test': fileName, 'output': output })

			except subprocess.CalledProcessError:
				failed_tests[arch][plat].append({ 'test': fileName, 'output': "Non-zero exit code" })

def writeTestResults(outFile, sourcesDir):
	doc = Document()
	file = open(outFile, "w")

	testLog = doc.createElement("TestLog");

	for arch in TEST_ARCHITECTURES:

		archNode = doc.createElement("TestSuite")
		archNode.setAttribute("name", arch)

		for plat in TEST_PLATFORMS:

			platNode = doc.createElement("TestSuite")
			platNode.setAttribute("name", plat)

			for test in failed_tests[arch][plat]:
				testNode = doc.createElement("TestCase")
				testNode.setAttribute("name", test['test'])

				errorNode = doc.createElement("Error")
				errorNode.setAttribute("file", sourcesDir + '/' + test['test'])
				errorNode.appendChild(doc.createTextNode(test['output']))

				testNode.appendChild(errorNode)
				platNode.appendChild(testNode)

			for test in successful_tests[arch][plat]:
				testNode = doc.createElement("TestCase")
 				testNode.setAttribute("name", test['test'])
				platNode.appendChild(testNode)

			archNode.appendChild(platNode)
		testLog.appendChild(archNode)

	doc.appendChild(testLog)
	doc.writexml(file)
	file.close()


if __name__ == "__main__":
    main()

