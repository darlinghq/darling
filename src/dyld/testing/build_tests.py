#!/usr/bin/python2.7

import plistlib
import string
import argparse
import sys
import os
import tempfile
import shutil
import subprocess
import uuid


#
# Scan files in .dtest directory looking for BUILD: or RUN: directives.
# Return a dictionary of all directives.
#
def parseDirectives(testCaseSourceDir):
    onlyLines = []
    buildLines = []
    extractLines = []
    runLines = []
    minOS = ""
    timeout = ""
    noCrashLogs = []
    bootArgs = []
    for file in os.listdir(testCaseSourceDir):
        if file.endswith((".c", ".cpp", ".cxx", ".m", ".mm")):
            with open(testCaseSourceDir + "/" + file) as f:
                for line in f.read().splitlines():
                    buildIndex = string.find(line, "BUILD:")
                    if buildIndex != -1:
                        buildLines.append(line[buildIndex + 6:].lstrip())
                    runIndex = string.find(line, "RUN:")
                    if runIndex != -1:
                        runLines.append(line[runIndex+4:].lstrip())
                    onlyIndex = string.find(line, "BUILD_ONLY:")
                    if onlyIndex != -1:
                        onlyLines.append(line[onlyIndex+11:].lstrip())
                    minOsIndex = string.find(line, "BUILD_MIN_OS:")
                    if minOsIndex != -1:
                        minOS = line[minOsIndex+13:].lstrip()
                    timeoutIndex = string.find(line, "RUN_TIMEOUT:")
                    if timeoutIndex != -1:
                        timeout = line[timeoutIndex+12:].lstrip()
                    noCrashLogsIndex = string.find(line, "NO_CRASH_LOG:")
                    if noCrashLogsIndex != -1:
                        noCrashLogs.append(line[noCrashLogsIndex+13:].lstrip())
                    bootArgsIndex = string.find(line, "BOOT_ARGS:")
                    if bootArgsIndex != -1:
                        bootArgs.append(line[bootArgsIndex+10:].lstrip())
    return {
        "BUILD":        buildLines,
        "BUILD_ONLY":   onlyLines,
        "BUILD_MIN_OS": minOS,
        "RUN":          runLines,
        "RUN_TIMEOUT":  timeout,
        "NO_CRASH_LOG": noCrashLogs,
        "BOOT_ARGS":    bootArgs,
   }


#
# Look at directives dictionary to see if this test should be skipped for this platform
#
def useTestCase(testName, testCaseDirectives, platformName):
    onlyLines = testCaseDirectives["BUILD_ONLY"]
    for only in onlyLines:
        if only == "MacOSX" and platformName != "macosx":
            return False
        if only == "iOS" and platformName != "iphoneos":
            return False
    return True


