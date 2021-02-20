@echo off
if "%XSRC%" == "" set XSRC=.
test -f %XSRC%/tests/stateful-check.orig
if errorlevel 1 update %XSRC%/tests/stateful-check %XSRC%/tests/stateful-check.orig
sed -f %XSRC%/djgpp/stateful-check.sed %XSRC%/tests/stateful-check.orig > stateful-check
if errorlevel 1 goto SedError
update ./stateful-check %XSRC%/tests/stateful-check
rm -f ./stateful-check

test -f %XSRC%/tests/stateless-check.orig
if errorlevel 1 update %XSRC%/tests/stateless-check %XSRC%/tests/stateless-check.orig
sed -f %XSRC%/djgpp/stateless-check.sed %XSRC%/tests/stateless-check.orig > stateless-check
if errorlevel 1 goto SedError
update ./stateless-check %XSRC%/tests/stateless-check
rm -f ./stateless-check

test -f %XSRC%/tests/failuretranslit-check.orig
if errorlevel 1 update %XSRC%/tests/failuretranslit-check %XSRC%/tests/failuretranslit-check.orig
sed -f %XSRC%/djgpp/translit-check.sed %XSRC%/tests/failuretranslit-check.orig > failuretranslit-check
if errorlevel 1 goto SedError
update ./failuretranslit-check %XSRC%/tests/failuretranslit-check
rm -f ./failuretranslit-check

test -f %XSRC%/tests/translit-check.orig
if errorlevel 1 update %XSRC%/tests/translit-check %XSRC%/tests/translit-check.orig
sed -f %XSRC%/djgpp/translit-check.sed %XSRC%/tests/translit-check.orig > translit-check
if errorlevel 1 goto SedError
update ./translit-check %XSRC%/tests/translit-check
rm -f ./translit-check
goto End

:SedError
echo test script editing failed!

:End
