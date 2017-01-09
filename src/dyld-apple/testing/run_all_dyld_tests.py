#!/usr/bin/python2.7

import plistlib
import string
import sys
import os
import shutil
import subprocess




#
# Parse dyld's BATS input file and run each test
#
if __name__ == "__main__":
    batsPlist = plistlib.readPlist("/AppleInternal/CoreOS/BATS/unit_tests/dyld.plist")
    tests = batsPlist["Tests"]
    passedCount = 0
    failedCount = 0
    for test in tests:
        cwd = test["WorkingDirectory"]
        if cwd:
            os.chdir(cwd)
        cmd = test["Command"]
        testName = test["TestName"]
        if cwd:
            try:
                runOutput = subprocess.check_output(cmd,stderr= subprocess.STDOUT)
                lines = runOutput.splitlines()
                foundBegin = False
                passed = False
                failed = False
                currentTestName = ""
                for line in lines:
                    line = line.lstrip().rstrip()
                    #print testName + ": " + line
                    beginIndex = string.find(line, "[BEGIN]")
                    if beginIndex != -1:
                        beginName = line[beginIndex + 7:].lstrip()
                        foundBegin = True
                        currentTestName = beginName
                    passIndex = string.find(line, "[PASS]")
                    if passIndex != -1:
                        passName = line[passIndex + 6:].lstrip()
                        passed = True
                        if passName != currentTestName:
                            print >> sys.stderr, "[PASS] name does not match [BEGIN] name for test " + testName
                    failIndex = string.find(line, "[FAIL]")
                    if failIndex != -1:
                        failName = line[failIndex + 6:].lstrip()
                        failed = True
                        if failName != currentTestName:
                            print >> sys.stderr, "[FAIL] name does not match [BEGIN] name for test " + testName
                if foundBegin:
                    if not passed and not failed:
                        print >> sys.stderr, "[BEGIN] found [PASS] or [FAIL] for test " + testName
                else:
                    print >> sys.stderr, "Missing [BEGIN] for test " + testName
                if passed:
                    print "PASSED: " + testName
                    passedCount = passedCount + 1
                elif failed:
                    print "FAILED: " + testName
                    failedCount = failedCount + 1
            except subprocess.CalledProcessError as e:
                print >> sys.stderr, "FAILED: " + testName + " (execution failure)"
    print "Total PASS count: " + str(passedCount)
    print "Total FAIL count: " + str(failedCount)


def alt():
    testsTopDir = "/AppleInternal/CoreOS/tests/dyld/"
    for f in os.listdir(testsTopDir):
        testRunner = testsTopDir + f + "/run.sh"
        if os.path.isfile(testRunner):
            try:
                runOutput = subprocess.check_output([testRunner],stderr= subprocess.STDOUT)
                lines = runOutput.splitlines()
                foundBegin = False
                passed = False
                failed = False
                currentTestName = ""
                for line in lines:
                    line = line.lstrip().rstrip()
                    #print f + ": " + line
                    beginIndex = string.find(line, "[BEGIN]")
                    if beginIndex != -1:
                        beginName = line[beginIndex + 7:].lstrip()
                        foundBegin = True
                        currentTestName = beginName
                    passIndex = string.find(line, "[PASS]")
                    if passIndex != -1:
                        passName = line[passIndex + 6:].lstrip()
                        passed = True
                        if passName != currentTestName:
                            print >> sys.stderr, "[PASS] name does not match [BEGIN] name for test " + f
                    failIndex = string.find(line, "[FAIL]")
                    if failIndex != -1:
                        failName = line[failIndex + 6:].lstrip()
                        failed = True
                        if failName != currentTestName:
                            print >> sys.stderr, "[FAIL] name does not match [BEGIN] name for test " + f
                if foundBegin:
                    if not passed and not failed:
                        print >> sys.stderr, "[BEGIN] found [PASS] or [FAIL] for test " + f
                else:
                    print >> sys.stderr, "Missing [BEGIN] for test " + f
                if passed:
                    print "PASSED: " + f
                elif failed:
                    print "FAILED: " + f
            except subprocess.CalledProcessError as e:
                print >> sys.stderr, "FAILED: " + f + " (execution failure)"