#
# Use BUILD directives to construct the test case
# Use RUN directives to generate a shell script to run test(s)
#
def buildTestCase(testCaseDirectives, testCaseSourceDir, toolsDir, sdkDir, dyldIncludesDir, minOsOptionsName, defaultMinOS, archOptions, testCaseDestDirBuild, testCaseDestDirRun, plistDir):
    scratchDir = tempfile.mkdtemp()
    if testCaseDirectives["BUILD_MIN_OS"]:
        minOS = testCaseDirectives["BUILD_MIN_OS"]
    else:
        minOS = defaultMinOS
    compilerSearchOptions = " -isysroot " + sdkDir + " -I" + sdkDir + "/System/Library/Frameworks/System.framework/PrivateHeaders" + " -I" + dyldIncludesDir + " -I" + testsSrcTopDir + "../include/"
    defines = " -DINSTALL_PATH=\"" + testCaseDestDirRun + "\""
    if minOsOptionsName == "mmacosx-version-min":
        taskForPidCommand = "touch "
        envEnableCommand  = "touch "
    else:
        taskForPidCommand = "codesign --force --sign - --entitlements " + testCaseSourceDir + "/../../task_for_pid_entitlement.plist "
        envEnableCommand  = "codesign --force --sign - --entitlements " + testCaseSourceDir + "/../../get_task_allow_entitlement.plist "
    buildSubs = {
        "CC":                   toolsDir + "/usr/bin/clang "   + archOptions + " -" + minOsOptionsName + "=" + str(minOS) + compilerSearchOptions + defines,
        "CXX":                  toolsDir + "/usr/bin/clang++ " + archOptions + " -" + minOsOptionsName + "=" + str(minOS) + compilerSearchOptions + defines,
        "BUILD_DIR":            testCaseDestDirBuild,
        "RUN_DIR":              testCaseDestDirRun,
        "TEMP_DIR":             scratchDir,
        "TASK_FOR_PID_ENABLE":  taskForPidCommand,
        "DYLD_ENV_VARS_ENABLE": envEnableCommand
    }
    os.makedirs(testCaseDestDirBuild)
    os.chdir(testCaseSourceDir)
    outputfiles = []
    alreadySigned = []
    print >> sys.stderr, "cd " + testCaseSourceDir
    for line in testCaseDirectives["BUILD"]:
        cmd = string.Template(line).safe_substitute(buildSubs)
        if "codesign" in cmd:
            alreadySigned.append(string.split(cmd).pop())
        print >> sys.stderr, cmd
        if "&&" in cmd:
            result = subprocess.call(cmd, shell=True)
        else:
            cmdList = []
            cmdList = string.split(cmd)
            result = subprocess.call(cmdList)
        if result:
            return result
        args = cmd.split()
        for index, arg in enumerate(args):
            if arg == "-o":
                outputfiles.append(args[index+1])
                break
    print >> sys.stderr, "outfiles: " +  ' '.join(outputfiles)  + "already signed: " + ' '.join(alreadySigned)
    for outfile in outputfiles:
        if outfile not in alreadySigned:
            cmd = "codesign --force --sign - " + outfile
            print >> sys.stderr, cmd
            subprocess.call(string.split(cmd))
    shutil.rmtree(scratchDir, ignore_errors=True)
    sudoSub = ""
    if minOsOptionsName == "mmacosx-version-min":
        sudoSub = "sudo"
    runSubs = {
        "RUN_DIR":        testCaseDestDirRun,
        "REQUIRE_CRASH":  "nocr -require_crash",
        "SUDO":           sudoSub,
    }
    runFilePath = testCaseDestDirBuild + "/run.sh"
    with open(runFilePath, "a") as runFile:
        runFile.write("#!/bin/sh\n")
        runFile.write("cd " + testCaseDestDirRun + "\n")
        os.chmod(runFilePath, 0755)

        runFile.write("echo \"run in dyld2 mode\" \n");
        for runline in testCaseDirectives["RUN"]:
          subLine = string.Template(runline).safe_substitute(runSubs)
          subLine = "TEST_DYLD_MODE=2 DYLD_USE_CLOSURES=0 " + subLine
          runFile.write(subLine + "\n")

        if minOsOptionsName == "mmacosx-version-min":
            runFile.write("echo \"run in dyld2 mode with no shared cache\" \n");
            for runline in testCaseDirectives["RUN"]:
                subLine = string.Template(runline).safe_substitute(runSubs)
                subLine = "TEST_DYLD_MODE=2 DYLD_SHARED_REGION=avoid " + subLine
                runFile.write(subLine + "\n")

        runFile.write("echo \"run in dyld3 mode\" \n");
        for runline in testCaseDirectives["RUN"]:
            subLine = string.Template(runline).safe_substitute(runSubs)
            if subLine.startswith("sudo "):
                subLine = "sudo TEST_DYLD_MODE=3 DYLD_USE_CLOSURES=1 " + subLine[5:]
            else:
                subLine = "TEST_DYLD_MODE=3 DYLD_USE_CLOSURES=1 " + subLine
            runFile.write(subLine + "\n")

        if minOsOptionsName == "mmacosx-version-min":
            runFile.write("echo \"run in dyld3 mode with no shared cache\" \n");
            for runline in testCaseDirectives["RUN"]:
                subLine = string.Template(runline).safe_substitute(runSubs)
                if subLine.startswith("sudo "):
                    subLine = "sudo TEST_DYLD_MODE=3 DYLD_SHARED_REGION=avoid DYLD_USE_CLOSURES=1 " + subLine[5:]
                else:
                    subLine = "TEST_DYLD_MODE=3 DYLD_SHARED_REGION=avoid DYLD_USE_CLOSURES=1 " + subLine
                runFile.write(subLine + "\n")

        runFile.write("\n")
        runFile.close()
    for runline in testCaseDirectives["RUN"]:
        runTarget = runline.split().pop()
        os.system("xcrun dt_extractmeta extract -i " + testCaseDestDirRun + "/" + runTarget + " -b " + testCaseDestDirBuild + "/" + runTarget + " -o " + plistDir + "/" + str(uuid.uuid4()) + ".plist 2> /dev/null")
    return 0



