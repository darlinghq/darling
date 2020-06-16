#!/bin/sh

# Run all the tests in the testsuite.
# The first argument to the shell script may be a directory containing
# an alternative libSystem, good for testing keymgr before installing it.

if [ x"$1" != x ] ; then
  DYLD_LIBRARY_PATH=$1
  export DYLD_LIBRARY_PATH
fi

for i in basic-eh-app-jaguar-2.95 basic-eh-app-jaguar-3.1 \
	 basic-eh-app-tiger-4.0 basic-eh-app-10.4u-4.0 \
	 3043647/main 3464244/3464244 ; do
  rm -f $i
  uudecode -o $i ${i}.uu || echo "uudecode failed for $i"
  chmod +x $i || echo "chmod failed for $i"
  if ./$i ; then
    echo PASS: $i
  else
    echo FAIL: $i
  fi
done

cd 3074709
for i in main plugin plugin2 ; do
  rm -f $i
  uudecode -o $i ${i}.uu || echo "uudecode failed for $i"
  chmod +x $i || echo "chmod failed for $i"
done
if ./main ; then
  echo PASS: 3074709
else
  echo FAIL: 3074709
fi
cd ..

exit 0