#
# Use XCode build settings to build all unit tests for specified platform
# Generate a .plist for BATS to use to run all tests
#
if __name__ == "__main__":
    dstDir = os.getenv("DSTROOT", "/tmp/dyld_tests/")
    testsRunDstTopDir = "/AppleInternal/CoreOS/tests/dyld/"
    testsBuildDstTopDir = dstDir + testsRunDstTopDir
    # If we want to run directly from the dstroot then override that now
    runFromDstRoot = os.getenv("RUN_FROM_DSTROOT", "")
    if runFromDstRoot:
        testsRunDstTopDir = testsBuildDstTopDir
    shutil.rmtree(testsBuildDstTopDir, ignore_errors=True)
    dyldSrcDir = os.getenv("SRCROOT", "")
    if not dyldSrcDir:
        dyldSrcDir = os.getcwd()
    testsSrcTopDir = dyldSrcDir + "/testing/test-cases/"
    dyldIncludesDir = dyldSrcDir + "/include/"
    sdkDir = os.getenv("SDKROOT", "")
    if not sdkDir:
        sdkDir = subprocess.check_output(["xcrun", "-sdk", "macosx.internal", "--show-sdk-path"]).rstrip()
    toolsDir = os.getenv("TOOLCHAIN_DIR", "/")
    defaultMinOS = ""
    minVersNum = "10.14"
    minOSOption = os.getenv("DEPLOYMENT_TARGET_CLANG_FLAG_NAME", "")
    if minOSOption:
        minOSVersName = os.getenv("DEPLOYMENT_TARGET_CLANG_ENV_NAME", "")
        if minOSVersName:
            minVersNum = os.getenv(minOSVersName, "")
    else:
        minOSOption = "mmacosx-version-min"
    platformName = os.getenv("PLATFORM_NAME", "macosx")
    archOptions = ""
    archList = os.getenv("RC_ARCHS", "")
    if archList:
        for arch in string.split(archList, " "):
            archOptions = archOptions + " -arch " + arch
    else:
        if platformName == "watchos":
            archOptions = "-arch armv7k"
        elif platformName == "appletvos":
            archOptions = "-arch arm64"
        elif platformName == "macosx":
            archList = os.getenv("ARCHS_STANDARD_64_BIT", "")
            if archList:
                for arch in string.split(archList, " "):
                    archOptions = archOptions + " -arch " + arch
            else:
                archOptions = "-arch x86_64"
        else:
            archList = os.getenv("ARCHS_STANDARD_32_64_BIT", "")
            if archList:
                for arch in string.split(archList, " "):
                    archOptions = archOptions + " -arch " + arch
            else:
                archOptions = "-arch x86_64"
    allTests = []
    suppressCrashLogs = []
    plistDir = tempfile.mkdtemp()
    for f in sorted(os.listdir(testsSrcTopDir)):
        if f.endswith(".dtest"):
            testName = f[0:-6]
            outDirBuild = testsBuildDstTopDir + testName
            outDirRun = testsRunDstTopDir + testName
            testCaseDir = testsSrcTopDir + f
            onlyTestDir = os.getenv("ONLY_BUILD_TEST", "")
            if onlyTestDir:
                if onlyTestDir != testName:
                    continue
                print >> sys.stderr, "Going to build " + testName
            testCaseDirectives = parseDirectives(testCaseDir)
            if useTestCase(testName, testCaseDirectives, platformName):
                result = buildTestCase(testCaseDirectives, testCaseDir, toolsDir, sdkDir, dyldIncludesDir, minOSOption, minVersNum, archOptions, outDirBuild, outDirRun, plistDir)
                if result:
                    sys.exit(result)
                mytest = {}
                mytest["TestName"] = testName
                mytest["Arch"] = "platform-native"
                mytest["WorkingDirectory"] = testsRunDstTopDir + testName
                mytest["Command"] = []
                mytest["Command"].append("./run.sh")
                for runline in testCaseDirectives["RUN"]:
                    if "$SUDO" in runline:
                        mytest["AsRoot"] = True
                if testCaseDirectives["RUN_TIMEOUT"]:
                    mytest["Timeout"] = testCaseDirectives["RUN_TIMEOUT"]
                if testCaseDirectives["BOOT_ARGS"]:
                    mytest["BootArgsSet"] = ",".join(testCaseDirectives["BOOT_ARGS"]);
                allTests.append(mytest)
            if testCaseDirectives["NO_CRASH_LOG"]:
                for skipCrash in testCaseDirectives["NO_CRASH_LOG"]:
                    suppressCrashLogs.append(skipCrash)
    batsInfo = { "BATSConfigVersion": "0.1.0",
                 "Project":           "dyld_tests",
                 "Tests":             allTests }
    if suppressCrashLogs:
        batsInfo["IgnoreCrashes"] = []
        for skipCrash in suppressCrashLogs:
            batsInfo["IgnoreCrashes"].append(skipCrash)
    batsFileDir = dstDir + "/AppleInternal/CoreOS/BATS/unit_tests/"
    shutil.rmtree(batsFileDir, ignore_errors=True)
    os.makedirs(batsFileDir)
    batsFilePath = batsFileDir + "dyld.plist"
    with open(batsFilePath, "w") as batsFile:
        batsFile.write(plistlib.writePlistToString(batsInfo))
        batsFile.close()
    os.system('plutil -convert binary1 ' + batsFilePath) # convert the plist in place to binary
    runHelper = dstDir + "/AppleInternal/CoreOS/tests/dyld/run_all_dyld_tests.sh"
    print runHelper
    with open(runHelper, "w") as shFile:
        shFile.write("#!/bin/sh\n")
        for test in allTests:
            shFile.write(test["WorkingDirectory"] + "/run.sh\n")
        shFile.close()
    os.chmod(runHelper, 0755)
    if not os.path.exists(dstDir + "/AppleInternal/CoreOS/tests/metadata/dyld/"): os.makedirs(dstDir + "/AppleInternal/CoreOS/tests/metadata/dyld/")
    os.system("xcrun dt_extractmeta merge -o " + dstDir + "/AppleInternal/CoreOS/tests/metadata/dyld/dyld.plist " + plistDir + "/*")
#   FIXME: Enable this once all tests move to darwintest
#    os.system("xcrun dt_convertmeta " + dstDir + "/AppleInternal/CoreOS/BATS/unit_tests/dyld.plist dyld_tests " + dstDir + "/AppleInternal/CoreOS/tests/metadata/dyld/dyld.plist")
    shutil.rmtree(plistDir, ignore_errors=True)

